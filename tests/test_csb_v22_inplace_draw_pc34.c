/* CSB V2.2 must not consume generated host-art cache bytes. */
#include "csb_v22_inplace_draw_pc34.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(expr, message) do { \
    ++checks; \
    if (!(expr)) { \
        ++failures; \
        fprintf(stderr, "FAIL %s:%d: %s — %s\n", \
                __FILE__, __LINE__, #expr, message); \
    } \
} while (0)

static void test_generated_cache_is_never_activated(void)
{
    int width = -1;
    int height = -1;

    csb_v22_inplace_draw_shutdown();
    CHECK(csb_v22_inplace_draw_init() == 0,
          "V2.2 has no original-CSB replacement-art decoder");
    CHECK(csb_v22_inplace_draw_active() == 0,
          "generated host-art cache cannot become a live source");
    CHECK(csb_v22_inplace_get_bitmap_by_id("door_shapes", "door_d0_01",
                                           &width, &height) == NULL,
          "no generated bitmap pointer is exposed");
    CHECK(width == 0 && height == 0,
          "no-draw lookup clears output dimensions");
}

static void test_f0128_framebuffer_is_preserved(void)
{
    CSB_V1_ViewportRuntimeDrawCommandPc34 command;
    unsigned char framebuffer[320 * 200];
    unsigned char before[320 * 200];
    uint8_t palette[256][3];

    memset(&command, 0, sizeof(command));
    memset(framebuffer, 0x5a, sizeof(framebuffer));
    memcpy(before, framebuffer, sizeof(before));
    memset(palette, 0x3f, sizeof(palette));

    CHECK(csb_v22_inplace_draw_set_indexed_palette_rgb6(palette) == 1,
          "source palette remains accepted for a future authenticated path");
    CHECK(csb_v22_inplace_render_f0128_command(&command, framebuffer,
                                                320, 200) == 0,
          "no V2.2 replacement is composed without original material");
    CHECK(memcmp(framebuffer, before, sizeof(framebuffer)) == 0,
          "no-draw path preserves source-owned F0128 pixels byte-for-byte");
    csb_v22_inplace_draw_clear_indexed_palette();
    csb_v22_inplace_draw_shutdown();
    CHECK(csb_v22_inplace_draw_active() == 0,
          "shutdown remains idempotently inactive");
}

static void test_source_evidence_records_the_boundary(void)
{
    const char *evidence = csb_v22_inplace_draw_source_evidence();
    CHECK(evidence != NULL && strstr(evidence, "ReDMCSB") != NULL,
          "evidence cites the source compositor");
    CHECK(evidence != NULL && strstr(evidence, "v22_inplace_cache.bin") != NULL,
          "evidence names the rejected generated cache format");
    CHECK(evidence != NULL && strstr(evidence, "original") != NULL,
          "evidence states the original-data admission requirement");
}

int main(void)
{
    test_generated_cache_is_never_activated();
    test_f0128_framebuffer_is_preserved();
    test_source_evidence_records_the_boundary();
    printf("csb_v22_inplace_draw_pc34: checks=%d failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
