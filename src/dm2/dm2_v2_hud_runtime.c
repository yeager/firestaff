/* DM2 V2 HUD: original GDAT-backed presentation only. */
#include "dm2_v2_hud_runtime.h"
#include "dm2_v1_viewport_renderer.h"
#include <string.h>

static DM2_V2_HudOverlay s_hud;
static int s_initialized;
static const DM2_V2_PhaseGateConfig *s_gate_config;
static int s_force_active;
static DM2_V2_HudGdatFetch s_gdat_fetch;
static DM2_V2_HudGdatPaletteFetch s_gdat_palette_fetch;
static void *s_gdat_user;
static int s_original_data_mounted;

/* Per-slot path-mode record updated by
 * dm2_v2_hud_runtime_render_with_assets(). Restored after the a192cb2b0
 * worktree merge kept the public header contract and the runtime-hook
 * probe but dropped the definitions. Initialised to PROCEDURAL_FALLBACK
 * so callers observing the array before any asset-aware render still see
 * the documented default. */
static DM2_V2_HudRuntimePathMode
    s_last_path_mode[DM2_V2_HUD_WIDGET_COUNT];
static DM2_V2_HudWidgetClass
    s_last_slot_class[DM2_V2_HUD_WIDGET_COUNT];
static int s_last_path_real;
static int s_last_path_fallback;
static int s_last_render_with_assets; /* 1 if render_with_assets() ran */

static void reset_path_record(void)
{
    int i;
    for (i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_NO_DRAW;
        s_last_slot_class[i] = DM2_V2_HUD_WIDGET_CLASS_UNKNOWN;
    }
    s_last_path_real = 0;
    s_last_path_fallback = 0;
    s_last_render_with_assets = 0;
}

