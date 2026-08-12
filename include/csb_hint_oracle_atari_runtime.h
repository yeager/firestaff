/*
 * Complete source-owned Atari ST R1 Hint Oracle handoff.
 *
 * This owns the three original Utility Disk/save inputs named by ReDMCSB
 * HINTLOAD.C: HCSB.HTC, HCSB.DAT and CSBGAME/MINI.DAT.  The admitted pair is
 * deliberately the documented R1 set shared by Atari ST 2.0/2.1: HTC
 * 8ce69b54cf255a15e98e909bb45b9742 and DAT
 * 708e113c869ab922633e885aa72a3c77.  Other language/release pairs remain
 * separate until their matching graphic pairing is source-verified.
 */
#ifndef FIRESTAFF_CSB_HINT_ORACLE_ATARI_RUNTIME_H
#define FIRESTAFF_CSB_HINT_ORACLE_ATARI_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

#include "csb_hint_oracle_atari_save_session.h"
#include "csb_hint_oracle_graphics_surface.h"
#include "csb_hint_oracle_ui_panel.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_HINT_ORACLE_ATARI_R1_HTC_MD5 "8ce69b54cf255a15e98e909bb45b9742"
#define CSB_HINT_ORACLE_ATARI_R1_DAT_MD5 "708e113c869ab922633e885aa72a3c77"

typedef enum {
    CSB_HINT_ORACLE_ATARI_RUNTIME_OK = 0,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT = -1,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_HTC = -2,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_GRAPHICS = -3,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY = -4,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SAVE = -5,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SESSION = -6,
    CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER = -7
} CSB_HintOracleAtariRuntime_Result;

typedef struct {
    CSB_HintOracleUIPanel htc_panel;
    CSB_HintOracleGraphicsSurface graphics;
    CSB_HintOracleSession session;
    int assets_loaded;
} CSB_HintOracleAtariRuntime;

void csb_hint_oracle_atari_runtime_init(CSB_HintOracleAtariRuntime *runtime);
void csb_hint_oracle_atari_runtime_free(CSB_HintOracleAtariRuntime *runtime);

/* Hash-admit the matching R1 HCSB.HTC/HCSB.DAT pair. `cache_dir` is used only
 * if an HTC source lives in a virtual container; ordinary original files are
 * read in place. */
int csb_hint_oracle_atari_runtime_load_assets(
    CSB_HintOracleAtariRuntime *runtime,
    const char *data_dir, const char *cache_dir, int max_depth);

/* Feed only a successfully decoded native Atari MINI.DAT receipt. */
int csb_hint_oracle_atari_runtime_select_save(
    CSB_HintOracleAtariRuntime *runtime,
    const CSB_V1_AtariSaveInfo *info);

int csb_hint_oracle_atari_runtime_open_hint_row(
    CSB_HintOracleAtariRuntime *runtime, size_t row);
int csb_hint_oracle_atari_runtime_previous_page(
    CSB_HintOracleAtariRuntime *runtime);
int csb_hint_oracle_atari_runtime_next_page(
    CSB_HintOracleAtariRuntime *runtime);
int csb_hint_oracle_atari_runtime_done(CSB_HintOracleAtariRuntime *runtime);

/* Render the current original 320x200 R1 page. No host frame, font or palette
 * fallback is generated when the session is not on a valid page. */
int csb_hint_oracle_atari_runtime_render_page(
    const CSB_HintOracleAtariRuntime *runtime,
    uint8_t *frame, size_t frame_size);

const char *csb_hint_oracle_atari_runtime_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_ATARI_RUNTIME_H */
