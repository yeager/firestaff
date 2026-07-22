#ifndef FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_PRESENTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_TOP_ROW_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_champion_top_row_assets_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CHAMPION_TOP_ROW_PRESENTATION_MAX_OPS_PC34 64

typedef enum Dm1V1ChampionTopRowPresentationOpKindPc34 {
    DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_STATUS_PC34 = 1,
    DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_DEAD_STATUS_PC34,
    DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_NAME_PC34,
    DM1_V1_CHAMPION_TOP_ROW_OP_DRAW_NAME_PC34,
    DM1_V1_CHAMPION_TOP_ROW_OP_COMPOSE_ICON_PC34,
    DM1_V1_CHAMPION_TOP_ROW_OP_CLEAR_BAR_PC34,
    DM1_V1_CHAMPION_TOP_ROW_OP_FILL_BAR_PC34,
    DM1_V1_CHAMPION_TOP_ROW_OP_BLIT_HAND_PC34
} Dm1V1ChampionTopRowPresentationOpKindPc34;

typedef struct Dm1V1ChampionTopRowPresentationOpPc34 {
    Dm1V1ChampionTopRowPresentationOpKindPc34 kind;
    int championSlot;
    int zoneId;
    int graphicIndex;
    const uint8_t *sourcePixels;
    int sourceX;
    int sourceY;
    int sourceWidth;
    int sourceHeight;
    int x;
    int y;
    int width;
    int height;
    int color;
} Dm1V1ChampionTopRowPresentationOpPc34;

typedef struct Dm1V1ChampionTopRowPresentationReceiptPc34 {
    int valid;
    int operationCount;
    Dm1V1ChampionTopRowAssetsReceiptPc34 assetReceipt;
    Dm1V1ChampionTopRowPresentationOpPc34
        operations[DM1_V1_CHAMPION_TOP_ROW_PRESENTATION_MAX_OPS_PC34];
} Dm1V1ChampionTopRowPresentationReceiptPc34;

/* Converts an admitted live PC34 top-row frame into ordered, source-bound
 * presentation operations. This is deliberately renderer-neutral: M11 can
 * consume the receipt later, but no fallback pixels are emitted here. */
int dm1_v1_champion_top_row_presentation_from_frame_pc34(
    const Dm1V1ChampionTopRowFramePc34 *frame,
    const Dm1V1ChampionTopRowAssetsReceiptPc34 *assetReceipt,
    Dm1V1ChampionTopRowPresentationReceiptPc34 *outReceipt);

const char *dm1_v1_champion_top_row_presentation_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
