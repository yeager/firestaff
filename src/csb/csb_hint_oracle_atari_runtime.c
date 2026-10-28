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

static int load_profile_pair(CSB_HintOracleAtariRuntime *runtime,
                             const char *data_dir, const char *cache_dir,
                             int max_depth, const char *htc_hash,
                             const char *dat_hash,
                             CSB_HintOracleRuntimeProfile profile)
{
    if (csb_hint_oracle_ui_panel_load_md5(
            &runtime->htc_panel, data_dir, cache_dir, max_depth,
            htc_hash) != CSB_HINT_ORACLE_UI_PANEL_OK) {
        return 0;
    }
    if (csb_hint_oracle_graphics_surface_load(
            &runtime->graphics, data_dir, max_depth,
            dat_hash) != CSB_HINT_ORACLE_DAT_REAL_OK) {
        /* Do not retain an HTC from an unmatched profile. */
        csb_hint_oracle_ui_panel_free(&runtime->htc_panel);
        return 0;
    }
    runtime->profile = profile;
    runtime->assets_loaded = 1;
    return 1;
}

int csb_hint_oracle_atari_runtime_load_assets_for_profile(
    CSB_HintOracleAtariRuntime *runtime,
    const char *data_dir, const char *cache_dir, int max_depth,
    CSB_HintOracleRuntimeProfile profile)
{
    if (!runtime || !data_dir || !data_dir[0] || max_depth <= 0 ||
        (profile != CSB_HINT_ORACLE_RUNTIME_PROFILE_ATARI_R1 &&
         profile != CSB_HINT_ORACLE_RUNTIME_PROFILE_AMIGA_AU2_EN)) {
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    }
    csb_hint_oracle_atari_runtime_free(runtime);
    if (profile == CSB_HINT_ORACLE_RUNTIME_PROFILE_ATARI_R1) {
        return load_profile_pair(
                   runtime, data_dir, cache_dir, max_depth,
                   CSB_HINT_ORACLE_ATARI_R1_HTC_MD5,
                   CSB_HINT_ORACLE_ATARI_R1_DAT_MD5, profile)
            ? CSB_HINT_ORACLE_ATARI_RUNTIME_OK
            : CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_HTC;
    }
    if (load_profile_pair(runtime, data_dir, cache_dir, max_depth,
                          CSB_HINT_ORACLE_AMIGA_AU2_R3_HTC_MD5,
                          CSB_HINT_ORACLE_AMIGA_AU2_EN_DAT_MD5, profile) ||
        load_profile_pair(runtime, data_dir, cache_dir, max_depth,
                          CSB_HINT_ORACLE_AMIGA_AU2_R2_HTC_MD5,
                          CSB_HINT_ORACLE_AMIGA_AU2_EN_DAT_MD5, profile)) {
        return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
    }
    return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_HTC;
}

