/**
 * dm1_v1_endgame_system_pc34_compat.h — DM1 V1 Endgame System
 *
 * Source-locked from ReDMCSB (WIP 2021-02-06):
 *   ENDGAME.C   — F0444_STARTEND_Endgame, F0445_STARTEND_FuseSequenceUpdate,
 *                 F0446_STARTEND_FuseSequence
 *   PROJEXPL.C  — F0222_GROUP_IsLordChaosOnSquare, F0223_GROUP_IsLordChaosAllowed,
 *                 F0224_GROUP_FluxCageAction, F0225_GROUP_FuseAction
 *   MENU.C      — C043_ACTION_FUSE dispatch (line 1499-1504)
 *   CHAMPION.C  — Firestaff/Power Gem skill modifiers (lines 771-805)
 *   DEFS.H      — Constants:
 *                 C23_CREATURE_LORD_CHAOS, C25_CREATURE_LORD_ORDER,
 *                 C26_CREATURE_GREY_LORD, C0xFF_SINGLE_CENTERED_CREATURE,
 *                 C043_ACTION_FUSE, C035_ACTION_FLUXCAGE,
 *                 C027_ICON_WEAPON_THE_FIRESTAFF,
 *                 C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE,
 *                 C120_ICON_JUNK_GEM_OF_AGES,
 *                 C050_EXPLOSION_FLUXCAGE,
 *                 C0xFF80_THING_EXPLOSION_FIREBALL,
 *                 C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL
 *   DATA.C      — Weapon table: entry 0 = THE FIRESTAFF (24,255,25,10,0x20FF),
 *                 entry 45 = THE FIRESTAFF COMPLETE (36,255,100,50,0x20FF)
 *   CSBWin      — Attack.cpp: _Fusion, _FusionSequence (secondary reference)
 *
 * Implements:
 *   - Firestaff assembly checks (staff + Gem of Ages -> complete staff)
 *   - Power Gem (Gem of Ages) activation detection
 *   - Fluxcage presence detection around Lord Chaos
 *   - Fuse action: Lord Chaos confrontation trigger
 *   - Fuse sequence: morphing animation (Lord Chaos <-> Lord Order -> Grey Lord)
 *   - Ending sequence: text messages, "THE END", credits trigger
 */
#ifndef DM1_V1_ENDGAME_SYSTEM_PC34_COMPAT_H
#define DM1_V1_ENDGAME_SYSTEM_PC34_COMPAT_H

#include <stdint.h>

/* -- Endgame creature constants (DEFS.H) -- */
#define DM1_CREATURE_LORD_CHAOS_ID          23  /* C23_CREATURE_LORD_CHAOS */
#define DM1_CREATURE_RED_DRAGON_ID          24  /* C24_CREATURE_RED_DRAGON */
#define DM1_CREATURE_LORD_ORDER_ID          25  /* C25_CREATURE_LORD_ORDER */
#define DM1_CREATURE_GREY_LORD_ID           26  /* C26_CREATURE_GREY_LORD */
#define DM1_SINGLE_CENTERED_CREATURE      0xFF  /* C0xFF_SINGLE_CENTERED_CREATURE */

/* -- Action constants (DEFS.H/MENU.C) -- */
#define DM1_ACTION_FLUXCAGE                 35  /* C035_ACTION_FLUXCAGE */
#define DM1_ACTION_FUSE                     43  /* C043_ACTION_FUSE */

/* -- Item icon indices (DEFS.H) -- */
#define DM1_ICON_WEAPON_THE_FIRESTAFF       27  /* C027 */
#define DM1_ICON_WEAPON_THE_FIRESTAFF_COMPLETE 28 /* C028 */
#define DM1_ICON_JUNK_GEM_OF_AGES          120  /* C120 */

/* -- Explosion types (DEFS.H) -- */
#define DM1_EXPLOSION_FLUXCAGE_TYPE         50  /* C050_EXPLOSION_FLUXCAGE */
#define DM1_THING_EXPLOSION_FIREBALL    0xFF80  /* C0xFF80 */
#define DM1_THING_EXPLOSION_HARM_NONMAT 0xFF83  /* C0xFF83 */

