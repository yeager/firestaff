/**
 * dm1_v1_endgame_system_pc34_compat.c — DM1 V1 Endgame System Implementation
 *
 * Source-locked from ReDMCSB ENDGAME.C, PROJEXPL.C, MENU.C, CHAMPION.C, DEFS.H, DATA.C.
 * Secondary reference: CSBWin Attack.cpp (_Fusion, _FusionSequence).
 */
#include "dm1_v1_endgame_system_pc34_compat.h"
#include <string.h>

/* ===================================================================
 * Firestaff Assembly
 * Source: CHAMPION.C lines 771-774, DEFS.H C027/C028/C120
 * =================================================================*/

int DM1_Endgame_CanAssembleFirestaff(int32_t actionHandIconIndex,
                                     int32_t secondItemIconIndex)
{
    if (actionHandIconIndex == DM1_ICON_WEAPON_THE_FIRESTAFF &&
        secondItemIconIndex == DM1_ICON_JUNK_GEM_OF_AGES) {
        return 1;
    }
    if (actionHandIconIndex == DM1_ICON_JUNK_GEM_OF_AGES &&
        secondItemIconIndex == DM1_ICON_WEAPON_THE_FIRESTAFF) {
        return 1;
    }
    return 0;
}

int32_t DM1_Endgame_GetAssembledFirestaffIcon(int32_t actionHandIconIndex,
                                               int32_t otherItemIconIndex)
{
    if (DM1_Endgame_CanAssembleFirestaff(actionHandIconIndex, otherItemIconIndex)) {
        return DM1_ICON_WEAPON_THE_FIRESTAFF_COMPLETE;
    }
    return actionHandIconIndex;
}

/**
 * Source: CHAMPION.C lines 771-774:
 *   if (L0910 == C027_ICON_WEAPON_THE_FIRESTAFF) L0908_i_SkillLevel++;
 *   else if (L0910 == C028_ICON_WEAPON_THE_FIRESTAFF_COMPLETE) L0908 += 2;
 */
int32_t DM1_Endgame_GetFirestaffSkillBonus(int32_t actionHandIconIndex)
{
    if (actionHandIconIndex == DM1_ICON_WEAPON_THE_FIRESTAFF) {
        return 1;
    }
    if (actionHandIconIndex == DM1_ICON_WEAPON_THE_FIRESTAFF_COMPLETE) {
        return 2;
    }
    return 0;
}

/* ===================================================================
 * Fluxcage Detection
 * Source: PROJEXPL.C F0225_GROUP_FuseAction — counts fluxcages on
 * 4 cardinal neighbors: west[0], east[1], north[2], south[3].
 * =================================================================*/

int32_t DM1_Endgame_CountFluxcagesAroundSquare(const int32_t fluxcagePresent[4])
{
    int32_t count = 0;
    int i;
    if (!fluxcagePresent) return 0;
    for (i = 0; i < 4; i++) {
        if (fluxcagePresent[i]) count++;
    }
    return count;
}

/* ===================================================================
 * Lord Chaos Identification
 * Source: PROJEXPL.C F0222_GROUP_IsLordChaosOnSquare:
 *   if (L0543_ps_Group->Type == C23_CREATURE_LORD_CHAOS) return L0542_T_Thing;
 * =================================================================*/

int32_t DM1_Endgame_IsLordChaosOnSquare(int32_t creatureTypeOnSquare)
{
    return (creatureTypeOnSquare == DM1_CREATURE_LORD_CHAOS_ID) ? 1 : 0;
}

/* ===================================================================
 * Fuse Action Evaluation
 * Source: PROJEXPL.C F0225_GROUP_FuseAction:
 *   1. Bounds check (MEDIA260/MEDIA495)
 *   2. Create harm-non-material explosion at target
 *   3. Check IsLordChaosOnSquare
 *   4. Count fluxcages on 4 neighbors
 *   5. While count < 4, try to move Lord Chaos to escape
 *   6. If no escape possible -> F0446_STARTEND_FuseSequence
 * =================================================================*/

