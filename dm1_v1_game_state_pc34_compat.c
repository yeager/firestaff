#include "dm1_v1_game_state_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Game State Machine — implementation
 *
 * Source lock: ReDMCSB WIP20210206
 *   STARTUP1.C: F0448_STARTUP1_InitializeMemoryManager_CPSADEF
 *   STARTUP2.C: F0462_START_StartGame_CPSEF
 *     Sets: G0331=0, G0332=0, G0333=0, G0334=0, G0341=0, G0300=0
 *     G0506_ui_ActingChampionOrdinal = NONE
 *     G0509_B_ActionAreaContainsIcons = TRUE
 *     G0441/G0442/G0443/G0444 = default mouse/keyboard input tables
 *     F0003(G0309_i_PartyMapIndex) — process initial map
 *     If !G0298_B_NewGame: show "Game loaded" dialog
 *     If G0298_B_NewGame: F0267 move party to starting position
 *     G0301_B_GameTimeTicking = TRUE
 *   STARTUP2.C: F0463_START_InitializeGame_CPSADEF — full sequence:
 *     F0448 → F0479 → F0460 → F0437 title → F0094/F0095 load sets →
 *     F0054 text → F0031 objects → F0066 mouse → F0441 entrance →
 *     F0435 load loop → F0477/F0396/F0476 → F0462 start → F0478 close
 *   ENDGAME.C: F0444_STARTEND_Endgame — game over with credits
 *   TITLE.C: F0437_STARTEND_DrawTitle — 18-step zoom swoosh animation
 *
 * Valid transitions (derived from control flow):
 *   NONE → TITLE_SCREEN (boot)
 *   TITLE_SCREEN → NEWGAME_INIT | LOADGAME_INIT
 *   NEWGAME_INIT → ENTRANCE_SCREEN
 *   LOADGAME_INIT → ENTRANCE_SCREEN | DUNGEON_VIEWPORT
 *   ENTRANCE_SCREEN → DUNGEON_VIEWPORT
 *   DUNGEON_VIEWPORT → INVENTORY | MAP_SCREEN | RESURRECT | GAME_OVER | VICTORY
 *   INVENTORY → DUNGEON_VIEWPORT
 *   MAP_SCREEN → ENTRANCE_SCREEN
 *   RESURRECT_SCREEN → ENTRANCE_SCREEN
 *   GAME_OVER → RESTART | TITLE_SCREEN
 *   VICTORY → TITLE_SCREEN
 *   RESTART → TITLE_SCREEN
 */

/* ── State name table ─────────────────────────────────────────────── */
static const char *s_state_names[DM1_STATE_COUNT] = {
    "NONE",
    "TITLE_SCREEN",
    "NEWGAME_INIT",
    "LOADGAME_INIT",
    "ENTRANCE_SCREEN",
    "DUNGEON_VIEWPORT",
    "INVENTORY",
    "MAP_SCREEN",
    "RESURRECT_SCREEN",
    "GAME_OVER",
    "VICTORY",
    "RESTART"
};

/* ── Transition validation table ──────────────────────────────────── */
/* Bit flags: which target states are reachable from each source state */

/* For each source state, a bitmask of valid DM1_STATE_* targets */
static uint32_t s_valid_transitions[DM1_STATE_COUNT];

