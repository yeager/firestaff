/*
 * test_m11_champion_scent_pc34_compat.c
 *
 * Source-locked to ReDMCSB CHAMPION.C F0316 / F0317.
 * MOV-06 (DM1 V1 functional-divergence-report.md):
 *   "F0316 / F0317 scent add/delete are exercised through M11,
 *    not the new compat layer."
 *
 * The compat-layer stub here mirrors F0316/F0317 on a bounded
 * 16-slot ring so V2 presentation overlays can read
 * M11_ChampionScentRing_Compat.scents[] without reaching into
 * the amalgam G0407_s_Party globals.
 *
 * Pins:
 *  - Init: count=0, all strengths=0
 *  - F0317 add: appends new entry if (mapX,mapY) absent and
 *    ring has room; strength starts at min(cycleCount, 255)
 *  - F0317 add: bumps strength if (mapX,mapY) present,
 *    capped at 255
 *  - F0316 delete: shifts tail down, decrements count
 *  - F0316 delete: returns 0 on out-of-range scentIndex
 *  - Ring full: F0317 add returns 0 (drops new scent, does
 *    NOT overwrite an existing entry)
 */

#include "m11_champion_scent_pc34_compat.h"

#include <stddef.h>
#include <string.h>

void m11_champion_scent_ring_init(M11_ChampionScentRing_Compat* ring) {
    if (!ring) return;
    memset(ring, 0, sizeof(*ring));
}

unsigned char m11_champion_scent_max_uc(unsigned char a, unsigned char b) {
    return (a > b) ? a : b;
}

/* Find scent index for (mapX,mapY), -1 if absent. */
static int find_scent(const M11_ChampionScentRing_Compat* ring,
                      unsigned short mapX, unsigned short mapY) {
    int i;
    if (!ring) return -1;
    for (i = 0; i < (int)ring->count; ++i) {
        if (ring->scents[i].mapX == mapX && ring->scents[i].mapY == mapY) {
            return i;
        }
    }
    return -1;
}

int m11_champion_scent_ring_add(
    M11_ChampionScentRing_Compat* ring,
    unsigned short mapX, unsigned short mapY,
    unsigned short cycleCount)
{
    int idx;
    unsigned char newStrength;
    if (!ring) return 0;
    if (cycleCount > 255) cycleCount = 255;

    idx = find_scent(ring, mapX, mapY);
    if (idx >= 0) {
        /* F0317 inner: bump strength, capped at 255. */
        newStrength = (unsigned char)cycleCount;
        if (ring->scents[idx].strength < newStrength) {
            ring->scents[idx].strength = newStrength;
        } else {
            /* Already at least as strong; keep. */
        }
        return 1;
    }
    if (ring->count >= FS_MAX_SCENTS) {
        /* Ring full; drop new scent (preserves existing data). */
        return 0;
    }
    ring->scents[ring->count].mapX = mapX;
    ring->scents[ring->count].mapY = mapY;
    ring->scents[ring->count].strength = (unsigned char)cycleCount;
    ring->count++;
    return 1;
}

int m11_champion_scent_ring_delete(
    M11_ChampionScentRing_Compat* ring, int scentIndex)
{
    if (!ring) return 0;
    if (scentIndex < 0 || scentIndex >= (int)ring->count) return 0;
    /* F0316: shift tail down one slot. */
    if (scentIndex < (int)ring->count - 1) {
        size_t tailBytes = sizeof(M11_ChampionScent_Compat) *
                           (size_t)((int)ring->count - 1 - scentIndex);
        memmove(&ring->scents[scentIndex],
                &ring->scents[scentIndex + 1],
                tailBytes);
    }
    ring->count--;
    /* Clear the now-unused tail slot so stale data doesn't leak. */
    memset(&ring->scents[ring->count], 0, sizeof(M11_ChampionScent_Compat));
    return 1;
}

/* ── Regression test ─────────────────────────────────────────── */