static void ensure_init(void)
{
    if (!s_initialized) {
        dm2_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
}

static int render_allowed(void)
{
    if (!s_initialized || !s_hud.visible || s_hud.opacity == 0) return 0;
    if (s_force_active) return 1;
    return s_gate_config && s_gate_config->v2LaunchEnabled &&
        s_gate_config->v2ProfileEnabled;
}

static void blit_source(uint8_t *fb, int fb_w, int fb_h, int x, int y,
                        int dst_w, int dst_h, const uint8_t *pixels,
                        int src_w, int src_h, int src_stride,
                        const uint8_t palette16[16])
{
    for (int dy = 0; dy < dst_h; ++dy) {
        int sy = dy * src_h / dst_h;
        if (y + dy < 0 || y + dy >= fb_h) continue;
        for (int dx = 0; dx < dst_w; ++dx) {
            int sx = dx * src_w / dst_w;
            uint8_t pixel;
            if (x + dx < 0 || x + dx >= fb_w) continue;
            pixel = pixels[sy * src_stride + sx];
            if (pixel != 0) fb[(y + dy) * fb_w + x + dx] = palette16[pixel & 15u];
        }
    }
}

static int render_gdat_image(uint8_t *fb, int fb_w, int fb_h,
                             int x, int y, int dst_w, int dst_h, int key)
{
    const uint8_t *pixels = NULL;
    int src_w = 0, src_h = 0, stride = 0;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    if (!s_gdat_fetch || !s_gdat_palette_fetch || key == 0 ||
        s_gdat_fetch(s_gdat_user, key, &pixels, &src_w, &src_h, &stride) != 0 ||
        s_gdat_palette_fetch(s_gdat_user, key, palette16, &palette_hash) != 0 ||
        palette_hash == 0u ||
        !pixels || src_w <= 0 || src_h <= 0 || stride < src_w) return 0;
    blit_source(fb, fb_w, fb_h, x, y, dst_w, dst_h, pixels, src_w, src_h, stride,
                palette16);
    return 1;
}

static void render_original_hud(uint8_t *fb, int w, int h)
{
    DM2_V1_HudChromeRenderPlan plan;
    if (!dm2_v1_viewport_build_hud_chrome_plan(0, &plan)) return;

    (void)render_gdat_image(fb, w, h, plan.top_bar_rect.x, plan.top_bar_rect.y,
                            plan.top_bar_rect.w, plan.top_bar_rect.h,
                            plan.top_bar_gdat_index);
    (void)render_gdat_image(fb, w, h, plan.action_strip_rect.x,
                            plan.action_strip_rect.y, plan.action_strip_rect.w,
                            plan.action_strip_rect.h, plan.action_strip_gdat_index);
    (void)render_gdat_image(fb, w, h, plan.gold_box_rect.x, plan.gold_box_rect.y,
                            plan.gold_box_rect.w, plan.gold_box_rect.h,
                            plan.gold_box_gdat_index);
    (void)render_gdat_image(fb, w, h, plan.portrait_panel_rect.x,
                            plan.portrait_panel_rect.y, plan.portrait_panel_rect.w,
                            plan.portrait_panel_rect.h, plan.portrait_panel_gdat_index);
    for (int i = 0; i < plan.action_icon_count; ++i) {
        (void)render_gdat_image(fb, w, h, plan.action_icons[i].fill_rect.x,
            plan.action_icons[i].fill_rect.y, plan.action_icons[i].fill_rect.w,
            plan.action_icons[i].fill_rect.h, plan.action_icons[i].gdat_index);
    }
    for (int i = 0; i < plan.champion_slot_count; ++i) {
        (void)render_gdat_image(fb, w, h, plan.champion_slots[i].portrait_rect.x,
            plan.champion_slots[i].portrait_rect.y, plan.champion_slots[i].portrait_rect.w,
            plan.champion_slots[i].portrait_rect.h,
            dm2_v1_viewport_hud_portrait_graphic_index(i));
    }
}

void dm2_v2_hud_runtime_init(void) { ensure_init(); s_force_active = 0; reset_path_record(); }
void dm2_v2_hud_runtime_shutdown(void)
{
    if (s_initialized) dm2_v2_hud_reset(&s_hud);
    s_initialized = 0; s_gate_config = NULL; s_force_active = 0;
    s_gdat_fetch = NULL; s_gdat_palette_fetch = NULL; s_gdat_user = NULL; s_original_data_mounted = 0;
    reset_path_record();
}
void dm2_v2_hud_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config) { s_gate_config = config; }
void dm2_v2_hud_runtime_set_gdat_source(DM2_V2_HudGdatFetch fetch,
                                         DM2_V2_HudGdatPaletteFetch palette_fetch,
                                         void *user, int mounted)
{ s_gdat_fetch = fetch; s_gdat_palette_fetch = palette_fetch; s_gdat_user = user; s_original_data_mounted = mounted ? 1 : 0; }
void dm2_v2_hud_runtime_set_party_gold(int v) { ensure_init(); dm2_v2_hud_set_gold(&s_hud, v); }
void dm2_v2_hud_runtime_set_direction(int v) { ensure_init(); dm2_v2_hud_set_direction(&s_hud, v); }
void dm2_v2_hud_runtime_set_level(int a, int b) { ensure_init(); dm2_v2_hud_set_level(&s_hud, a, b); }
void dm2_v2_hud_runtime_set_champion(int i, int a, int b, int c, bool d, bool e)
{ ensure_init(); dm2_v2_hud_set_champion_bar(&s_hud, i, a, b, c, d, e); }
void dm2_v2_hud_runtime_set_action_active(DM2_V2_ActionIcon v) { ensure_init(); dm2_v2_hud_set_action_active(&s_hud, v); }
void dm2_v2_hud_runtime_trigger_hit_flash(void) { ensure_init(); dm2_v2_hud_trigger_hit_flash(&s_hud); }
void dm2_v2_hud_runtime_set_opacity(uint8_t v) { ensure_init(); dm2_v2_hud_set_opacity(&s_hud, v); }
void dm2_v2_hud_runtime_render(uint8_t *fb, int w, int h)
{
    if (!fb || w <= 0 || h <= 0 || !render_allowed()) return;
    /* skproject c_gui_vp.cpp uses INTERFACE_GENERAL / CHAMPIONS images.
     * A missing source image is simply absent: no procedural replacement. */
    if (s_original_data_mounted && s_gdat_fetch && s_gdat_palette_fetch) render_original_hud(fb, w, h);
}
int dm2_v2_hud_runtime_is_active(void) { return render_allowed() && s_original_data_mounted && s_gdat_fetch && s_gdat_palette_fetch; }
void dm2_v2_hud_runtime_force_active_for_test(int active) { s_force_active = active ? 1 : 0; }

