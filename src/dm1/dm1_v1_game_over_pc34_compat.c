#include "dm1_v1_game_over_pc34_compat.h"
#include <stdio.h>
#include <string.h>

/*
 * DM1 V1 Game Over + Death Effects + Title Screen — implementation
 *
 * Source lock: ReDMCSB WIP20210206
 *   ENDGAME.C: F0444_STARTEND_Endgame(P0856_B_DoNotDrawCreditsOnly)
 *     Phase 1: If !P0856 (credits only): draw "THE END" bitmap
 *     Phase 2: Fill screen black, fade to dark blue palette
 *     Phase 3: For each champion (alive): draw mirrored portrait,
 *       print name, then for each skill (Fighter/Ninja/Priest/Wizard):
 *       determine highest skill level, look up title string,
 *       convert A-Z to scroll font (char -= 64), print with F0053
 *     Phase 4: Fade to palette, wait for input
 *     Phase 5: If party dead + restart allowed:
 *       Draw "RESTART THIS GAME" / "RESUME SAVED GAME" buttons
 *       Wait for click: if restart → G0523=TRUE, longjmp(G2023)
 *     Key: F0443_EndgamePrintString converts uppercase to scroll font
 *
 *   TITLE.C: F0437_STARTEND_DrawTitle
 *     Memory check: need ~133KB (ST) or ~146KB (Amiga) free heap
 *     If insufficient: return silently (skip title animation)
 *     Phase 1: Allocate title bitmap (320×200 @ 4bpp) + all zoom steps
 *     Phase 2: Load + decompress title graphic (C001_GRAPHIC_TITLE)
 *     Phase 3: "FTL Games Presents" at y=137 with palette[15]=white
 *     Phase 4: Extract "Master"/"Strikes Back" subtitle (320×57)
 *     Phase 5: Amiga: extract "Dungeon"/"Chaos" top part (320×80)
 *     Phase 6: Generate 18 zoom steps via F0129_VIDEO_BlitShrinkWithPaletteChanges:
 *       Step 0: 48×12 centered at (136,74)
 *       Step 17: 320×80 centered at (0,40)
 *       Each step: +16 width, +4 height
 *     Phase 7: Set title palette (dark blue + gold/brown/red/yellow)
 *     Phase 8: Animate zoom (swap double-buffers each step)
 *     Phase 9: Blit subtitle "Master"/"Strikes Back"
 *     Phase 10: Fade to dark blue, then gameplay palette
 */

/* ── Skill title lookup (from ENDGAME.C champion display) ─────────── */
static const char *s_skill_titles_by_level[] = {
    /* Level 0 */ "NEOPHYTE",
    /* Level 1 */ "NOVICE",
    /* Level 2 */ "APPRENTICE",
    /* Level 3 */ "JOURNEYMAN",
    /* Level 4 */ "CRAFTSMAN",
    /* Level 5 */ "ARTISAN",
    /* Level 6 */ "ADEPT",
    /* Level 7 */ "EXPERT",
    /* Level 8 */ "---LO MASTER",
    /* Level 9 */ "---UM MASTER",
    /* Level 10 */ "---HI MASTER",
    /* Level 11 */ "---ON MASTER",
    /* Level 12 */ "ARCH MASTER",
    /* Level 13 */ "ARCH MASTER",
    /* Level 14 */ "ARCH MASTER",
    /* Level 15 */ "ARCH MASTER"
};

static const char *s_skill_names[4] = {
    "FIGHTER", "NINJA", "PRIEST", "WIZARD"
};

/* ── Death effect ─────────────────────────────────────────────────── */

void m11_death_effect_init(M11_DeathEffectState *state)
{
    memset(state, 0, sizeof(*state));
    state->phase = DM1_DEATH_IDLE;
}

void m11_death_effect_start(M11_DeathEffectState *state, int gameWon,
                            int restartAllowed)
{
    state->gameWon = gameWon;
    state->restartAllowed = restartAllowed;
    state->restartRequested = 0;
    state->currentFrame = 0;
    state->fadeStep = 0;
    state->waitBeforeRestart = 0;

    if (gameWon) {
        /* Victory: skip skeleton, go straight to stats display */
        state->phase = DM1_DEATH_SHOW_STATS;
    } else {
        /* Death: start skeleton animation */
        state->phase = DM1_DEATH_SKELETON_ANIM;
    }
}