int DM1_Endgame_EvaluateFuseAction(int32_t targetMapX,
                                   int32_t targetMapY,
                                   int32_t mapWidth,
                                   int32_t mapHeight,
                                   int32_t creatureTypeOnTarget,
                                   const int32_t fluxcagePresent[4],
                                   int32_t escapeSquareAvailable,
                                   DM1EndgameFuseActionResult* outResult)
{
    if (!outResult) return 0;
    memset(outResult, 0, sizeof(*outResult));
    outResult->escapeMapX = -1;
    outResult->escapeMapY = -1;

    /* Bounds check — PROJEXPL.C MEDIA260/MEDIA495 */
    if (targetMapX < 0 || targetMapX >= mapWidth ||
        targetMapY < 0 || targetMapY >= mapHeight) {
        outResult->sourceEvidence =
            "PROJEXPL.C:1068-1070 MEDIA260/MEDIA495 bounds check — Fuse aborted, out of map";
        return 1;
    }

    /* Always creates harm-non-material explosion at target
     * PROJEXPL.C:1076 F0213_EXPLOSION_Create(C0xFF83, 255, ...) */
    outResult->lordChaosPresent = DM1_Endgame_IsLordChaosOnSquare(creatureTypeOnTarget);

    if (!outResult->lordChaosPresent) {
        outResult->sourceEvidence =
            "PROJEXPL.C:1078 IsLordChaosOnSquare returned 0 — no Lord Chaos, Fuse creates explosion only";
        return 1;
    }

    /* Count fluxcages — PROJEXPL.C:1079-1082 */
    outResult->fluxcageCount = DM1_Endgame_CountFluxcagesAroundSquare(fluxcagePresent);

    if (outResult->fluxcageCount < 4 && escapeSquareAvailable) {
        /* Lord Chaos escapes — PROJEXPL.C:1115-1120 */
        outResult->lordChaosEscaped = 1;
        outResult->fuseSequenceTriggered = 0;
        outResult->sourceEvidence =
            "PROJEXPL.C:1083-1120 Lord Chaos not fully trapped (fluxcages < 4), escapes to adjacent square";
        return 1;
    }

    /* Lord Chaos is trapped — trigger Fuse sequence
     * PROJEXPL.C:1121 F0446_STARTEND_FuseSequence() */
    outResult->fuseSequenceTriggered = 1;
    outResult->lordChaosEscaped = 0;
    outResult->sourceEvidence =
        "PROJEXPL.C:1121 Lord Chaos fully trapped by Fluxcages — F0446_STARTEND_FuseSequence triggered";
    return 1;
}

/* ===================================================================
 * Chaos/Order Cycling
 * Source: ENDGAME.C F0446:
 *   L1428_ps_Group->Type = (AL1424_i_CreatureTypeSwitchCount & 0x0001)
 *                          ? C25_CREATURE_LORD_ORDER : C23_CREATURE_LORD_CHAOS;
 * =================================================================*/

int32_t DM1_Endgame_GetCycleCreatureType(int32_t switchCount)
{
    return (switchCount & 1) ? DM1_CREATURE_LORD_ORDER_ID : DM1_CREATURE_LORD_CHAOS_ID;
}

/* ===================================================================
 * Fuse Sequence State Machine
 * Source: ENDGAME.C F0446_STARTEND_FuseSequence (PC/F20E path)
 * =================================================================*/

void DM1_Endgame_FuseSequence_Init(DM1EndgameFuseState* state,
                                   int32_t lordChaosMapX,
                                   int32_t lordChaosMapY)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->phase = DM1_FUSE_PHASE_INIT;
    state->lordChaosMapX = lordChaosMapX;
    state->lordChaosMapY = lordChaosMapY;
    state->lordChaosHealth = 10000; /* F0446: L1428_ps_Group->Health[0] = 10000 */
    state->currentCreatureType = DM1_CREATURE_LORD_CHAOS_ID;
    state->fireballAttackValue = 55; /* first attack value in barrage loop */
    state->cycleCount = 4;  /* AL1425_i_CycleCount = 4 */
    state->switchCount = 5; /* AL1424_i_CreatureTypeSwitchCount = 5 */
    state->lastSourceEvidence = "ENDGAME.C F0446: FuseSequence initialized";
}

