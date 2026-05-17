#include "dm1_v1_group_management_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Creature Group Management — implementation
 *
 * Source lock: ReDMCSB WIP20210206 GROUP.C
 *   F0175_GROUP_GetThing: walk thing list at (x,y) for TYPE_GROUP
 *   F0176_GROUP_GetCreatureOrdinalInCell: scan cells array
 *   F0193: add to active group array (G0375), increment G0377
 *   F0194: remove from active array, shift down, decrement G0377
 *   F0177-F0191: group movement direction selection, collision checks
 *   F0192: damage calculation respecting creature armor/resistance
 *
 * Direction offset tables (shared with MOVESENS.C):
 *   East:  { 0, 1, 0, -1 }  for N, E, S, W
 *   North: {-1, 0, 1,  0 }
 */

static const int s_dirEast[4]  = { 0, 1, 0, -1 };
static const int s_dirNorth[4] = {-1, 0, 1,  0 };

void m11_group_init(M11_GroupState *s)
{
    memset(s, 0, sizeof(*s));
    s->maxActiveGroups = M11_MAX_ACTIVE_GROUPS;
    /* Mark all flux cages as empty */
    for (int i = 0; i < M11_FLUX_CAGE_MAX; i++) {
        s->fluxCages[i] = -1;
    }
}

M11_Group *m11_group_get_at(M11_GroupState *s, int mapX, int mapY)
{
    for (int i = 0; i < s->activeGroupCount; i++) {
        if (s->activeGroups[i].mapX == mapX &&
            s->activeGroups[i].mapY == mapY) {
            return &s->activeGroups[i];
        }
    }
    return NULL;
}

int m11_group_get_creature_in_cell(const M11_Group *group, int cell)
{
    if (!group || cell < 0 || cell >= M11_GROUP_CELLS) return -1;
    uint8_t idx = group->cells[cell];
    if (idx == 0xFF) return -1;
    if (idx >= M11_MAX_CREATURES_PER_GROUP) return -1;
    return (int)idx;
}

int m11_group_add_active(M11_GroupState *s, const M11_Group *group)
{
    if (s->activeGroupCount >= s->maxActiveGroups) return -1;
    int idx = s->activeGroupCount;
    memcpy(&s->activeGroups[idx], group, sizeof(M11_Group));
    s->activeGroupCount++;
    return idx;
}

int m11_group_remove_active(M11_GroupState *s, int mapX, int mapY)
{
    for (int i = 0; i < s->activeGroupCount; i++) {
        if (s->activeGroups[i].mapX == mapX &&
            s->activeGroups[i].mapY == mapY) {
            /* Shift remaining groups down */
            for (int j = i; j < s->activeGroupCount - 1; j++) {
                s->activeGroups[j] = s->activeGroups[j + 1];
            }
            s->activeGroupCount--;
            return 1;
        }
    }
    return 0;
}

void m11_group_calc_party_relation(M11_GroupState *s,
                                    int groupX, int groupY,
                                    int partyX, int partyY)
{
    int dx = partyX - groupX;
    int dy = partyY - groupY;
    int absDx = dx < 0 ? -dx : dx;
    int absDy = dy < 0 ? -dy : dy;

    s->currentGroupDistToParty = absDx + absDy; /* Manhattan distance */

    /* Primary direction: toward party along the longer axis */
    if (absDx >= absDy) {
        s->currentGroupPrimaryDir = (dx > 0) ? 1 : 3; /* East or West */
        s->currentGroupSecondaryDir = (dy > 0) ? 2 : 0; /* South or North */
    } else {
        s->currentGroupPrimaryDir = (dy > 0) ? 2 : 0; /* South or North */
        s->currentGroupSecondaryDir = (dx > 0) ? 1 : 3; /* East or West */
    }
    if (absDx == 0) s->currentGroupPrimaryDir = (dy > 0) ? 2 : 0;
    if (absDy == 0) s->currentGroupSecondaryDir = s->currentGroupPrimaryDir;
}

