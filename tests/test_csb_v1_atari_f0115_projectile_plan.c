#include "csb_v1_atari_f0115_projectile_plan.h"
#include "csb_v1_atari_f0115_explosion_plan.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_graphics_atari_st_loader_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ReDMCSB DUNVIEW.C G20E MEDIA379 G0210_as_Graphic558_ProjectileAspects,
 * lines 1303-1318. Test expectations are source metadata, while the buffer
 * under test is always decoded from the installed original Atari GRAPHICS.DAT. */
static const uint8_t k_source_g0210[14u * 6u] = {
    0, 0, 32, 11, 0x00, 0x11, 3, 18, 16, 11, 0x00, 0x11,
    6, 36, 24, 47, 0x00, 0x10, 9, 54, 32, 15, 0x01, 0x12,
    11, 54, 32, 12, 0x00, 0x11, 14, 72, 24, 47, 0x00, 0x10,
    17, 90, 24, 47, 0x00, 0x10, 20, 108, 16, 11, 0x00, 0x11,
    23, 126, 48, 18, 0x00, 0x11, 26, 144, 8, 15, 0x00, 0x12,
    28, 156, 16, 28, 0x01, 0x03, 29, 156, 16, 11, 0x01, 0x03,
    30, 156, 16, 28, 0x01, 0x03, 31, 156, 16, 24, 0x01, 0x03
};

static int check_selector_resolution(void)
{
    static const struct {
        uint16_t source_slot;
        int weapon_ordinal;
        int object_aspect;
        int expected;
    } cases[] = {
        {0xff80u, 0, -1, -11}, /* C15 fireball -> C10 */
        {0xff81u, 0, -1, -13}, /* C15 slime -> C12 */
        {0xff82u, 0, -1, -4},  /* C15 lightning -> C03 */
        {0xff86u, 0, -1, -14}, /* C15 poison -> C13 */
        {0xff87u, 0, -1, -14},
        {0xff83u, 0, -1, -12}, /* other C15 explosion -> C11 */
        {0x1401u, 3, -1, -3},   /* C05 weapon with M066 ordinal */
        {0x1401u, 0, 20, 20},  /* weapon without M066 art -> G0237/F0141 */
        {0x1c01u, 0, 22, 22}   /* other slot -> G0237/F0141 */
    };
    size_t index;
    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        int selector = 999;
        if (!csb_v1_atari_f0115_resolve_f0142_selector(
                cases[index].source_slot, cases[index].weapon_ordinal,
                cases[index].object_aspect, &selector) ||
            selector != cases[index].expected) {
            fprintf(stderr, "F0142 selector mismatch at case %zu: got %d\n",
                    index, selector);
            return 0;
        }
    }
    {
        int selector = 999;
        if (csb_v1_atari_f0115_resolve_f0142_selector(
                0xffffu, 0, 0, &selector) ||
            csb_v1_atari_f0115_resolve_f0142_selector(
                0x1401u, 15, 0, &selector) ||
            csb_v1_atari_f0115_resolve_f0142_selector(
                0x1401u, 0, 85, &selector)) {
            fprintf(stderr, "F0142 accepted unavailable/invalid source state\n");
            return 0;
        }
    }
    return 1;
}