static void init_transition_table(void)
{
    static int initialized = 0;
    if (initialized) return;
    initialized = 1;

    memset(s_valid_transitions, 0, sizeof(s_valid_transitions));

    #define ALLOW(src, dst) s_valid_transitions[src] |= (1u << (dst))

    ALLOW(DM1_STATE_NONE,             DM1_STATE_TITLE_SCREEN);
    ALLOW(DM1_STATE_NONE,             DM1_STATE_NEWGAME_INIT);   /* quick start */
    ALLOW(DM1_STATE_NONE,             DM1_STATE_LOADGAME_INIT);

    ALLOW(DM1_STATE_TITLE_SCREEN,     DM1_STATE_NEWGAME_INIT);
    ALLOW(DM1_STATE_TITLE_SCREEN,     DM1_STATE_LOADGAME_INIT);
    ALLOW(DM1_STATE_TITLE_SCREEN,     DM1_STATE_ENTRANCE_SCREEN);

    ALLOW(DM1_STATE_NEWGAME_INIT,     DM1_STATE_ENTRANCE_SCREEN);

    ALLOW(DM1_STATE_LOADGAME_INIT,    DM1_STATE_ENTRANCE_SCREEN);
    ALLOW(DM1_STATE_LOADGAME_INIT,    DM1_STATE_DUNGEON_VIEWPORT);

    ALLOW(DM1_STATE_ENTRANCE_SCREEN,  DM1_STATE_DUNGEON_VIEWPORT);
    ALLOW(DM1_STATE_ENTRANCE_SCREEN,  DM1_STATE_LOADGAME_INIT);

    ALLOW(DM1_STATE_DUNGEON_VIEWPORT, DM1_STATE_INVENTORY);
    ALLOW(DM1_STATE_DUNGEON_VIEWPORT, DM1_STATE_MAP_SCREEN);
    ALLOW(DM1_STATE_DUNGEON_VIEWPORT, DM1_STATE_RESURRECT_SCREEN);
    ALLOW(DM1_STATE_DUNGEON_VIEWPORT, DM1_STATE_GAME_OVER);
    ALLOW(DM1_STATE_DUNGEON_VIEWPORT, DM1_STATE_VICTORY);

    ALLOW(DM1_STATE_INVENTORY,        DM1_STATE_DUNGEON_VIEWPORT);

    ALLOW(DM1_STATE_MAP_SCREEN,       DM1_STATE_ENTRANCE_SCREEN);
    ALLOW(DM1_STATE_MAP_SCREEN,       DM1_STATE_DUNGEON_VIEWPORT);

    ALLOW(DM1_STATE_RESURRECT_SCREEN, DM1_STATE_ENTRANCE_SCREEN);
    ALLOW(DM1_STATE_RESURRECT_SCREEN, DM1_STATE_DUNGEON_VIEWPORT);

    ALLOW(DM1_STATE_GAME_OVER,        DM1_STATE_RESTART);
    ALLOW(DM1_STATE_GAME_OVER,        DM1_STATE_TITLE_SCREEN);

    ALLOW(DM1_STATE_VICTORY,          DM1_STATE_TITLE_SCREEN);

    ALLOW(DM1_STATE_RESTART,          DM1_STATE_TITLE_SCREEN);
    ALLOW(DM1_STATE_RESTART,          DM1_STATE_NEWGAME_INIT);
    ALLOW(DM1_STATE_RESTART,          DM1_STATE_LOADGAME_INIT);

    #undef ALLOW
}

/* ── Initialization ───────────────────────────────────────────────── */

void m11_game_state_init(M11_GameStateMachine *sm)
{
    memset(sm, 0, sizeof(*sm));
    sm->currentState = DM1_STATE_NONE;
    sm->previousState = DM1_STATE_NONE;
    sm->restartAllowed = 1; /* Default: restart allowed (G0524=TRUE) */
    init_transition_table();
}

void m11_game_state_set_callbacks(M11_GameStateMachine *sm,
                                  M11_StateCallback onEnter,
                                  M11_StateCallback onExit,
                                  void *userdata)
{
    sm->onEnter = onEnter;
    sm->onExit = onExit;
    sm->callbackUserdata = userdata;
}

/* ── Core transition ──────────────────────────────────────────────── */

int m11_game_state_can_transition(const M11_GameStateMachine *sm,
                                  M11_GameStateId target)
{
    init_transition_table();
    if (target >= DM1_STATE_COUNT) return 0;
    return (s_valid_transitions[sm->currentState] >> target) & 1;
}

M11_TransitionResult m11_game_state_transition(M11_GameStateMachine *sm,
                                                M11_GameStateId newState)
{
    if (newState >= DM1_STATE_COUNT) return DM1_TRANS_INVALID;
    if (newState == sm->currentState) return DM1_TRANS_SAME_STATE;

    if (!m11_game_state_can_transition(sm, newState)) {
        return DM1_TRANS_INVALID;
    }

    M11_GameStateId oldState = sm->currentState;

    /* Exit callback */
    if (sm->onExit) {
        sm->onExit(oldState, newState, sm->callbackUserdata);
    }

    sm->previousState = oldState;
    sm->currentState = newState;
    sm->transitionCount++;

    /* Update internal flags based on state */
    switch (newState) {
        case DM1_STATE_NEWGAME_INIT:
            sm->newGame = 1;
            sm->gameTimeTicking = 0;
            sm->partyDead = 0;
            sm->gameWon = 0;
            break;
        case DM1_STATE_LOADGAME_INIT:
            sm->newGame = 0;
            sm->gameTimeTicking = 0;
            sm->partyDead = 0;
            sm->gameWon = 0;
            break;
        case DM1_STATE_DUNGEON_VIEWPORT:
            sm->gameTimeTicking = 1;
            sm->inventoryChampionOrdinal = 0;
            sm->disabledMenusDrawn = 0;
            break;
        case DM1_STATE_INVENTORY:
            /* inventoryChampionOrdinal set by caller */
            break;
        case DM1_STATE_GAME_OVER:
            sm->partyDead = 1;
            sm->gameTimeTicking = 0;
            break;
        case DM1_STATE_VICTORY:
            sm->gameWon = 1;
            sm->gameTimeTicking = 0;
            break;
        case DM1_STATE_RESTART:
            sm->restartRequested = 1;
            sm->gameTimeTicking = 0;
            break;
        case DM1_STATE_TITLE_SCREEN:
            /* Reset most flags for fresh start */
            sm->partyDead = 0;
            sm->gameWon = 0;
            sm->restartRequested = 0;
            sm->gameTimeTicking = 0;
            sm->partyResting = 0;
            sm->inventoryChampionOrdinal = 0;
            sm->disabledMenusDrawn = 0;
            break;
        default:
            break;
    }

    /* Enter callback */
    if (sm->onEnter) {
        sm->onEnter(oldState, newState, sm->callbackUserdata);
    }

    return DM1_TRANS_OK;
}

