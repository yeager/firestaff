/*
 * Source-owned Atari ST R1 and Amiga AU2 Hint Oracle handoff.
 *
 * This owns the three original Utility Disk/save inputs named by ReDMCSB
 * HINTLOAD.C: HCSB.HTC, HCSB.DAT and CSBGAME/MINI.DAT.  The admitted pair is
 * deliberately the documented R1 set shared by Atari ST 2.0/2.1: HTC
 * 8ce69b54cf255a15e98e909bb45b9742 and DAT
 * 708e113c869ab922633e885aa72a3c77. Amiga AU2 R2/R3 English pairs are
 * admitted separately using their source-verified HTC hashes and shared
 * original English HCSB.DAT; other language/release pairs remain unadmitted.
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
#define CSB_HINT_ORACLE_AMIGA_AU2_R2_HTC_MD5 "334fc18cb98d1280a4c55a16566d5ef9"
#define CSB_HINT_ORACLE_AMIGA_AU2_R3_HTC_MD5 "c06862298f193b1fe479eaeff6acd57e"
#define CSB_HINT_ORACLE_AMIGA_AU2_EN_DAT_MD5 "7496b3b8b9ff6e2368eac9a16be8230b"

typedef enum {
    CSB_HINT_ORACLE_RUNTIME_PROFILE_ATARI_R1 = 0,
    CSB_HINT_ORACLE_RUNTIME_PROFILE_AMIGA_AU2_EN = 1
} CSB_HintOracleRuntimeProfile;

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
    CSB_HintOracleRuntimeProfile profile;
    int assets_loaded;
} CSB_HintOracleAtariRuntime;

void csb_hint_oracle_atari_runtime_init(CSB_HintOracleAtariRuntime *runtime);
void csb_hint_oracle_atari_runtime_free(CSB_HintOracleAtariRuntime *runtime);

/* Auto-detect a hash-admitted Atari R1 or Amiga AU2 English pair.
 * `cache_dir` is used only if an HTC source lives in a virtual container;
 * ordinary original files are read in place. */
int csb_hint_oracle_atari_runtime_load_assets(
    CSB_HintOracleAtariRuntime *runtime,
    const char *data_dir, const char *cache_dir, int max_depth);

/* Load one platform profile selected from the native save header. The AU2
 * profile admits only authentic Amiga R2/R3 English HCSB.HTC paired with the
 * shared authentic AU2 English HCSB.DAT. */
int csb_hint_oracle_atari_runtime_load_assets_for_profile(
    CSB_HintOracleAtariRuntime *runtime,
    const char *data_dir, const char *cache_dir, int max_depth,
    CSB_HintOracleRuntimeProfile profile);

/* Feed only a successfully decoded native Atari MINI.DAT or Amiga
 * 1CSBGAME.DAT receipt. */
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

/* HINTDATA.C / HINTMAIN.C pointer command dispatcher for the selected
 * Atari R1 or Amiga AU2 profile. `info` is consumed only by the LOAD
 * rectangle while the runtime waits for a save; other commands ignore it. */
int csb_hint_oracle_atari_runtime_handle_click(
    CSB_HintOracleAtariRuntime *runtime, int x, int y,
    const CSB_V1_AtariSaveInfo *info);

/* Render the current original 320x200 Hint Oracle state. This covers the
 * LOAD prompt, selected-hint list, no-clue acknowledgement and hint page.
 * Every state starts with HCSB.DAT's decoded original screen; its text uses
 * the decoded original HCSB font and HINTDATA.C/HINTMAIN.C rectangles.
 * No host frame, font or palette fallback is generated. */
int csb_hint_oracle_atari_runtime_render_frame(
    const CSB_HintOracleAtariRuntime *runtime,
    uint8_t *frame, size_t frame_size);

/* Compatibility entry point for callers that specifically require a hint
 * page. It rejects every other session state. */
int csb_hint_oracle_atari_runtime_render_page(
    const CSB_HintOracleAtariRuntime *runtime,
    uint8_t *frame, size_t frame_size);

const char *csb_hint_oracle_atari_runtime_result_name(int result);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HINT_ORACLE_ATARI_RUNTIME_H */
