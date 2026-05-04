/*
 * DM1 V1 Resurrection & Reincarnation System — Source-locked from ReDMCSB.
 *
 * See dm1_v1_resurrection_pc34_compat.h for full source references.
 */

#include "dm1_v1_resurrection_pc34_compat.h"

/* -------- Small math helpers (same pattern as other phases) --------- */

static int16_t max_i16(int16_t a, int16_t b) {
    return (a > b) ? a : b;
}

/* ================================================================
 *  F0860: Bones creation parameters
 *  Source: CHAMPION.C F0319 — bones creation block
 *  ReDMCSB lines:
 *    L0964_T_Thing = F0166_DUNGEON_GetUnusedThing(
 *        MASK0x8000_CHAMPION_BONES | C10_THING_TYPE_JUNK);
 *    if (L0964_T_Thing == C0xFFFF_THING_NONE) {
 *    } else {
 *        L0966_ps_Junk->Type = C05_JUNK_BONES;
 *        L0966_ps_Junk->DoNotDiscard = C1_TRUE;
 *        L0966_ps_Junk->ChargeCount = P0661_ui_ChampionIndex;
 *        AL0962_ui_Cell = L0965_ps_Champion->Cell;
 *        F0267_MOVE_GetMoveResult_CPSCE(
 *            M015_THING_WITH_NEW_CELL(L0964_T_Thing, AL0962_ui_Cell),
 *            CM1_MAPX_NOT_ON_A_SQUARE, 0,
 *            G0306_i_PartyMapX, G0307_i_PartyMapY);
 *    }
 * ================================================================ */

BonesCreationResult_Compat F0860_RESURRECTION_ComputeBonesCreation_Compat(
    uint16_t championIndex,
    uint16_t championCell)
{
    BonesCreationResult_Compat result;
    result.junkType = DM1_JUNK_TYPE_BONES;    /* C05_JUNK_BONES */
    result.doNotDiscard = 1;                   /* C1_TRUE */
    result.chargeCount = (uint8_t)(championIndex & 0x03);
    result.cell = championCell;
    result.valid = 1;
    return result;
}

/* ================================================================
 *  F0861: Vi Altar rebirth trigger detection
 *  Source: CLIKVIEW.C F0374:
 *    if (L1146_B_DroppingIntoAnAlcove && G0287_B_FacingViAltar &&
 *        (F0033_OBJECT_GetIconIndex(L1142_T_Thing) ==
 *         C147_ICON_JUNK_CHAMPION_BONES)) {
 *        ... create C13_EVENT_VI_ALTAR_REBIRTH event ...
 *    }
 * ================================================================ */

int F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(
    int droppingIntoAlcove,
    int facingViAltar,
    int objectIconIndex)
{
    return (droppingIntoAlcove &&
            facingViAltar &&
            (objectIconIndex == DM1_ICON_CHAMPION_BONES)) ? 1 : 0;
}

/* ================================================================
 *  F0862: Extract champion index from bones
 *  Source: CLIKVIEW.C F0374:
 *    L1143_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L1142_T_Thing);
 *    L1147_s_Event.A.A.Priority = L1143_ps_Junk->ChargeCount;
 * ================================================================ */

uint8_t F0862_RESURRECTION_GetChampionIndexFromBones_Compat(
    uint8_t bonesChargeCount)
{
    return bonesChargeCount & 0x03;  /* Champion index 0..3 */
}

/* ================================================================
 *  F0863: Vi Altar rebirth health penalty
 *  Source: REVIVE.C F0283:
 *    AL0831_ui_MaximumHealth = L0832_ps_Champion->MaximumHealth;
 *    L0832_ps_Champion->CurrentHealth =
 *      (L0832_ps_Champion->MaximumHealth =
 *        F0025_MAIN_GetMaximumValue(25,
 *          AL0831_ui_MaximumHealth
 *            - (AL0831_ui_MaximumHealth >> 6) - 1)) >> 1;
 *
 *  The health penalty is: newMax = max(25, oldMax - oldMax/64 - 1)
 *  Current health is set to newMax / 2.
 * ================================================================ */

