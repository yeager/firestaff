#include "csb_v1_viewport_d3c_wall_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *label, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d\n", label, got, want);
    }
}

static void expect_contains(const char *label, const char *text, const char *needle)
{
    expect_int(label, text && strstr(text, needle) != NULL, 1);
}

int main(void)
{
    const CSB_V1_D3CWallSpecPc34 *spec = csb_v1_viewport_d3c_wall_spec_pc34();
    const char *evidence = csb_v1_viewport_d3c_wall_source_evidence_pc34();

    expect_int("spec.present", spec != NULL, 1);
    expect_int("contract.only", spec ? spec->contract_only : 0, 1);
    expect_int("asset.parity.absent", spec ? spec->real_asset_pixel_parity : 1, 0);
    expect_int("frame.x1", spec ? spec->frame.x1 : -1, 74);
    expect_int("frame.x2", spec ? spec->frame.x2 : -1, 149);
    expect_int("frame.y1", spec ? spec->frame.y1 : -1, 25);
    expect_int("frame.y2", spec ? spec->frame.y2 : -1, 75);
    expect_int("effective.source.x1", spec ? spec->effective_source_x1 : -1, 18);
    expect_int("effective.source.x2", spec ? spec->effective_source_x2 : -1, 63);
    expect_int("effective.viewport.x1", spec ? spec->effective_viewport_x1 : -1, 74);
    expect_int("effective.viewport.x2", spec ? spec->effective_viewport_x2 : -1, 119);
    expect_int("effective.width", spec ? spec->effective_visible_width : -1, 46);
    expect_int("effective.height", spec ? spec->effective_visible_height : -1, 51);
    expect_int("f0118.route", spec ? spec->uses_f0118_d3c_wall_route : 0, 1);
    expect_int("f0101.route", spec ? spec->uses_f0101_no_transparency : 0, 1);
    expect_int("f0100.reference", spec ? spec->preserves_f0100_c10_reference : 0, 1);
    expect_int("f0121.rejected", spec ? spec->rejects_f0121_d2c_path : 0, 1);
    expect_int("d3c.door.absent", spec ? spec->door_front_draws_d3c_wall_pixels : 1, 0);
    expect_int("d3c.stairs.absent", spec ? spec->stairs_front_draws_d3c_wall_pixels : 1, 0);
    expect_int("d3c.pit.absent", spec ? spec->pit_draws_d3c_wall_pixels : 1, 0);
    expect_int("f0100.transparent", csb_v1_viewport_d3c_wall_blend_f0100_transparent_pc34(0x44, 10, 10), 0x44);
    expect_int("f0100.opaque", csb_v1_viewport_d3c_wall_blend_f0100_transparent_pc34(0x44, 0x51, 10), 0x51);
    expect_int("f0101.no_transparency", csb_v1_viewport_d3c_wall_blend_f0101_no_transparency_pc34(0x44, 10), 10);
    expect_contains("evidence.f0118", evidence, "F0118");
    expect_contains("evidence.f0100", evidence, "F0100");
    expect_contains("evidence.f0101", evidence, "F0101");
    expect_contains("evidence.g0163", evidence, "G0163");
    expect_contains("evidence.g0698", evidence, "G0698");
    expect_contains("evidence.csbwin", evidence, "Viewport.cpp");

    printf("CSB D3C wall metadata: assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures != 0;
}