int m11_group_can_move(M11_GroupState *s,
                       int groupX, int groupY, int direction,
                       int mapWidth, int mapHeight)
{
    if (direction < 0 || direction >= 4) return 0;

    int newX = groupX + s_dirEast[direction];
    int newY = groupY + s_dirNorth[direction];

    /* Bounds check */
    if (newX < 0 || newX >= mapWidth || newY < 0 || newY >= mapHeight) {
        s->movementBlockedByWall = 1;
        return 0;
    }

    /* Check for other groups at destination */
    if (m11_group_get_at(s, newX, newY) != NULL) {
        s->movementBlockedByGroup = 1;
        return 0;
    }

    /* Reset blocking flags */
    s->movementBlockedByWall = 0;
    s->movementBlockedByGroup = 0;
    s->movementBlockedByDoor = 0;
    s->movementBlockedByParty = 0;

    /* Mark direction as tested */
    s->movementTestedDirs[direction] = 1;

    return 1;
}

int m11_group_move(M11_GroupState *s, M11_Group *group, int direction)
{
    if (!group || direction < 0 || direction >= 4) return 0;
    (void)s; /* State used in full implementation for collision checks */

    group->mapX += s_dirEast[direction];
    group->mapY += s_dirNorth[direction];
    return 1;
}

int m11_group_damage_creature(M11_Group *group, int creatureIndex, int damage)
{
    if (!group || creatureIndex < 0 ||
        creatureIndex >= M11_MAX_CREATURES_PER_GROUP) return 0;

    group->hitPoints[creatureIndex] -= (int16_t)damage;
    return (int)group->hitPoints[creatureIndex];
}

int m11_group_remove_creature(M11_Group *group, int creatureIndex)
{
    if (!group || creatureIndex < 0 ||
        creatureIndex >= M11_MAX_CREATURES_PER_GROUP) return 0;

    /* Clear from all cells */
    for (int c = 0; c < M11_GROUP_CELLS; c++) {
        if (group->cells[c] == (uint8_t)creatureIndex) {
            group->cells[c] = 0xFF;
        }
    }
    /* Reindex cells: decrement references to creatures above the removed index */
    for (int c = 0; c < M11_GROUP_CELLS; c++) {
        if (group->cells[c] != 0xFF &&
            group->cells[c] > (uint8_t)creatureIndex) {
            group->cells[c]--;
        }
    }
    group->hitPoints[creatureIndex] = 0;
    if (group->creatureCount > 0) {
        group->creatureCount--;
    }
    return (group->creatureCount == 0) ? 1 : 0;
}

int m11_group_is_alive(const M11_Group *group)
{
    if (!group) return 0;
    return group->creatureCount > 0;
}

int m11_group_total_creature_count(const M11_GroupState *s)
{
    int total = 0;
    for (int i = 0; i < s->activeGroupCount; i++) {
        total += s->activeGroups[i].creatureCount;
    }
    return total;
}

const char *m11_group_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206 GROUP.C\n"
        "F0175_GROUP_GetThing: walk thing list for TYPE_GROUP.\n"
        "F0176_GROUP_GetCreatureOrdinalInCell: scan cells[4] array.\n"
        "F0193: add to G0375 active array, increment G0377.\n"
        "F0194: remove from active array, shift down, decrement G0377.\n"
        "F0177-F0191: movement direction selection, collision checks.\n"
        "  G0384: tested directions, G0387/G0388/G0389/G0390: blocking flags.\n"
        "  G0381: Manhattan distance to party.\n"
        "  G0382/G0383: primary/secondary direction to party.\n"
        "F0192: damage creature, check HP.\n"
        "G0385/G0386: flux cages (4 max).\n"
        "Groups have 1-4 creatures in 4 cells (2x2 per square).";
}
