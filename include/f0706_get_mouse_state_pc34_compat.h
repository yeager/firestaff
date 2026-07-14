#ifndef FIRESTAFF_F0706_GET_MOUSE_STATE_PC34_COMPAT_H
#define FIRESTAFF_F0706_GET_MOUSE_STATE_PC34_COMPAT_H

/*
 * ReDMCSB PC 3.4 compatibility: IO.C F0706_GetMouseState.
 *
 * The I34E/I34M branch is deliberately only an I/O-driver dispatch:
 *
 *   (*(G2161_IODriver->IODRV_13_GetMouseState))(x, y, buttons);
 *
 * Keep the driver-owned coordinates and button word opaque.  In
 * particular, F0706 neither scales nor normalizes them before returning
 * them to CLICKVIEW.C, DRAWVIEW.C, PANEL.C, or selector code.  A missing
 * host driver is a no-op in this compatibility boundary; an installed driver
 * still receives the original pointers without alteration.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ReDMCSB_F0706_GetMouseStateDriverPc34)(
    int16_t *outX,
    int16_t *outY,
    int16_t *outButtons);

typedef struct {
    ReDMCSB_F0706_GetMouseStateDriverPc34 getMouseState;
} ReDMCSB_F0706_IODriverPc34;

/* ReDMCSB: IO.C F0706_GetMouseState, I34E/I34M branch, line ~3746. */
void ReDMCSB_F0706_GetMouseStatePc34Compat(
    const ReDMCSB_F0706_IODriverPc34 *ioDriver,
    int16_t *outX,
    int16_t *outY,
    int16_t *outButtons);

#ifdef __cplusplus
}
#endif

#endif