int DM1_Endgame_FuseSequence_Step(DM1EndgameFuseState* state)
{
    if (!state) return 0;

    switch (state->phase) {
    case DM1_FUSE_PHASE_INIT:
        state->phase = DM1_FUSE_PHASE_SETUP_SHIELDS_LIGHT;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: G0302_B_GameWon = C1_TRUE; close inventory; "
            "G0407_s_Party.MagicalLightAmount = 200; shields = 100";
        return 1;

    case DM1_FUSE_PHASE_SETUP_SHIELDS_LIGHT:
        state->gameWon = 1;
        state->phase = DM1_FUSE_PHASE_LOCATE_LORD_CHAOS;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: locate Lord Chaos at party facing square, "
            "set Health[0] = 10000, center creature, face opposite";
        return 1;

    case DM1_FUSE_PHASE_LOCATE_LORD_CHAOS:
        state->lordChaosHealth = 10000;
        state->currentCreatureType = DM1_CREATURE_LORD_CHAOS_ID;
        state->phase = DM1_FUSE_PHASE_REMOVE_FLUXCAGES;
        state->lastSourceEvidence =
            "ENDGAME.C F0446 MEDIA233/CHANGE2_20_FIX: remove Fluxcages "
            "from party square and Lord Chaos square";
        return 1;

    case DM1_FUSE_PHASE_REMOVE_FLUXCAGES:
        state->phase = DM1_FUSE_PHASE_FIREBALL_BARRAGE;
        state->fireballAttackValue = 55;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: first explosion loop — "
            "F0213(C0xFF80_FIREBALL, 55..255 step 40) on Lord Chaos square";
        return 1;

    case DM1_FUSE_PHASE_FIREBALL_BARRAGE:
        /* Fireball explosions: 55, 95, 135, 175, 215, 255 */
        if (state->fireballAttackValue <= 255) {
            state->fireballAttackValue += 40;
            state->gameTime++;
            if (state->fireballAttackValue > 255) {
                state->phase = DM1_FUSE_PHASE_MORPH_TO_LORD_ORDER;
                state->lastSourceEvidence =
                    "ENDGAME.C F0446: F0064_SOUND(M560_SOUND_BUZZ); "
                    "L1428_ps_Group->Type = C25_CREATURE_LORD_ORDER";
            }
            return 1;
        }
        state->phase = DM1_FUSE_PHASE_MORPH_TO_LORD_ORDER;
        return 1;

    case DM1_FUSE_PHASE_MORPH_TO_LORD_ORDER:
        state->currentCreatureType = DM1_CREATURE_LORD_ORDER_ID;
        state->phase = DM1_FUSE_PHASE_HARM_BARRAGE;
        state->fireballAttackValue = 55;
        state->gameTime++;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: second explosion loop — "
            "F0213(C0xFF83_HARM_NON_MATERIAL, 55..255 step 40)";
        return 1;

    case DM1_FUSE_PHASE_HARM_BARRAGE:
        if (state->fireballAttackValue <= 255) {
            state->fireballAttackValue += 40;
            state->gameTime++;
            if (state->fireballAttackValue > 255) {
                state->phase = DM1_FUSE_PHASE_CHAOS_ORDER_CYCLE;
                state->cycleCount = 4;
                state->switchCount = 5;
                state->lastSourceEvidence =
                    "ENDGAME.C F0446: nested Chaos/Order cycling — "
                    "AL1425=4 cycles, AL1424=5 switches per cycle, "
                    "(switch & 1) ? LORD_ORDER : LORD_CHAOS";
            }
            return 1;
        }
        state->phase = DM1_FUSE_PHASE_CHAOS_ORDER_CYCLE;
        state->cycleCount = 4;
        state->switchCount = 5;
        return 1;

    case DM1_FUSE_PHASE_CHAOS_ORDER_CYCLE:
        /* Source: for (AL1425=4; --AL1425; ) for (AL1424=5; --AL1424; ) */
        if (state->cycleCount > 1) {
            state->cycleCount--;
            if (state->switchCount > 1) {
                state->switchCount--;
                state->currentCreatureType =
                    DM1_Endgame_GetCycleCreatureType(state->switchCount);
                state->gameTime++;
                state->lastSourceEvidence =
                    "ENDGAME.C F0446: cycle morph — "
                    "SOUND_BUZZ; Type = (switch & 1) ? LORD_ORDER : LORD_CHAOS";
                return 1;
            }
            state->switchCount = 5;
            return 1;
        }
        state->phase = DM1_FUSE_PHASE_FINAL_EXPLOSIONS;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: final explosions — "
            "F0213(FIREBALL, 255) + F0213(HARM, 255)";
        return 1;

    case DM1_FUSE_PHASE_FINAL_EXPLOSIONS:
        state->gameTime++;
        state->phase = DM1_FUSE_PHASE_MORPH_TO_GREY_LORD;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: L1428_ps_Group->Type = C26_CREATURE_GREY_LORD";
        return 1;

    case DM1_FUSE_PHASE_MORPH_TO_GREY_LORD:
        state->currentCreatureType = DM1_CREATURE_GREY_LORD_ID;
        state->gameTime++;
        state->phase = DM1_FUSE_PHASE_CLEAR_ALL_MONSTERS;
        state->lastSourceEvidence =
            "ENDGAME.C F0446 MEDIA141: G0077_B_DoNotDrawFluxcages = TRUE; "
            "delete all monsters except Grey Lord square";
        return 1;

    case DM1_FUSE_PHASE_CLEAR_ALL_MONSTERS:
        state->doNotDrawFluxcages = 1;
        state->phase = DM1_FUSE_PHASE_VICTORY_TEXT;
        state->lastSourceEvidence =
            "ENDGAME.C F0446 MEDIA503/MEDIA130: play C2_MUSIC_GAME_WON; "
            "enumerate text things at (0,0), print ordered by first letter";
        return 1;

    case DM1_FUSE_PHASE_VICTORY_TEXT:
        state->phase = DM1_FUSE_PHASE_TRIGGER_ENDGAME;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: F0022_MAIN_Delay(600); "
            "G0524_B_RestartGameAllowed = C0_FALSE; "
            "F0444_STARTEND_Endgame(C1_TRUE)";
        return 1;

    case DM1_FUSE_PHASE_TRIGGER_ENDGAME:
        state->restartAllowed = 0;
        state->phase = DM1_FUSE_PHASE_COMPLETE;
        state->lastSourceEvidence =
            "ENDGAME.C F0446: Fuse sequence complete, endgame triggered";
        return 0;

    case DM1_FUSE_PHASE_COMPLETE:
        return 0;

    default:
        return 0;
    }
}

