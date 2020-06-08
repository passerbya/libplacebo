/*
 * This file is part of libplacebo.
 *
 * libplacebo is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libplacebo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libplacebo.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBPLACEBO_DISPATCH_H_
#define LIBPLACEBO_DISPATCH_H_

#include <libplacebo/shaders.h>
#include <libplacebo/gpu.h>

struct pl_dispatch;

// Creates a new shader dispatch object. This object provides a translation
// layer between generated shaders (pl_shader) and the ra context such that it
// can be used to execute shaders. This dispatch object will also provide
// shader caching (for efficient re-use).
struct pl_dispatch *pl_dispatch_create(struct pl_context *ctx,
                                       const struct pl_gpu *gpu);
void pl_dispatch_destroy(struct pl_dispatch **dp);

// Returns a blank pl_shader object, suitable for recording rendering commands.
// For more information, see the header documentation in `shaders/*.h`.
struct pl_shader *pl_dispatch_begin(struct pl_dispatch *dp);

struct pl_dispatch_params {
    // The shader to execute. The pl_dispatch will take over ownership
    // of this shader, and return it back to the internal pool.
    //
    // This shader must have a compatible signature, i.e. inputs
    // `PL_SHADER_SIG_NONE` and outputs `PL_SHADER_SIG_COLOR`.
    struct pl_shader **shader;

    // The texture to render to. This must have params compatible with the
    // shader, i.e. `target->params.renderable` for fragment shaders and
    // `target->params.storable` for compute shaders.
    //
    // Note: Even when not using compute shaders, users are advised to always
    // set `target->params.storable` if permitted by the `pl_fmt`, since this
    // allows the use of compute shaders instead of full-screen quads, which is
    // faster on some platforms.
    const struct pl_tex *target;

    // The target rect to render to. Optional, if left as {0}, then the
    // entire texture will be rendered to.
    struct pl_rect2d rect;

    // If set, enables and controls the blending for this pass. Optional. When
    // using this with fragment shaders, `target->params.fmt->caps` must
    // include `PL_FMT_CAP_BLENDABLE`.
    const struct pl_blend_params *blend_params;

    // If set, records the execution time of this dispatch into the given
    // timer object. Optional.
    struct pl_timer *timer;

    // If set, record this dispatch into the given dispatch command instead of
    // actually executing it. Optional. Note that `pl_dispatch_finish` may
    // still fail in this case, e.g. if the shader would exhaust the `pl_gpu`'s
    // resources, or if shader compilation fails.
    struct pl_dispatch_cmd *cmd;
};

// Dispatch a generated shader (via the pl_shader mechanism). Returns whether
// or not the dispatch was successful.
bool pl_dispatch_finish(struct pl_dispatch *dp, const struct pl_dispatch_params *params);

struct pl_dispatch_compute_params {
    // The shader to execute. This must be a compute shader with both the
    // input and output signature set to PL_SHADER_SIG_NONE.
    //
    // Note: There is currently no way to actually construct such a shader with
    // the currently available public APIs. (However, it's still used
    // internally, and may be needed in the future)
    struct pl_shader **shader;

    // The number of work groups to dispatch in each dimension. Must be
    // nonzero for all three dimensions.
    int dispatch_size[3];

    // If set, simulate vertex attributes (similar to `pl_dispatch_finish`)
    // according to the given dimensions. The first two components of the
    // thread's global ID will be interpreted as the X and Y locations.
    //
    // Optional, ignored if either component is left as 0.
    int width, height;

    // If set, records the execution time of this dispatch into the given
    // timer object. Optional.
    struct pl_timer *timer;

    // If set, record this dispatch into the given dispatch command instead of
    // actually executing it. Optional. Note that `pl_dispatch_compute` may
    // still fail in this case, e.g. if the shader would exhaust the `pl_gpu`'s
    // resources, or if shader compilation fails.
    struct pl_dispatch_cmd *cmd;
};

// A variant of `pl_dispatch_finish`, this one only dispatches a compute shader
// that has no output.
bool pl_dispatch_compute(struct pl_dispatch *dp,
                         const struct pl_dispatch_compute_params *params);

// Cancel an active shader without submitting anything. Useful, for example,
// if the shader was instead merged into a different shader.
void pl_dispatch_abort(struct pl_dispatch *dp, struct pl_shader **sh);

// Dispatch command system. This can be used to avoid re-generating shaders
// every single frame, which may be useful if the user intends to re-execute
// the same set of shaders many times without modification. (Conceptually
// similar to command buffers in various graphics APIs)
//
// Note: In most scenarios this is not really needed and only represents a
// relatively minor CPU / battery lifetime optimization. Re-generating shaders
// is relatively cheap, and importantly, the resulting shaders only need to
// be checked against the `pl_dispatch` cache - not re-compiled.

struct pl_dispatch_cmd;

// Metadata describing a dynamic descriptor object. These descriptors can be
// swapped out by different objects at command execution time.
struct pl_dispatch_object {
    // Arbitrary ID used to re-identify this object later. Can be anything
    // the user wants, but must uniquely identify this particular object.
    uint64_t id;

    // Descriptor object (`pl_shader_desc.object`).
    const void *object;
};

// Create a new blank dispatch command, optionally parametrized by the given
// list of dynamic dispatch objects, terminated by a {0} entry. If `objects` is
// NULL, the resulting dispatch command will not have any dynamic descriptors.
struct pl_dispatch_cmd *pl_dispatch_cmd_create(struct pl_dispatch *dp,
                                               const struct pl_dispatch_object *objects);

void pl_dispatch_cmd_destroy(struct pl_dispatch_cmd **cmd);

// Execute all of the dispatch commands recorded in this `pl_dispatch_cmd`,
// with any dynamic descriptors being replaced by entries specified in
// `objects`, terminated by a {0} entry. If `objects `is NULL, no parameters
// are updated.
//
// Entries not present in `objects` will retain their binding from the previous
// execution of this command, or the initial values if never executed.
//
// Returns false (and aborts execution) if any of the recorded commands failed.
bool pl_dispatch_cmd_execute(struct pl_dispatch_cmd *cmd,
                             const struct pl_dispatch_object *objects);

/* XXX: example usage:

enum {
    DESC_PLANE0 = 0,
    DESC_PLANE1,
    DESC_PLANE2,
    DESC_PLANE3,
    DESC_FBO,
};

struct pl_dispatch_cmd *cmd;
cmd = pl_dispatch_cmd_create(dp, (struct pl_dispatch_object[]) {
    { DESC_PLANE0, image->planes[0].tex },
    { DESC_PLANE1, image->planes[1].tex },
    { DESC_PLANE2, image->planes[2].tex },
    { DESC_PLANE3, image->planes[3].tex },
    { DESC_FBO, target->fbo },
    {0},
});

// perform rendering as usual but with `.cmd = cmd` in the params structs

while (true) {
    // update plane textures and get new `target`

    pl_dispatch_cmd_execute(cmd, (struct pl_dispatch_objects[]) {
        { DESC_PLANE0, image->planes[0].tex },
        { DESC_PLANE1, image->planes[1].tex },
        { DESC_PLANE2, image->planes[2].tex },
        { DESC_PLANE3, image->planes[3].tex },
        { DESC_FBO, target->fbo },
        {0},
    });
}

*/

#endif // LIBPLACEBO_DISPATCH_H