int csb_hint_oracle_atari_runtime_load_assets(
    CSB_HintOracleAtariRuntime *runtime,
    const char *data_dir, const char *cache_dir, int max_depth)
{
    int rc = csb_hint_oracle_atari_runtime_load_assets_for_profile(
        runtime, data_dir, cache_dir, max_depth,
        CSB_HINT_ORACLE_RUNTIME_PROFILE_ATARI_R1);
    if (rc == CSB_HINT_ORACLE_ATARI_RUNTIME_OK ||
        rc == CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT) {
        return rc;
    }
    return csb_hint_oracle_atari_runtime_load_assets_for_profile(
        runtime, data_dir, cache_dir, max_depth,
        CSB_HINT_ORACLE_RUNTIME_PROFILE_AMIGA_AU2_EN);
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

static int in_button(int x, int y, int left, int right, int top, int bottom)
{
    return x >= left && x <= right && y >= top && y <= bottom;
}

static void m12_button_rect(const CSB_HintOracleAtariRuntime *runtime,
                            int *left, int *right, int *top, int *bottom)
{
    *top = 179;
    *bottom = 192;
    if (runtime->profile == CSB_HINT_ORACLE_RUNTIME_PROFILE_AMIGA_AU2_EN) {
        if (*left == 33) {
            *left = 20;
            *right = 111;
            *top = 180;
            *bottom = 193;
        } else if (*left == 127) {
            *left = 110;
            *right = 200;
        } else if (*left == 221) {
            *left = 205;
            *right = 296;
        }
    }
}

static int m12_button(const CSB_HintOracleAtariRuntime *runtime,
                      int x, int y, int left, int right)
{
    int top, bottom;
    m12_button_rect(runtime, &left, &right, &top, &bottom);
    return in_button(x, y, left, right, top, bottom);
}

int csb_hint_oracle_atari_runtime_handle_click(
    CSB_HintOracleAtariRuntime *runtime, int x, int y,
    const CSB_V1_AtariSaveInfo *info)
{
    size_t row;
    if (!runtime) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    if (!runtime->assets_loaded) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    /* HINTDATA.C MEDIA762_AU1E / MEDIA767_AU2E rectangles. */
    if (runtime->session.state == CSB_HINT_ORACLE_SESSION_AWAIT_LOAD) {
        if (m12_button(runtime, x, y, 127, 194))
            return csb_hint_oracle_atari_runtime_select_save(runtime, info);
        if (m12_button(runtime, x, y, 221, 289)) {
            csb_hint_oracle_session_close(&runtime->session);
            return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
        }
        return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
    }
    if (runtime->session.state == CSB_HINT_ORACLE_SESSION_HINT_LIST) {
        for (row = 0u; row < CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS; ++row) {
            if (in_button(x, y, 40, 280, 30 + (int)row * 16,
                          42 + (int)row * 16))
                return csb_hint_oracle_atari_runtime_open_hint_row(runtime, row);
        }
        if (m12_button(runtime, x, y, 221, 289))
            return csb_hint_oracle_atari_runtime_done(runtime);
        return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
    }
    if (runtime->session.state == CSB_HINT_ORACLE_SESSION_HINT_PAGE) {
        if (m12_button(runtime, x, y, 33, 100))
            return csb_hint_oracle_atari_runtime_previous_page(runtime);
        if (m12_button(runtime, x, y, 127, 194))
            return csb_hint_oracle_atari_runtime_next_page(runtime);
        if (m12_button(runtime, x, y, 221, 289))
            return csb_hint_oracle_atari_runtime_done(runtime);
        return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
    }
    if (runtime->session.state == CSB_HINT_ORACLE_SESSION_NO_CLUE &&
        m12_button(runtime, x, y, 127, 194))
        return csb_hint_oracle_atari_runtime_done(runtime);
    return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
}

static const char *runtime_text(const CSB_HintOracleAtariRuntime *runtime,
                                size_t au2_text_index,
                                const char *atari_text)
{
    const char *text = NULL;
    if (runtime->profile != CSB_HINT_ORACLE_RUNTIME_PROFILE_AMIGA_AU2_EN)
        return atari_text;
    return csb_hint_oracle_graphics_surface_get_au2_text(
               &runtime->graphics, au2_text_index, &text) ? text : NULL;
}

static int render_centered(const CSB_HintOracleAtariRuntime *runtime,
                           uint8_t *frame, int left, int right,
                           int top, int bottom, const char *text)
{
    return csb_hint_oracle_graphics_surface_blit_st_centered_box(
        &runtime->graphics, frame, 64000u, left, right, top, bottom, text);
}

static int render_button(const CSB_HintOracleAtariRuntime *runtime,
                         uint8_t *frame, int left, int right,
                         size_t au2_text_index, const char *atari_label)
{
    int top, bottom;
    m12_button_rect(runtime, &left, &right, &top, &bottom);
    return render_centered(runtime, frame, left, right, top, bottom,
                           runtime_text(runtime, au2_text_index, atari_label));
}

int csb_hint_oracle_atari_runtime_render_frame(
    const CSB_HintOracleAtariRuntime *runtime, uint8_t *frame, size_t frame_size)
{
    size_t row;
    CSB_HintOracleHTC_Hint hint;
    if (!runtime || !frame) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    if (!runtime->assets_loaded || !runtime->graphics.pixels || frame_size < 64000u)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    memcpy(frame, runtime->graphics.pixels, 64000u);
    switch (runtime->session.state) {
    case CSB_HINT_ORACLE_SESSION_AWAIT_LOAD:
        if (!render_centered(runtime, frame, 31, 290, 50, 150,
                runtime_text(runtime, 0u,
                    "Chaos Strikes Back/Hint Oracle//Please insert a Chaos Strikes Back saved game disk in drive A://Then press LOAD")) ||
            !render_button(runtime, frame, 127, 194, 9u, "LOAD") ||
            !render_button(runtime, frame, 221, 289, 14u, "EXIT"))
            return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
        break;
    case CSB_HINT_ORACLE_SESSION_HINT_LIST:
        if (runtime->session.selected_hint_count == 0u ||
            runtime->session.selected_hint_count > CSB_HINT_ORACLE_SESSION_MAX_SELECTED_HINTS)
            return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
        for (row = 0u; row < runtime->session.selected_hint_count; ++row) {
            if (csb_hint_oracle_htc_get_hint(
                    &runtime->htc_panel.cache.htc,
                    runtime->session.selected_hint_indices[row], &hint) !=
                CSB_HINT_ORACLE_HTC_OK ||
                !render_centered(runtime, frame, 40, 280,
                                 30 + (int)row * 16, 42 + (int)row * 16,
                                 hint.name))
                return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
        }
        if (!render_button(runtime, frame, 221, 289, 12u, "DONE"))
            return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
        break;
    case CSB_HINT_ORACLE_SESSION_NO_CLUE:
        if (!render_centered(runtime, frame, 31, 290, 70, 150,
                runtime_text(runtime, 4u,
                             "There are no clues for this location.")) ||
            !render_button(runtime, frame, 127, 194, 13u, "OK"))
            return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
        break;
    case CSB_HINT_ORACLE_SESSION_HINT_PAGE:
        if (runtime->session.selected_row >= runtime->session.selected_hint_count ||
            !csb_hint_oracle_graphics_surface_render_st_hint_page(
                &runtime->graphics, &runtime->htc_panel.cache.htc,
                runtime->session.selected_hint_indices[runtime->session.selected_row],
                runtime->session.page_number, frame, frame_size))
            return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
        if (csb_hint_oracle_htc_get_hint(
                &runtime->htc_panel.cache.htc,
                runtime->session.selected_hint_indices[runtime->session.selected_row],
                &hint) != CSB_HINT_ORACLE_HTC_OK ||
            !render_button(runtime, frame, 221, 289, 12u, "DONE") ||
            (runtime->session.page_number > 1u &&
             !render_button(runtime, frame, 33, 100, 10u, "LAST")) ||
            (runtime->session.page_number < hint.page_count &&
             !render_button(runtime, frame, 127, 194, 11u, "NEXT")))
            return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_RENDER;
        break;
    default:
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    }
    return CSB_HINT_ORACLE_ATARI_RUNTIME_OK;
}

int csb_hint_oracle_atari_runtime_render_page(
    const CSB_HintOracleAtariRuntime *runtime, uint8_t *frame, size_t frame_size)
{
    if (!runtime || !frame) return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_ARGUMENT;
    if (runtime->session.state != CSB_HINT_ORACLE_SESSION_HINT_PAGE)
        return CSB_HINT_ORACLE_ATARI_RUNTIME_ERR_NOT_READY;
    return csb_hint_oracle_atari_runtime_render_frame(runtime, frame, frame_size);
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
