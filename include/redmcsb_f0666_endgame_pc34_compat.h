#ifndef FIRESTAFF_REDMCSB_F0666_ENDGAME_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0666_ENDGAME_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB ENDGAME.C F0666_endgame, PC I34E/I34M transaction. */
typedef void (*redmcsb_f0666_step_pc34_compat)(void *context);

typedef struct redmcsb_f0666_endgame_state_pc34_compat {
    int hide_mouse_pointer_request_count;
} redmcsb_f0666_endgame_state_pc34_compat;

typedef struct redmcsb_f0666_endgame_runtime_pc34_compat {
    redmcsb_f0666_step_pc34_compat hide_pointer;
    redmcsb_f0666_step_pc34_compat close_graphics_dat;
    redmcsb_f0666_step_pc34_compat restore_cpsx;
    /* Production callback transfers through the caller-owned jump boundary. */
    redmcsb_f0666_step_pc34_compat transfer_to_endgame_boundary;
    void *context;
} redmcsb_f0666_endgame_runtime_pc34_compat;

/* Executes ENDGAME.C F0666's PC route. Transfer callback normally does not return. */
int redmcsb_f0666_endgame_pc34_compat(
    redmcsb_f0666_endgame_state_pc34_compat *state,
    const redmcsb_f0666_endgame_runtime_pc34_compat *runtime);

const char *redmcsb_f0666_endgame_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
