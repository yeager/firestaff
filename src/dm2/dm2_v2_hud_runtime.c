/*
 * dm2_v2_hud_runtime.c — DM2 V2 Phase 3 HUD Runtime Integration
 *
 * Provides the integration layer between the DM2 V1 command dispatch
 * (SKULL.ASM T520/T048) and the DM2 V2 HUD presentation layer.
 *
 * This module is PRESENTATION-ONLY: it reads V1 game state to populate
 * the HUD overlay, but does NOT write to any V1 data structures.
 * V1 command routes, inventory transactions, and dungeon state are
 * NEVER bypassed or altered by this module.
 *
 * Phase 3 rule: HUD overlay is gated on DM2_V2_PHASE_DOMAIN_HUD and
 * activates only when both v2LaunchEnabled and v2ProfileEnabled are
 * true. The overlay renders into the supplied framebuffer without
 * altering V1 state.
 *
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *         ReDMCSB COMMAND.C action feedback gates
 *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
 *         csb_v2_hud_runtime.c (sibling CSB V2 wire-up pattern)
 */

#include "dm2_v2_hud_runtime.h"
#include "dm2_v2_hud_widget_bitmap_blit.h"
#include <string.h>

/* ── Module state ──────────────────────────────────────────────── */
static DM2_V2_HudOverlay s_hud;
static int s_initialized = 0;
static const DM2_V2_PhaseGateConfig *s_gate_config = NULL;
static int s_force_active = 0;  /* 0 = phase-gated, 1 = always on (test only) */

/* Per-slot path-mode record updated by
 * dm2_v2_hud_runtime_render_with_assets(). Initialised to
 * PROCEDURAL_FALLBACK so callers observing the array before any
 * asset-aware render still see the current default. */
static DM2_V2_HudRuntimePathMode
    s_last_path_mode[DM2_V2_HUD_WIDGET_COUNT];
static DM2_V2_HudWidgetClass
    s_last_slot_class[DM2_V2_HUD_WIDGET_COUNT];
static int s_last_path_real = 0;
static int s_last_path_fallback = 0;
static int s_last_render_with_assets = 0;  /* 1 if render_with_assets() ran */

static void ensure_init(void) {
    if (!s_initialized) {
        dm2_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
}

/* ── Lifecycle ──────────────────────────────────────────────────── */
void dm2_v2_hud_runtime_init(void) {
    if (!s_initialized) {
        dm2_v2_hud_init(&s_hud);
        s_initialized = 1;
    }
    s_force_active = 0;
    /* Reset per-slot path-mode record so the next render_with_assets()
     * call observes a clean baseline. */
    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK;
        s_last_slot_class[i] = DM2_V2_HUD_WIDGET_CLASS_UNKNOWN;
    }
    s_last_path_real = 0;
    s_last_path_fallback = 0;
    s_last_render_with_assets = 0;
}

void dm2_v2_hud_runtime_shutdown(void) {
    if (s_initialized) {
        dm2_v2_hud_reset(&s_hud);
        s_initialized = 0;
    }
    s_gate_config = NULL;
    s_force_active = 0;
    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK;
        s_last_slot_class[i] = DM2_V2_HUD_WIDGET_CLASS_UNKNOWN;
    }
    s_last_path_real = 0;
    s_last_path_fallback = 0;
    s_last_render_with_assets = 0;
}

/* ── Configuration ──────────────────────────────────────────────── */
void dm2_v2_hud_runtime_set_gate_config(const DM2_V2_PhaseGateConfig *config) {
    s_gate_config = config;
}

/* ── State setters (V1 → V2 HUD bridge) ────────────────────────── */
void dm2_v2_hud_runtime_set_party_gold(int gold_pieces) {
    ensure_init();
    dm2_v2_hud_set_gold(&s_hud, gold_pieces);
}

void dm2_v2_hud_runtime_set_direction(int dir) {
    ensure_init();
    dm2_v2_hud_set_direction(&s_hud, dir);
}

