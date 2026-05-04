#ifndef FIRESTAFF_DM1_V1_ENTRANCE_CHAMPION_SELECT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ENTRANCE_CHAMPION_SELECT_PC34_COMPAT_H

/*
 * DM1 V1 Entrance & Champion Selection — source-locked to ReDMCSB ENTRANCE.C
 *
 * Mirror hall champion selection, resurrection, reincarnation, door animation.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   ENTRANCE.C: F0438 (open entrance doors), F0439 (draw entrance door),
 *               F0797 (draw micro dungeon behind doors),
 *               F0440/F0441 (champion selection/mirror interaction)
 *   CHAMPION.C: F0280 (add champion to party from mirror)
 *   REVIVE.C:   resurrection/reincarnation flows
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum champions in DM1 — 4 party slots, 24 mirrors total */
#define M11_MAX_CHAMPIONS          4
#define M11_MAX_MIRROR_SLOTS      24

/* Entrance states */
typedef enum {
    DM1_ENTRANCE_IDLE = 0,        /* Not in entrance */
    DM1_ENTRANCE_DOOR_OPENING,    /* Door animation playing */
    DM1_ENTRANCE_VIEWING,         /* Viewing mirror hall */
    DM1_ENTRANCE_SELECTING,       /* Clicked on a mirror, viewing champion */
    DM1_ENTRANCE_RESURRECTING,    /* Resurrect dialog active */
    DM1_ENTRANCE_REINCARNATING,   /* Reincarnate dialog active */
    DM1_ENTRANCE_DONE             /* All champions selected, entering dungeon */
} M11_EntranceState;

/* Mirror slot state */
typedef struct {
    int occupied;           /* 1 if a champion is in this mirror */
    int championIndex;      /* index into champion data, -1 if empty */
    int selected;           /* 1 if already recruited to party */
    int dead;               /* 1 if bones (needs resurrect/reincarnate) */
    int mapX;               /* mirror position X */
    int mapY;               /* mirror position Y */
    int facing;             /* direction mirror faces (0-3) */
} M11_MirrorSlot;

/* Door animation state */
typedef struct {
    int animationStep;      /* current step (0..9 for 10 frames) */
    int totalSteps;         /* 10 in original */
    int frameDelayMs;       /* delay between frames */
    uint32_t lastFrameMs;   /* timestamp of last frame */
    int complete;           /* 1 when animation finished */
} M11_DoorAnimation;

/* Entrance persistent state */
typedef struct {
    M11_EntranceState state;
    M11_MirrorSlot mirrors[M11_MAX_MIRROR_SLOTS];
    int mirrorCount;                    /* actual number of occupied mirrors */
    int selectedMirrorIndex;            /* currently viewed mirror, -1 if none */
    int partyChampionCount;             /* champions recruited so far */
    int partyChampionIndices[M11_MAX_CHAMPIONS]; /* recruited champion indices */
    M11_DoorAnimation doorAnim;
    int microDungeonBuilt;              /* F0797 micro dungeon constructed */
    uint32_t lastInteractionMs;
} M11_EntranceCtx;

/* Result of an entrance tick */
typedef struct {
    int stateChanged;                   /* entrance state transitioned */
    M11_EntranceState newState;
    int doorAnimationAdvanced;          /* door frame progressed */
    int doorAnimationComplete;
    int mirrorSelected;                 /* a mirror was clicked */
    int mirrorIndex;                    /* which mirror */
    int championRecruited;              /* a champion joined the party */
    int championIndex;                  /* which champion */
    int needsRedraw;                    /* screen needs refresh */
    int entranceComplete;               /* all done, enter dungeon */
} M11_EntranceTickResult;

/*
 * Initialize entrance context. Call when entering the mirror hall.
 * mirrorData: array of mirror slot descriptions from dungeon data.
 */
void m11_entrance_init(M11_EntranceCtx *ctx);

/*
 * Add a mirror slot (champion in a mirror).
 * Returns slot index or -1 if full.
 */
int m11_entrance_add_mirror(M11_EntranceCtx *ctx, int championIndex,
                            int mapX, int mapY, int facing, int isDead);

/*
 * Start the door opening animation (F0438).
 */
void m11_entrance_start_door_animation(M11_EntranceCtx *ctx, uint32_t nowMs);

/*
 * Tick the door animation. Returns 1 if a new frame is ready.
 */
int m11_entrance_tick_door_animation(M11_EntranceCtx *ctx, uint32_t nowMs);

/*
 * Handle a click on a mirror (F0440/F0441).
 * mirrorIndex: which mirror was clicked.
 * Returns result with selection info.
 */
M11_EntranceTickResult m11_entrance_click_mirror(M11_EntranceCtx *ctx,
                                                  int mirrorIndex,
                                                  uint32_t nowMs);

/*
 * Recruit the currently selected champion to the party (F0280).
 * Returns 1 on success, 0 if party full or no champion selected.
 */
int m11_entrance_recruit_champion(M11_EntranceCtx *ctx);

/*
 * Attempt resurrection of the selected dead champion.
 * Returns 1 on success, 0 if not applicable.
 */
int m11_entrance_resurrect(M11_EntranceCtx *ctx);

/*
 * Attempt reincarnation of the selected dead champion.
 * Returns 1 on success, 0 if not applicable.
 */
int m11_entrance_reincarnate(M11_EntranceCtx *ctx);

/*
 * Cancel current selection, return to viewing state.
 */
void m11_entrance_cancel_selection(M11_EntranceCtx *ctx);

/*
 * Finalize entrance — mark all done, transition to dungeon.
 * Call when player has selected champions and walks past the hall.
 */
M11_EntranceTickResult m11_entrance_finalize(M11_EntranceCtx *ctx);

/*
 * Query: is entrance complete?
 */
int m11_entrance_is_complete(const M11_EntranceCtx *ctx);

/*
 * Get current party count in entrance.
 */
int m11_entrance_get_party_count(const M11_EntranceCtx *ctx);

/*
 * Source evidence string.
 */
const char *m11_entrance_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ENTRANCE_CHAMPION_SELECT_PC34_COMPAT_H */