/* -- Fuse sequence phase enum -- */
typedef enum DM1EndgameFusePhase {
    DM1_FUSE_PHASE_INIT = 0,
    DM1_FUSE_PHASE_SETUP_SHIELDS_LIGHT,
    DM1_FUSE_PHASE_LOCATE_LORD_CHAOS,
    DM1_FUSE_PHASE_REMOVE_FLUXCAGES,
    DM1_FUSE_PHASE_FIREBALL_BARRAGE,
    DM1_FUSE_PHASE_MORPH_TO_LORD_ORDER,
    DM1_FUSE_PHASE_HARM_BARRAGE,
    DM1_FUSE_PHASE_CHAOS_ORDER_CYCLE,
    DM1_FUSE_PHASE_FINAL_EXPLOSIONS,
    DM1_FUSE_PHASE_MORPH_TO_GREY_LORD,
    DM1_FUSE_PHASE_CLEAR_ALL_MONSTERS,
    DM1_FUSE_PHASE_VICTORY_TEXT,
    DM1_FUSE_PHASE_TRIGGER_ENDGAME,
    DM1_FUSE_PHASE_COMPLETE,
    DM1_FUSE_PHASE_COUNT = DM1_FUSE_PHASE_COMPLETE
} DM1EndgameFusePhase;

/* -- Fuse sequence state -- */
typedef struct DM1EndgameFuseState {
    DM1EndgameFusePhase phase;
    int32_t lordChaosMapX;
    int32_t lordChaosMapY;
    int32_t lordChaosHealth;
    int32_t currentCreatureType;
    int32_t fireballAttackValue;
    int32_t cycleCount;
    int32_t switchCount;
    int32_t fuseSequenceUpdateCount;
    int32_t monstersDeleted;
    int32_t textMessageIndex;
    int32_t textMessageCount;
    int32_t gameWon;
    int32_t restartAllowed;
    int32_t doNotDrawFluxcages;
    int32_t gameTime;
    const char* lastSourceEvidence;
} DM1EndgameFuseState;

/* -- Firestaff assembly check -- */
int DM1_Endgame_CanAssembleFirestaff(int32_t actionHandIconIndex,
                                     int32_t secondItemIconIndex);

int32_t DM1_Endgame_GetAssembledFirestaffIcon(int32_t actionHandIconIndex,
                                               int32_t otherItemIconIndex);

int32_t DM1_Endgame_GetFirestaffSkillBonus(int32_t actionHandIconIndex);

/* -- Fluxcage detection -- */
int32_t DM1_Endgame_CountFluxcagesAroundSquare(const int32_t fluxcagePresent[4]);

/* -- Lord Chaos identification -- */
int32_t DM1_Endgame_IsLordChaosOnSquare(int32_t creatureTypeOnSquare);

/* -- Fuse action evaluation -- */
typedef struct DM1EndgameFuseActionResult {
    int32_t fuseSequenceTriggered;
    int32_t lordChaosPresent;
    int32_t fluxcageCount;
    int32_t lordChaosEscaped;
    int32_t escapeMapX;
    int32_t escapeMapY;
    const char* sourceEvidence;
} DM1EndgameFuseActionResult;

int DM1_Endgame_EvaluateFuseAction(int32_t targetMapX,
                                   int32_t targetMapY,
                                   int32_t mapWidth,
                                   int32_t mapHeight,
                                   int32_t creatureTypeOnTarget,
                                   const int32_t fluxcagePresent[4],
                                   int32_t escapeSquareAvailable,
                                   DM1EndgameFuseActionResult* outResult);

/* -- Fuse sequence state machine -- */
void DM1_Endgame_FuseSequence_Init(DM1EndgameFuseState* state,
                                   int32_t lordChaosMapX,
                                   int32_t lordChaosMapY);

int DM1_Endgame_FuseSequence_Step(DM1EndgameFuseState* state);

int32_t DM1_Endgame_FuseSequence_GetCreatureType(const DM1EndgameFuseState* state);

int DM1_Endgame_FuseSequence_GetExplosionParams(const DM1EndgameFuseState* state,
                                                 int32_t* outExplosionType,
                                                 int32_t* outAttackValue);

/* -- Chaos/Order cycling -- */
int32_t DM1_Endgame_GetCycleCreatureType(int32_t switchCount);

/* -- Ending sequence parameters -- */
typedef struct DM1EndgameEndingParams {
    int32_t finalDelayTicks;
    int32_t restartAllowedAfterWin;
    int32_t endgameCalledWithTrue;
    int32_t victoryMusicId;
    const char* sourceEvidence;
} DM1EndgameEndingParams;

const DM1EndgameEndingParams* DM1_Endgame_GetEndingParams(void);

/* -- Source evidence -- */
const char* DM1_Endgame_System_GetSourceEvidence(void);

#endif /* DM1_V1_ENDGAME_SYSTEM_PC34_COMPAT_H */
