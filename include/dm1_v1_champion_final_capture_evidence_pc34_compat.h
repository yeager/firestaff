#ifndef FIRESTAFF_DM1_V1_CHAMPION_FINAL_CAPTURE_EVIDENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_FINAL_CAPTURE_EVIDENCE_PC34_COMPAT_H

#include "dm1_v1_champion_live_m11_capture_lifecycle_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionFinalCaptureOriginalConfigPc34 {
    const uint8_t *c008Pixels;
    int c008Width;
    int c008Height;
    const uint8_t *c028Pixels;
    int c028Width;
    int c028Height;
    const uint8_t *c033Pixels;
    const uint8_t *c034Pixels;
    const uint8_t *c035Pixels;
    int handWidth;
    int handHeight;
    const uint8_t *indexedPalette;
    int indexedPaletteEntryCount;
    uint8_t *indexedSurface;
} Dm1V1ChampionFinalCaptureOriginalConfigPc34;

typedef struct Dm1V1ChampionFinalCaptureEvidenceStatePc34 {
    unsigned int lastTick;
    unsigned int lastGeneration;
} Dm1V1ChampionFinalCaptureEvidenceStatePc34;

typedef struct Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    int evidenceCount;
    Dm1V1ChampionRuntimeSourceM11CommandPc34
        evidence[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionFinalCaptureEvidenceReceiptPc34;

void dm1_v1_champion_final_capture_evidence_init_pc34(
    Dm1V1ChampionFinalCaptureEvidenceStatePc34 *state);

/* Final source-only capture gate. Only configured original C008/C028/C033-
 * C035/palette/surface pointers are admitted; stale or unproven frames become
 * clear-only capture evidence with no material pointers. */
int dm1_v1_champion_final_capture_evidence_pc34(
    Dm1V1ChampionFinalCaptureEvidenceStatePc34 *state,
    const Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *lifecycle,
    const Dm1V1ChampionFinalCaptureOriginalConfigPc34 *originalConfig,
    Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *outReceipt);

const char *dm1_v1_champion_final_capture_evidence_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
