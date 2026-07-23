#include "dm1_v1_f0069_f0076_mouse_input_bundle_pc34_compat.h"

#include <string.h>

static int surface_valid(const DM1_V1_MouseSourceSurfacePc34 *surface)
{
    size_t required;
    if (!surface || !surface->sourceOwned || surface->width <= 0 ||
        surface->height <= 0 || !surface->pixels) return 0;
    required = (size_t)surface->width * (size_t)surface->height;
    return surface->pixelCount == required &&
           dm1_v1_f0069_f0070_f0073_fnv1a_pc34(surface->pixels, required) != 0U &&
           dm1_v1_f0069_f0070_f0073_fnv1a_pc34(surface->pixels, required) ==
               surface->pixelsFnv1a;
}

static int mutable_surface_valid(const DM1_V1_MouseMutableSurfacePc34 *surface)
{
    size_t required;
    if (!surface || !surface->sourceOwned || surface->width <= 0 ||
        surface->height <= 0 || !surface->pixels) return 0;
    required = (size_t)surface->width * (size_t)surface->height;
    return surface->pixelCount == required;
}

static int clamp_coordinate(int value, int maximum)
{
    if (value < 0) return 0;
    if (value > maximum) return maximum;
    return value;
}

static int champion_icon_at(int x, int y)
{
    static const int left[4] = {281, 301, 301, 281};
    static const int top[4] = {0, 0, 15, 15};
    int i;
    for (i = 0; i < 4; ++i) {
        if (x >= left[i] && x < left[i] + DM1_V1_MOUSE_CHAMPION_ICON_WIDTH_PC34 &&
            y >= top[i] && y < top[i] + DM1_V1_MOUSE_CHAMPION_ICON_HEIGHT_PC34) {
            return i;
        }
    }
    return -1;
}

void dm1_v1_f0069_f0076_mouse_state_init_pc34(
    DM1_V1_MousePointerStatePc34 *state, int pointerX, int pointerY)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->pointerX = clamp_coordinate(pointerX, DM1_V1_MOUSE_SOURCE_WIDTH_PC34 - 1);
    state->pointerY = clamp_coordinate(pointerY, DM1_V1_MOUSE_SOURCE_HEIGHT_PC34 - 1);
}

int F0073_MOUSE_BuildPointerScreenArea(
    DM1_V1_MousePointerStatePc34 *state,
    const DM1_V1_MouseSourceSurfacePc34 *screen,
    int pointerWidth, int pointerHeight, int hotspotX, int hotspotY)
{
    int sourceX;
    int sourceY;
    int x;
    int y;
    size_t dst = 0U;

    if (!state || !surface_valid(screen) || pointerWidth <= 0 || pointerHeight <= 0 ||
        pointerWidth > DM1_V1_MOUSE_SCREEN_AREA_MAX_WIDTH_PC34 ||
        pointerHeight > DM1_V1_MOUSE_SCREEN_AREA_MAX_HEIGHT_PC34 ||
        hotspotX < 0 || hotspotX >= pointerWidth || hotspotY < 0 || hotspotY >= pointerHeight) {
        return 0;
    }
    sourceX = state->pointerX - hotspotX;
    sourceY = state->pointerY - hotspotY;
    state->savedLeft = sourceX < 0 ? 0 : sourceX;
    state->savedTop = sourceY < 0 ? 0 : sourceY;
    state->savedWidth = pointerWidth - (state->savedLeft - sourceX);
    state->savedHeight = pointerHeight - (state->savedTop - sourceY);
    if (state->savedLeft + state->savedWidth > screen->width) {
        state->savedWidth = screen->width - state->savedLeft;
    }
    if (state->savedTop + state->savedHeight > screen->height) {
        state->savedHeight = screen->height - state->savedTop;
    }
    if (state->savedWidth <= 0 || state->savedHeight <= 0) return 0;
    for (y = 0; y < state->savedHeight; ++y) {
        for (x = 0; x < state->savedWidth; ++x) {
            state->savedPixels[dst++] = screen->pixels[
                (size_t)(state->savedTop + y) * (size_t)screen->width +
                (size_t)(state->savedLeft + x)];
        }
    }
    state->savedPixelCount = dst;
    state->savedPixelsFnv1a = dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
        state->savedPixels, state->savedPixelCount);
    state->savedAreaValid = state->savedPixelsFnv1a != 0U;
    return state->savedAreaValid;
}