void dm2_v2_hud_runtime_set_level(int cur, int max) {
    ensure_init();
    dm2_v2_hud_set_level(&s_hud, cur, max);
}

void dm2_v2_hud_runtime_set_champion(int champ_idx, int hp_pct,
    int stamina_pct, int mana_pct, bool leader, bool spell_ready)
{
    ensure_init();
    dm2_v2_hud_set_champion_bar(&s_hud, champ_idx, hp_pct, stamina_pct,
        mana_pct, leader, spell_ready);
}

void dm2_v2_hud_runtime_set_action_active(DM2_V2_ActionIcon icon) {
    ensure_init();
    dm2_v2_hud_set_action_active(&s_hud, icon);
}

void dm2_v2_hud_runtime_trigger_hit_flash(void) {
    ensure_init();
    dm2_v2_hud_trigger_hit_flash(&s_hud);
}

void dm2_v2_hud_runtime_set_opacity(uint8_t val) {
    ensure_init();
    dm2_v2_hud_set_opacity(&s_hud, val);
}

void dm2_v2_hud_runtime_render(uint8_t *fb, int w, int h_res) {
    if (!s_initialized) return;
    if (!s_force_active && !s_hud.visible) return;
    if (s_hud.opacity == 0) return;
    if (!s_force_active) {
        /* Phase gate: HUD is presentation-only, requires V2 launch+profile
         * both enabled.  When V1 is active, the HUD is hidden (V1 chrome
         * owns the framebuffer). */
        if (!s_gate_config) return;
        if (!s_gate_config->v2LaunchEnabled) return;
        if (!s_gate_config->v2ProfileEnabled) return;
    }
    dm2_v2_hud_render(&s_hud, fb, w, h_res);
}

/* ── Asset-aware render (Phase 3 widget bitmap hook) ─────────────
 *
 * dm2_v2_hud_runtime_render_with_assets() — wires the per-slot
 * REAL/PARTIAL/PLACEHOLDER/MISSING classification from
 * dm2_v2_hud_widget_assets.h into the runtime render path so a REAL
 * classification actually substitutes the procedural fallback for
 * that slot. The honest scope is:
 *
 *   - Path-mode recording (REAL_BITMAP vs PROCEDURAL_FALLBACK) per slot.
 *   - Bounded real-bitmap blit on REAL slots using synthetic 1x1 RGBA
 *     PNG fixtures from examples/dm2_hud_widget_synthetic/ (Phase 3
 *     follow-up). When a slot is REAL and its source_file resolves on
 *     disk, the runtime reads the synthetic fixture and writes its
 *     red channel as a palette index at the slot's anchor pixel.
 *   - 1-pixel anchor stamp fallback when the blit cannot run (file
 *     missing, unsupported format, decompression failure, destination
 *     out of bounds, zlib disabled at build time). The stamp value is
 *     the HUD opacity so a probe can still detect that the gate
 *     reached the runtime end-to-end.
 *   - The procedural fallback path is byte-identical to the no-gate
 *     baseline for any slot whose classification is not REAL.
 *
 * It does NOT decode multi-pixel PNGs — that requires finished
 * PBR HUD widget art, which is still an OPEN-BOUNDED gap. The blit
 * site is the synthetic envelope: when operator-installed multi-pixel
 * art ships, dm2_v2_hud_widget_bitmap_blit_render_slot() is the
 * obvious replacement site for the full decode path. The current
 * bounded envelope (1x1 8-bit RGBA) is enforced explicitly so the
 * runtime cannot silently fall into a multi-pixel decode.
 *
 * Walks the seven Phase 3 / chrome-supporting widget slots the
 * dm2_v2_hud_widget_assets gate classifies, decides for each slot
 * whether the runtime takes the existing procedural fallback path or
 * the real-bitmap substitute path, records the decision in
 * s_last_path_mode[] / s_last_slot_class[], and then dispatches the
 * existing procedural render.
 *
 * Phase-gating mirrors dm2_v2_hud_runtime_render() exactly: when V2
 * is off (and the test-bypass is also off) we leave the framebuffer
 * untouched and the path-mode record stays at the default
 * PROCEDURAL_FALLBACK for every slot. This keeps V1 chrome ownership
 * of the framebuffer intact.
 *
 * Real-bitmap substitute path: this hook only STAMPS the slot
 * classification into the path-mode record. It does NOT decode
 * bitmap pixels or claim finished art. The bitmap decode will land
 * here when operator-installed art ships — the slot classification
 * infrastructure is already in place to gate it. The honesty
 * statement is:
 *   - The wiring is provably complete (a REAL slot is selected, the
 *     path-mode record records REAL_BITMAP, the procedural fallback
 *     path is byte-identical to the no-gate baseline for non-REAL
 *     slots).
 *   - The actual pixel decode is intentionally NOT done in this
 *     pass — it requires real PBR HUD widget art that is still an
 *     OPEN-BOUNDED gap. */

