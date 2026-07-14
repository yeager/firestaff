/*
 * ReDMCSB IO.C F0700_TriggerImmediateMouseEvent, PC 3.4 (I34E/I34M).
 *
 * The original does not manufacture a mouse position or call the game mouse
 * handler directly.  It asks the installed I/O driver to bind an impossible
 * rectangle, which makes the driver report a change-screen-region event.
 */
#ifndef FIRESTAFF_REDMCSB_F0700_TRIGGER_IMMEDIATE_MOUSE_EVENT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0700_TRIGGER_IMMEDIATE_MOUSE_EVENT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0700_MOUSE_EVENT_CHANGE_SCREEN_REGION_PC34_COMPAT = 32
};

typedef void (*redmcsb_f0700_set_mouse_bound_event_pc34_compat)(
    void *context,
    const int16_t box[4],
    int16_t mouse_event);

typedef struct {
    redmcsb_f0700_set_mouse_bound_event_pc34_compat set_mouse_bound_event;
    void *context;
} redmcsb_f0700_io_driver_pc34_compat;

/*
 * Returns false only when the caller has not supplied the PC 3.4 I/O-driver
 * callback.  On success it issues the one source call, with G2050's exact
 * impossible-box coordinates: left=1, right=0, top=1, bottom=0.
 */
bool redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat(
    const redmcsb_f0700_io_driver_pc34_compat *io_driver);

const char *redmcsb_f0700_trigger_immediate_mouse_event_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
