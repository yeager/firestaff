/* DM1 V1 Teleporter/Pit — ReDMCSB MOVESENS.C F0276, F0267.
 * Generated via Q3.6, fixed by Opus (added x,y to structs, fixed pit chain). */
#include "dm1_v1_teleporter_pit_pc34_compat.h"
#include <string.h>

void m11_teleporter_pit_init(M11_TeleporterPitState* s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->chainDepthLimit = 1000; /* ReDMCSB F0267: 1000 for party/creature, 100 for projectile */
}

int m11_add_teleporter(M11_TeleporterPitState* s, int x, int y, int destX, int destY, int destLevel, int destFacing, int visible) {
    if (!s || s->teleporterCount >= M11_MAX_TELEPORTERS) return 0;
    M11_TeleporterDef* t = &s->teleporters[s->teleporterCount++];
    t->x = x; t->y = y; t->destX = destX; t->destY = destY;
    t->destLevel = destLevel; t->destFacing = destFacing;
    t->isVisible = visible; t->soundEffect = 0; t->absoluteRotation = 0;
    return 1;
}

int m11_add_pit(M11_TeleporterPitState* s, int x, int y, int open, int damage, int sourceLevel, int destLevel, int destX, int destY) {
    if (!s || s->pitCount >= M11_MAX_PITS) return 0;
    M11_PitDef* p = &s->pits[s->pitCount++];
    p->x = x; p->y = y; p->isOpen = open; p->damageOnFall = damage;
    p->level = sourceLevel;
    p->destLevel = destLevel; p->destX = destX; p->destY = destY;
    return 1;
}

int m11_check_teleporter(const M11_TeleporterPitState* s, int x, int y, M11_TeleporterDef* out) {
    if (!s || !out) return 0;
    for (int i = 0; i < s->teleporterCount; i++) {
        if (s->teleporters[i].x == x && s->teleporters[i].y == y) {
            *out = s->teleporters[i]; return 1;
        }
    }
    return 0;
}

int m11_check_pit(const M11_TeleporterPitState* s, int x, int y, int currentLevel, M11_PitDef* out) {
    if (!s || !out) return 0;
    for (int i = 0; i < s->pitCount; i++) {
        if (s->pits[i].x == x && s->pits[i].y == y && s->pits[i].level == currentLevel) {
            *out = s->pits[i]; return 1;
        }
    }
    return 0;
}

static int m11_normalize_direction_or_cell(int value) {
    return value & 3;
}

int m11_apply_teleporter_rotation(int thingKind,
                                   int sourceMapX,
                                   const M11_TeleporterDef* teleporter,
                                   int inDirection,
                                   int inCell,
                                   int* outDirection,
                                   int* outCell) {
    int rotation;

    if (!teleporter || !outDirection || !outCell) return 0;

    rotation = m11_normalize_direction_or_cell(teleporter->destFacing);
    *outDirection = m11_normalize_direction_or_cell(inDirection);
    *outCell = m11_normalize_direction_or_cell(inCell);

    switch (thingKind) {
    case M11_TELEPORTER_ROTATE_THING_PARTY:
        if (teleporter->absoluteRotation) {
            *outDirection = rotation;
        } else {
            *outDirection = m11_normalize_direction_or_cell(*outDirection + rotation);
        }
        return 1;
    case M11_TELEPORTER_ROTATE_THING_PROJECTILE:
        if (teleporter->absoluteRotation) {
            *outDirection = rotation;
        } else {
            *outDirection = m11_normalize_direction_or_cell(*outDirection + rotation);
            *outCell = m11_normalize_direction_or_cell(*outCell + rotation);
        }
        return 1;
    case M11_TELEPORTER_ROTATE_THING_OBJECT:
        if (!teleporter->absoluteRotation &&
            sourceMapX != M11_MAPX_PROJECTILE_ASSOCIATED_OBJECT) {
            *outCell = m11_normalize_direction_or_cell(*outCell + rotation);
        }
        return 1;
    default:
        return 0;
    }
}