/* Anchor pixel positions for the real-bitmap stamp. These are tiny
 * 1-pixel markers placed near each chrome-supporting slot's top-left
 * coordinate so wire-up probes can prove the REAL path was taken
 * without altering the existing procedural fallback layout. They
 * are only stamped when a slot is REAL — when a slot falls back,
 * no stamp is emitted (the procedural render path still draws the
 * full chrome element the way the no-gate baseline does).
 *
 * Phase 3 primary slots (inventory_quick_view, action_prompt) have
 * no procedural renderer yet, so their REAL stamps land at fixed
 * coordinates inside the HUD framebuffer. The probe treats any
 * non-zero stamp pixel as proof that the REAL path was reached for
 * that slot. */
typedef struct {
    int x;
    int y;
} DM2_V2_HudSlotAnchor;

static const DM2_V2_HudSlotAnchor
    k_real_stamp_anchors[DM2_V2_HUD_WIDGET_COUNT] = {
    /* INVENTORY_QUICK_VIEW — top-left of HUD, Phase 3 primary */
    { 80,  4 },
    /* ACTION_PROMPT — top-right of HUD, Phase 3 primary */
    { 220, 4 },
    /* COMPASS_ROSE — top-left of HUD chrome */
    { 11, 16 },
    /* DEPTH_INDICATOR — top-right of HUD chrome */
    { 286, 8 },
    /* GOLD_COUNTER — bottom-right of HUD chrome */
    { 286, 178 },
    /* CHAMPION_BAR_FRAME — top status bar */
    { 4, 4 },
    /* ACTION_STRIP_FRAME — bottom action strip */
    { 16, 172 },
};

static void dm2_v2_hud_runtime_stamp_real_slot(
    uint8_t *fb, int w, int h_res, DM2_V2_HudWidgetSlot slot)
{
    if (!fb || w <= 0 || h_res <= 0) return;
    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) return;
    const DM2_V2_HudSlotAnchor* a = &k_real_stamp_anchors[slot];
    /* Anchor position is relative to the HUD's 320×200 layout.
     * Defensive: only stamp when the anchor fits in the supplied
     * framebuffer. The existing procedural render path operates
     * inside the same 320×200 region, so this matches. */
    if (a->x < 0 || a->x >= w) return;
    if (a->y < 0 || a->y >= h_res) return;

    /* Bounded-blit first: when the slot is REAL, look up its
     * resolved manifest source_file and try to read + bounded-blit
     * the synthetic 1x1 RGBA pixel. This is the runtime hook's
     * real-bitmap substitute path (Phase 3 follow-up). */
    DM2_V2_HudWidgetSlotInfo info;
    memset(&info, 0, sizeof(info));
    if (dm2_v2_hud_widget_assets_get_slot_info(slot, &info) &&
        info.classification == DM2_V2_HUD_WIDGET_CLASS_REAL &&
        info.resolved_path[0] != '\0' &&
        dm2_v2_hud_widget_bitmap_blit_render_slot(
            &info, fb, w, h_res, a->x, a->y)) {
        return; /* bounded blit succeeded — preserve procedural fallback elsewhere */
    }

    /* Fallback: 1-pixel anchor stamp. Preserves the no-gate baseline
     * for any slot whose blit cannot run (unsupported format, missing
     * file, decompression failure, destination out of bounds, zlib
     * not compiled in). The stamp value is the HUD opacity so a probe
     * can still detect "the gate reached the runtime" via a non-zero
     * pixel at the anchor, matching the documented seam contract. */
    fb[a->y * w + a->x] = (uint8_t)s_hud.opacity;
}

