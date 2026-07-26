#ifndef DM1_V1_HOC_PRESENTED_FRAME_CONSUMER_PC34_COMPAT_H
#define DM1_V1_HOC_PRESENTED_FRAME_CONSUMER_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Final HoC host-frame admission.  M11 supplies facts only after its real
 * GRAPHICS.DAT blits complete; this DM1 boundary checks the independent
 * C127/C026, M648 and F0115 lanes together before a frame is publishable. */
typedef struct {
    int valid;
    int backingGraphicIndex;
    int portraitGraphicIndex;
    int backingWidth;
    int backingHeight;
    int portraitWidth;
    int portraitHeight;
    int backingTransparentColor;
    int portraitTransparentColor;
    uint32_t backingHash;
    uint32_t portraitHash;
} DM1_V1_HocPresentedMirrorMaterialPc34;

typedef struct {
    int valid;
    int fontGraphicIndex;
    int glyphByteCount;
    int lineCount;
    int glyphSourceWidth;
    int glyphSourceHeight;
    int glyphCellCount;
    int opaqueGlyphPixelCount;
    int transparentGlyphPixelCount;
    uint32_t textDataHash;
    uint32_t glyphBytesHash;
    uint32_t fontPixelsHash;
    uint32_t sourceCellsHash;
} DM1_V1_HocPresentedInscriptionMaterialPc34;

typedef struct {
    int valid;
    int graphicsId;
    int transparentColor;
    int sourceZone;
    int destinationW;
    int destinationH;
    int assetWidth;
    int assetHeight;
    uint32_t materialHash;
} DM1_V1_HocPresentedObjectMaterialPc34;

typedef struct {
    int valid;
    int primaryGraphicId;
    int secondaryGraphicId;
    int primaryZoneId;
    int secondaryZoneId;
    int fontGraphicId;
    int requiresRealActionMenuLayout;
    int requiresRealSpellAreaLayout;
    int suppressSyntheticFallback;
    uint32_t sourceTick;
    uint32_t serial;
} DM1_V1_HocPresentedActionSpellMaterialPc34;

typedef struct {
    int valid;
    int entryCount;
    uint32_t paletteHash;
} DM1_V1_HocPresentedPaletteMaterialPc34;

typedef struct {
    int valid;
    int mirrorSquareCount;
    int materializedCount;
    int allMaterialized;
    uint32_t coverageHash;
} DM1_V1_HocPresentedViewportCoverageMaterialPc34;

typedef struct {
    uint32_t runtimeTick;
    const DM1_V1_HocPresentedMirrorMaterialPc34 *mirror;
    const DM1_V1_HocPresentedInscriptionMaterialPc34 *inscription;
    const DM1_V1_HocPresentedObjectMaterialPc34 *object;
    const DM1_V1_HocPresentedActionSpellMaterialPc34 *actionSpell;
    const DM1_V1_HocPresentedPaletteMaterialPc34 *palette;
    const DM1_V1_HocPresentedViewportCoverageMaterialPc34 *viewportCoverage;
} DM1_V1_HocPresentedFrameConsumerInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int consumedMirror;
    int consumedInscription;
    int consumedObject;
    int consumedActionSpell;
    int consumedPalette;
    int consumedViewportCoverage;
    int suppressFallbackVisuals;
    uint32_t runtimeTick;
    uint32_t frameHash;
    const char *sourceAnchor;
} DM1_V1_HocPresentedFrameConsumerReceiptPc34;

int dm1_v1_hoc_presented_frame_consumer_build_receipt_pc34(
    const DM1_V1_HocPresentedFrameConsumerInputPc34 *input,
    DM1_V1_HocPresentedFrameConsumerReceiptPc34 *outReceipt);

const char *dm1_v1_hoc_presented_frame_consumer_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
