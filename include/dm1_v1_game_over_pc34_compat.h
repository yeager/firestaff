#ifndef FIRESTAFF_DM1_V1_GAME_OVER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GAME_OVER_PC34_COMPAT_H

/* DM1 V1 Game Over + Death Effects — source-locked from ReDMCSB
 * DEATHEFF.C F0139: G0551_DEATHEFF_DeathEffect — fade + skeleton animation
 * DEATHEFF.C F0141: Death screen text rendering
 * Title screen animation follows TITLE.C swoosh pattern
 * Game over renders champion death animation then final stats
 *
 * Death effect:
 * - 30-frame skeleton animation
 * - Progressive screen fade (black → red tint → black)
 * - Text: game over message, champion stats, area reached
 *
 * Title screen:
 * - Logo swoosh animation (Daniel bug #1 target)
 * - Fade in/out sequence
 * - "Press any key" prompt
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   ENDGAME.C: F0444_STARTEND_Endgame(P0856_B_DoNotDrawCreditsOnly)
 *     If party won: show champion portraits + stats + credits
 *     If party dead: show death screen + restart option
 *     Champion skills: iterate C00_SKILL_FIGHTER..C03_SKILL_WIZARD,
 *       find highest level title, display "TITLE CHAMPION_NAME"
 *     L1423_B_WaitBeforeDrawingRestart: delay before restart prompt
 *     Restart flow: G0523_B_RestartGameRequested + longjmp(G2023_jmp_buf)
 *   ENDGAME.C: F0443_STARTEND_EndgamePrintString(x, y, color, string)
 *     Converts A-Z to scroll font (char -= 64)
 *     Calls F0053_TEXT_PrintToLogicalScreen
 *   TITLE.C: F0437_STARTEND_DrawTitle
 *     18-step zoom swoosh of "Dungeon"/"Chaos" text
 *     Steps: 48×12 → 320×80 in 16px/4px increments
 *     Palette: dark blue → gold/brown/red/yellow → fade
 *     Double-buffer: alternate L1386_apuc[0]/[1] screens
 *     "Master"/"Strikes Back" subtitle: separate blit
 *     "FTL Games Presents" at y=137 with white-on-blue palette
 *   TITLE.C: Memory check: requires ~133KB (ST) / ~146KB (Amiga)
 *     If insufficient: silently skip title animation
 *   Key globals:
 *     G0302_B_GameWon — victory flag
 *     G0303_i_PartyDeath — party dead flag
 *     G0524_B_RestartGameAllowed — restart permitted
 *     G0523_B_RestartGameRequested — restart requested
 *     G2023_jmp_buf — longjmp target for restart
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Death effect animation parameters ────────────────────────────── */
#define DM1_DEATH_ANIM_FRAMES           30    /* Skeleton animation frames */
#define DM1_DEATH_FADE_STEPS            16    /* Screen fade steps */
#define DM1_DEATH_RED_TINT_INTENSITY    0x0F  /* Red channel for death tint */

/* ── Title swoosh animation parameters (from TITLE.C) ─────────────── */
#define DM1_TITLE_ZOOM_STEPS            18    /* 18 zoom levels */
#define DM1_TITLE_INITIAL_WIDTH         48    /* First zoom step width */
#define DM1_TITLE_INITIAL_HEIGHT        12    /* First zoom step height */
#define DM1_TITLE_WIDTH_INCREMENT       16    /* Width increase per step */
#define DM1_TITLE_HEIGHT_INCREMENT       4    /* Height increase per step */
#define DM1_TITLE_FINAL_WIDTH          320    /* Full resolution width */
#define DM1_TITLE_FINAL_HEIGHT          80    /* Full resolution height */
#define DM1_TITLE_MEMORY_REQUIRED   133056    /* Minimum memory (Atari ST) */
#define DM1_TITLE_MEMORY_AMIGA      145856    /* Minimum memory (Amiga) */

/* ── Title palette colors (from TITLE.C palette setup) ────────────── */
#define DM1_RGB_DARK_BLUE       0x0002  /* M501 */
#define DM1_RGB_LIGHT_BROWN     0x0642  /* M502 */
#define DM1_RGB_LIGHTER_BROWN   0x0862  /* M504 */
#define DM1_RGB_DARK_GOLD       0x0620  /* M505 */
#define DM1_RGB_GOLD            0x0840  /* M506 */
#define DM1_RGB_RED             0x0A00  /* M507 */
#define DM1_RGB_YELLOW          0x0AA0  /* M508 */
#define DM1_RGB_WHITE           0x0FFF  /* M509 */

/* ── Death effect state ───────────────────────────────────────────── */
typedef enum {
    DM1_DEATH_IDLE = 0,
    DM1_DEATH_SKELETON_ANIM,     /* Playing skeleton animation */
    DM1_DEATH_FADE_RED,          /* Fading to red tint */
    DM1_DEATH_FADE_BLACK,        /* Fading to black */
    DM1_DEATH_SHOW_TEXT,         /* Displaying game over text */
    DM1_DEATH_SHOW_STATS,        /* Displaying champion stats */
    DM1_DEATH_WAIT_RESTART,      /* Waiting for restart input */
    DM1_DEATH_COMPLETE           /* Animation complete */
} M11_DeathEffectPhase;