static int check_shared_g0349_random_mod_and_mask(void)
{
    CSB_V1_RuntimeProfile profile;
    uint32_t state1;
    uint32_t state2;
    int expected_mod;
    int expected_mask;
    memset(&profile, 0, sizeof(profile));
    profile.csbwin_random_seed_valid = 1;
    profile.csbwin_random_seed = 0x12345678u;
    state1 = 0x12345678u * UINT32_C(0xbb40e62d) + UINT32_C(11);
    state2 = state1 * UINT32_C(0xbb40e62d) + UINT32_C(11);
    expected_mod = (int)((state1 >> 8) % 4u);
    expected_mask = (int)((state2 >> 8) & 63u);
    if (csb_v1_runtime_main_random_mod(&profile, 4) != expected_mod ||
        csb_v1_runtime_main_random_mask(&profile, 63) != expected_mask ||
        profile.csbwin_random_seed != state2) {
        fprintf(stderr, "M004/M003 did not consume exactly two shared G0349 draws\n");
        return 0;
    }
    profile.csbwin_random_seed_valid = 0;
    if (csb_v1_runtime_main_random_mod(&profile, 4) != -1 ||
        csb_v1_runtime_main_random_mask(&profile, 63) != -1) {
        fprintf(stderr, "M004/M003 accepted an unavailable G0349 stream\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_ATARI_GRAPHICS");
    CSB_AtariStLoader loader;
    uint8_t c558[CSB_V1_ATARI_F0115_PROJECTILE_C558_SIZE];
    CSB_V1_AtariF0115ProjectileInput input;
    CSB_V1_AtariF0115ProjectilePlan plan;
    CSB_V1_AtariF0115ExplosionInput explosion_input;
    CSB_V1_AtariF0115ExplosionPlan explosion_plan;
    int result;

    if (!path || !path[0]) return 77;
    csb_atari_st_graphics_loader_init(&loader);
    if (!csb_atari_st_graphics_loader_open(&loader, path)) {
        fprintf(stderr, "cannot open original Atari GRAPHICS.DAT: %s\n", path);
        return 1;
    }
    if (csb_atari_st_graphics_loader_read_item(
            &loader, 558u, c558, sizeof(c558)) != (int)sizeof(c558)) {
        fprintf(stderr, "cannot decode original Atari GRAPHICS.DAT item 558\n");
        csb_atari_st_graphics_loader_close(&loader);
        return 2;
    }
    csb_atari_st_graphics_loader_close(&loader);
    if (memcmp(c558 + CSB_V1_ATARI_F0115_PROJECTILE_G0210_OFFSET,
               k_source_g0210, sizeof(k_source_g0210)) != 0 ||
        c558[CSB_V1_ATARI_F0115_PROJECTILE_G0215_OFFSET] != 13u ||
        c558[CSB_V1_ATARI_F0115_PROJECTILE_G0215_OFFSET + 6u] != 32u) {
        fprintf(stderr, "original C558 does not match ReDMCSB MEDIA020 G0210/G0215\n");
        return 3;
    }
    memset(&explosion_input, 0, sizeof(explosion_input));
    explosion_input.c558 = c558;
    explosion_input.c558_size = sizeof(c558);
    explosion_input.explosion_type = 100;
    explosion_input.party_direction = 2;
    explosion_input.last_view_cell = 1;
    explosion_input.cell = 3;
    {
        static const int expected[7][3] = {
            {112,53,15},{24,53,15},{194,53,15},{112,59,20},
            {15,59,20},{208,59,20},{112,70,32}
        };
        int square;
        for (square = 0; square < 7; ++square) {
            explosion_input.view_square = square;
            if (csb_v1_atari_f0115_plan_explosion(
                    &explosion_input, &explosion_plan) !=
                    CSB_V1_ATARI_F0115_EXPLOSION_SPRITE ||
                !explosion_plan.uses_projectile_bitmap ||
                explosion_plan.graphic_index != 326 ||
                explosion_plan.aspect_index != 3 ||
                explosion_plan.source_byte_width != 32 ||
                explosion_plan.source_height != 15 ||
                explosion_plan.scale_32 != expected[square][2] ||
                explosion_plan.center_x != expected[square][0] ||
                explosion_plan.center_y != expected[square][1]) {
                fprintf(stderr,
                        "authentic C100 lightning/G0228 plan mismatch at row %d\n",
                        square);
                return 10;
            }
        }
    }
    explosion_input.view_square = 6;
    explosion_input.cell = 2;
    if (csb_v1_atari_f0115_plan_explosion(
            &explosion_input, &explosion_plan) !=
            CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN) {
        fprintf(stderr, "C100 rebirth ignored its visible-cell gate\n");
        return 11;
    }
    explosion_input.cell = 3;
    explosion_input.view_square = 7;
    if (csb_v1_atari_f0115_plan_explosion(
            &explosion_input, &explosion_plan) !=
            CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN) {
        fprintf(stderr, "C100 rebirth drew beyond D1C\n");
        return 12;
    }
    if (!check_selector_resolution() ||
        !check_shared_g0349_random_mod_and_mask()) return 4;

    memset(&input, 0, sizeof(input));
    input.c558 = c558;
    input.c558_size = sizeof(c558);
    input.f0142_selector = -1; /* source C10 ordinal => G0210 index 0 */
    input.view_square = 0;     /* D3C */
    input.view_cell = 2;       /* back-right; D3C front cells have X=0 */
    input.projectile_direction = 0;
    input.party_direction = 0;
    input.map_x = 1;
    input.map_y = 1;
    input.kinetic_energy = 255;
    result = csb_v1_atari_f0115_plan_projectile(&input, &plan);
    if (result != CSB_V1_ATARI_F0115_PROJECTILE_DRAW_PROJECTILE ||
        plan.aspect_index != 0 || plan.source_coordinate_x != 125 ||
        plan.source_coordinate_y != 72 || plan.native_graphic_index != 317 ||
        plan.bitmap_delta != 1 || plan.source_scale_index != 1 ||
        plan.source_scale_32 != 16 || plan.source_byte_width != 32 ||
        plan.draw_byte_width != 16 || plan.draw_width_pixels != 32 ||
        plan.draw_height != 5 || plan.derived_bitmap_index != 289 ||
        plan.palette_change_index != 0 || plan.flip_horizontal ||
        plan.flip_vertical || plan.destination_center_y != 47) {
        fprintf(stderr, "authentic C10/D3C F0115 projectile plan mismatch: result=%d aspect=%d coord=%d,%d native=%d delta=%d scale=%d/%d dimensions=%d,%d,%d derived=%d palette=%d flip=%d,%d\n",
                result, plan.aspect_index, plan.source_coordinate_x,
                plan.source_coordinate_y, plan.native_graphic_index,
                plan.bitmap_delta, plan.source_scale_index,
                plan.source_scale_32, plan.draw_byte_width,
                plan.draw_width_pixels, plan.draw_height,
                plan.derived_bitmap_index, plan.palette_change_index,
                plan.flip_horizontal, plan.flip_vertical);
        return 5;
    }

    input.f0142_selector = -4; /* source C03 lightning ordinal */
    input.view_square = 9;     /* D0C */
    input.view_cell = 0;
    input.projectile_direction = 0;
    input.kinetic_energy = 255;
    result = csb_v1_atari_f0115_plan_projectile(&input, &plan);
    if (result != CSB_V1_ATARI_F0115_PROJECTILE_DRAW_PROJECTILE ||
        plan.aspect_index != 3 || plan.source_coordinate_x != 66 ||
        plan.source_scale_index != 6 || plan.source_scale_32 != 0 ||
        plan.native_graphic_index != 325 || plan.draw_byte_width != 32 ||
        plan.draw_width_pixels != 64 || plan.draw_height != 15 ||
        plan.palette_change_index != CSB_V1_ATARI_F0115_PROJECTILE_NO_PALETTE ||
        plan.derived_bitmap_index != -1) {
        fprintf(stderr, "authentic C03/D0C F0115 projectile plan mismatch\n");
        return 6;
    }

    input.f0142_selector = -4;
    input.view_square = 4; /* D2L */
    input.view_cell = 0;
    input.projectile_direction = 1;
    input.party_direction = 0;
    input.kinetic_energy = 100;
    result = csb_v1_atari_f0115_plan_projectile(&input, &plan);
    if (result != CSB_V1_ATARI_F0115_PROJECTILE_DRAW_PROJECTILE ||
        plan.source_coordinate_x != 10 || plan.bitmap_delta != 1 ||
        plan.native_graphic_index != 326 || plan.source_scale_index != 2 ||
        plan.source_scale_32 != 7 || plan.draw_byte_width != 7 ||
        plan.draw_width_pixels != 14 || plan.draw_height != 3 ||
        plan.palette_change_index != 1 || plan.flip_vertical ||
        plan.flip_horizontal) {
        fprintf(stderr, "authentic C03/D2L energy-scaled projectile plan mismatch\n");
        return 7;
    }

    input.f0142_selector = 12;
    input.view_square = 0;
    input.view_cell = 2;
    result = csb_v1_atari_f0115_plan_projectile(&input, &plan);
    if (result != CSB_V1_ATARI_F0115_PROJECTILE_DRAW_OBJECT ||
        !plan.uses_object_aspect || plan.object_aspect_index != 12 ||
        plan.source_coordinate_x != 125 || plan.destination_center_y != 47) {
        fprintf(stderr, "authentic positive F0142 object-as-projectile plan mismatch\n");
        return 8;
    }

    input.f0142_selector = -1;
    input.view_square = 1; /* D3L C00 has no projectile coordinate */
    input.view_cell = 0;
    if (csb_v1_atari_f0115_plan_projectile(&input, &plan) !=
        CSB_V1_ATARI_F0115_PROJECTILE_NOT_VISIBLE) {
        fprintf(stderr, "F0115 failed to suppress the authentic zero-X G0218 cell\n");
        return 9;
    }
    puts("PASS: ReDMCSB MEDIA020 F0142/F0115 plans match original Atari C558");
    return 0;
}
