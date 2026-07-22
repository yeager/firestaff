#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_FRAME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_FRAME_PC34_COMPAT_H

#include <stdint.h>

#include "memory_champion_state_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB CHAMDRAW.C F0287/F0291/F0292/F0293 live top-row frame plan.
 * The caller supplies only GRAPHICS.DAT surfaces retained by the PC34
 * loader. A missing required surface rejects the complete frame; this API
 * intentionally has no host-art or procedural substitute. */

#define DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34 4
#define DM1_V1_CHAMPION_TOP_ROW_STAT_COUNT_PC34 3
#define DM1_V1_CHAMPION_TOP_ROW_HAND_COUNT_PC34 2

typedef struct Dm1V1ChampionTopRowSurfacePc34 {
    int graphicIndex;
    int loaded;
    const uint8_t *pixels;
    int width;
    int height;
} Dm1V1ChampionTopRowSurfacePc34;

typedef struct Dm1V1ChampionTopRowAssetsPc34 {
    Dm1V1ChampionTopRowSurfacePc34 deadStatusBox;
    Dm1V1ChampionTopRowSurfacePc34 championIcons;
    Dm1V1ChampionTopRowSurfacePc34 slotNormal;
    Dm1V1ChampionTopRowSurfacePc34 slotWounded;
    Dm1V1ChampionTopRowSurfacePc34 slotActing;
} Dm1V1ChampionTopRowAssetsPc34;

typedef struct Dm1V1ChampionTopRowStatPc34 {
    int zoneId;
    int x;
    int y;
    int width;
    int height;
    int current;
    int maximum;
    int blankHeight;
    int fillHeight;
    int blankColor;
    int fillColor;
} Dm1V1ChampionTopRowStatPc34;

typedef struct Dm1V1ChampionTopRowHandPc34 {
    int zoneId;
    int x;
    int y;
    int width;
    int height;
    int graphicIndex;
} Dm1V1ChampionTopRowHandPc34;

typedef struct Dm1V1ChampionTopRowSlotPc34 {
    int present;
    int alive;
    int statusBoxZoneId;
    int statusX;
    int statusY;
    int statusWidth;
    int statusHeight;
    int nameClearZoneId;
    int nameTextZoneId;
    int nameColor;
    int iconSourceX;
    int iconFillColor;
    Dm1V1ChampionTopRowStatPc34 stats[DM1_V1_CHAMPION_TOP_ROW_STAT_COUNT_PC34];
    Dm1V1ChampionTopRowHandPc34 hands[DM1_V1_CHAMPION_TOP_ROW_HAND_COUNT_PC34];
} Dm1V1ChampionTopRowSlotPc34;

typedef struct Dm1V1ChampionTopRowFramePc34 {
    int valid;
    int partyChampionCount;
    int activeChampionIndex;
    int actingChampionOrdinal;
    int invisibilityCount;
    Dm1V1ChampionTopRowSlotPc34 slots[DM1_V1_CHAMPION_TOP_ROW_SLOT_COUNT_PC34];
} Dm1V1ChampionTopRowFramePc34;

/* Builds the exact PC34 top-row plan from the live party state. The caller
 * must retain C008/C028/C033/C034/C035 source surfaces; unused C008 is not a
 * precondition when every present champion is alive. */
int dm1_v1_champion_top_row_frame_from_party_pc34(
    const struct PartyState_Compat *party,
    int actingChampionOrdinal,
    int invisibilityCount,
    const Dm1V1ChampionTopRowAssetsPc34 *assets,
    Dm1V1ChampionTopRowFramePc34 *outFrame);

const char *dm1_v1_champion_top_row_frame_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
