#ifndef FIRESTAFF_REDMCSB_F0539_INPUT_CCONIS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0539_INPUT_CCONIS_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB IO2.C F0539_INPUT_Cconis, PC 3.4-compatible I34E/I34M path.
 *
 * The original routine performs no buffering or consumption. It delegates
 * exactly once to IO_DRIVER::IODRV_01_IsKeyboardInputPresent and returns the
 * driver's Boolean result. This adapter keeps the driver boundary explicit
 * for the modern input host.
 */
typedef bool (*RedmcsbF0539KeyboardInputPresentPc34Compat)(void *context);

/*
 * Returns the status reported by the bound non-blocking keyboard driver.
 * A missing host binding is reported as no input; a bound driver is invoked
 * once and is solely responsible for preserving its input queue.
 */
bool redmcsb_f0539_input_cconis_pc34_compat(
    RedmcsbF0539KeyboardInputPresentPc34Compat input_present,
    void *context);

const char *redmcsb_f0539_input_cconis_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0539_INPUT_CCONIS_PC34_COMPAT_H */
