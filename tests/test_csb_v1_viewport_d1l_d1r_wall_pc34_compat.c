#include "csb/csb_v1_viewport_d1l_d1r_wall_pc34_compat.h"
#include "csb_v1_pc34_wallset_graphics_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(expr) do { ++checks; if (!(expr)) { \
    printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); ++failures; } \
} while (0)

static const char *real_graphics_path(void)
{
    const char *from_env = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    if (from_env && from_env[0]) return from_env;
    return "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
}

static void check_metadata(void)
{
    const CSB_V1_D1LD1RWallSpecPc34 *left =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(1);
    const CSB_V1_D1LD1RWallSpecPc34 *right =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(2);
    const char *e = csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34();
    int source_x = -1;

    CHECK(csb_v1_viewport_d1l_d1r_wall_spec_count_pc34() == 2u);
    CHECK(left && right);
    CHECK(left->native_wall_index == 3 && right->native_wall_index == 2);
    CHECK(left->wall_zone == 713 && right->wall_zone == 714);
    CHECK(csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(4) == 713);
    CHECK(csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(5) == 714);
    CHECK(csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(3) == -1);
    CHECK(csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
        left, 0, 0, &source_x) == 0 && source_x == 0);
    CHECK(csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
        right, 160, 1, &source_x) == 0 && source_x == 63);
    CHECK(e && strstr(e, "86 + WallSet*40 + C03/C02") &&
          strstr(e, "dmweb") && strstr(e, "CSBWin") &&
          strstr(e, "no generated bitmap"));
}

static void check_real_material(void)
{
    const char *path = real_graphics_path();
    unsigned char *left = NULL;
    unsigned char *right = NULL;
    CSB_V1_D1LD1RWallMaterialPc34 left_material;
    CSB_V1_D1LD1RWallMaterialPc34 right_material;
    uint32_t expected_left = 0u;
    uint32_t expected_right = 0u;
    FILE *file = fopen(path, "rb");

    if (!file) {
        printf("SKIP: no local CSB PC3.4 GRAPHICS.DAT\n");
        return;
    }
    fclose(file);
    CHECK(csb_v1_pc34_wallset_graphics_entry_index(
        0, CSB_V1_PC34_WALLSET_WALL_D1L, 749u, &expected_left));
    CHECK(csb_v1_pc34_wallset_graphics_entry_index(
        0, CSB_V1_PC34_WALLSET_WALL_D1R, 749u, &expected_right));
    CHECK(expected_left == 96u && expected_right == 95u);
    CHECK(csb_v1_viewport_d1l_d1r_wall_load_graphics_dat_material_pc34(
        path, 0, 1, 749u, &left, &left_material));
    CHECK(csb_v1_viewport_d1l_d1r_wall_load_graphics_dat_material_pc34(
        path, 0, 2, 749u, &right, &right_material));
    CHECK(left && right && left_material.valid && right_material.valid);
    CHECK(left_material.graphics_entry_index == expected_left &&
          right_material.graphics_entry_index == expected_right);
    CHECK(left_material.width == 60 && left_material.height == 111 &&
          right_material.width == 60 && right_material.height == 111);
    CHECK(left_material.decode_receipt.valid && right_material.decode_receipt.valid);
    CHECK(left_material.decode_receipt.compressed_record_sha256[0] &&
          right_material.decode_receipt.compressed_record_sha256[0]);
    CHECK(strcmp(left_material.decode_receipt.compressed_record_sha256,
                 right_material.decode_receipt.compressed_record_sha256) != 0);
    CHECK(!csb_v1_viewport_d1l_d1r_wall_load_graphics_dat_material_pc34(
        path, 0, 99, 749u, &left, &left_material));
    free(left);
    free(right);
}

int main(void)
{
    check_metadata();
    check_real_material();
    printf("csb_v1_viewport_d1l_d1r_wall_pc34_compat: checks=%d failures=%d\n",
           checks, failures);
    return failures ? 1 : 0;
}