int32_t DM1_Endgame_FuseSequence_GetCreatureType(const DM1EndgameFuseState* state)
{
    if (!state) return DM1_CREATURE_LORD_CHAOS_ID;
    return state->currentCreatureType;
}

int DM1_Endgame_FuseSequence_GetExplosionParams(const DM1EndgameFuseState* state,
                                                 int32_t* outExplosionType,
                                                 int32_t* outAttackValue)
{
    if (!state || !outExplosionType || !outAttackValue) return 0;

    if (state->phase == DM1_FUSE_PHASE_FIREBALL_BARRAGE) {
        *outExplosionType = (int32_t)DM1_THING_EXPLOSION_FIREBALL;
        *outAttackValue = state->fireballAttackValue;
        return 1;
    }
    if (state->phase == DM1_FUSE_PHASE_HARM_BARRAGE) {
        *outExplosionType = (int32_t)DM1_THING_EXPLOSION_HARM_NONMAT;
        *outAttackValue = state->fireballAttackValue;
        return 1;
    }
    if (state->phase == DM1_FUSE_PHASE_FINAL_EXPLOSIONS) {
        *outExplosionType = (int32_t)DM1_THING_EXPLOSION_FIREBALL;
        *outAttackValue = 255;
        return 1;
    }
    return 0;
}

/* ===================================================================
 * Ending Sequence Parameters
 * =================================================================*/

static const DM1EndgameEndingParams kEndingParams = {
    600,  /* finalDelayTicks — ENDGAME.C F0446: F0022_MAIN_Delay(600) */
    0,    /* restartAllowedAfterWin — G0524_B_RestartGameAllowed = C0_FALSE */
    1,    /* endgameCalledWithTrue — F0444_STARTEND_Endgame(C1_TRUE) */
    2,    /* victoryMusicId — C2_MUSIC_GAME_WON (MEDIA503 F0741) */
    "ENDGAME.C F0446 final: Delay(600), RestartGameAllowed=FALSE, "
    "Endgame(TRUE); MEDIA503: MUSIC_PlayGameMusic(C2_MUSIC_GAME_WON)"
};

const DM1EndgameEndingParams* DM1_Endgame_GetEndingParams(void)
{
    return &kEndingParams;
}

/* ===================================================================
 * Source Evidence
 * =================================================================*/

const char* DM1_Endgame_System_GetSourceEvidence(void)
{
    return
        "ReDMCSB source lock: "
        "ENDGAME.C F0444_STARTEND_Endgame (endgame presentation), "
        "F0445_STARTEND_FuseSequenceUpdate (per-tick render/sound), "
        "F0446_STARTEND_FuseSequence (Lord Chaos morph + victory); "
        "PROJEXPL.C F0222_IsLordChaosOnSquare (checks Type==C23), "
        "F0223_IsLordChaosAllowed (walkability check for escape), "
        "F0225_FuseAction (bounds check, explosion, fluxcage count, escape or fuse); "
        "MENU.C:1499-1504 C043_ACTION_FUSE dispatch; "
        "CHAMPION.C:771-774 Firestaff skill bonus (+1 base, +2 complete); "
        "DEFS.H constants: C23/C25/C26 creatures, C027/C028/C120 items, "
        "C050 fluxcage, C0xFF80/C0xFF83 explosions; "
        "DATA.C weapon table entries 0 and 45 for THE FIRESTAFF. "
        "CSBWin Attack.cpp _Fusion/_FusionSequence as secondary reference.";
}
