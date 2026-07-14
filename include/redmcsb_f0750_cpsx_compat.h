/*
 * ReDMCSB F0750_CPSX shutdown routes.
 *
 * Source-locked to STARTUP2.C and AMIGINIT.C.  The original routine has
 * separate bodies per target; this adapter exposes the PC 3.4 and Amiga
 * bodies without pretending that their host services are interchangeable.
 */
#ifndef FIRESTAFF_REDMCSB_F0750_CPSX_COMPAT_H
#define FIRESTAFF_REDMCSB_F0750_CPSX_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    REDMCSB_F0750_STARTUP2_DOS,
    REDMCSB_F0750_STARTUP2_FMTOWNS,
    REDMCSB_F0750_STARTUP2_PC98
} redmcsb_f0750_startup2_target_compat;

typedef void (*redmcsb_f0750_void_callback_compat)(void *context);
typedef void (*redmcsb_f0750_play_animation_callback_compat)(
    void *context,
    const char *filename);
typedef void (*redmcsb_f0750_terminate_callback_compat)(
    void *context,
    int status);
typedef void (*redmcsb_f0750_install_ending_callback_compat)(
    void *context,
    const char *animation_filename);

typedef struct {
    redmcsb_f0750_void_callback_compat pause_music;
    redmcsb_f0750_void_callback_compat release_ems_handle;
    redmcsb_f0750_void_callback_compat free_base_memory;
    redmcsb_f0750_void_callback_compat enable_screen_update;
    redmcsb_f0750_play_animation_callback_compat play_animation;
    redmcsb_f0750_void_callback_compat restore_towns;
    redmcsb_f0750_install_ending_callback_compat install_ending_interrupt;
    redmcsb_f0750_terminate_callback_compat terminate_process;
    void *context;
} redmcsb_f0750_startup2_services_compat;

/*
 * Executes STARTUP2.C:293-332 for one source target. terminate_process is
 * the host boundary for _exit()/exit(); the adapter returns after invoking
 * it so tests and hosts need not terminate their own process.
 */
void redmcsb_f0750_shutdown_startup2_compat(
    redmcsb_f0750_startup2_target_compat target,
    bool game_won,
    const redmcsb_f0750_startup2_services_compat *services);

typedef bool (*redmcsb_f0750_is_copy_protection_reading_callback_compat)(
    void *context);

typedef struct {
    redmcsb_f0750_void_callback_compat finish_game_won;
    redmcsb_f0750_void_callback_compat finish_game_not_won;
    redmcsb_f0750_void_callback_compat wait_blit;
    redmcsb_f0750_is_copy_protection_reading_callback_compat
        is_copy_protection_reading;
    redmcsb_f0750_void_callback_compat close_display;
    redmcsb_f0750_void_callback_compat deinitialize_input;
    redmcsb_f0750_void_callback_compat free_amiga_stuff;
    void *context;
} redmcsb_f0750_amiga_services_compat;

/* Executes AMIGINIT.C:673-690. */
void redmcsb_f0750_shutdown_amiga_compat(
    bool game_won,
    bool copy_protection_enabled,
    const redmcsb_f0750_amiga_services_compat *services);

const char *redmcsb_f0750_cpsx_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
