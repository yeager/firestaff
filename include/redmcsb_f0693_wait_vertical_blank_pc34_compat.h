#ifndef FIRESTAFF_REDMCSB_F0693_WAIT_VERTICAL_BLANK_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0693_WAIT_VERTICAL_BLANK_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB VBLANK.C F0693_WaitVerticalBlank (lines 626-645).
 *
 * F0693 sets G3976_B_ and waits until the installed VBlank task clears it.
 * This adapter keeps that gate caller-owned: the callback represents the
 * host VBlank delivery and must call F0693_VerticalBlankCallback_PC34().
 * It deliberately provides no timer, sleep, or synthetic frame cadence.
 */
typedef void (*ReDMCSBF0693VBlankCallbackPc34Compat)(void *context);

typedef struct {
    volatile bool waiting_for_vertical_blank;
    ReDMCSBF0693VBlankCallbackPc34Compat deliver_vertical_blank;
    void *context;
} ReDMCSBF0693WaitVerticalBlankPc34Compat;

/* Delivers the source task's G3976_B_ = C0_FALSE transition. */
void F0693_VerticalBlankCallback_PC34(
    ReDMCSBF0693WaitVerticalBlankPc34Compat *gate);

/* Sets the F0693 gate and waits for the configured VBlank callback to clear
 * it. Returns false without changing the gate when no callback is available.
 */
bool F0693_WaitVerticalBlank_PC34(
    ReDMCSBF0693WaitVerticalBlankPc34Compat *gate);

#ifdef __cplusplus
}
#endif

#endif