void dm2_v2_hud_runtime_render_with_assets(uint8_t *fb, int w, int h_res) {
    int i;
    int render_will_run = 1;

    reset_path_record();
    s_last_render_with_assets = 1;
    for (i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        s_last_slot_class[i] = DM2_V2_HUD_WIDGET_CLASS_MISSING;
    }

    if (!s_initialized) render_will_run = 0;
    if (!s_force_active && !s_hud.visible) render_will_run = 0;
    if (s_hud.opacity == 0) render_will_run = 0;
    if (!s_force_active) {
        if (!s_gate_config) render_will_run = 0;
        else if (!s_gate_config->v2LaunchEnabled) render_will_run = 0;
        else if (!s_gate_config->v2ProfileEnabled) render_will_run = 0;
    }

    /* Classify every slot up-front so probe code can read the gate's
     * verdict regardless of whether we actually rendered. */
    for (i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetClass cls =
            dm2_v2_hud_widget_assets_classify_slot(
                (DM2_V2_HudWidgetSlot)i);
        s_last_slot_class[i] = cls;
        if (cls == DM2_V2_HUD_WIDGET_CLASS_REAL) {
            s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP;
            ++s_last_path_real;
        } else {
            s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_NO_DRAW;
            ++s_last_path_fallback;
        }
    }

    if (!render_will_run) {
        /* V1 chrome owns the framebuffer; the path-mode record is the
         * only state this function emits. */
        return;
    }

    /* The widget manifest may classify external art for diagnostics, but it
     * is never a DM2 pixel owner.  SKWIN's HUD consumes INTERFACE_GENERAL
     * and CHAMPIONS GDAT records, so this compatibility entry point uses the
     * same authenticated GDAT route as the production renderer. */
    dm2_v2_hud_runtime_render(fb, w, h_res);
}

DM2_V2_HudRuntimePathMode dm2_v2_hud_runtime_last_path_mode(
    DM2_V2_HudWidgetSlot slot)
{
    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) {
        return DM2_V2_HUD_RUNTIME_PATH_NO_DRAW;
    }
    return s_last_path_mode[slot];
}

int dm2_v2_hud_runtime_last_path_counts(int* out_real, int* out_fallback) {
    if (out_real)     *out_real     = s_last_path_real;
    if (out_fallback) *out_fallback = s_last_path_fallback;
    return s_last_path_real + s_last_path_fallback;
}

DM2_V2_HudWidgetClass dm2_v2_hud_runtime_last_slot_class(
    DM2_V2_HudWidgetSlot slot)
{
    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) {
        return DM2_V2_HUD_WIDGET_CLASS_UNKNOWN;
    }
    return s_last_slot_class[slot];
}

const char *dm2_v2_hud_runtime_source_evidence(void)
{
    return "DM2 V2 HUD GDAT source-lock\n"
        "Source: ReDMCSB SKULL.ASM T560       (DM2 HUD rendering pipeline)\n"
        "Source: skproject/SKULLWIN/c_gui_vp.cpp DRAW_CHAMPION_PICTURE and UI chrome\n"
        "Source: ReDMCSB PANEL.C F0354        (champion status-box drawing)\n"
        "INTERFACE_GENERAL image fields: top bar, action strip, gold, panel, icons\n"
        "CHAMPIONS image field 0: portrait pixels\n"
        "Each IMG3 draw maps logical pixels through its paired dtPalette16 table.\n"
        "Source: dm2_v2_hud_widget_assets     (per-slot REAL/PARTIAL/PLACEHOLDER gate, Phase 3 hook)\n"
        "Source: dm2_v2_hud_widget_bitmap_blit (fixture decoder; strict no-draw)\n"
        "V1 invariant: V1 command routes, inventory, dungeon state NEVER bypassed\n"
        "Rule: mounted original data draws decoded GDAT pixels only; missing pixels or palettes are not synthesized.\n"
        "Manifest or fixture art is never a HUD pixel owner.\n";
}