#include <stdio.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    M11_ChampionScentRing_Compat ring;
    int i;

    /* T1: init leaves ring empty. */
    m11_champion_scent_ring_init(&ring);
    CHECK(ring.count == 0, "T1: count==0 after init");
    for (i = 0; i < FS_MAX_SCENTS; ++i) {
        CHECK(ring.scents[i].strength == 0, "T1: strength 0 after init");
    }

    /* T2: F0317 add appends new entry. */
    CHECK(m11_champion_scent_ring_add(&ring, 10, 20, 5) == 1, "T2: add returns 1");
    CHECK(ring.count == 1, "T2: count==1 after add");
    CHECK(ring.scents[0].mapX == 10, "T2: mapX==10");
    CHECK(ring.scents[0].mapY == 20, "T2: mapY==20");
    CHECK(ring.scents[0].strength == 5, "T2: strength==5");

    /* T3: F0317 add at same (mapX,mapY) bumps strength. */
    CHECK(m11_champion_scent_ring_add(&ring, 10, 20, 12) == 1, "T3: add returns 1");
    CHECK(ring.count == 1, "T3: still 1 entry (no duplicate)");
    CHECK(ring.scents[0].strength == 12, "T3: strength bumped to 12");

    /* T4: F0317 add at different (mapX,mapY) appends. */
    CHECK(m11_champion_scent_ring_add(&ring, 11, 21, 3) == 1, "T4: add returns 1");
    CHECK(ring.count == 2, "T4: count==2");

    /* T5: F0316 delete middle entry shifts tail. */
    CHECK(m11_champion_scent_ring_delete(&ring, 0) == 1, "T5: delete returns 1");
    CHECK(ring.count == 1, "T5: count==1 after delete");
    /* The remaining entry is the one that was at index 1. */
    CHECK(ring.scents[0].mapX == 11, "T5: tail shifted, mapX==11");
    CHECK(ring.scents[0].mapY == 21, "T5: tail shifted, mapY==21");
    CHECK(ring.scents[0].strength == 3, "T5: tail shifted, strength==3");

    /* T6: F0316 delete out-of-range -> 0. */
    CHECK(m11_champion_scent_ring_delete(&ring, 5) == 0, "T6: out-of-range delete");
    CHECK(ring.count == 1, "T6: count unchanged");

    /* T7: F0316 delete last entry leaves count==0. */
    CHECK(m11_champion_scent_ring_delete(&ring, 0) == 1, "T7: delete last");
    CHECK(ring.count == 0, "T7: count==0");

    /* T8: Ring-full: F0317 add drops new scent. */
    for (i = 0; i < FS_MAX_SCENTS; ++i) {
        CHECK(m11_champion_scent_ring_add(&ring, (unsigned short)i, 0, 1) == 1,
              "T8: fill ring");
    }
    CHECK(ring.count == FS_MAX_SCENTS, "T8: ring full");
    /* Try adding a new (mapX,mapY) that doesn't exist. */
    CHECK(m11_champion_scent_ring_add(&ring, 99, 99, 1) == 0,
          "T8: ring full -> drop new scent");
    CHECK(ring.count == FS_MAX_SCENTS, "T8: count unchanged at FS_MAX_SCENTS");

    /* T9: Ring-full with existing (mapX,mapY) DOES bump. */
    CHECK(m11_champion_scent_ring_add(&ring, 5, 0, 50) == 1,
          "T9: existing (mapX,mapY) bumps even when full");
    CHECK(ring.scents[5].strength == 50, "T9: strength updated to 50");

    /* T10: cycleCount > 255 is clamped. */
    m11_champion_scent_ring_init(&ring);
    CHECK(m11_champion_scent_ring_add(&ring, 1, 2, 300) == 1, "T10: clamp cycleCount");
    CHECK(ring.scents[0].strength == 255, "T10: strength capped at 255");

    /* T11: F0025-style max helper. */
    CHECK(m11_champion_scent_max_uc(5, 3) == 5, "T11: max(5,3)==5");
    CHECK(m11_champion_scent_max_uc(3, 5) == 5, "T11: max(3,5)==5");
    CHECK(m11_champion_scent_max_uc(7, 7) == 7, "T11: max(7,7)==7");

    printf("PASS: M11 champion scent F0316/F0317 compat stub\n");
    return 0;
}
