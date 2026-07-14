#ifndef FIRESTAFF_DM1_V2_STARTUP_TITLE_FILTER_HANDOFF_PC34_H
#define FIRESTAFF_DM1_V2_STARTUP_TITLE_FILTER_HANDOFF_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int handled;
    int filters_active;
    int source_palette_preserved;
    int source_timing_consumed;
} DM1_V2_StartupTitleFilterHandoffReceiptPc34;

/* Admit the V2.0 post-filter lane for one source-authoritative DM1 title
 * command. The title renderer retains the original indexed pixels, special
 * palette selection, and VBlank-derived timing; this only controls the
 * presentation copy after those source facts have been consumed. */
int dm1_v2_startup_title_filter_handoff_pc34(
    const char *game_id,
    int presentation_mode,
    int source_timing_consumed,
    int special_palette_valid,
    DM1_V2_StartupTitleFilterHandoffReceiptPc34 *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_STARTUP_TITLE_FILTER_HANDOFF_PC34_H */
