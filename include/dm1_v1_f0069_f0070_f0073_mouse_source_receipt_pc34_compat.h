/*
 * ReDMCSB MOUSESET.C F0069 and IO.C F0070/F0073 PC-34 input receipt.
 *
 * F0073's saved-screen cursor compositor is compiled for MEDIA007 only;
 * I34E/I34M use the I/O driver pointer path instead.  This contract keeps
 * that boundary explicit: it never creates a software cursor buffer for PC.
 */
#ifndef FIRESTAFF_DM1_V1_F0069_F0070_F0073_MOUSE_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0069_F0070_F0073_MOUSE_SOURCE_RECEIPT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

enum {
    DM1_V1_F0070_C028_GRAPHIC_PC34 = 28,
    DM1_V1_F0070_C028_WIDTH_PC34 = 76,
    DM1_V1_F0070_C028_HEIGHT_PC34 = 14,
    DM1_V1_F0070_CHAMPION_ICON_COUNT_PC34 = 4
};

typedef enum DM1_V1_F0069PointerKindPc34 {
    DM1_V1_F0069_POINTER_NONE_PC34 = 0,
    DM1_V1_F0069_POINTER_ARROW_PC34 = 1,
    DM1_V1_F0069_POINTER_HAND_PC34 = 2,
    DM1_V1_F0069_POINTER_OBJECT_PC34 = 3,
    DM1_V1_F0069_POINTER_CHAMPION_PC34 = 4
} DM1_V1_F0069PointerKindPc34;

typedef struct DM1_V1_F0069F0070SourceMaterialPc34 {
    int graphicsDatAuthenticated;
    int graphicIndex;
    int width;
    int height;
    const uint8_t *indexedPixels;
    size_t indexedPixelCount;
    uint32_t indexedPixelsFnv1a;
} DM1_V1_F0069F0070SourceMaterialPc34;

typedef struct DM1_V1_F0069F0070RuntimeInputPc34 {
    DM1_V1_F0069F0070SourceMaterialPc34 championIcons;
    int leaderEmptyHanded;
    int leaderIndex;
    const uint8_t *leaderObjectPointerPixels;
    size_t leaderObjectPointerPixelCount;
    uint32_t leaderObjectPointerFnv1a;
    int partyDirection;
    int championIndexByCell[DM1_V1_F0070_CHAMPION_ICON_COUNT_PC34];
    int targetChampionIconIndex;
    unsigned int heldChampionIconOrdinal;
} DM1_V1_F0069F0070RuntimeInputPc34;

typedef struct DM1_V1_F0069F0070F0073ReceiptPc34 {
    int valid;
    int suppressSyntheticUi;
    DM1_V1_F0069PointerKindPc34 f0069Pointer;
    int f0070TargetIconIndex;
    int f0070ChampionIndex;
    int f0070PickedUp;
    int f0070Dropped;
    unsigned int nextHeldChampionIconOrdinal;
    uint32_t c028MaterialFnv1a;
    int f0073Pc34ScreenAreaUnavailable;
    int f0073SyntheticScreenAreaSuppressed;
    const char *sourceEvidence;
} DM1_V1_F0069F0070F0073ReceiptPc34;

uint32_t dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
    const uint8_t *bytes, size_t byteCount);

/* Returns a receipt only for authenticated original C028 material. */
int dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(
    const DM1_V1_F0069F0070RuntimeInputPc34 *input,
    DM1_V1_F0069F0070F0073ReceiptPc34 *outReceipt);

const char *dm1_v1_f0069_f0070_f0073_source_evidence_pc34(void);

#endif