/* ── Query ────────────────────────────────────────────────────────── */

M11_GameStateId m11_game_state_current(const M11_GameStateMachine *sm)
{
    return sm->currentState;
}

M11_GameStateId m11_game_state_previous(const M11_GameStateMachine *sm)
{
    return sm->previousState;
}

/* ── Convenience state setters ────────────────────────────────────── */

M11_TransitionResult m11_game_state_start_new_game(M11_GameStateMachine *sm)
{
    return m11_game_state_transition(sm, DM1_STATE_NEWGAME_INIT);
}

M11_TransitionResult m11_game_state_load_game(M11_GameStateMachine *sm)
{
    return m11_game_state_transition(sm, DM1_STATE_LOADGAME_INIT);
}

M11_TransitionResult m11_game_state_enter_dungeon(M11_GameStateMachine *sm)
{
    return m11_game_state_transition(sm, DM1_STATE_DUNGEON_VIEWPORT);
}

M11_TransitionResult m11_game_state_open_inventory(M11_GameStateMachine *sm,
                                                    int championOrdinal)
{
    M11_TransitionResult r = m11_game_state_transition(sm, DM1_STATE_INVENTORY);
    if (r == DM1_TRANS_OK) {
        sm->inventoryChampionOrdinal = championOrdinal;
    }
    return r;
}

M11_TransitionResult m11_game_state_close_inventory(M11_GameStateMachine *sm)
{
    sm->inventoryChampionOrdinal = 0;
    return m11_game_state_transition(sm, DM1_STATE_DUNGEON_VIEWPORT);
}

M11_TransitionResult m11_game_state_party_died(M11_GameStateMachine *sm)
{
    return m11_game_state_transition(sm, DM1_STATE_GAME_OVER);
}

M11_TransitionResult m11_game_state_victory(M11_GameStateMachine *sm)
{
    return m11_game_state_transition(sm, DM1_STATE_VICTORY);
}

M11_TransitionResult m11_game_state_request_restart(M11_GameStateMachine *sm)
{
    if (!sm->restartAllowed) return DM1_TRANS_BLOCKED;
    return m11_game_state_transition(sm, DM1_STATE_RESTART);
}

/* ── Name lookup ──────────────────────────────────────────────────── */

const char *m11_game_state_name(M11_GameStateId state)
{
    if (state >= DM1_STATE_COUNT) return "UNKNOWN";
    return s_state_names[state];
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *m11_game_state_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206\n"
        "STARTUP1.C: F0448_STARTUP1_InitializeMemoryManager_CPSADEF —\n"
        "  Memory init, VBlank init, graphic data init, title draw\n"
        "STARTUP2.C: F0462_START_StartGame_CPSEF —\n"
        "  G0331/G0333=0, G0300=0, G0341=0\n"
        "  G0506=NONE, G0509=TRUE, G0441-G0444=default input tables\n"
        "  F0003(G0309), G0298 new/load path, G0301=TRUE\n"
        "STARTUP2.C: F0463_START_InitializeGame_CPSADEF —\n"
        "  Full sequence: memory → graphics.dat → graphic data → title →\n"
        "  floor/wall sets → text → objects → mouse → entrance loop →\n"
        "  load game → spell area → graphic memory → start game\n"
        "ENDGAME.C: F0444_STARTEND_Endgame —\n"
        "  Credits + champion stats + restart option\n"
        "TITLE.C: F0437_STARTEND_DrawTitle —\n"
        "  18-step zoom swoosh + palette fade + double-buffer\n"
        "Key globals: G0298 newGame, G0300 resting, G0301 timeTicking,\n"
        "  G0302 gameWon, G0303 partyDead, G0423 inventoryChampion,\n"
        "  G0523 restartRequested, G0524 restartAllowed,\n"
        "  G2167 disabledMenusDrawn";
}