int S0072_MOUSE_DrawPointerHiddenScreenArea(
    DM1_V1_MousePointerStatePc34 *state,
    DM1_V1_MouseMutableSurfacePc34 *screen)
{
    int x;
    int y;
    size_t src = 0U;
    size_t required;

    if (!state || !mutable_surface_valid(screen) || !state->savedAreaValid ||
        state->savedLeft < 0 || state->savedTop < 0 || state->savedWidth <= 0 ||
        state->savedHeight <= 0 || state->savedLeft + state->savedWidth > screen->width ||
        state->savedTop + state->savedHeight > screen->height) return 0;
    required = (size_t)state->savedWidth * (size_t)state->savedHeight;
    if (state->savedPixelCount != required ||
        dm1_v1_f0069_f0070_f0073_fnv1a_pc34(state->savedPixels, required) !=
            state->savedPixelsFnv1a) return 0;
    for (y = 0; y < state->savedHeight; ++y) {
        for (x = 0; x < state->savedWidth; ++x) {
            screen->pixels[(size_t)(state->savedTop + y) * (size_t)screen->width +
                           (size_t)(state->savedLeft + x)] = state->savedPixels[src++];
        }
    }
    state->pointerVisible = 0;
    state->savedAreaValid = 0;
    return 1;
}

int S0074_MOUSE_DrawPointerScreenArea(
    DM1_V1_MousePointerStatePc34 *state,
    DM1_V1_MouseMutableSurfacePc34 *screen,
    const DM1_V1_MouseSourceSurfacePc34 *pointer,
    int hotspotX, int hotspotY, uint8_t transparentIndex)
{
    DM1_V1_MouseSourceSurfacePc34 sourceScreen;
    int sourceX;
    int sourceY;
    int x;
    int y;

    if (!state || !mutable_surface_valid(screen) || !surface_valid(pointer) ||
        pointer->width > DM1_V1_MOUSE_SCREEN_AREA_MAX_WIDTH_PC34 ||
        pointer->height > DM1_V1_MOUSE_SCREEN_AREA_MAX_HEIGHT_PC34 ||
        hotspotX < 0 || hotspotX >= pointer->width || hotspotY < 0 || hotspotY >= pointer->height) {
        return 0;
    }
    if (state->pointerVisible && !S0072_MOUSE_DrawPointerHiddenScreenArea(state, screen)) {
        return 0;
    }
    sourceScreen.sourceOwned = 1;
    sourceScreen.width = screen->width;
    sourceScreen.height = screen->height;
    sourceScreen.pixels = screen->pixels;
    sourceScreen.pixelCount = screen->pixelCount;
    sourceScreen.pixelsFnv1a = dm1_v1_f0069_f0070_f0073_fnv1a_pc34(
        screen->pixels, screen->pixelCount);
    if (!F0073_MOUSE_BuildPointerScreenArea(state, &sourceScreen, pointer->width,
                                             pointer->height, hotspotX, hotspotY)) return 0;
    sourceX = state->pointerX - hotspotX;
    sourceY = state->pointerY - hotspotY;
    for (y = 0; y < pointer->height; ++y) {
        for (x = 0; x < pointer->width; ++x) {
            int targetX = sourceX + x;
            int targetY = sourceY + y;
            uint8_t pixel = pointer->pixels[(size_t)y * (size_t)pointer->width + (size_t)x];
            if (targetX >= 0 && targetX < screen->width && targetY >= 0 &&
                targetY < screen->height && pixel != transparentIndex) {
                screen->pixels[(size_t)targetY * (size_t)screen->width + (size_t)targetX] = pixel;
            }
        }
    }
    state->pointerVisible = 1;
    return 1;
}

int S0075_MOUSE_Exception70Handler_IKBD_MIDI_MouseStatus(
    DM1_V1_MousePointerStatePc34 *state,
    const DM1_V1_MousePacketPc34 *packet)
{
    int oldX;
    int oldY;
    if (!state || !packet) return 0;
    oldX = state->pointerX;
    oldY = state->pointerY;
    state->pointerX = clamp_coordinate(state->pointerX + packet->deltaX,
                                       DM1_V1_MOUSE_SOURCE_WIDTH_PC34 - 1);
    state->pointerY = clamp_coordinate(state->pointerY + packet->deltaY,
                                       DM1_V1_MOUSE_SOURCE_HEIGHT_PC34 - 1);
    return oldX != state->pointerX || oldY != state->pointerY;
}

