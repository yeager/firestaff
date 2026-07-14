#ifndef FIRESTAFF_REDMCSB_F0540_INPUT_CRAWCIN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0540_INPUT_CRAWCIN_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB IO2.C F0540_INPUT_Crawcin reads one raw PC keyboard value through
 * IODRV_00_GetKeyboardInput.  The host callback owns its wait/block policy;
 * this adapter deliberately performs exactly one read per invocation.
 */
typedef uint16_t (*ReDMCSBF0540GetKeyboardInputPc34Compat)(void *context);

typedef struct ReDMCSBF0540InputCrawcinPc34Compat {
    ReDMCSBF0540GetKeyboardInputPc34Compat get_keyboard_input;
    void *keyboard_input_context;
    bool exit_game_immediately; /* ReDMCSB G2151_ExitGameImmediately */
} ReDMCSBF0540InputCrawcinPc34Compat;

/*
 * Returns the source raw key value except for the four shifted extended-arrow
 * values normalized by IO2.C:47-59. Ctrl-Alt-Del and Ctrl-Q set the persistent
 * exit flag while retaining their raw return values, as in the source.
 * Returns zero when no IODRV callback is supplied.
 */
uint16_t redmcsb_f0540_input_crawcin_pc34_compat(
    ReDMCSBF0540InputCrawcinPc34Compat *input);

const char *redmcsb_f0540_input_crawcin_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0540_INPUT_CRAWCIN_PC34_COMPAT_H */
