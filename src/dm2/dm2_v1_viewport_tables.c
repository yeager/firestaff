#include "dm2_v1_viewport_tables.h"

const uint8_t dm2_v1_vp_render_order[20] = {
    0x13, 0x14, 0x11, 0x12, 0x10,
    0x0e, 0x0f, 0x0c, 0x0d, 0x0b,
    0x09, 0x0a, 0x07, 0x08, 0x06,
    0x04, 0x05, 0x03, 0x01, 0x02
};

const uint8_t dm2_v1_vp_column_count[23] = {
    0x00, 0x02, 0x02, 0x01, 0x03,
    0x03, 0x01, 0x03, 0x03, 0x02,
    0x02, 0x01, 0x03, 0x03, 0x03,
    0x03, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00
};

const int16_t dm2_v1_vp_wall_face_near[18] = {
    -1, -1, 0x0340, 0x0340, 0x0341, 0x0341,
    -1, -1, 0x033e, 0x033c, 0x033f, 0x033d,
    -1, -1, 0x033a, 0x033a, 0x033b, 0x033b
};

const int8_t dm2_v1_vp_wall_ornament_near[18] = {
    -1, -1, (int8_t)0xcd, (int8_t)0xc7, (int8_t)0xce, (int8_t)0xc8,
    -1, -1, (int8_t)0xcf, (int8_t)0xc9, (int8_t)0xd0, (int8_t)0xca,
    -1, -1, (int8_t)0xd1, (int8_t)0xcb, (int8_t)0xd2, (int8_t)0xcc
};

const int16_t dm2_v1_vp_wall_face_mid[32] = {
    -1, -1, -1, -1, -1, -1, 0x0336, 0x0329,
    0x0335, 0x0328, 0x0337, 0x032a, 0x0333, 0x0326, 0x0332, 0x0325,
    0x0334, 0x0327, -1, -1, -1, -1, 0x0330, 0x0323,
    0x032f, 0x0322, 0x0331, 0x0324, 0x0320, 0x0320, 0x0321, 0x0321
};

const int8_t dm2_v1_vp_wall_ornament_mid[32] = {
    -1, -1, -1, -1, -1, -1, 0x4f, 0x3b,
    0x50, 0x3c, 0x50, 0x3c, 0x52, 0x3e, 0x53, 0x3f,
    0x53, 0x3f, -1, -1, -1, -1, 0x55, 0x41,
    0x56, 0x42, 0x56, 0x42, 0x58, 0x44, 0x58, 0x44
};

const int8_t dm2_v1_vp_wall_ornament_mid_alt[32] = {
    -1, -1, -1, -1, -1, -1, 0x4f, 0x3b,
    0x50, 0x3c, 0x51, 0x3d, 0x52, 0x3e, 0x53, 0x3f,
    0x54, 0x40, -1, -1, -1, -1, 0x55, 0x41,
    0x56, 0x42, 0x57, 0x43, 0x58, 0x44, 0x59, 0x45
};

const uint8_t dm2_v1_vp_wall_visible[16] = {
    1, 0, 0, 1, 1, 1, 1, 1,
    1, 0, 0, 1, 1, 1, 1, 1
};

const int16_t dm2_v1_vp_wall_rect_id[16] = {
    0x0ee2, -1, -1, 0x0ece, 0x0ec4, 0x0ed8, 0x0eb0, 0x0ea6,
    0x0eba, -1, -1, 0x0e92, 0x0e88, 0x0e9c, 0x0e74, 0x0e7e
};

const int8_t dm2_v1_vp_depth_index[5] = { 3, 2, 1, 0, -1 };

const int16_t dm2_v1_vp_floor_item_near[14] = {
    0, 0, 0, 0x02f2, 0x02f1, 0x02f3, 0x02ef,
    0x02ee, 0x02f0, 0, 0, 0, 0, 0
};

const int8_t dm2_v1_vp_tile_walk_dx[8][2] = {
    {-1, 0}, {0, -1}, {0, -1}, {1, 0},
    {1, 0},  {0, 1},  {-1, 0}, {0, 1}
};

const int16_t dm2_v1_vp_tile_scan_dx[4][2] = {
    {0, 1}, {0, -1}, {1, 0}, {-1, 0}
};

const int8_t dm2_v1_vp_facing_remap[32] = {
    3, 2, 1, 0, 4, 4, 4, 4,
    3, 2, 1, 0, 4, 4, 4, 4,
    4, 4, 4, 4, 1, 0, 3, 2,
    4, 4, 4, 4, 1, 0, 3, 2
};

const int8_t dm2_v1_vp_facing_reverse[4] = { 0, 3, 2, 1 };

const int8_t dm2_v1_vp_creature_order[8][4] = {
    {0, 1, 3, 2}, {1, 0, 2, 3},
    {1, 2, 0, 3}, {2, 1, 3, 0},
    {3, 2, 0, 1}, {2, 3, 1, 0},
    {0, 3, 1, 2}, {3, 0, 2, 1}
};

const int8_t dm2_v1_vp_creature_subpos[4] = { 0, 1, 0, -1 };

const int16_t dm2_v1_vp_champion_pane_rect[8] = {
    0x01c8, 0x01c9, 0x01cc, 0x01cd,
    0x01ca, 0x01cb, 0x01cc, 0x01cd
};

const int16_t dm2_v1_vp_champion_pane_rect2[10] = {
    0x01ce, 0x01cf, 0x01ce, 0x01cf, 0x01d2,
    0x01d3, 0x01d0, 0x01d1, 0x01d2, 0x01d3
};

const int8_t dm2_v1_vp_light_curve[16] = {
    0, 5, 12, 24, 33, 40, 46, 51,
    59, 68, 76, 82, 89, 94, 97, 100
};

const uint32_t dm2_v1_vp_palette_mask_5bit[8] = {
    0x00000000, 0x00010000, 0x00020000, 0x000b0000,
    0x00040000, 0x00050000, 0x00060000, 0x00070000
};

const uint32_t dm2_v1_vp_palette_mask_full[8] = {
    0x00000000, 0x00010800, 0x00204000, 0x002b5800,
    0x00408000, 0x0055a800, 0x0060c000, 0x007ff800
};

const uint32_t dm2_v1_vp_palette_mask_rgb[8] = {
    0x00000000, 0x00010840, 0x00204200, 0x002b5ac0,
    0x00408400, 0x0055ad40, 0x0060c600, 0x007fffc0
};

const uint32_t dm2_v1_vp_palette_alpha[4] = {
    0x00000000, 0x00000200, 0x00000400, 0x00000600
};

const char *dm2_v1_viewport_tables_source_evidence(void)
{
    return "skproject SKULLWIN/dm2data.cpp "
           "table1d7029:258 table1d7012:266 table1d6fee:275 table1d6fdc:282 "
           "table1d6f9c:289 table1d6f7c:297 table1d6f5c:305 table1d6f4c:313 "
           "table1d6f2c:319 table1d6f27:325 table1d6f0b:330 "
           "table1d26a8:347 table1d62e8:365 table1d62d0:387 table1d62b0:375 "
           "table1d26d0:441 table1d3ffc:395 table1d27c4:405 table1d27d4:411 "
           "table1d6702:60 table1d7092:214 table1d7072:226 table1d7052:238 "
           "table1d7042:250; DM2 viewport rendering constant tables.";
}