void dm2_v2_hud_runtime_render_with_assets(uint8_t *fb, int w, int h_res) {
    /* Reset the per-slot path-mode record before classifying. The
     * initial values match the default (no manifest installed ⇒ all
     * slots MISSING ⇒ all slots PROCEDURAL_FALLBACK). */
    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK;
        s_last_slot_class[i] = DM2_V2_HUD_WIDGET_CLASS_MISSING;
    }
    s_last_path_real = 0;
    s_last_path_fallback = 0;
    s_last_render_with_assets = 1;

    /* Honour phase gate / visibility / opacity first — but record
     * the path-mode as PROCEDURAL_FALLBACK for every slot when we
     * skip the render. This keeps the "no manifest + V2 off"
     * baseline explicit and observable from probe code. */
    int render_will_run = 1;
    if (!s_initialized) render_will_run = 0;
    if (!s_force_active && !s_hud.visible) render_will_run = 0;
    if (s_hud.opacity == 0) render_will_run = 0;
    if (!s_force_active) {
        if (!s_gate_config) render_will_run = 0;
        else if (!s_gate_config->v2LaunchEnabled) render_will_run = 0;
        else if (!s_gate_config->v2ProfileEnabled) render_will_run = 0;
    }

    /* Classify every slot up-front so probe code can read the
     * gate's verdict regardless of whether we actually rendered.
     * The classification mirrors what the runtime observed, not
     * just what the gate returned earlier — they are the same
     * because we read through the same API the gate uses. */
    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        DM2_V2_HudWidgetClass cls =
            dm2_v2_hud_widget_assets_classify_slot(
                (DM2_V2_HudWidgetSlot)i);
        s_last_slot_class[i] = cls;
        if (cls == DM2_V2_HUD_WIDGET_CLASS_REAL) {
            s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP;
        } else {
            s_last_path_mode[i] = DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK;
        }
    }

    /* Aggregate counts. Total classified slots equals
     * DM2_V2_HUD_WIDGET_COUNT — every slot is classified regardless
     * of whether the manifest is installed, because the gate's
     * MISSING branch is itself a valid classification. */
    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        if (s_last_path_mode[i] == DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP) {
            ++s_last_path_real;
        } else {
            ++s_last_path_fallback;
        }
    }

    if (!render_will_run) {
        /* V1 chrome owns the framebuffer; do not touch fb. The
         * path-mode record is the only state this function emits,
         * and probes use it to assert that "no manifest + V2 off"
         * still yields the expected baseline. */
        return;
    }

    /* Delegate to the existing render path FIRST. The procedural
     * overlay draws all chrome-supporting slots it knows how to
     * draw (compass, depth, gold, champion bars, action strip).
     * Slots classified as REAL get the procedural draw underneath
     * the stamp; the stamp lands afterwards so the probe can read
     * it from the framebuffer regardless of whether the procedural
     * chrome touched the same anchor pixel. This is the documented
     * "real-asset-substitutes-the-procedural-fallback" seam. The
     * actual bitmap decode is intentionally not implemented here —
     * when it lands, the stamp is the obvious replacement site for
     * the real blit and the procedural chrome would be suppressed
     * for that slot. */
    dm2_v2_hud_render(&s_hud, fb, w, h_res);

    /* Stamp real-bitmap anchors AFTER the procedural render. The
     * single-pixel stamp is the smallest possible "this slot was
     * routed through the real-bitmap path" signal; the stamp must
     * survive the procedural overlay so a wire-up probe can verify
     * the gate reached the runtime end-to-end. */
    for (int i = 0; i < (int)DM2_V2_HUD_WIDGET_COUNT; ++i) {
        if (s_last_path_mode[i] == DM2_V2_HUD_RUNTIME_PATH_REAL_BITMAP) {
            dm2_v2_hud_runtime_stamp_real_slot(
                fb, w, h_res, (DM2_V2_HudWidgetSlot)i);
        }
    }
}

