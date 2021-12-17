#include "tests.h"
#include "log.h"

//#define PRINT_LUTS

int main()
{
    pl_log log = pl_test_logger();

    // PQ unit tests
    REQUIRE(feq(pl_hdr_rescale(PL_HDR_PQ, PL_HDR_NITS, 0.0), 0.0,     1e-2));
    REQUIRE(feq(pl_hdr_rescale(PL_HDR_PQ, PL_HDR_NITS, 1.0), 10000.0, 1e-2));
    REQUIRE(feq(pl_hdr_rescale(PL_HDR_PQ, PL_HDR_NITS, 0.58), 203.0,  1e-2));

    // Test round-trip
    for (float x = 0.0f; x < 1.0f; x += 0.01f) {
        REQUIRE(feq(x, pl_hdr_rescale(PL_HDR_RELATIVE, PL_HDR_PQ,
                       pl_hdr_rescale(PL_HDR_PQ, PL_HDR_RELATIVE, x)),
                    1e-5));
    }

    static float lut[128];
    struct pl_tone_map_params params = {
        .input_scaling = PL_HDR_PQ,
        .output_scaling = PL_HDR_PQ,
        .lut_size = PL_ARRAY_SIZE(lut),
    };

    params.input_min = pl_hdr_rescale(PL_HDR_NITS, params.input_scaling, 0.0001);
    params.input_max = pl_hdr_rescale(PL_HDR_NITS, params.input_scaling, 1000.0);
    params.output_min = pl_hdr_rescale(PL_HDR_RELATIVE, params.output_scaling, 0.001);
    params.output_max = pl_hdr_rescale(PL_HDR_RELATIVE, params.output_scaling, 1.0);

    struct pl_tone_map_params params_inv = params;
    PL_SWAP(params_inv.input_min, params_inv.output_min);
    PL_SWAP(params_inv.input_max, params_inv.output_max);

    // Generate example tone mapping curves, forward and inverse
    for (int i = 0; i < pl_num_tone_map_functions; i++) {
        const struct pl_tone_map_function *fun = pl_tone_map_functions[i];
        printf("Testing tone-mapping function %s\n", fun->name);
        params.function = params_inv.function = fun;
        clock_t start = clock();
        pl_tone_map_generate(lut, &params);
        pl_log_cpu_time(log, start, clock(), "generating LUT");
        for (int j = 0; j < PL_ARRAY_SIZE(lut); j++) {
            REQUIRE(isfinite(lut[j]) && !isnan(lut[j]));
#ifdef PRINT_LUTS
            printf("%f, %f\n", j / (PL_ARRAY_SIZE(lut) - 1.0f), lut[j]);
#endif
        }

        if (fun->map_inverse) {
            start = clock();
            pl_tone_map_generate(lut, &params_inv);
            pl_log_cpu_time(log, start, clock(), "generating inverse LUT");
            for (int j = 0; j < PL_ARRAY_SIZE(lut); j++) {
                REQUIRE(isfinite(lut[j]) && !isnan(lut[j]));
#ifdef PRINT_LUTS
                printf("%f, %f\n", j / (PL_ARRAY_SIZE(lut) - 1.0f), lut[j]);
#endif
            }
        }
    }

    pl_log_destroy(&log);
}
