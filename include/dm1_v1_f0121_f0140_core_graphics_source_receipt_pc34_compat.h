/* ReDMCSB DUNVIEW.C F0121-F0128 and video helpers F0129-F0140.
 *
 * This gate deliberately makes no pixels.  It admits the early dungeon-view
 * dispatch only when the caller carries authenticated raw PC34 game data and
 * the original 224x136/112-byte viewport contract.  Missing evidence is a
 * no-draw result, never a generated wall, cursor, palette, or UI surface.
 */
#ifndef FIRESTAFF_DM1_V1_F0121_F0140_CORE_GRAPHICS_SOURCE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0121_F0140_CORE_GRAPHICS_SOURCE_RECEIPT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

enum {
    DM1_V1_F0121_F0140_VIEWPORT_WIDTH_PC34 = 224,
    DM1_V1_F0121_F0140_VIEWPORT_HEIGHT_PC34 = 136,
    DM1_V1_F0121_F0140_VIEWPORT_BYTE_WIDTH_PC34 = 112,
    DM1_V1_F0121_F0140_SQUARE_COUNT_PC34 = 7
};

typedef enum DM1_V1_F0121F0140SquarePc34 {
    DM1_V1_F0121_F0140_D2C_PC34 = 0,
    DM1_V1_F0121_F0140_D1L_PC34 = 1,
    DM1_V1_F0121_F0140_D1R_PC34 = 2,
    DM1_V1_F0121_F0140_D1C_PC34 = 3,
    DM1_V1_F0121_F0140_D0L_PC34 = 4,
    DM1_V1_F0121_F0140_D0R_PC34 = 5,
    DM1_V1_F0121_F0140_D0C_PC34 = 6
} DM1_V1_F0121F0140SquarePc34;

typedef struct DM1_V1_F0121F0140RawSourcePc34 {
    int authenticated;
    const uint8_t *bytes;
    size_t byteCount;
    uint32_t fnv1a;
} DM1_V1_F0121F0140RawSourcePc34;

typedef struct DM1_V1_F0121F0140RuntimeInputPc34 {
    DM1_V1_F0121F0140RawSourcePc34 graphicsDat;
    DM1_V1_F0121F0140RawSourcePc34 dungeonDat;
    DM1_V1_F0121F0140RawSourcePc34 paletteChanges;
    DM1_V1_F0121F0140RawSourcePc34 sourceSurface;
    DM1_V1_F0121F0140RawSourcePc34 destinationSurface;
    int viewportOwned;
    int viewportWidth;
    int viewportHeight;
    int viewportByteWidth;
    int square;
    int sourceWidth;
    int sourceHeight;
    int destinationWidth;
    int destinationHeight;
    int transparentColor;
    int flipHorizontal;
    int flipVertical;
} DM1_V1_F0121F0140RuntimeInputPc34;

typedef struct DM1_V1_F0121F0140ReceiptPc34 {
    int valid;
    int suppressSyntheticPresentation;
    int f0121ToF0127SquareAdmitted;
    int f0128ViewportOwnerAdmitted;
    int f0129ShrinkPaletteAdmitted;
    int f0130HorizontalFlipAdmitted;
    int f0131VerticalFlipAdmitted;
    int f0132BlitAdmitted;
    int f0133MaskedBlitAdmitted;
    int f0136HatchAdmitted;
    int f0137CpsfUnavailableOnPc34;
    int f0139DelegatedToExistingOwner;
    int f0140DelegatedToExistingOwner;
    uint32_t materialFingerprint;
    const char *sourceEvidence;
} DM1_V1_F0121F0140ReceiptPc34;

uint32_t dm1_v1_f0121_f0140_fnv1a_pc34(const uint8_t *bytes, size_t byteCount);
int dm1_v1_f0121_f0140_core_graphics_source_receipt_pc34(
    const DM1_V1_F0121F0140RuntimeInputPc34 *input,
    DM1_V1_F0121F0140ReceiptPc34 *outReceipt);
const char *dm1_v1_f0121_f0140_source_evidence_pc34(void);

#endif
