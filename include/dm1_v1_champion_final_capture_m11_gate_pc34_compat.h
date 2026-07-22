#ifndef FIRESTAFF_DM1_V1_CHAMPION_FINAL_CAPTURE_M11_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_FINAL_CAPTURE_M11_GATE_PC34_COMPAT_H

#include "dm1_v1_champion_final_capture_evidence_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The portrait is selected by the live champion path, so it is separately
 * configured alongside the fixed PC34 C008/C028/C033-C035 material set. */
typedef struct Dm1V1ChampionFinalCaptureM11GateConfigPc34 {
    Dm1V1ChampionFinalCaptureOriginalConfigPc34 originals;
    const uint8_t *portraitPixels;
} Dm1V1ChampionFinalCaptureM11GateConfigPc34;

typedef struct Dm1V1ChampionFinalCaptureM11GateStatePc34 {
    unsigned int lastTick;
    unsigned int lastGeneration;
} Dm1V1ChampionFinalCaptureM11GateStatePc34;

typedef struct Dm1V1ChampionFinalCaptureM11GateReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    int commandCount;
    Dm1V1ChampionRuntimeSourceM11CommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionFinalCaptureM11GateReceiptPc34;

void dm1_v1_champion_final_capture_m11_gate_init_pc34(
    Dm1V1ChampionFinalCaptureM11GateStatePc34 *state);

/* Final configured-original admission boundary before a future M11 capture
 * consumer. It does not invoke M11. Only exact C008/C028/C033-C035, portrait,
 * palette and logical-surface evidence can publish; stale or incomplete proof
 * produces material-free clears for the affected zones. */
int dm1_v1_champion_final_capture_m11_gate_pc34(
    Dm1V1ChampionFinalCaptureM11GateStatePc34 *state,
    const Dm1V1ChampionFinalCaptureEvidenceReceiptPc34 *finalEvidence,
    const Dm1V1ChampionFinalCaptureM11GateConfigPc34 *config,
    Dm1V1ChampionFinalCaptureM11GateReceiptPc34 *outReceipt);

const char *dm1_v1_champion_final_capture_m11_gate_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
