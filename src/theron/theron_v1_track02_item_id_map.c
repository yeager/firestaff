/* theron_v1_track02_item_id_map.c — TQ-to-DM1 item ID translation.
 *
 * TQ uses different item value IDs than standard DM1/CSB for actuators.
 * Mapping from gbsphenx/dmbuilder FIX_TQActuatorList().
 * Wall/floor decoration tables from FIX_TQDecorations().
 */

#include "theron_v1_track02_item_id_map.h"
#include <stddef.h>

static const Theron_ItemIdMapping g_item_id_map[] = {
    { 0x32, 0x6E, "Shield Defiant (Shield of Lyte)" },
    { 0x38, 0x7F, "Gold Coin" },
    { 0x55, 0xB0, "Iron Key" },
    { 0x56, 0xB8, "Gold Key" },
    { 0x57, 0xBE, "Ra Key" },
};

#define ITEM_ID_MAP_COUNT (sizeof(g_item_id_map) / sizeof(g_item_id_map[0]))

unsigned int theron_v1_track02_item_id_map_count(void) {
    return (unsigned int)ITEM_ID_MAP_COUNT;
}

const Theron_ItemIdMapping *theron_v1_track02_item_id_map(unsigned int index) {
    if (index >= ITEM_ID_MAP_COUNT) return NULL;
    return &g_item_id_map[index];
}

uint8_t theron_v1_track02_translate_item_id(uint8_t tq_id) {
    for (unsigned int i = 0; i < ITEM_ID_MAP_COUNT; i++) {
        if (g_item_id_map[i].tq_id == tq_id)
            return g_item_id_map[i].dm1_id;
    }
    return tq_id;
}

/* AKUTUBA wall ornaments per level, from dmbuilder FIX_TQDecorations. */
static const Theron_WallOrnament g_akutuba_l1_walls[] = {
    {  0, 0x05, "Iron Lock" },
    {  1, 0x2D, "Lever Down" },
    {  2, 0x2C, "Lever Up" },
    {  3, 0x01, "Alcove" },
    {  4, 0x31, "Brick" },
    {  5, 0x1A, "Gold Lock" },
    {  6, 0x36, "Brick Pushed" },
    {  7, 0x35, "Red Button Pushed" },
    {  8, 0x2F, "Red Button" },
    {  9, 0x37, "Hidden Switch Pushed" },
    { 10, 0x32, "Hidden Switch" },
    { 11, 0x02, "Vi Altar" },
    { 12, 0x26, "Torch Holder Empty" },
    { 13, 0x2E, "Torch Holder Full" },
};

static const Theron_WallOrnament g_akutuba_l2_walls[] = {
    {  0, 0x11, "Coin Slot" },
    {  1, 0x2D, "Lever Down" },
    {  2, 0x2C, "Lever Up" },
    {  3, 0x02, "Vi Altar" },
    {  4, 0x35, "Red Button Pushed" },
    {  5, 0x2F, "Red Button" },
    {  6, 0x26, "Torch Holder Empty" },
    {  7, 0x2E, "Torch Holder Full" },
    {  8, 0x25, "Demon Face" },
    {  9, 0x29, "Spell Holes" },
    { 10, 0x07, "Small Switch" },
    { 11, 0x36, "Brick Pushed" },
    { 12, 0x31, "Brick" },
    { 13, 0x10, "Cyan Button" },
};

static const Theron_WallOrnament g_akutuba_l3_walls[] = {
    {  0, 0x01, "Alcove" },
    {  1, 0x02, "Vi Altar" },
    {  2, 0x23, "Fountain" },
    {  3, 0x07, "Small Switch" },
    {  4, 0x1E, "Ra Lock" },
    {  5, 0x05, "Iron Lock" },
};

/* Level 0 (hub) has only a mirror. */
static const Theron_WallOrnament g_akutuba_l0_walls[] = {
    { 0, 0x2B, "Mirror" },
};

unsigned int theron_v1_track02_akutuba_wall_ornament_count(unsigned int level) {
    switch (level) {
        case 0: return sizeof(g_akutuba_l0_walls) / sizeof(g_akutuba_l0_walls[0]);
        case 1: return sizeof(g_akutuba_l1_walls) / sizeof(g_akutuba_l1_walls[0]);
        case 2: return sizeof(g_akutuba_l2_walls) / sizeof(g_akutuba_l2_walls[0]);
        case 3: return sizeof(g_akutuba_l3_walls) / sizeof(g_akutuba_l3_walls[0]);
        default: return 0;
    }
}

const Theron_WallOrnament *theron_v1_track02_akutuba_wall_ornaments(
    unsigned int level, unsigned int *count)
{
    if (!count) return NULL;
    *count = theron_v1_track02_akutuba_wall_ornament_count(level);
    switch (level) {
        case 0: return g_akutuba_l0_walls;
        case 1: return g_akutuba_l1_walls;
        case 2: return g_akutuba_l2_walls;
        case 3: return g_akutuba_l3_walls;
        default: return NULL;
    }
}
