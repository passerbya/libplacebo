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

#ifndef LIBPLACEBO_SHADERS_MOTION_H_
#define LIBPLACEBO_SHADERS_MOTION_H_

// Functions and helpers for analyzing scenes for motion vectors, performing
// interpolation along these vectors, and so forth.

#include <libplacebo/shaders.h>

// Texture "pyramid". This is essentially equivalent to reinventing the concept
// of mipmaps, but with less annoying implementation details.
struct pl_tex_stack {
    int num_levels;
    const struct pl_tex **levels;
};

// Compute a `pl_tex_stack` from a starting image.
//
// Note: `tex->params.blit_src` must be enabled.
const struct pl_tex_stack *pl_tex_stack_create(const struct pl_gpu *gpu,
                                               const struct pl_tex *tex);

// Destroy a `pl_tex_stack` and the referenced textures.
//
// Note: This will *not* destroy the initial texture, i.e. `stack->levels[0]`.
void pl_tex_stack_destroy(const struct pl_gpu *gpu, const struct pl_tex_stack **stack);

#endif // LIBPLACEBO_SHADERS_MOTION_H_
