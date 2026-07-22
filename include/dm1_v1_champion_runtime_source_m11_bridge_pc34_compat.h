#ifndef FIRESTAFF_DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_BRIDGE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_champion_runtime_source_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 {
    unsigned int lastTick;
    unsigned int lastGeneration;
} Dm1V1ChampionRuntimeSourceM11BridgeStatePc34;

typedef enum Dm1V1ChampionRuntimeSourceM11CommandKindPc34 {
    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_CLEAR_PC34 = 1,
    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C008_PC34,
    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_C028_PC34,
    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_STATUS_BAR_PC34,
    DM1_V1_CHAMPION_RUNTIME_SOURCE_M11_HAND_PC34
} Dm1V1ChampionRuntimeSourceM11CommandKindPc34;

typedef struct Dm1V1ChampionRuntimeSourceM11CommandPc34 {
    Dm1V1ChampionRuntimeSourceM11CommandKindPc34 kind;
    int championIndex;
    int zoneId;
    int graphicIndex;
    const uint8_t *originalPixels;
    const uint8_t *portraitPixels;
    const uint8_t *originalPalette;
    uint8_t *originalSurface;
} Dm1V1ChampionRuntimeSourceM11CommandPc34;

typedef struct Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    int commandCount;
    Dm1V1ChampionRuntimeSourceM11CommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34;

void dm1_v1_champion_runtime_source_m11_bridge_init_pc34(
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 *state);

/* Converts a fully proven runtime source receipt into a later M11-consumable
 * command receipt without invoking M11. A stale receipt emits only relevant
 * clears for portrait/status/bar/hand zones and carries no source material. */
int dm1_v1_champion_runtime_source_m11_bridge_pc34(
    Dm1V1ChampionRuntimeSourceM11BridgeStatePc34 *state,
    const Dm1V1ChampionRuntimeSourceReceiptPc34 *runtimeSource,
    Dm1V1ChampionRuntimeSourceM11BridgeReceiptPc34 *outReceipt);

const char *dm1_v1_champion_runtime_source_m11_bridge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
