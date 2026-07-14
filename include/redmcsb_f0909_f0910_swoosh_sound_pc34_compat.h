#ifndef FIRESTAFF_REDMCSB_F0909_F0910_SWOOSH_SOUND_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0909_F0910_SWOOSH_SOUND_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Host boundary for the Amiga audio.device requests used by SWSHSND.C.
 * Requests and command/flag representations stay host-owned.
 */
typedef void (*redmcsb_f0909_f0910_io_request_pc34_compat)(
    void *host_context,
    void *request);

typedef void (*redmcsb_f0909_f0910_set_control_command_pc34_compat)(
    void *host_context,
    void *control_request,
    uint16_t command);

typedef void (*redmcsb_f0909_f0910_update_control_flag_pc34_compat)(
    void *host_context,
    void *control_request,
    uint8_t flag);

typedef void (*redmcsb_f0909_f0910_free_memory_pc34_compat)(
    void *host_context,
    void *memory,
    long byte_count);

typedef struct {
    void *host_context;
    void *sound1_left_channel;
    void *sound1_right_channel;
    void *sound1_control;
    redmcsb_f0909_f0910_io_request_pc34_compat begin_io;
    redmcsb_f0909_f0910_io_request_pc34_compat do_io;
    redmcsb_f0909_f0910_io_request_pc34_compat wait_io;
    redmcsb_f0909_f0910_set_control_command_pc34_compat set_control_command;
    redmcsb_f0909_f0910_update_control_flag_pc34_compat set_control_flag;
    redmcsb_f0909_f0910_update_control_flag_pc34_compat clear_control_flag;
    uint16_t command_start;
    uint16_t command_finish;
    uint16_t command_stop;
    uint8_t flag_sync_cycle;
} redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat;

/* SWSHSND.C F0909 in both source configurations. */
void redmcsb_f0909_play_swoosh_sound_pc34_compat(
    const redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat *host);

/* SWSHSND.C F0910, MEDIA618_A31E configuration. */
void redmcsb_f0910_release_swoosh_sound_pc34_compat(
    const redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat *host);

/*
 * SWSHSND.C F0910, MEDIA429_A20ED_A20E_A20F_A21E_A31E configuration.
 * The caller owns the buffer-location storage; it is set to null after the
 * host release primitive, matching the original branch.
 */
void redmcsb_f0910_release_swoosh_sound_and_buffer_pc34_compat(
    const redmcsb_f0909_f0910_swoosh_sound_host_pc34_compat *host,
    redmcsb_f0909_f0910_free_memory_pc34_compat free_memory,
    void **swoosh_sound_data_location,
    long swoosh_sound_data_byte_count);

const char *redmcsb_f0909_f0910_swoosh_sound_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0909_F0910_SWOOSH_SOUND_PC34_COMPAT_H */
