#include "csb_hint_oracle_atari_runtime.h"

#include <string.h>

void csb_hint_oracle_atari_runtime_init(CSB_HintOracleAtariRuntime *runtime)
{
    if (!runtime) return;
    memset(runtime, 0, sizeof(*runtime));
    csb_hint_oracle_ui_panel_init(&runtime->htc_panel);
    csb_hint_oracle_graphics_surface_init(&runtime->graphics);
    csb_hint_oracle_session_init(&runtime->session);
}

void csb_hint_oracle_atari_runtime_free(CSB_HintOracleAtariRuntime *runtime)
{
    if (!runtime) return;
    csb_hint_oracle_ui_panel_free(&runtime->htc_panel);
    csb_hint_oracle_graphics_surface_free(&runtime->graphics);
    csb_hint_oracle_session_init(&runtime->session);
    runtime->assets_loaded = 0;
}

int csb_hint_oracle_atari_runtime_load_assets(
    CSB_HintOracleAtariRuntime *runtime,
    const char *data_dir, const char *cache_dir, int max_depth)
{
    int rc;
    if (!runtime || !data_dir || !data_dir[0] || max_depth <= 0)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    csb_hint_oracle_atari_runtime_free(runtime);
    rc = csb_hint_oracle_ui_panel_load_md5(
        &runtime->htc_panel, data_dir, cache_dir, max_depth,
        CSB_HINT_ORACLE_ATARI_R1_HTC_MD5);
    if (rc != CSB_HINT_ORACLE_UI_PANEL_OK)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_HTC;
    rc = csb_hint_oracle_graphics_surface_load(
        &runtime->graphics, data_dir, max_depth,
        CSB_HINT_ORACLE_ATARI_R1_DAT_MD5);
    if (rc != CSB_HINT_ORACLE_DAT_REAL_OK) {
        csb_hint_oracle_ui_panel_free(&runtime->htc_panel);
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_GRAPHICS;
    }
    runtime->assets_loaded = 1;
    return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
}

int csb_hint_oracle_atari_runtime_select_save(
    CSB_HintOracleAtariRuntime *runtime,
    const CSB_V1_AtariSaveInfo *info)
{
    if (!runtime || !info)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    if (!runtime->assets_loaded || !runtime->htc_panel.cache.loaded ||
        !runtime->graphics.source.loaded)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    return csb_hint_oracle_atari_save_session_select(
        &runtime->session, &runtime->htc_panel.cache.htc, info) ==
        CSB_HINT_ORACLE_ATARI_SAVE_SESSION_OK ?
        CSB_HINT_ORACLE_ATARI_RUNTIME_OK :
        CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SAVE;
}

int csb_hint_oracle_atari_runtime_open_hint_row(
    CSB_HintOracleAtariRuntime *runtime, size_t row)
{
    if (!runtime) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    if (!runtime->assets_loaded) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    return csb_hint_oracle_session_open_hint_row(&runtime->session, row) ==
        CSB_HINT_ORACLE_SESSION_OK ? CSB_HINT_ORACLE_ATARI_RUNTIME_OK :
        CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SESSION;
}

int csb_hint_oracle_atari_runtime_previous_page(CSB_HintOracleAtariRuntime *runtime)
{
    if (!runtime) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    return csb_hint_oracle_session_previous_page(&runtime->session) ==
        CSB_HINT_ORACLE_SESSION_OK ? CSB_HINT_ORACLE_ATARI_RUNTIME_OK :
        CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SESSION;
}

int csb_hint_oracle_atari_runtime_next_page(CSB_HintOracleAtariRuntime *runtime)
{
    if (!runtime) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    return csb_hint_oracle_session_next_page(&runtime->session) ==
        CSB_HINT_ORACLE_SESSION_OK ? CSB_HINT_ORACLE_ATARI_RUNTIME_OK :
        CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SESSION;
}

int csb_hint_oracle_atari_runtime_done(CSB_HintOracleAtariRuntime *runtime)
{
    if (!runtime) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    return csb_hint_oracle_session_done(&runtime->session) ==
        CSB_HINT_ORACLE_SESSION_OK ? CSB_HINT_ORACLE_ATARI_RUNTIME_OK :
        CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SESSION;
}

int csb_hint_oracle_atari_runtime_render_page(
    const CSB_HintOracleAtariRuntime *runtime, uint8_t *frame, size_t frame_size)
{
    if (!runtime || !frame) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    if (!runtime->assets_loaded ||
        runtime->session.state != CSB_HINT_ORACLE_SESSION_HINT_PAGE ||
        runtime->session.selected_row >= runtime->session.selected_hint_count)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    return csb_hint_oracle_graphics_surface_render_st_hint_page(
        &runtime->graphics, &runtime->htc_panel.cache.htc,
        runtime->session.selected_hint_indices[runtime->session.selected_row],
        runtime->session.page_number, frame, frame_size) ?
        CSB_HINT_ORACLE_ATARI_RUNTIME_OK :
        CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
}

const char *csb_hint_oracle_atari_runtime_result_name(int result)
{
    switch (result) {
    case CSB_HINT_ORACLE_ATARI_RUNTIME_OK: return "OK";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT: return "argument";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_HTC: return "htc";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_GRAPHICS: return "graphics";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY: return "not-ready";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SAVE: return "save";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_SESSION: return "session";
    case CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER: return "render";
    default: return "unknown";
    }
}