M11_DeathEffectPhase m11_death_effect_tick(M11_DeathEffectState *state,
                                            uint32_t nowMs)
{
    switch (state->phase) {
        case DM1_DEATH_IDLE:
            break;

        case DM1_DEATH_SKELETON_ANIM:
            /*
             * 30-frame skeleton animation.
             * Each frame is ~66ms (50fps * 30 frames = 600ms total)
             */
            state->currentFrame++;
            if (state->currentFrame >= DM1_DEATH_ANIM_FRAMES) {
                state->phase = DM1_DEATH_FADE_RED;
                state->fadeStep = 0;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_DEATH_FADE_RED:
            /* Progressive red tint (8 steps, ~40ms each = 320ms) */
            state->fadeStep++;
            if (state->fadeStep >= DM1_DEATH_FADE_STEPS / 2) {
                state->phase = DM1_DEATH_FADE_BLACK;
                state->fadeStep = 0;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_DEATH_FADE_BLACK:
            /* Fade to black (8 steps, ~40ms each = 320ms) */
            state->fadeStep++;
            if (state->fadeStep >= DM1_DEATH_FADE_STEPS / 2) {
                state->phase = DM1_DEATH_SHOW_TEXT;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_DEATH_SHOW_TEXT:
            /* Display game over message for ~2 seconds */
            if (nowMs - state->phaseStartMs >= 2000) {
                state->phase = DM1_DEATH_SHOW_STATS;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_DEATH_SHOW_STATS:
            /*
             * F0444: iterate champions, display stats.
             * Hold for ~3 seconds for victory, or proceed to restart.
             */
            if (state->gameWon) {
                if (nowMs - state->phaseStartMs >= 3000) {
                    state->phase = DM1_DEATH_COMPLETE;
                }
            } else {
                if (nowMs - state->phaseStartMs >= 1000) {
                    if (state->restartAllowed) {
                        state->phase = DM1_DEATH_WAIT_RESTART;
                        state->phaseStartMs = nowMs;
                    } else {
                        state->phase = DM1_DEATH_COMPLETE;
                    }
                }
            }
            break;

        case DM1_DEATH_WAIT_RESTART:
            /* Waiting for player to choose restart or resume */
            /* L1423_B_WaitBeforeDrawingRestart: brief delay before showing */
            if (!state->waitBeforeRestart &&
                (nowMs - state->phaseStartMs >= 500)) {
                state->waitBeforeRestart = 1;
            }
            /* Actual restart is triggered by caller calling
             * m11_death_effect_restart_requested after user input */
            break;

        case DM1_DEATH_COMPLETE:
            break;
    }

    return state->phase;
}

void m11_death_effect_set_champion(M11_DeathEffectState *state,
                                    int index,
                                    const char *name,
                                    const int skillLevels[4],
                                    int alive)
{
    if (index < 0 || index >= 4) return;

    M11_ChampionEndgameStats *cs = &state->champions[index];
    memset(cs, 0, sizeof(*cs));

    /* Copy name (max 7 chars) */
    if (name) {
        int i;
        for (i = 0; i < 7 && name[i]; i++) {
            cs->name[i] = name[i];
        }
        cs->name[i] = '\0';
    }

    /* Copy skill levels and find highest */
    int highestLevel = -1;
    int highestIdx = 0;
    for (int s = 0; s < 4; s++) {
        cs->skillLevels[s] = skillLevels[s];
        if (skillLevels[s] > highestLevel) {
            highestLevel = skillLevels[s];
            highestIdx = s;
        }
    }
    cs->highestSkillIndex = highestIdx;
    cs->alive = alive;

    /* Build title string: "TITLE SKILL_NAME" */
    int level = highestLevel;
    if (level < 0) level = 0;
    if (level > 15) level = 15;
    const char *titleStr = s_skill_titles_by_level[level];
    const char *skillStr = s_skill_names[highestIdx];

    /* If title starts with "---", replace prefix with specific skill name */
    if (titleStr[0] == '-' && titleStr[1] == '-' && titleStr[2] == '-') {
        /* Format: "LO/UM/HI/ON MASTER SKILL_NAME" */
        /* Simplified: just use the suffix */
        const char *suffix = titleStr + 3;
        snprintf(cs->title, sizeof(cs->title), "%s %s", suffix, skillStr);
    } else {
        snprintf(cs->title, sizeof(cs->title), "%s %s", titleStr, skillStr);
    }

    if (index >= state->championCount) {
        state->championCount = index + 1;
    }
}

int m11_death_effect_fade_level(const M11_DeathEffectState *state)
{
    return state->fadeStep;
}

int m11_death_effect_restart_requested(const M11_DeathEffectState *state)
{
    return state->restartRequested;
}

int m11_death_effect_is_complete(const M11_DeathEffectState *state)
{
    return state->phase == DM1_DEATH_COMPLETE;
}

/* ── Title screen ─────────────────────────────────────────────────── */

void m11_title_screen_init(M11_TitleScreenState *state,
                           uint32_t availableMemory)
{
    memset(state, 0, sizeof(*state));
    state->phase = DM1_TITLE_IDLE;

    /* Memory check: F0437 checks if heap >= required */
    state->hasEnoughMemory = (availableMemory >= DM1_TITLE_MEMORY_REQUIRED);

    /* Precompute zoom step dimensions (from TITLE.C loop) */
    int16_t w = DM1_TITLE_INITIAL_WIDTH;
    int16_t h = DM1_TITLE_INITIAL_HEIGHT;
    for (int i = 0; i < DM1_TITLE_ZOOM_STEPS; i++) {
        state->zoomSteps[i].width = w;
        state->zoomSteps[i].height = h;
        /* Center on screen (320×200, centered at 160×100ish) */
        state->zoomSteps[i].x = (320 - w) / 2;
        state->zoomSteps[i].y = (160 - h) / 2;
        w += DM1_TITLE_WIDTH_INCREMENT;
        h += DM1_TITLE_HEIGHT_INCREMENT;
    }

    /* Initialize palette to dark blue */
    for (int i = 0; i < 16; i++) {
        state->palette[i] = DM1_RGB_DARK_BLUE;
    }
}

void m11_title_screen_start(M11_TitleScreenState *state)
{
    if (!state->hasEnoughMemory) {
        /* Silently skip title (matches TITLE.C behavior) */
        state->phase = DM1_TITLE_COMPLETE;
        return;
    }

    state->phase = DM1_TITLE_FADE_IN_PRESENTS;
    state->currentZoomStep = 0;
    state->frameCounter = 0;
    state->phaseStartMs = 0;

    /* Set initial palette: dark blue + white for "Presents" text */
    for (int i = 0; i < 16; i++) {
        state->palette[i] = DM1_RGB_DARK_BLUE;
    }
    state->palette[15] = DM1_RGB_WHITE;
}

M11_TitlePhase m11_title_screen_tick(M11_TitleScreenState *state,
                                      uint32_t nowMs)
{
    if (state->phaseStartMs == 0) {
        state->phaseStartMs = nowMs;
    }

    uint32_t elapsed = nowMs - state->phaseStartMs;

    switch (state->phase) {
        case DM1_TITLE_IDLE:
            break;

        case DM1_TITLE_FADE_IN_PRESENTS:
            /* "FTL Games Presents" fade in over ~1 second */
            if (elapsed >= 1000) {
                state->phase = DM1_TITLE_PRESENTS_DISPLAY;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_TITLE_PRESENTS_DISPLAY:
            /* Hold "Presents" for ~2 seconds */
            if (elapsed >= 2000) {
                state->phase = DM1_TITLE_FADE_OUT_PRESENTS;
                state->phaseStartMs = nowMs;
                /* Fade palette[15] from white to dark blue */
                state->palette[15] = DM1_RGB_DARK_BLUE;
            }
            break;

        case DM1_TITLE_FADE_OUT_PRESENTS:
            /* Fade out "Presents" + set up title palette */
            if (elapsed >= 500) {
                /* Set title palette (from TITLE.C) */
                state->palette[3] = DM1_RGB_DARK_GOLD;
                state->palette[4] = DM1_RGB_LIGHT_BROWN;
                state->palette[5] = DM1_RGB_GOLD;
                state->palette[6] = DM1_RGB_LIGHTER_BROWN;
                state->palette[8] = DM1_RGB_YELLOW;
                state->palette[15] = DM1_RGB_RED;
                state->palette[10] = DM1_RGB_DARK_BLUE;
                state->palette[12] = DM1_RGB_DARK_BLUE;

                state->phase = DM1_TITLE_ZOOM_SWOOSH;
                state->currentZoomStep = 0;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_TITLE_ZOOM_SWOOSH:
            /*
             * 18-step zoom animation.
             * Each step held for ~3 frames at 50fps = 60ms.
             * Total zoom: ~1080ms.
             * Uses double-buffer swap between screens.
             */
            state->frameCounter++;
            if (state->frameCounter >= 3) {
                state->frameCounter = 0;
                state->currentZoomStep++;
                if (state->currentZoomStep >= DM1_TITLE_ZOOM_STEPS) {
                    state->phase = DM1_TITLE_SUBTITLE_BLIT;
                    state->phaseStartMs = nowMs;
                }
            }
            break;

        case DM1_TITLE_SUBTITLE_BLIT:
            /* Blit "Master"/"Strikes Back" subtitle (~500ms hold) */
            if (elapsed >= 500) {
                state->phase = DM1_TITLE_HOLD_DISPLAY;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_TITLE_HOLD_DISPLAY:
            /* Hold completed title for ~2 seconds */
            if (elapsed >= 2000) {
                state->phase = DM1_TITLE_FADE_OUT;
                state->phaseStartMs = nowMs;
            }
            break;

        case DM1_TITLE_FADE_OUT:
            /* Fade to gameplay palette (~500ms) */
            if (elapsed >= 500) {
                state->phase = DM1_TITLE_COMPLETE;
            }
            break;

        case DM1_TITLE_COMPLETE:
            break;
    }

    return state->phase;
}

void m11_title_screen_get_zoom(const M11_TitleScreenState *state,
                                int *width, int *height,
                                int *x, int *y)
{
    int step = state->currentZoomStep;
    if (step < 0) step = 0;
    if (step >= DM1_TITLE_ZOOM_STEPS) step = DM1_TITLE_ZOOM_STEPS - 1;

    if (width) *width = state->zoomSteps[step].width;
    if (height) *height = state->zoomSteps[step].height;
    if (x) *x = state->zoomSteps[step].x;
    if (y) *y = state->zoomSteps[step].y;
}

const uint16_t *m11_title_screen_get_palette(const M11_TitleScreenState *state)
{
    return state->palette;
}

int m11_title_screen_is_complete(const M11_TitleScreenState *state)
{
    return state->phase == DM1_TITLE_COMPLETE;
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *m11_game_over_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206\n"
        "ENDGAME.C: F0444_STARTEND_Endgame(P0856_B_DoNotDrawCreditsOnly)\n"
        "  If gameWon: champion portraits + skill titles + credits\n"
        "  If partyDead: death screen + restart option\n"
        "  F0443: uppercase→scroll font (char-=64), F0053 print\n"
        "  Champion skills: C00_FIGHTER..C03_WIZARD, title lookup\n"
        "  Restart: G0523=TRUE, longjmp(G2023_jmp_buf)\n"
        "  L1423_B_WaitBeforeDrawingRestart: delay before prompt\n"
        "TITLE.C: F0437_STARTEND_DrawTitle\n"
        "  Memory check: ~133KB(ST)/~146KB(Amiga); skip if insufficient\n"
        "  18-step zoom: 48×12→320×80 (+16w/+4h per step)\n"
        "  F0129_VIDEO_BlitShrinkWithPaletteChanges for each step\n"
        "  Double-buffer: L1386_apuc[0]/[1] alternate screens\n"
        "  Palette: [3]=DARK_GOLD, [4]=LIGHT_BROWN, [5]=GOLD,\n"
        "    [6]=LIGHTER_BROWN, [8]=YELLOW, [15]=RED\n"
        "  'FTL Games Presents' at y=137, palette[15]=WHITE\n"
        "  'Master'/'Strikes Back' subtitle: separate blit 320×57\n"
        "Key globals: G0302 gameWon, G0303 partyDead,\n"
        "  G0523 restartReq, G0524 restartAllowed, G2023 jmp_buf";
}
