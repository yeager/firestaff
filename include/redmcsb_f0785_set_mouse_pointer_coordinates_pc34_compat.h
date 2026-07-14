/*
 * ReDMCSB IO.C F0785_SetMousePointerCoordinates, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0785_SET_MOUSE_POINTER_COORDINATES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0785_SET_MOUSE_POINTER_COORDINATES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The original routine delegates coordinate delivery to IODRV_12. */
typedef void (*redmcsb_f0785_set_mouse_pointer_coordinates_callback_pc34_compat)(
    int16_t x,
    int16_t y);

typedef struct redmcsb_f0785_io_driver_pc34_compat {
    redmcsb_f0785_set_mouse_pointer_coordinates_callback_pc34_compat
        set_mouse_pointer_coordinates;
} redmcsb_f0785_io_driver_pc34_compat;

void redmcsb_f0785_set_mouse_pointer_coordinates_pc34_compat(
    const redmcsb_f0785_io_driver_pc34_compat *io_driver,
    int16_t x,
    int16_t y);

const char *redmcsb_f0785_set_mouse_pointer_coordinates_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