const char* m11_teleporter_rotation_source_evidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: MOVESENS.C:120-133 F0263 projectile teleporter rotation; MOVESENS.C:316-322 F0267 source-map sentinel contract; MOVESENS.C:493-518 party absolute/relative teleporter rotation; MOVESENS.C:526-531 projectile/object teleporter rotation and projectile-associated object exception";
}

int m11_resolve_pit_chain(const M11_TeleporterPitState* s, int startX, int startY,
                           int startLevel, int levitating,
                           int* finalX, int* finalY, int* finalLevel, int* totalDamage) {
    /* ReDMCSB F0267: pit fall chains — party falls through consecutive open pits.
     * F0264_MOVE_IsLevitating gate: levitating entities (flying creatures,
     * projectiles) do not fall into pits (ReDMCSB MOVESENS.C ~line 493-500). */
    if (levitating) return 0;

    if (!s || !finalX || !finalY || !finalLevel || !totalDamage) return 0;
    int cx = startX, cy = startY, currentLevel = startLevel, lastLevel = startLevel, fallen = 0;
    *totalDamage = 0;
    for (int i = 0; i < s->chainDepthLimit; i++) {
        M11_PitDef pit;
        if (!m11_check_pit(s, cx, cy, currentLevel, &pit) || !pit.isOpen) break;
        *totalDamage += pit.damageOnFall;
        currentLevel = pit.destLevel;
        lastLevel = pit.destLevel;
        cx = pit.destX; cy = pit.destY;
        fallen++;
    }
    *finalX = cx; *finalY = cy; *finalLevel = lastLevel;
    return fallen;
}


int m11_plan_group_move_removal_after_pit_teleporter(
        int fallKilledGroup,
        int creatureAllowedOnDestinationMap,
        int sourceMapX,
        int sourceMapY,
        int destinationMapX,
        int destinationMapY,
        M11_GroupMoveRemovalPlan* outPlan) {
    M11_GroupMoveRemovalPlan plan;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.reason = M11_GROUP_MOVE_REMOVAL_REASON_NONE;
    plan.dropGroupPossessionsSoundMode = -1;
    plan.dropMapX = destinationMapX;
    plan.dropMapY = destinationMapY;
    plan.deleteMapX = sourceMapX;
    plan.deleteMapY = sourceMapY;

    if (!fallKilledGroup && creatureAllowedOnDestinationMap) {
        *outPlan = plan;
        return 1;
    }

    /* ReDMCSB MOVESENS.C F0267 lines 656-663: after pit/teleporter
     * resolution, a group killed by fall damage or disallowed on the
     * destination map drops moving fixed possessions, drops group possessions
     * at the destination with C02_MODE_PLAY_ONE_TICK_LATER, deletes the source
     * group only when it came from a square, and reports move prevented. */
    plan.movePrevented = 1;
    plan.reason = fallKilledGroup
        ? M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED
        : M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED;
    plan.dropMovingCreatureFixedPossessions = 1;
    plan.dropGroupPossessions = 1;
    plan.dropGroupPossessionsSoundMode = M11_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER;
    plan.deleteSourceGroup = (sourceMapX >= 0);

    *outPlan = plan;
    return 1;
}

const char* m11_group_move_removal_source_evidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source: "
           "MOVESENS.C F0267 lines 608-624 damages falling groups and drops moving fixed possessions on partial death; "
           "MOVESENS.C F0267 lines 656-663 handles fall-killed or destination-map-disallowed group removal via F0187/F0188/F0189; "
           "GROUP.C F0187 lines 648-674 drops moving creature fixed possessions; "
           "GROUP.C F0188 lines 676-737 drops group possessions with caller sound mode; "
           "GROUP.C F0189 lines 739-762 deletes the group from the source square";
}
