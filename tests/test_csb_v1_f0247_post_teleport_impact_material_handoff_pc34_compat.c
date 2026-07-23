/* ReDMCSB PROJEXPL.C F0219 -> DUNVIEW.C F0115 ownership handoff. */
#include "csb_v1_viewport_f0115_projectile_metadata_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static void make_post_teleport_fixture(CSB_V1_DungeonData *dungeon,
                                       unsigned char raw[224])
{
    int i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 224u);
    dungeon->level_count = 2;
    dungeon->level_widths[0] = dungeon->level_widths[1] = 2;
    dungeon->level_heights[0] = dungeon->level_heights[1] = 2;
    dungeon->level_offsets[0] = 0;
    dungeon->level_offsets[1] = 4;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 224;
    dungeon->square_first_thing_base = 96;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[1] = 144;
    dungeon->thing_type_counts[1] = 1;
    dungeon->thing_data_bases[3] = 160;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[14] = 184;
    dungeon->thing_type_counts[14] = 1;

    for (i = 0; i < 8; ++i) raw[i] = (unsigned char)(1u << 5);

    /* Map 0 (1,0): real open C05 teleporter whose loaded target is map 1. */
    raw[2] = (unsigned char)((5u << 5) | 0x18u);
    put_le16(raw, 76, 0u);
    put_le16(raw, 96, (unsigned short)(1u << 10));
    put_le16(raw, 144, 0xfffeu);
    put_le16(raw, 148, 0u);
    put_le16(raw, 150, 0x0100u);

    /* Map 1 (0,0): a C03 precedes the moved C14 in the real Thing chain. */
    raw[4] = (unsigned char)((1u << 5) | 0x10u);
    put_le16(raw, 80, 1u);
    put_le16(raw, 98, (unsigned short)(3u << 10));
    put_le16(raw, 160, (unsigned short)(14u << 10));
    put_le16(raw, 184, 0xfffeu);
    put_le16(raw, 186, 0x1407u); /* Actual associated weapon Thing. */
    raw[188] = 200u;
    raw[189] = 45u;
    put_le16(raw, 190, 17u);
}

static void test_owned_destination_c14_gets_material_handoff(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 handoff;
    unsigned char raw[224];
    const unsigned short projectile = (unsigned short)(14u << 10);

    make_post_teleport_fixture(&dungeon, raw);
    CHECK(csb_v1_f0219_post_teleport_projectile_impact_material_handoff_pc34(
              &dungeon, 1, 0, 0, projectile,
              CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
              CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
              CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE,
              &handoff) == 1,
          "F0219 resolved C05 destination admits its owned C14 to F0115");
    CHECK(handoff.valid && handoff.projectileThing == projectile &&
              handoff.associatedThing == 0x1407u &&
              handoff.kineticEnergy == 200 && handoff.attack == 45,
          "handoff retains the real C14 record and associated Thing bytes");
    CHECK(handoff.mapIndex == 1 && handoff.mapX == 0 && handoff.mapY == 0 &&
              handoff.bitmapIndex == 487 && handoff.zOrder == 2905 &&
              handoff.transparentFlag == 1,
          "handoff carries F0115 material only for the resolved destination");
}

static void test_detached_or_source_c14_never_gets_marker(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 handoff;
    unsigned char raw[224];
    const unsigned short projectile = (unsigned short)(14u << 10);

    make_post_teleport_fixture(&dungeon, raw);
    CHECK(csb_v1_f0219_post_teleport_projectile_impact_material_handoff_pc34(
              &dungeon, 0, 1, 0, projectile,
              CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
              CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
              CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR,
              &handoff) == 0 && !handoff.valid,
          "source C05 square cannot manufacture a post-teleport C14 handoff");

    put_le16(raw, 160, 0xfffeu);
    CHECK(csb_v1_f0219_post_teleport_projectile_impact_material_handoff_pc34(
              &dungeon, 1, 0, 0, projectile,
              CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
              CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
              CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR,
              &handoff) == 0 && !handoff.valid,
          "detached C14 record cannot create a synthetic presentation marker");
}

int main(void)
{
    test_owned_destination_c14_gets_material_handoff();
    test_detached_or_source_c14_never_gets_marker();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
