/* DM1 V1 Teleporter/Pit — ReDMCSB MOVESENS.C F0276, F0267.
 * Generated via Q3.6, fixed by Opus (added x,y to structs, fixed pit chain). */
#include "dm1_v1_teleporter_pit_pc34_compat.h"
#include <string.h>

void m11_teleporter_pit_init(M11_TeleporterPitState* s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->chainDepthLimit = 10;
}

int m11_add_teleporter(M11_TeleporterPitState* s, int x, int y, int destX, int destY, int destLevel, int destFacing, int visible) {
    if (!s || s->teleporterCount >= M11_MAX_TELEPORTERS) return 0;
    M11_TeleporterDef* t = &s->teleporters[s->teleporterCount++];
    t->x = x; t->y = y; t->destX = destX; t->destY = destY;
    t->destLevel = destLevel; t->destFacing = destFacing;
    t->isVisible = visible; t->soundEffect = 0;
    return 1;
}

int m11_add_pit(M11_TeleporterPitState* s, int x, int y, int open, int damage, int destLevel, int destX, int destY) {
    if (!s || s->pitCount >= M11_MAX_PITS) return 0;
    M11_PitDef* p = &s->pits[s->pitCount++];
    p->x = x; p->y = y; p->isOpen = open; p->damageOnFall = damage;
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

int m11_check_pit(const M11_TeleporterPitState* s, int x, int y, M11_PitDef* out) {
    if (!s || !out) return 0;
    for (int i = 0; i < s->pitCount; i++) {
        if (s->pits[i].x == x && s->pits[i].y == y) {
            *out = s->pits[i]; return 1;
        }
    }
    return 0;
}

int m11_resolve_pit_chain(const M11_TeleporterPitState* s, int startX, int startY,
                           int* finalX, int* finalY, int* finalLevel, int* totalDamage) {
    /* ReDMCSB F0267: pit fall chains — party falls through consecutive open pits */
    if (!s || !finalX || !finalY || !finalLevel || !totalDamage) return 0;
    int cx = startX, cy = startY, lastLevel = 0, fallen = 0;
    *totalDamage = 0;
    for (int i = 0; i < s->chainDepthLimit; i++) {
        M11_PitDef pit;
        if (!m11_check_pit(s, cx, cy, &pit) || !pit.isOpen) break;
        *totalDamage += pit.damageOnFall;
        lastLevel = pit.destLevel;
        cx = pit.destX; cy = pit.destY;
        fallen++;
    }
    *finalX = cx; *finalY = cy; *finalLevel = lastLevel;
    return fallen;
}