int S0076_MOUSE_OnMouseButtonsStatusChange(
    DM1_V1_MousePointerStatePc34 *state,
    const DM1_V1_F0069F0070RuntimeInputPc34 *runtimeInput,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    uint16_t nextButtonStatus,
    DM1_V1_F0069F0076DispatchReceiptPc34 *outReceipt)
{
    uint16_t oldStatus;
    uint16_t changed;
    int championIcon;
    struct Dm1V1InputEventPc34Compat event;

    if (!state || !queue || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->sourceEvidence = dm1_v1_f0069_f0076_mouse_input_source_evidence_pc34();
    oldStatus = state->buttonStatus;
    changed = (uint16_t)(oldStatus ^ nextButtonStatus);
    state->buttonStatus = nextButtonStatus;
    if (changed == 0U) return 0;
    outReceipt->sampled = 1;
    outReceipt->buttonTransition = 1;
    outReceipt->coordinateOrderValid = state->pointerX >= 0 &&
        state->pointerX < DM1_V1_MOUSE_SOURCE_WIDTH_PC34 && state->pointerY >= 0 &&
        state->pointerY < DM1_V1_MOUSE_SOURCE_HEIGHT_PC34;
    if (!outReceipt->coordinateOrderValid) return 0;

    memset(&event, 0, sizeof(event));
    event.kind = DM1_V1_INPUT_KIND_MOUSE;
    event.x = state->pointerX;
    event.y = state->pointerY;
    if ((changed & DM1_V1_BUTTON_LEFT) && (nextButtonStatus & DM1_V1_BUTTON_LEFT)) {
        championIcon = champion_icon_at(event.x, event.y);
        if (championIcon >= 0) {
            DM1_V1_F0069F0070RuntimeInputPc34 transaction;
            if (!runtimeInput) return 0;
            transaction = *runtimeInput;
            transaction.targetChampionIconIndex = championIcon;
            if (!dm1_v1_f0069_f0070_f0073_mouse_runtime_receipt_pc34(
                    &transaction, &outReceipt->f0069F0070Receipt)) return 0;
            outReceipt->f0069Pointer = outReceipt->f0069F0070Receipt.f0069Pointer;
            outReceipt->f0070ChampionCommand = DM1_V1_COMMAND_CHAMPION_ICON_TOP_LEFT + championIcon;
            event.buttonMask = DM1_V1_BUTTON_LEFT;
            if (!DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(queue, event)) return 0;
            outReceipt->queued = 1;
            outReceipt->valid = 1;
            return 1;
        }
        event.buttonMask = DM1_V1_BUTTON_LEFT;
    } else if ((changed & DM1_V1_BUTTON_RIGHT) && (nextButtonStatus & DM1_V1_BUTTON_RIGHT)) {
        event.buttonMask = DM1_V1_BUTTON_RIGHT;
    } else if ((changed & DM1_V1_BUTTON_LEFT) && !(nextButtonStatus & DM1_V1_BUTTON_LEFT)) {
        event.buttonMask = DM1_V1_BUTTON_LEFT_UP;
    } else if ((changed & DM1_V1_BUTTON_RIGHT) && !(nextButtonStatus & DM1_V1_BUTTON_RIGHT)) {
        event.buttonMask = DM1_V1_BUTTON_RIGHT_UP;
    } else {
        return 0;
    }
    if (!DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(queue, event)) return 0;
    outReceipt->queued = 1;
    outReceipt->valid = 1;
    return 1;
}

const char *dm1_v1_f0069_f0076_mouse_input_source_evidence_pc34(void)
{
    return "ReDMCSB MOUSESET.C:5-16 F0069 pointer ownership; IO.C:2395-2647 "
           "F0070 champion icon command 125-128 transaction; IO.C:2741 F0073 "
           "captures pointer screen coordinates before draw; BASE.C:828 S0072 "
           "restores hidden pointer area; BASE.C:914 S0074 saves/draws pointer area; "
           "IO.C:2908 S0075 samples status before IO.C:3194 S0076 button-edge dispatch. "
           "COMMAND.C F0359 remains the sole command-table and queue owner.";
}
