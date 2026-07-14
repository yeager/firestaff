/* ReDMCSB IO.C F0784_UnlockMouse, PC 3.4 I34E/I34M route. */
#ifndef FIRESTAFF_REDMCSB_F0784_UNLOCK_MOUSE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0784_UNLOCK_MOUSE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Portable boundary for G2161_IODriver->IODRV_06_UnlockMouse. */
typedef void (*redmcsb_f0784_unlock_mouse_callback_pc34_compat)(void *context);

typedef struct {
    redmcsb_f0784_unlock_mouse_callback_pc34_compat unlock_mouse;
    void *context;
} redmcsb_f0784_io_driver_pc34_compat;

/*
 * Executes the sole PC 3.4 F0784 action:
 * G2161_IODriver->IODRV_06_UnlockMouse().
 *
 * The original has no state mutation, balancing counter, or input emulation;
 * the supplied driver callback owns the platform-specific mouse operation.
 */
void redmcsb_f0784_unlock_mouse_pc34_compat(
    const redmcsb_f0784_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0784_unlock_mouse_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
