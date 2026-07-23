/*
 * ReDMCSB MOUSESET.C / IO.C pointer input bundle for PC-34.
 *
 * This is deliberately separate from the host window layer.  The bundle
 * consumes caller-owned source pixels and logical 320x200 coordinates, then
 * passes commands through the existing COMMAND.C queue owner.
 */
#ifndef FIRESTAFF_DM1_V1_F0069_F0076_MOUSE_INPUT_BUNDLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0069_F0076_MOUSE_INPUT_BUNDLE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_f0069_f0070_f0073_mouse_source_receipt_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MOUSE_SOURCE_WIDTH_PC34 = 320,
    DM1_V1_MOUSE_SOURCE_HEIGHT_PC34 = 200,
    DM1_V1_MOUSE_SCREEN_AREA_MAX_WIDTH_PC34 = 32,
    DM1_V1_MOUSE_SCREEN_AREA_MAX_HEIGHT_PC34 = 32,
    DM1_V1_MOUSE_CHAMPION_ICON_WIDTH_PC34 = 19,
    DM1_V1_MOUSE_CHAMPION_ICON_HEIGHT_PC34 = 14
};

typedef struct DM1_V1_MouseSourceSurfacePc34 {
    int sourceOwned;
    int width;
    int height;
    const uint8_t *pixels;
    size_t pixelCount;
    uint32_t pixelsFnv1a;
} DM1_V1_MouseSourceSurfacePc34;

typedef struct DM1_V1_MouseMutableSurfacePc34 {
    int sourceOwned;
    int width;
    int height;
    uint8_t *pixels;
    size_t pixelCount;
} DM1_V1_MouseMutableSurfacePc34;

typedef struct DM1_V1_MousePointerStatePc34 {
    int pointerX;
    int pointerY;
    uint16_t buttonStatus;
    int pointerVisible;
    int savedLeft;
    int savedTop;
    int savedWidth;
    int savedHeight;
    uint8_t savedPixels[DM1_V1_MOUSE_SCREEN_AREA_MAX_WIDTH_PC34 *
                        DM1_V1_MOUSE_SCREEN_AREA_MAX_HEIGHT_PC34];
    size_t savedPixelCount;
    uint32_t savedPixelsFnv1a;
    int savedAreaValid;
} DM1_V1_MousePointerStatePc34;

typedef struct DM1_V1_MousePacketPc34 {
    int deltaX;
    int deltaY;
    uint16_t buttonStatus;
} DM1_V1_MousePacketPc34;

typedef struct DM1_V1_F0069F0076DispatchReceiptPc34 {
    int valid;
    int sampled;
    int coordinateOrderValid;
    int f0070ChampionCommand;
    int queued;
    int buttonTransition;
    int restoredPreviousPointerArea;
    int drewPointerArea;
    DM1_V1_F0069PointerKindPc34 f0069Pointer;
    DM1_V1_F0069F0070F0073ReceiptPc34 f0069F0070Receipt;
    const char *sourceEvidence;
} DM1_V1_F0069F0076DispatchReceiptPc34;

void dm1_v1_f0069_f0076_mouse_state_init_pc34(
    DM1_V1_MousePointerStatePc34 *state, int pointerX, int pointerY);

/* F0073: capture the source-owned pixels beneath the logical pointer. */
int F0073_MOUSE_BuildPointerScreenArea(
    DM1_V1_MousePointerStatePc34 *state,
    const DM1_V1_MouseSourceSurfacePc34 *screen,
    int pointerWidth, int pointerHeight, int hotspotX, int hotspotY);

/* S0072/S0074: restore the former area, or save and draw the next one. */
int S0072_MOUSE_DrawPointerHiddenScreenArea(
    DM1_V1_MousePointerStatePc34 *state,
    DM1_V1_MouseMutableSurfacePc34 *screen);
int S0074_MOUSE_DrawPointerScreenArea(
    DM1_V1_MousePointerStatePc34 *state,
    DM1_V1_MouseMutableSurfacePc34 *screen,
    const DM1_V1_MouseSourceSurfacePc34 *pointer,
    int hotspotX, int hotspotY, uint8_t transparentIndex);

/* S0075: caller-owned mouse packet sampling; no host polling occurs here. */
int S0075_MOUSE_Exception70Handler_IKBD_MIDI_MouseStatus(
    DM1_V1_MousePointerStatePc34 *state,
    const DM1_V1_MousePacketPc34 *packet);

/*
 * S0076 owns button-edge ordering.  Champion-icon clicks first authenticate
 * F0069/F0070 before the matching COMMAND.C entry is queued.  All other
 * clicks are admitted by the existing source command table.
 */
int S0076_MOUSE_OnMouseButtonsStatusChange(
    DM1_V1_MousePointerStatePc34 *state,
    const DM1_V1_F0069F0070RuntimeInputPc34 *runtimeInput,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    uint16_t nextButtonStatus,
    DM1_V1_F0069F0076DispatchReceiptPc34 *outReceipt);

const char *dm1_v1_f0069_f0076_mouse_input_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
