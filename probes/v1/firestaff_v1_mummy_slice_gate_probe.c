/*
 * firestaff_v1_mummy_slice_gate_probe.c
 *
 * DM1 V1 Mummy (C010_CREATURE_MUMMY) source-locked slice gate probe.
 *
 * Prints the source-locked Mummy aspect + per-frame aspect offset cycle
 * + single/multi-creature group placement to stdout, then exits 0.  No
 * GRAPHICS.DAT is needed and no SDL3 dependency is required at runtime.
 *
 * Source-locked references:
 *   DUNGEON.C:683        G0243_as_Graphic559_CreatureInfo[10] (I34E)
 *   DUNVIEW.C:1656       G0219_as_Graphic558_CreatureAspects[10]
 *   DUNVIEW.C:2392       M618_GRAPHIC_FIRST_CREATURE = 584
 *   DEFS.H:1367          C0xFF_SINGLE_CENTERED_CREATURE = 0xFF
 *   DEFS.H:1631-1632     M052/M053 MAX offset macros
 *   DEFS.H:591-606       M022/M023/M024/M025 offset accessors
 *   GROUP.C:187-308      F0179_GROUP_GetCreatureAspectUpdateTime
 *   GROUP.C:524-560      F0185 single/multi-creature group placement
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "dm1_v1_creature_render_pc34_compat.h"

/* Deterministic splitmix64 PRNG so the probe output is reproducible. */
static uint64_t g_rngState = 0x9E3779B97F4A7C15ull;
static int probe_rng(void* user, int range) {
    uint64_t* s = (uint64_t*)user;
    uint64_t z = (*s += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    z = z ^ (z >> 31);
    if (range <= 0) return 0;
    return (int)(z % (uint64_t)range);
}

static void print_hex_byte(const char* label, unsigned value) {
    printf("  %-32s = 0x%02X\n", label, value & 0xFFu);
}

static void print_dec(const char* label, int value) {
    printf("  %-32s = %d\n", label, value);
}

static void print_str(const char* label, const char* value) {
    printf("  %-32s = %s\n", label, value);
}

static void print_section(const char* name) {
    printf("\n[ %s ]\n", name);
}

int main(void) {
    int failures = 0;
    const DM1_CreatureAspect* aspects = dm1_creature_aspects();
    const DM1_CreatureAspect* mummy = &aspects[DM1_CREATURE_MUMMY];
    int i;

    print_section("Mummy Aspect (G0219[10], DUNVIEW.C:1656)");
    print_hex_byte("firstNativeBitmapRelativeIndex",
                   (unsigned)mummy->firstNativeBitmapRelativeIndex);
    print_hex_byte("firstDerivedBitmapIndex",
                   (unsigned)mummy->firstDerivedBitmapIndex);
    print_hex_byte("coordinateSet_transparentColor",
                   mummy->coordinateSet_transparentColor);
    print_hex_byte("replacementColorSetIndices",
                   mummy->replacementColorSetIndices);
    printf("  %-32s = 0x%04X\n", "graphicInfo", mummy->graphicInfo);
    print_dec("coordinate set (high nibble)",
              dm1_creature_coordinate_set(DM1_CREATURE_MUMMY));
    print_dec("transparent color (low nibble)",
              dm1_creature_transparent_color(DM1_CREATURE_MUMMY));
    print_str("name", dm1_creature_type_name(DM1_CREATURE_MUMMY));

    print_section("M052 / M053 Offset Range (DEFS.H:1631-1632)");
    print_dec("M052 max horizontal offset",
              dm1_creature_max_horizontal_offset(mummy->graphicInfo));
    print_dec("M053 max vertical offset",
              dm1_creature_max_vertical_offset(mummy->graphicInfo));

    print_section("Native Bitmap Index per Pose (DUNVIEW.C:5354-5379)");
    print_dec("FRONT",
              (int)dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
                                                     DM1_CREATURE_POSE_FRONT));
    print_dec("SIDE",
              (int)dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
                                                     DM1_CREATURE_POSE_SIDE));
    print_dec("BACK",
              (int)dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
                                                     DM1_CREATURE_POSE_BACK));
    print_dec("ATTACK",
              (int)dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
                                                     DM1_CREATURE_POSE_ATTACK));

    /* Front-index must equal M618 + firstNativeBitmapRelativeIndex. */
    if ((int)dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
            DM1_CREATURE_POSE_FRONT) !=
        DM1_GRAPHIC_FIRST_CREATURE +
        (int)mummy->firstNativeBitmapRelativeIndex) {
        printf("FAIL: Mummy front bitmap != M618 + firstNative\n");
        failures++;
    }

    print_section("F0179 Aspect Frame Cycle (16 iterations)");
    printf("  %-6s %-6s %-6s %-8s %-8s %-8s %-8s %-8s\n",
           "iter", "rand", "atk", "aspect", "flip", "attack", "hOff", "vOff");
    {
        uint8_t aspect = 0;
        for (i = 0; i < 16; ++i) {
            int randBit = (int)(g_rngState & 0x7);
            int attacking = (i & 1);
            aspect = dm1_creature_cycle_aspect_frame(DM1_CREATURE_MUMMY,
                                                      aspect, attacking,
                                                      randBit);
            printf("  %-6d %-6d %-6d 0x%02X     %-8d %-8d %-8d %-8d\n",
                   i, randBit, attacking, aspect,
                   (aspect & DM1_CREATURE_ASPECT_FLIP_BITMAP) ? 1 : 0,
                   (aspect & DM1_CREATURE_ASPECT_IS_ATTACKING) ? 1 : 0,
                   dm1_creature_aspect_horizontal_offset(aspect),
                   dm1_creature_aspect_vertical_offset(aspect));
        }
    }

    print_section("F0185 Group Placement");
    print_dec("single Mummy (count=0)",
              dm1_creature_place_group_cells(0, DM1_CREATURE_SIZE_QUARTER,
                                              NULL, NULL));
    print_dec("two Mummies (count=2, NULL rng)",
              dm1_creature_place_group_cells(2, DM1_CREATURE_SIZE_QUARTER,
                                              NULL, NULL));
    print_dec("four Mummies (count=4, NULL rng)",
              dm1_creature_place_group_cells(4, DM1_CREATURE_SIZE_QUARTER,
                                              NULL, NULL));

    /* Deterministic rng: shows that the start cell is randomised before
     * the loop and that subsequent slot cells step +1 (quarter-square)
     * or +2 (half-square). */
    {
        uint64_t seedA = 0xA1B2C3D4ull;
        uint64_t seedB = 0x12345678ull;
        int cells;
        g_rngState = seedA;
        cells = dm1_creature_place_group_cells(4,
                                                DM1_CREATURE_SIZE_QUARTER,
                                                probe_rng, &g_rngState);
        printf("  four Mummies (rng state=0x%016llX) -> 0x%02X\n",
               (unsigned long long)seedA, cells & 0xFF);
        g_rngState = seedB;
        cells = dm1_creature_place_group_cells(2,
                                                DM1_CREATURE_SIZE_FULL,
                                                probe_rng, &g_rngState);
        printf("  two full-sq creatures (rng state=0x%016llX) -> 0x%02X\n",
               (unsigned long long)seedB, cells & 0xFF);
    }

    printf("\nFAILURES=%d\n", failures);
    return failures > 0 ? 1 : 0;
}
