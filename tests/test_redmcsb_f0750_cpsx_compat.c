#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0750_cpsx_compat.h"

typedef struct {
    char calls[32];
    size_t count;
    int exit_status;
    int copy_protection_polls_remaining;
    const char *animation;
} capture;

static void record(capture *capture_state, char call)
{
    capture_state->calls[capture_state->count++] = call;
    capture_state->calls[capture_state->count] = '\0';
}

static void pause_music(void *context) { record(context, 'P'); }
static void release_ems(void *context) { record(context, 'E'); }
static void free_memory(void *context) { record(context, 'F'); }
static void enable_screen(void *context) { record(context, 'U'); }
static void restore_towns(void *context) { record(context, 'R'); }
static void ending_interrupt(void *context, const char *filename)
{
    capture *capture_state = context;
    record(capture_state, 'I');
    capture_state->animation = filename;
}
static void animation(void *context, const char *filename)
{
    capture *capture_state = context;
    record(capture_state, 'A');
    capture_state->animation = filename;
}
static void terminate(void *context, int status)
{
    capture *capture_state = context;
    record(capture_state, 'X');
    capture_state->exit_status = status;
}
static void won(void *context) { record(context, 'W'); }
static void not_won(void *context) { record(context, 'N'); }
static void wait_blit(void *context) { record(context, 'B'); }
static void close_display(void *context) { record(context, 'D'); }
static void deinitialize_input(void *context) { record(context, 'Q'); }
static void free_amiga(void *context) { record(context, 'G'); }
static bool copy_protection_reading(void *context)
{
    capture *capture_state = context;
    record(capture_state, 'C');
    if (capture_state->copy_protection_polls_remaining == 0) {
        return false;
    }
    capture_state->copy_protection_polls_remaining--;
    return true;
}

static bool expect(const capture *capture_state, const char *calls)
{
    return strcmp(capture_state->calls, calls) == 0;
}

int main(void)
{
    capture capture_state = {{0}, 0, -1, 0, NULL};
    redmcsb_f0750_startup2_services_compat startup2 = {
        pause_music, release_ems, free_memory, enable_screen, animation,
        restore_towns, ending_interrupt, terminate, &capture_state};
    redmcsb_f0750_amiga_services_compat amiga = {
        won, not_won, wait_blit, copy_protection_reading, close_display,
        deinitialize_input, free_amiga, &capture_state};

    redmcsb_f0750_shutdown_startup2_compat(
        REDMCSB_F0750_STARTUP2_DOS, true, &startup2);
    if (!expect(&capture_state, "PEFX") || capture_state.exit_status != 1) {
        return 1;
    }

    capture_state = (capture){{0}, 0, -1, 0, NULL};
    redmcsb_f0750_shutdown_startup2_compat(
        REDMCSB_F0750_STARTUP2_FMTOWNS, true, &startup2);
    if (!expect(&capture_state, "PFUARX") ||
        capture_state.exit_status != 0 ||
        strcmp(capture_state.animation, "ending.anm") != 0) {
        return 1;
    }

    capture_state = (capture){{0}, 0, -1, 0, NULL};
    redmcsb_f0750_shutdown_startup2_compat(
        REDMCSB_F0750_STARTUP2_PC98, true, &startup2);
    if (!expect(&capture_state, "PFIX") ||
        capture_state.exit_status != 1 ||
        strcmp(capture_state.animation, "anim ending.anm") != 0) {
        return 1;
    }

    capture_state = (capture){{0}, 0, -1, 2, NULL};
    redmcsb_f0750_shutdown_amiga_compat(true, true, &amiga);
    if (!expect(&capture_state, "WBCCCDQG")) {
        return 1;
    }

    capture_state = (capture){{0}, 0, -1, 0, NULL};
    redmcsb_f0750_shutdown_amiga_compat(false, false, &amiga);
    if (!expect(&capture_state, "NBDQG") ||
        strstr(redmcsb_f0750_cpsx_source_evidence(), "ENDGAME.C:1020") == NULL) {
        return 1;
    }

    puts("ok: ReDMCSB F0750 CPSX target shutdown routes");
    return 0;
}