RebirthHealthResult_Compat F0863_RESURRECTION_ComputeRebirthHealth_Compat(
    int16_t currentMaxHealth)
{
    RebirthHealthResult_Compat result;
    int16_t penalty = (currentMaxHealth >> 6) + 1;
    result.newMaxHealth = max_i16(25, currentMaxHealth - penalty);
    result.newCurrentHealth = result.newMaxHealth >> 1;
    return result;
}

/* ================================================================
 *  F0864: Reincarnation stat changes (DM1 V1 — MEDIA265_S20E)
 *  Source: REVIVE.C F0282, inside
 *    if (P0598_i_Command == C161_COMMAND_CLICK_IN_PANEL_REINCARNATE):
 *
 *    F0281_CHAMPION_Rename(L0826_ps_Champion);          — UI (not ported)
 *    F0008_MAIN_ClearBytes(L0826_ps_Champion->Skills);  — skills zeroed
 *
 *    (MEDIA265_S20E — DM1 V1 Atari ST version):
 *    L0826_ps_Champion->CurrentHealth >>= 1;
 *    L0826_ps_Champion->MaximumHealth >>= 1;
 *    L0826_ps_Champion->CurrentStamina >>= 1;
 *    L0826_ps_Champion->MaximumStamina >>= 1;
 *    L0826_ps_Champion->CurrentMana >>= 1;
 *    L0826_ps_Champion->MaximumMana >>= 1;
 *
 *    (common — after MEDIA265 block):
 *    for (AL0823_ui_Counter = 0; AL0823_ui_Counter < 12; AL0823_ui_Counter++) {
 *        AL0824_ui_StatisticIndex = M002_RANDOM(7);
 *        L0826_ps_Champion->Statistics[AL0824_ui_StatisticIndex][C1_CURRENT]++;
 *        L0826_ps_Champion->Statistics[AL0824_ui_StatisticIndex][C0_MAXIMUM]++;
 *    }
 * ================================================================ */

ReincarnationResult_Compat F0864_RESURRECTION_ComputeReincarnation_Compat(
    int16_t maxHealth, int16_t currentHealth,
    int16_t maxStamina, int16_t currentStamina,
    int16_t maxMana, int16_t currentMana,
    const uint8_t rngValues[12])
{
    ReincarnationResult_Compat result;
    int i;

    /* Halve all vitals (DM1 V1 / MEDIA265_S20E) */
    result.newMaxHealth = maxHealth >> 1;
    result.newCurrentHealth = currentHealth >> 1;
    result.newMaxStamina = maxStamina >> 1;
    result.newCurrentStamina = currentStamina >> 1;
    result.newMaxMana = maxMana >> 1;
    result.newCurrentMana = currentMana >> 1;

    /* Clear stat increments */
    for (i = 0; i < 7; i++) {
        result.statIncrements[i] = 0;
    }

    /* 12 random stat increments: each rngValues[i] is pre-rolled RANDOM(7)
     * yielding a stat index 0..6. Both current and maximum are incremented. */
    for (i = 0; i < 12; i++) {
        uint8_t statIdx = rngValues[i] % 7;
        result.statIncrements[statIdx]++;
    }

    return result;
}

/* ================================================================
 *  F0865: Validate resurrect/reincarnate/cancel command
 *  Source: REVIVE.C F0282 entry:
 *    L0826_ps_Champion = &M516_CHAMPIONS[L0822_ui_ChampionIndex =
 *        G0305_ui_PartyChampionCount - 1];
 *    if (P0598_i_Command == C162_COMMAND_CLICK_IN_PANEL_CANCEL) { ... }
 *  A champion must exist (partyChampionCount >= 1) and command must be
 *  one of C160/C161/C162.
 * ================================================================ */

int F0865_RESURRECTION_IsCommandValid_Compat(
    int16_t command,
    uint16_t partyChampionCount)
{
    if (partyChampionCount == 0) return 0;
    if (command != DM1_COMMAND_RESURRECT &&
        command != DM1_COMMAND_REINCARNATE &&
        command != DM1_COMMAND_CANCEL) {
        return 0;
    }
    return 1;
}

/* -------- Evidence / invariant -------------------------------------- */

