/* Nexus V2 Phase 4 — Palette handoff regression.
 * Verifies a single indexed color transfer from V1 framebuffer palette into V2 output,
 * then confirms a palette edit on the same V1 state is applied on next render.
 */

#include "nexus_v2_render_pipeline.h"
#include <stdio.h>

static int s_passed = 0;
static int s_failed = 0;

static void check(const char *name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_passed++;
    } else {
        printf("  FAIL: %s\n", name);
        s_failed++;
    }
}

int main(void) {
    printf("=== Nexus V2 palette handoff regression ===\n");

    Nexus_V2_RenderPipeline pipe;
    if (nexus_v2_pipeline_init(&pipe, NEXUS_V2_UPSCALED) != 0) {
        printf("FAIL: pipeline_init\n");
        return 1;
    }

    pipe.config.bilinear_filter = 0;

    Nexus_Framebuffer fb = {0};
    fb.palette[0] = 0x00000000U;
    fb.palette[77] = 0xFFFF0000U; /* state: hot palette entry */
    fb.color_buffer[1 + NEXUS_FB_W * 1] = 77; /* single highlight pixel at (1,1) */

    nexus_v2_pipeline_render(&pipe, &fb, 0.0f, 0.0f, 0.0f, 0, 0.0f);
    check("initial handoff uses palette[77]=red",
          pipe.output_buffer[2 + 2 * pipe.output_w] == fb.palette[77]);

    check("background remains clear",
          pipe.output_buffer[0] == fb.palette[0]);

    fb.palette[77] = 0xFF00FF00U; /* mutate one palette state only */
    nexus_v2_pipeline_render(&pipe, &fb, 0.0f, 0.0f, 0.0f, 0, 0.0f);
    check("updated handoff uses palette[77]=green",
          pipe.output_buffer[2 + 2 * pipe.output_w] == fb.palette[77]);

    printf("\n=== Results: %d passed, %d failed ===\n", s_passed, s_failed);
    nexus_v2_pipeline_shutdown(&pipe);
    return s_failed > 0 ? 1 : 0;
}
