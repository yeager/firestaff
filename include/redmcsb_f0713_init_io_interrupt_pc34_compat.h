/* ReDMCSB IO.C F0713_InitIOInterrupt, PC 3.4 I34E/I34M route. */
#ifndef FIRESTAFF_REDMCSB_F0713_INIT_IO_INTERRUPT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0713_INIT_IO_INTERRUPT_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f0713_vertical_blank_routine_pc34_compat)(void *context);
typedef void *(*redmcsb_f0713_get_vector_pc34_compat)(void *context,
                                                       uint8_t interrupt_number);
typedef void *(*redmcsb_f0713_set_vblank_pc34_compat)(
    void *context,
    void *io_driver,
    redmcsb_f0713_vertical_blank_routine_pc34_compat routine,
    void *routine_context);
typedef uint16_t (*redmcsb_f0713_get_data_segment_pc34_compat)(void *context);

typedef struct {
    redmcsb_f0713_get_vector_pc34_compat get_vector;
    redmcsb_f0713_set_vblank_pc34_compat set_vertical_blank;
    redmcsb_f0713_get_data_segment_pc34_compat get_data_segment;
    redmcsb_f0713_vertical_blank_routine_pc34_compat vertical_blank_routine;
    void *context;
    void *vertical_blank_context;
    void *io_driver_primary;
    void *io_driver_secondary;
    void *previous_vertical_blank_routine;
    uint16_t data_segment_backup;
} redmcsb_f0713_state_pc34_compat;

/* Runs IO.C:3893-3900 in source order, using a portable host-vector bridge. */
bool redmcsb_f0713_init_io_interrupt_pc34_compat(
    redmcsb_f0713_state_pc34_compat *state);

const char *redmcsb_f0713_init_io_interrupt_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
