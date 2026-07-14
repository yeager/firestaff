/*
 * ReDMCSB IO.C F0783_LockMouse, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0783_LOCK_MOUSE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0783_LOCK_MOUSE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The original route does not implement mouse locking itself. It delegates
 * directly to IODRV_05_LockMouse supplied by the active I/O driver.
 */
typedef void (*redmcsb_f0783_lock_mouse_callback_pc34_compat)(void);

typedef struct redmcsb_f0783_io_driver_pc34_compat {
    redmcsb_f0783_lock_mouse_callback_pc34_compat lock_mouse;
} redmcsb_f0783_io_driver_pc34_compat;

void redmcsb_f0783_lock_mouse_pc34_compat(
    const redmcsb_f0783_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0783_lock_mouse_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