const char* dm1_v1_resurrection_GetEvidence(void) {
    return "REVIVE.C:F0280-F0283 resurrection altar processing, "
           "bones->champion, reincarnation stat halving (MEDIA265_S20E). "
           "CHAMPION.C:F0319 bones creation (Type=C05, ChargeCount=champIdx). "
           "CLIKVIEW.C:F0374 alcove+ViAltar bones detection, "
           "C13_EVENT_VI_ALTAR_REBIRTH event creation. "
           "DEFS.H: C05_JUNK_BONES, C147_ICON_JUNK_CHAMPION_BONES, "
           "C160/C161/C162 commands, MASK0x8000_CHAMPION_BONES.";
}

unsigned int dm1_v1_resurrection_GetInvariant(void) {
    /* Verify key constants match ReDMCSB DEFS.H */
    int ok = 1;
    ok = ok && (DM1_JUNK_TYPE_BONES == 5);
    ok = ok && (DM1_ICON_CHAMPION_BONES == 147);
    ok = ok && (DM1_EVENT_TYPE_VI_ALTAR_REBIRTH == 13);
    ok = ok && (DM1_COMMAND_RESURRECT == 160);
    ok = ok && (DM1_COMMAND_REINCARNATE == 161);
    ok = ok && (DM1_COMMAND_CANCEL == 162);

    /* Verify rebirth health: max=100 → max(25, 100-100/64-1) = max(25,98) = 98, current=49 */
    {
        RebirthHealthResult_Compat h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(100);
        ok = ok && (h.newMaxHealth == 98);
        ok = ok && (h.newCurrentHealth == 49);
    }

    /* Verify rebirth health floor: max=25 → max(25, 25-25/64-1) = max(25,24) = 25, current=12 */
    {
        RebirthHealthResult_Compat h = F0863_RESURRECTION_ComputeRebirthHealth_Compat(25);
        ok = ok && (h.newMaxHealth == 25);
        ok = ok && (h.newCurrentHealth == 12);
    }

    /* Verify bones creation */
    {
        BonesCreationResult_Compat b = F0860_RESURRECTION_ComputeBonesCreation_Compat(2, 3);
        ok = ok && (b.junkType == 5);
        ok = ok && (b.doNotDiscard == 1);
        ok = ok && (b.chargeCount == 2);
        ok = ok && (b.cell == 3);
        ok = ok && (b.valid == 1);
    }

    /* Verify Vi Altar trigger */
    ok = ok && (F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 1, 147) == 1);
    ok = ok && (F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(0, 1, 147) == 0);
    ok = ok && (F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 0, 147) == 0);
    ok = ok && (F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(1, 1, 100) == 0);

    /* Verify champion index from bones */
    ok = ok && (F0862_RESURRECTION_GetChampionIndexFromBones_Compat(0) == 0);
    ok = ok && (F0862_RESURRECTION_GetChampionIndexFromBones_Compat(3) == 3);

    /* Verify reincarnation (all rng→stat 0) */
    {
        uint8_t rng[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
        ReincarnationResult_Compat r = F0864_RESURRECTION_ComputeReincarnation_Compat(
            200, 100, 400, 200, 100, 50, rng);
        ok = ok && (r.newMaxHealth == 100);
        ok = ok && (r.newCurrentHealth == 50);
        ok = ok && (r.newMaxStamina == 200);
        ok = ok && (r.newCurrentStamina == 100);
        ok = ok && (r.newMaxMana == 50);
        ok = ok && (r.newCurrentMana == 25);
        ok = ok && (r.statIncrements[0] == 12);
        ok = ok && (r.statIncrements[1] == 0);
    }

    /* Verify command validation */
    ok = ok && (F0865_RESURRECTION_IsCommandValid_Compat(160, 1) == 1);
    ok = ok && (F0865_RESURRECTION_IsCommandValid_Compat(161, 1) == 1);
    ok = ok && (F0865_RESURRECTION_IsCommandValid_Compat(162, 2) == 1);
    ok = ok && (F0865_RESURRECTION_IsCommandValid_Compat(160, 0) == 0);
    ok = ok && (F0865_RESURRECTION_IsCommandValid_Compat(99, 1) == 0);

    return ok ? 1u : 0u;
}