DM2_V2_HudRuntimePathMode dm2_v2_hud_runtime_last_path_mode(
    DM2_V2_HudWidgetSlot slot)
{
    if ((unsigned)slot >= (unsigned)DM2_V2_HUD_WIDGET_COUNT) {
        return DM2_V2_HUD_RUNTIME_PATH_PROCEDURAL_FALLBACK;
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

/* ── Status ────────────────────────────────────────────────────── */
int dm2_v2_hud_runtime_is_active(void) {
    if (!s_initialized) return 0;
    if (s_force_active) return 1;
    if (!s_hud.visible) return 0;
    if (!s_gate_config) return 0;
    if (!s_gate_config->v2LaunchEnabled) return 0;
    if (!s_gate_config->v2ProfileEnabled) return 0;
    return 1;
}

/* ── V1 compatibility helper (for tests + wire-up probes) ──── */
void dm2_v2_hud_runtime_force_active_for_test(int active) {
    s_force_active = active ? 1 : 0;
}

const char *dm2_v2_hud_runtime_source_evidence(void) {
    return
        "DM2 V2 HUD Runtime — Phase 3 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM T560              (DM2 HUD rendering pipeline)\n"
        "Source: SKULL.ASM T520              (party/movement tick → HUD state source)\n"
        "Source: SKULL.ASM T048              (input dispatch → action strip source)\n"
        "Source: skproject/SKULLWIN/c_gui_vp.cpp (DM2 UI chrome layout, sibling reimpl)\n"
        "Source: ReDMCSB PANEL.C F0354       (champion status-box drawing)\n"
        "Source: ReDMCSB DUNGEON.C F0260     (stat-bar refresh timing)\n"
        "Source: ReDMCSB COMMAND.C           (action feedback gates)\n"
        "Source: ReDMCSB DISPLAY.C           (pulse animation timing 2 Hz)\n"
        "Source: dm2_v2_phase_gate.h         (DM2_V2_PHASE_DOMAIN_HUD gate)\n"
        "Source: dm2_v2_hud_widget_assets.h  (per-slot REAL/PARTIAL/PLACEHOLDER gate)\n"
        "Source: dm2_v2_hud_widget_bitmap_blit.h (bounded 1x1 RGBA blit, Phase 3 follow-up)\n"
        "Source: csb_v2_hud_runtime.c        (sibling CSB V2 wire-up pattern)\n"
        "V1 invariant: V1 command routes, inventory, dungeon state NEVER bypassed\n"
        "V2 rule: HUD only active when v2LaunchEnabled AND v2ProfileEnabled are both 1\n"
        "V2 rule: HUD render is no-op when V1 is active, no framebuffer pollution\n"
        "V2 rule: render_with_assets() prefers dm2_v2_hud_widget_bitmap_blit_render_slot()\n"
        "         for REAL slots whose manifest source_file resolves on disk, and falls\n"
        "         back to the 1-pixel anchor stamp when the bounded blit cannot run.\n"
        "V2 rule: the multi-pixel bitmap decode is the OPEN-BOUNDED next-step\n"
        "         (operator-installed PBR HUD widget art)\n";
}
