/*
 * DM1 V1 Creature Viewport Rendering — pc34 compat layer.
 * Based on ReDMCSB DUNVIEW.C G0219_as_Graphic558_CreatureAspects,
 * M618_GRAPHIC_FIRST_CREATURE = 558, F0128 creature drawing section.
 * Generated via Q3.6 + manual fixes for type names and collect signature.
 */

#include "dm1_v1_creature_render_pc34_compat.h"
#include <stdlib.h>
#include <string.h>

void m11_creature_render_init(M11_CreatureRenderList* list) {
    if (!list) return;
    list->count = 0;
    memset(list->entries, 0, sizeof(list->entries));
}

void m11_creature_render_collect(M11_CreatureRenderList* list,
                                  int partyX, int partyY, int partyDir,
                                  const void* dungeonData) {
    /*
     * Stub implementation — scans view frustum positions based on
     * party facing direction. Actual creature lookup requires wiring
     * to dungeon square data (F0267 style thing-list walk).
     * ReDMCSB: DUNVIEW.C F0128 iterates squares at each depth/column
     * relative to party facing, collecting creature groups.
     */
    if (!list) return;
    list->count = 0;
    /* TODO: Wire to actual dungeon creature group iteration.
     * For each depth 0..3 and column -1,0,1:
     *   - Compute absolute map coords from partyX/Y/Dir
     *   - Walk thing list at that square for creature groups
     *   - Extract creature type, aspect, attack state
     *   - Add to render list via m11_creature_get_graphic
     */
    (void)partyX; (void)partyY; (void)partyDir; (void)dungeonData;
}

void m11_creature_render_sort(M11_CreatureRenderList* list) {
    /* Sort by viewDepth descending (far first = back-to-front),
     * then by viewColumn for deterministic draw order.
     * ReDMCSB: F0128 draws D3 squares first, then D2, D1, D0. */
    int i, j, swap;
    M11_CreatureRenderEntry temp;
    if (!list) return;
    for (i = 0; i < list->count - 1; i++) {
        for (j = 0; j < list->count - i - 1; j++) {
            swap = 0;
            if (list->entries[j].viewDepth < list->entries[j + 1].viewDepth) {
                swap = 1;
            } else if (list->entries[j].viewDepth == list->entries[j + 1].viewDepth) {
                if (list->entries[j].viewColumn > list->entries[j + 1].viewColumn) {
                    swap = 1;
                }
            }
            if (swap) {
                temp = list->entries[j];
                list->entries[j] = list->entries[j + 1];
                list->entries[j + 1] = temp;
            }
        }
    }
}

int m11_creature_get_graphic(int creatureType, int attacking, int animFrame) {
    /* ReDMCSB G0219_as_Graphic558_CreatureAspects:
     * M618_GRAPHIC_FIRST_CREATURE = 558
     * Each creature type has 18 frames (9 normal + 9 attack).
     * Frame index within set = (attacking ? 9 : 0) + animFrame % 9 */
    return 558 + creatureType * 18 + (attacking ? 9 : 0) + (animFrame % 9);
}

const char* m11_creature_type_name(int creatureType) {
    /* ReDMCSB CHAMPION.H C000-C026 creature type names */
    switch (creatureType) {
        case DM1_CREATURE_GIANT_SCORPION:  return "Giant Scorpion";
        case DM1_CREATURE_SWAMP_SLIME:     return "Swamp Slime";
        case DM1_CREATURE_GIGGLER:         return "Giggler";
        case DM1_CREATURE_WIZARD_EYE:      return "Wizard Eye";
        case DM1_CREATURE_PAIN_RAT:        return "Pain Rat";
        case DM1_CREATURE_SCREAMER:        return "Screamer";
        case DM1_CREATURE_ROCKPILE:        return "Rockpile";
        case DM1_CREATURE_GHOST:           return "Ghost";
        case DM1_CREATURE_STONE_GOLEM:     return "Stone Golem";
        case DM1_CREATURE_MUMMY:           return "Mummy";
        case DM1_CREATURE_BLACK_FLAME:     return "Black Flame";
        case DM1_CREATURE_SKELETON:        return "Skeleton";
        case DM1_CREATURE_COUATL:          return "Couatl";
        case DM1_CREATURE_VEXIRK:          return "Vexirk";
        case DM1_CREATURE_MAGENTA_WORM:    return "Magenta Worm";
        case DM1_CREATURE_TROLIN:          return "Trolin";
        case DM1_CREATURE_GIANT_WASP:      return "Giant Wasp";
        case DM1_CREATURE_ANIMATED_ARMOUR: return "Animated Armour";
        case DM1_CREATURE_MATERIALIZER:    return "Materializer";
        case DM1_CREATURE_WATER_ELEMENTAL: return "Water Elemental";
        case DM1_CREATURE_OITU:            return "Oitu";
        case DM1_CREATURE_DEMON:           return "Demon";
        case DM1_CREATURE_LORD_CHAOS:      return "Lord Chaos";
        case DM1_CREATURE_RED_DRAGON:      return "Red Dragon";
        case DM1_CREATURE_LORD_ORDER:      return "Lord Order";
        case DM1_CREATURE_GREY_LORD:       return "Grey Lord";
        case DM1_CREATURE_ZYTAZ:           return "Zytaz";
        default:                           return "Unknown";
    }
}