/* ── Title screen state ───────────────────────────────────────────── */
typedef enum {
    DM1_TITLE_IDLE = 0,
    DM1_TITLE_FADE_IN_PRESENTS,  /* "FTL Games Presents" fade in */
    DM1_TITLE_PRESENTS_DISPLAY,  /* Hold presents text */
    DM1_TITLE_FADE_OUT_PRESENTS, /* Fade out presents */
    DM1_TITLE_ZOOM_SWOOSH,       /* 18-step zoom animation */
    DM1_TITLE_SUBTITLE_BLIT,     /* "Master"/"Strikes Back" */
    DM1_TITLE_HOLD_DISPLAY,      /* Hold completed title */
    DM1_TITLE_FADE_OUT,          /* Fade to gameplay */
    DM1_TITLE_COMPLETE           /* Title sequence done */
} M11_TitlePhase;

/* ── Champion endgame stats (from F0444 champion loop) ────────────── */
typedef struct {
    char name[8];                   /* Champion name (7+NUL) */
    char title[20];                 /* Skill title ("ARCH MASTER") */
    int  skillLevels[4];            /* Fighter, Ninja, Priest, Wizard levels */
    int  highestSkillIndex;         /* Which skill was highest */
    int  alive;                     /* Was champion alive at end */
} M11_ChampionEndgameStats;

/* ── Death effect persistent state ────────────────────────────────── */
typedef struct {
    M11_DeathEffectPhase phase;
    int currentFrame;               /* Current animation frame */
    int fadeStep;                   /* Current fade step (0-15) */
    int restartAllowed;             /* G0524 */
    int restartRequested;           /* G0523 */
    int gameWon;                    /* G0302 */
    int waitBeforeRestart;          /* L1423 from F0444 */
    uint32_t phaseStartMs;          /* Timestamp when phase started */

    /* Champion stats for display */
    M11_ChampionEndgameStats champions[4];
    int championCount;

    /* Text to display */
    char gameOverMessage[64];
    char areaReachedText[64];
} M11_DeathEffectState;

/* ── Title screen persistent state ────────────────────────────────── */
typedef struct {
    M11_TitlePhase phase;
    int currentZoomStep;            /* 0..17 zoom level */
    int frameCounter;               /* Frames in current phase */
    uint32_t phaseStartMs;          /* Timestamp when phase started */
    int hasEnoughMemory;            /* Memory check passed */

    /* Palette state (16 entries) */
    uint16_t palette[16];

    /* Zoom step dimensions (precomputed) */
    struct {
        int16_t width;
        int16_t height;
        int16_t x;                  /* Centered X */
        int16_t y;                  /* Centered Y */
    } zoomSteps[DM1_TITLE_ZOOM_STEPS];
} M11_TitleScreenState;

/* ── Death effect API ─────────────────────────────────────────────── */

/* Initialize death effect state. */
void m11_death_effect_init(M11_DeathEffectState *state);

/* Start the death effect animation sequence. */
void m11_death_effect_start(M11_DeathEffectState *state, int gameWon,
                            int restartAllowed);

/* Advance death effect by one frame. Returns current phase. */
M11_DeathEffectPhase m11_death_effect_tick(M11_DeathEffectState *state,
                                            uint32_t nowMs);

/* Set champion stats for endgame display. */
void m11_death_effect_set_champion(M11_DeathEffectState *state,
                                    int index,
                                    const char *name,
                                    const int skillLevels[4],
                                    int alive);

/* Get the fade intensity (0=full color, 15=black). */
int m11_death_effect_fade_level(const M11_DeathEffectState *state);

/* Check if restart was requested. */
int m11_death_effect_restart_requested(const M11_DeathEffectState *state);

/* Check if death sequence is complete. */
int m11_death_effect_is_complete(const M11_DeathEffectState *state);

/* ── Title screen API ─────────────────────────────────────────────── */

/* Initialize title screen state. availableMemory = heap bytes. */
void m11_title_screen_init(M11_TitleScreenState *state,
                           uint32_t availableMemory);

/* Start the title screen animation. */
void m11_title_screen_start(M11_TitleScreenState *state);

/* Advance title screen by one frame. Returns current phase. */
M11_TitlePhase m11_title_screen_tick(M11_TitleScreenState *state,
                                      uint32_t nowMs);

/* Get current zoom step dimensions for rendering. */
void m11_title_screen_get_zoom(const M11_TitleScreenState *state,
                                int *width, int *height,
                                int *x, int *y);

/* Get current palette for rendering. */
const uint16_t *m11_title_screen_get_palette(const M11_TitleScreenState *state);

/* Check if title sequence is complete. */
int m11_title_screen_is_complete(const M11_TitleScreenState *state);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *m11_game_over_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_GAME_OVER_PC34_COMPAT_H */
