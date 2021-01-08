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
 * License along with libplacebo. If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include "shaders.h"

void pl_tex_stack_destroy(const struct pl_gpu *gpu,
                          const struct pl_tex_stack **pstack)
{
    const struct pl_tex_stack *stack = *pstack;
    if (!stack)
        return;

    for (int i = 1; i < stack->num_levels; i++)
        pl_tex_destroy(gpu, &stack->levels[i]);

    talloc_free((void *) stack);
    *pstack = NULL;
}

#define TEX_DIV2(d) ((d) > 1 ? (d) / 2 : (d))

const struct pl_tex_stack *pl_tex_stack_create(const struct pl_gpu *gpu,
                                               const struct pl_tex *tex)
{
    if (!tex->params.blit_src) {
        PL_ERR(gpu, "Need `tex->params.blit_src` to create tex stack!");
        return NULL;
    }

    if (!(tex->params.format->caps & PL_FMT_CAP_LINEAR)) {
        PL_ERR(gpu, "Texture format needs to be linearly sampleable");
        return NULL;
    }

    int max_dim = PL_MAX(PL_MAX(tex->params.w, tex->params.h), tex->params.d);
    int num_levels = 1 + PL_LOG2(max_dim);

    struct pl_tex_stack *stack = talloc_ptrtype(NULL, stack);
    *stack = (struct pl_tex_stack) {
        .num_levels = num_levels,
        .levels = talloc_zero_array(stack, const struct pl_tex *, num_levels),
    };

    stack->levels[0] = tex;
    for (int i = 1; i < num_levels; i++) {
        const struct pl_tex *prev = stack->levels[i - 1];
        stack->levels[i] = pl_tex_create(gpu, &(struct pl_tex_params) {
            // Mip layer size is half the previous, clamping at 1
            .w = TEX_DIV2(prev->params.w),
            .h = TEX_DIV2(prev->params.h),
            .d = TEX_DIV2(prev->params.d),
            // Re-use texture format and general capabilities
            .format = tex->params.format,
            .sampleable = tex->params.sampleable,
            .renderable = tex->params.renderable,
            .storable = tex->params.storable,
            // Required for the mipmap generation process
            .blit_src = true,
            .blit_dst = true,
        });

        if (!stack->levels[i])
            goto error;

        // Fill in this mip layer by blitting the previous
        pl_tex_blit(gpu, &(struct pl_tex_blit_params) {
            .src = prev,
            .dst = stack->levels[i],
            .sample_mode = PL_TEX_SAMPLE_LINEAR,
        });
    }

    return stack;

error:
    PL_ERR(gpu, "Failed creating texture stack");
    pl_tex_stack_destroy(gpu, (const struct pl_tex_stack **) &stack);
    return NULL;
}
