/*
 * ReDMCSB TEXT.C F0050..F0054 and IO.C F0066..F0068, PC-34 source gate.
 *
 * This is deliberately a receipt, not a host text/cursor renderer.  TEXT.C
 * routes all five text helpers through the one-bit M653 interface font.  The
 * PC build also asks IODRV to register the original arrow/hand bitmaps.  A
 * caller may present either path only after supplying those raw, authenticated
 * source surfaces.  Missing material means no UI submission.
 */
#ifndef FIRESTAFF_DM1_V1_F0050_F0068_EARLY_UI_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0050_F0068_EARLY_UI_SOURCE_RECEIPT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

enum {
    DM1_V1_F0054_M653_GRAPHIC_PC34 = 695,
    DM1_V1_F0054_M653_GRAPHIC_LEGACY_PC34 = 557,
    DM1_V1_F0054_M653_BYTE_COUNT_PC34 = 768,
    DM1_V1_F0052_VIEWPORT_WIDTH_PC34 = 224,
    DM1_V1_F0052_VIEWPORT_HEIGHT_PC34 = 136,
    DM1_V1_F0053_SCREEN_WIDTH_PC34 = 320,
    DM1_V1_F0053_SCREEN_HEIGHT_PC34 = 200,
    DM1_V1_F0066_ARROW_POINTER_WIDTH_PC34 = 11,
    DM1_V1_F0066_ARROW_POINTER_HEIGHT_PC34 = 18,
    DM1_V1_F0066_HAND_POINTER_WIDTH_PC34 = 16,
    DM1_V1_F0066_HAND_POINTER_HEIGHT_PC34 = 18
};

typedef struct DM1_V1_F0050F0068RawMaterialPc34 {
    /* GRAPHICS.DAT for M653; original IO.C table provenance for G0042/G0043. */
    int sourceAuthenticated;
    int graphicIndex;
    int width;
    int height;
    const uint8_t *bytes;
    size_t byteCount;
    uint32_t fnv1a;
} DM1_V1_F0050F0068RawMaterialPc34;

typedef struct DM1_V1_F0050F0068RuntimeInputPc34 {
    DM1_V1_F0050F0068RawMaterialPc34 interfaceFont;
    DM1_V1_F0050F0068RawMaterialPc34 arrowPointer;
    DM1_V1_F0050F0068RawMaterialPc34 handPointer;
    int viewportBitmapOwned;
    int viewportWidth;
    int viewportHeight;
    int logicalScreenBitmapOwned;
    int logicalScreenWidth;
    int logicalScreenHeight;
    int ioDriverPointerRegistrationBound;
    int ioDriverMouseHandlerBound;
    int ioDriverScreenRegionBound;
} DM1_V1_F0050F0068RuntimeInputPc34;

typedef struct DM1_V1_F0050F0068ReceiptPc34 {
    int valid;
    int suppressSyntheticUi;
    int f0050SpaceUsesM653;
    int f0051LineFeedUsesM653;
    int f0052ViewportTextAdmitted;
    int f0053LogicalScreenTextAdmitted;
    int f0054InitializeFontAdmitted;
    int f0066MouseInitializeAdmitted;
    int f0067NormalPointerAdmitted;
    int f0068ObjectPointerAdmitted;
    uint32_t materialFingerprint;
    const char *sourceEvidence;
} DM1_V1_F0050F0068ReceiptPc34;

uint32_t dm1_v1_f0050_f0068_fnv1a_pc34(const uint8_t *bytes, size_t byteCount);

/* Produces no draw commands.  Callers must consume this before forwarding
 * TEXT.C or IODRV work to the production renderer/input backend. */
int dm1_v1_f0050_f0068_early_ui_source_receipt_pc34(
    const DM1_V1_F0050F0068RuntimeInputPc34 *input,
    DM1_V1_F0050F0068ReceiptPc34 *outReceipt);

const char *dm1_v1_f0050_f0068_source_evidence_pc34(void);

#endif
