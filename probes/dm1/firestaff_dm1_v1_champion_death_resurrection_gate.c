/*
 * firestaff_dm1_v1_champion_death_resurrection_gate.c
 *
 * DM1 V1 champion death/bones/altar resurrection integration probe.
 *
 * Hardens the existing F0860..F0868 helpers by exercising them as a
 * single end-to-end cycle:
 *
 *   CHAMPION.C F0319 death
 *     -> F0860_RESURRECTION_ComputeBonesCreation       (bones junk)
 *     -> F0862_RESURRECTION_GetChampionIndexFromBones  (bearer)
 *     -> CLIKVIEW.C F0374 alcove+Vi-altar+bones detection
 *     -> F0861_RESURRECTION_ShouldTriggerViAltarRebirth
 *     -> F0868_RESURRECTION_RunViAltarFullCycle        (event -> step2 -> step1 -> step0)
 *     -> REVIVE.C F0283 health penalty (F0863)
 *
 *   CHAMPION.C F0319 reincarnation
 *     -> F0864_RESURRECTION_ComputeReincarnation       (vitals halved + 12 stat increments)
 *     -> F0867_RESURRECTION_ProcessCandidatePanelCommand
 *
 *   COMMAND.C F0280 candidate add (Hall portrait click)
 *     -> F0866_RESURRECTION_RouteChampionPortraitClick
 *     -> F0867_RESURRECTION_ProcessCandidatePanelCommand
 *     -> F0867a_RESURRECTION_DisableFirstMirrorSensor (BUG0_87)
 *
 * Source-locks (ReDMCSB):
 *   CHAMPION.C F0319 lines 1552-1607   (CHAMPION_Kill -> bones)
 *   CLIKVIEW.C F0374 lines 173-186     (alcove + Vi-altar + bones -> C13 event)
 *   TIMELINE.C lines 1665-1698         (C13 step 2/1/0 + bones unlink + F0283)
 *   REVIVE.C F0282/F0283 lines 124-132/272-276/744-799/915-937 (panel + rebirth + mirror sensor)
 *   DEFS.H: C05_JUNK_BONES, C147_ICON_JUNK_CHAMPION_BONES,
 *           C13_EVENT_VI_ALTAR_REBIRTH, C160/C161/C162 commands,
 *           MASK0x8000_CHAMPION_BONES, MASK0x0400_ICON,
 *           MASK0x1000_STATUS_BOX, MASK0x8000_ACTION_HAND.
 *
 * This probe is data-free (no DUNGEON.DAT, no real assets). Every block
 * exercises the existing helpers as a single transition gate so a
 * regression in any of F0860..F0868 surfaces here even when the
 * isolated unit tests still pass.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dm1_v1_resurrection_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;
static int g_block = 0;

#define PASS(id, msg) do { ++g_pass; printf("PASS %s %s\n", id, msg); } while (0)
#define FAIL(id, msg) do { ++g_fail; printf("FAIL %s %s\n", id, msg); } while (0)
#define CHECK(cond, id, msg) do { \
    if (cond) { PASS(id, msg); } else { FAIL(id, msg); } \
} while (0)
#define BLOCK(id, msg) do { ++g_block; printf("BLOCK %s %s\n", id, msg); } while (0)

/* ===== Helpers ========================================================== */

/* Simulate F0319 CHAMPION_Kill producing a JUNK_BONES thing in the local
 * thing-list.  We keep a small, flat bones array because F0860 only
 * reports the creation parameters; the actual thing-list write happens
 * in the runtime that hosts F0319.  The probe treats the bones charge
 * count as the contract surface. */
typedef struct {
    uint8_t junkType;
    uint8_t doNotDiscard;
    uint8_t chargeCount;
    uint16_t cell;
} SimulatedBones;

static SimulatedBones simulate_f0319_kill(uint16_t championIndex,
                                            uint16_t championCell)
{
    BonesCreationResult_Compat r =
        F0860_RESURRECTION_ComputeBonesCreation_Compat(championIndex, championCell);
    SimulatedBones b;
    b.junkType = r.junkType;
    b.doNotDiscard = r.doNotDiscard;
    b.chargeCount = r.chargeCount;
    b.cell = r.cell;
    return b;
}

/* Simulate CLIKVIEW.C F0374 bones-drop-on-Vi-altar detection.
 * Returns 1 if the F0868 full-cycle helper should be invoked. */
static int simulate_f0374_bones_drop(const SimulatedBones* bones,
                                      int droppingIntoAlcove,
                                      int facingViAltar,
                                      uint16_t partyDirection)
{
    (void)partyDirection;
    if (!bones) return 0;
    /* The icon-index surface F0861 expects is C147_ICON_JUNK_CHAMPION_BONES,
     * which is the bones thing's icon in the original CLIKVIEW.C F0374 path. */
    return F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(
        droppingIntoAlcove, facingViAltar, DM1_ICON_CHAMPION_BONES);
}

/* ===== Block A: bones creation invariant ================================= */

/* CHAMPION.C F0319 writes C05_JUNK_BONES (type 5) with DoNotDiscard=1 and
 * ChargeCount=championIndex in the lower 2 bits.  ChargeCount must
 * round-trip through F0862 so the F0868 cycle picks the same candidate
 * champion that died.  We verify both the parameter shape and the
 * round-trip for all four party slots and one overflow mask. */
static void block_a_bones_round_trip(void)
{
    int idx;
    static const struct {
        uint16_t champ;
        uint16_t cell;
        const char* label;
    } cases[] = {
        { 0, 0, "champ0 cell0" },
        { 1, 1, "champ1 cell1" },
        { 2, 3, "champ2 cell3" },
        { 3, 2, "champ3 cell2" },
        { 7, 0, "masked_chargeCount_3 cell0" },
    };
    for (idx = 0; idx < (int)(sizeof(cases) / sizeof(cases[0])); ++idx) {
        SimulatedBones b = simulate_f0319_kill(cases[idx].champ, cases[idx].cell);
        uint8_t recovered;

        if (b.junkType != DM1_JUNK_TYPE_BONES) {
            FAIL("A-junkType", cases[idx].label);
            continue;
        }
        PASS("A-junkType", cases[idx].label);

        if (b.doNotDiscard != 1) {
            FAIL("A-doNotDiscard", cases[idx].label);
        } else {
            PASS("A-doNotDiscard", cases[idx].label);
        }

        if (b.cell != cases[idx].cell) {
            FAIL("A-cell", cases[idx].label);
        } else {
            PASS("A-cell", cases[idx].label);
        }

        recovered = F0862_RESURRECTION_GetChampionIndexFromBones_Compat(b.chargeCount);
        if (recovered != (uint8_t)(cases[idx].champ & 0x03u)) {
            FAIL("A-roundtrip", cases[idx].label);
        } else {
            PASS("A-roundtrip", cases[idx].label);
        }
    }
}

/* ===== Block B: Vi-altar trigger gate =================================== */

/* CLIKVIEW.C F0374 requires three things: alcove + Vi-altar + bones icon.
 * Each missing predicate must block the F0868 cycle. */
static void block_b_vi_altar_trigger(void)
{
    /* Full positive case */
    CHECK(simulate_f0374_bones_drop(NULL, 1, 1, 0) == 0,
          "B-null-bones", "missing bones blocks F0374 trigger");
    {
        SimulatedBones bones = simulate_f0319_kill(1, 1);
        CHECK(simulate_f0374_bones_drop(&bones, 1, 1, 0) == 1,
              "B-full-yes", "alcove+viAltar+bones -> F0868 reachable");
        CHECK(simulate_f0374_bones_drop(&bones, 0, 1, 0) == 0,
              "B-no-alcove", "missing alcove blocks F0374");
        CHECK(simulate_f0374_bones_drop(&bones, 1, 0, 0) == 0,
              "B-no-vi-altar", "not facing Vi-altar blocks F0374");
    }
}

/* ===== Block C: full Vi-altar cycle gate ================================ */

/* TIMELINE.C 1665-1698 runs the C13 event through three steps:
 *   step 2: C0xFFE4 rebirth explosion + 5-tick delay
 *   step 1: matching bones unlinked by icon/cell/ChargeCount
 *   step 0: REVIVE.C F0283 (F0863 health penalty) + dirty attributes
 *
 * We drive F0868 with both an aligned candidate (bones index matches
 * dying champion index) and a deliberately mismatched candidate so the
 * step-1 bones-unlink branch must reject the wrong bones. */
static void block_c_vi_altar_full_cycle(void)
{
    ViAltarFullCycleInput_Compat in;
    ViAltarFullCycleResult_Compat r;

    /* ----- C1: candidate 2, old cell 1 occupied, free cell 2 ---------- */
    memset(&in, 0, sizeof(in));
    in.championIndex = 2;
    in.oldChampionCell = 1;
    in.occupiedCellMask = 0x0Bu;       /* cells 0/1/3 occupied -> cell 2 free */
    in.partyDirection = 3;             /* DIR_WEST */
    in.maximumHealth = 100;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 1;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = 2;
    in.bonesCell = 3;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.eventCreated == 1, "C1-event", "F0868 creates C13_EVENT_VI_ALTAR_REBIRTH");
    CHECK(r.eventType == DM1_EVENT_TYPE_VI_ALTAR_REBIRTH,
          "C1-eventType", "event type is C13 (DEFS.H)");
    CHECK(r.eventPriority == 2,
          "C1-eventPri", "event priority = bones.ChargeCount = 2");
    CHECK(r.eventEffect == DM1_EFFECT_TOGGLE,
          "C1-eventEffect", "event effect = C02_EFFECT_TOGGLE");
    CHECK(r.step2ExplosionThing == DM1_EXPLOSION_REBIRTH_STEP1,
          "C1-step2-thing", "step 2 explosion thing C0xFFE4");
    CHECK(r.step2ExplosionType == DM1_EXPLOSION_TYPE_REBIRTH_STEP1,
          "C1-step2-type", "step 2 explosion type C100");
    CHECK(r.step2DelayTicks == 5,
          "C1-step2-delay", "step 2 delay = 5 ticks (TIMELINE.C:1665-1698)");
    CHECK(r.step1BonesMatched == 1,
          "C1-step1-match", "matching bones found by icon/cell/ChargeCount");
    CHECK(r.step1BonesUnlinked == 1,
          "C1-step1-unlink", "matching bones unlinked before F0283");
    CHECK(r.revived == 1,
          "C1-revived", "F0283 champion rebirth reached");
    CHECK(r.championIndex == 2,
          "C1-reborn-idx", "reborn champion index == candidate");
    CHECK(r.finalCell == 2,
          "C1-finalCell", "old occupied cell 1 relocates to first free cell 2");
    CHECK(r.finalMaximumHealth == 98,
          "C1-finalMaxHp", "maxHealth 100 -> 98 via F0863 health penalty");
    CHECK(r.finalCurrentHealth == 49,
          "C1-finalCurHp", "currentHealth = newMaxHealth / 2 = 49");
    CHECK(r.finalDirection == 3,
          "C1-finalDir", "rebirth copies party direction (DIR_WEST)");
    CHECK(r.dirtyAttributes ==
              (DM1_CHAMPION_ATTR_ACTION_HAND |
               DM1_CHAMPION_ATTR_STATUS_BOX |
               DM1_CHAMPION_ATTR_ICON),
          "C1-dirtyAttrs", "rebirth marks hand/status/icon redraw attrs");

    /* ----- C2: mismatched bones.ChargeCount must reject step 1 -------- */
    memset(&in, 0, sizeof(in));
    in.championIndex = 2;
    in.oldChampionCell = 1;
    in.occupiedCellMask = 0x0Bu;
    in.partyDirection = 3;
    in.maximumHealth = 100;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 1;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = 1;            /* wrong champion index */
    in.bonesCell = 3;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.eventCreated == 1,
          "C2-event", "F0374 still schedules the C13 event for any bones drop");
    CHECK(r.step1BonesMatched == 0,
          "C2-step1-match", "step 1 rejects bones with wrong ChargeCount");
    CHECK(r.step1BonesUnlinked == 0,
          "C2-step1-unlink", "mismatched bones are not unlinked");
    CHECK(r.revived == 0,
          "C2-revived", "mismatched bones skip F0283");
    CHECK(r.championIndex == 2,
          "C2-reborn-idx", "mismatched cycle keeps the original dying index");

    /* ----- C3: not facing Vi-altar blocks event creation -------------- */
    memset(&in, 0, sizeof(in));
    in.championIndex = 2;
    in.oldChampionCell = 1;
    in.occupiedCellMask = 0x0Bu;
    in.partyDirection = 3;
    in.maximumHealth = 100;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 0;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = 2;
    in.bonesCell = 3;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.eventCreated == 0,
          "C3-event", "non-Vi-altar drop does not create the C13 event");

    /* ----- C4: free cell fallback when old cell is unoccupied -------- */
    memset(&in, 0, sizeof(in));
    in.championIndex = 0;
    in.oldChampionCell = 2;
    in.occupiedCellMask = 0x00u;        /* no live champions */
    in.partyDirection = 1;             /* DIR_EAST */
    in.maximumHealth = 200;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 1;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = 0;
    in.bonesCell = 0;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.revived == 1,
          "C4-revived", "rebirth reachable with empty party mask");
    CHECK(r.finalCell == 2,
          "C4-finalCell", "old cell kept when mask does not include it");
    CHECK(r.finalMaximumHealth == (int16_t)(200 - (200 >> 6) - 1),
          "C4-finalMaxHp", "maxHealth 200 -> 198 via F0863");
    CHECK(r.finalCurrentHealth == (int16_t)((200 - (200 >> 6) - 1) >> 1),
          "C4-finalCurHp", "currentHealth = (maxHealth penalty) / 2");
    CHECK(r.finalDirection == 1,
          "C4-finalDir", "rebirth copies party direction DIR_EAST");

    /* ----- C5: at-floor maxHealth (25) ------------------------------- */
    memset(&in, 0, sizeof(in));
    in.championIndex = 1;
    in.oldChampionCell = 1;
    in.occupiedCellMask = 0x0Bu;
    in.partyDirection = 2;             /* DIR_SOUTH */
    in.maximumHealth = 25;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 1;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = 1;
    in.bonesCell = 0;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.finalMaximumHealth == 25,
          "C5-finalMaxHp", "maxHealth floor clamps to 25");
    CHECK(r.finalCurrentHealth == 12,
          "C5-finalCurHp", "currentHealth = 25 / 2 = 12");
}

/* ===== Block D: reincarnation vital halving ============================ */

/* REVIVE.C F0282 (MEDIA265_S20E branch) halves every vital before the
 * 12-random-stat block.  Skills are cleared by the F0282 panel path,
 * not by F0864 (F0864 only owns the math).  Each rng rollout must
 * produce 12 increments distributed across stats 0..6. */
static void block_d_reincarnation(void)
{
    uint8_t rng_zero[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    uint8_t rng_spread[12] = {0,1,2,3,4,5,6,0,1,2,3,4};
    ReincarnationResult_Compat r;
    int i, total;

    /* All-zero rng: every +1 lands on stat 0 */
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        200, 180, 500, 400, 100, 80, rng_zero);
    CHECK(r.newMaxHealth == 100,
          "D-maxHp-half", "maxHealth 200 -> 100");
    CHECK(r.newCurrentHealth == 90,
          "D-curHp-half", "currentHealth 180 -> 90");
    CHECK(r.newMaxStamina == 250,
          "D-maxStam-half", "maxStamina 500 -> 250");
    CHECK(r.newCurrentStamina == 200,
          "D-curStam-half", "currentStamina 400 -> 200");
    CHECK(r.newMaxMana == 50,
          "D-maxMana-half", "maxMana 100 -> 50");
    CHECK(r.newCurrentMana == 40,
          "D-curMana-half", "currentMana 80 -> 40");
    CHECK(r.statIncrements[0] == 12,
          "D-stat0", "all rng=0 -> stat[0] gets 12 increments");
    CHECK(r.statIncrements[1] == 0 && r.statIncrements[6] == 0,
          "D-stat-no-leak", "no other stat receives increments");

    /* Spread rng: 12 increments distributed across 7 stats */
    r = F0864_RESURRECTION_ComputeReincarnation_Compat(
        200, 180, 500, 400, 100, 80, rng_spread);
    total = 0;
    for (i = 0; i < 7; ++i) total += r.statIncrements[i];
    CHECK(total == 12,
          "D-total", "spread rng: total increments == 12");
    CHECK(r.statIncrements[0] == 2,
          "D-stat0-2", "rng 0,0 -> stat[0] gets 2 increments");
    CHECK(r.statIncrements[4] == 2,
          "D-stat4-2", "rng 4,4 -> stat[4] gets 2 increments");
}

/* ===== Block E: candidate recruit + panel cycle ========================= */

/* COMMAND.C F0280 (F0866) appends a Hall portrait candidate to the party
 * (G0305 += 1, G0299 = old G0305 + 1).  REVIVE.C F0282 (F0867) finalises
 * the candidate panel: cancel decrements G0305 and skips the mirror-sensor
 * disable; resurrect/reincarnate keep G0305 and disable the first C03
 * sensor on the mirror square (BUG0_87).  The cycle must be safe to
 * repeat so the same portrait can be retried after a cancel. */
static void block_e_recruit_panel_cycle(void)
{
    ChampionPortraitClickInput_Compat click;
    CandidateChampionAddResult_Compat add;
    CandidatePanelState_Compat panel;
    CandidatePanelResult_Compat decision;
    MirrorThing_Compat mirror[3];
    MirrorSensorDisableResult_Compat disable;
    int cycle;

    memset(&click, 0, sizeof(click));
    click.command = DM1_COMMAND_CLICK_IN_DUNGEON_VIEW;
    click.leaderEmptyHanded = 1;
    click.leaderIndex = DM1_CHAMPION_NONE;
    click.frontWallOrnamentHit = 1;
    click.facingAlcove = 0;
    click.frontSquareInBounds = 1;
    click.sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    click.sensorData = 11;             /* C026 portrait index */
    click.sensorCell = 2;
    click.clickedWallCell = 2;
    click.partyChampionCount = 0;

    /* Recruit + cancel cycle must be repeatable without breaking G0299. */
    for (cycle = 0; cycle < 3; ++cycle) {
        click.partyChampionCount = 0;
        add = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&click);
        CHECK(add.triggersCandidateAdd == 1,
              "E-cycle-recruit", "F0280 route armed after cancel");
        CHECK(add.candidateChampionOrdinal == 1,
              "E-cycle-ordinal", "G0299 ordinal resets to 1 each cycle");
        CHECK(add.nextPartyChampionCount == 1,
              "E-cycle-count", "G0305 increments to 1 each cycle");

        panel.partyChampionCount = add.nextPartyChampionCount;
        panel.candidateChampionOrdinal = add.candidateChampionOrdinal;
        decision = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(
            panel, DM1_COMMAND_CANCEL);
        CHECK(decision.valid == 1,
              "E-cycle-cancel-valid", "cancel valid with G0299 set");
        CHECK(decision.nextPartyChampionCount == 0,
              "E-cycle-cancel-count", "cancel decrements G0305");
        CHECK(decision.disablesMirrorSensor == 0,
              "E-cycle-cancel-no-disable",
              "cancel does not disable mirror sensor");
    }

    /* Resurrect must disable first C03 sensor on mirror square. */
    click.partyChampionCount = 0;
    add = F0866_RESURRECTION_RouteChampionPortraitClick_Compat(&click);
    panel.partyChampionCount = add.nextPartyChampionCount;
    panel.candidateChampionOrdinal = add.candidateChampionOrdinal;
    decision = F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(
        panel, DM1_COMMAND_RESURRECT);
    CHECK(decision.valid == 1 && decision.resurrected == 1,
          "E-resurrect", "resurrect finalises the candidate panel");

    /* BUG0_87 mirror-sensor disable helper: first C03 sensor wins
     * regardless of M039_TYPE(sensor); C02 textstrings are skipped. */
    mirror[0].thingType = DM1_THING_TYPE_TEXTSTRING;
    mirror[0].sensorType = 9999;
    mirror[1].thingType = DM1_THING_TYPE_SENSOR;
    mirror[1].sensorType = 12;
    mirror[2].thingType = DM1_THING_TYPE_SENSOR;
    mirror[2].sensorType = DM1_SENSOR_WALL_CHAMPION_PORTRAIT;
    disable = F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat(mirror, 3);
    CHECK(disable.foundSensor == 1,
          "E-disable-found", "first C03 sensor found (BUG0_87)");
    CHECK(disable.disabledThingIndex == 1,
          "E-disable-skip-text", "textstring before sensor is skipped");
    CHECK(disable.disabledOldSensorType == 12,
          "E-disable-old-type",
          "first sensor type recorded even when not C127");
    CHECK(disable.disabledNewSensorType == DM1_SENSOR_DISABLED,
          "E-disable-new-type", "sensor type becomes C000_DISABLED");
}

/* ===== Block F: full death -> bones -> Vi-altar cycle gate ============== */

/* Drive the entire F0319 -> F0374 -> TIMELINE.C -> F0283 chain for a
 * single champion through the F0860/F0862/F0861/F0868/F0863 helpers,
 * then verify the rebirth produces the same champion index, the
 * correct health penalty, and the expected dirty attributes. */
static void block_f_full_death_cycle(void)
{
    const uint16_t dyingChampion = 2;
    const uint16_t dyingCell = 1;
    const uint16_t partyMask = 0x0Bu;     /* cells 0/1/3 occupied */
    const uint16_t partyDirection = 3;    /* DIR_WEST */
    const int16_t oldMaxHealth = 100;

    SimulatedBones bones = simulate_f0319_kill(dyingChampion, dyingCell);
    ViAltarFullCycleInput_Compat in;
    ViAltarFullCycleResult_Compat r;

    /* Step 1: CHAMPION.C F0319 produced bones, with chargeCount == dying
     * champion index.  This is the contract that lets the F0868 cycle
     * select the right candidate. */
    CHECK(bones.junkType == DM1_JUNK_TYPE_BONES,
          "F-bones-type", "F0319 bones type == C05_JUNK_BONES");
    CHECK(bones.chargeCount == dyingChampion,
          "F-bones-charge", "F0319 bones.ChargeCount == dying champion index");
    CHECK(F0862_RESURRECTION_GetChampionIndexFromBones_Compat(bones.chargeCount)
              == dyingChampion,
          "F-bones-roundtrip", "F0862 round-trip recovers dying champion");

    /* Step 2: CLIKVIEW.C F0374 alcove + Vi-altar detection. */
    CHECK(simulate_f0374_bones_drop(&bones, 1, 1, partyDirection) == 1,
          "F-trigger", "F0374 detects alcove+Vi-altar+bones drop");

    /* Step 3: TIMELINE.C 1665-1698 + REVIVE.C F0283 via F0868. */
    memset(&in, 0, sizeof(in));
    in.championIndex = dyingChampion;
    in.oldChampionCell = dyingCell;
    in.occupiedCellMask = partyMask;
    in.partyDirection = partyDirection;
    in.maximumHealth = oldMaxHealth;
    in.droppingIntoAlcove = 1;
    in.facingViAltar = 1;
    in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
    in.bonesChargeCount = bones.chargeCount;
    in.bonesCell = bones.cell;
    r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);
    CHECK(r.revived == 1,
          "F-revived", "full death -> Vi-altar cycle revives the champion");
    CHECK(r.championIndex == dyingChampion,
          "F-reborn-idx", "rebirth preserves the dying champion index");
    CHECK(r.finalMaximumHealth == 98,
          "F-maxHp-penalty", "rebirth applies F0863 penalty 100 -> 98");
    CHECK(r.finalCurrentHealth == 49,
          "F-curHp-half", "rebirth sets currentHealth = newMaxHealth / 2");
    CHECK(r.finalDirection == partyDirection,
          "F-dir-copy", "rebirth copies party direction (G0308)");
    CHECK((r.dirtyAttributes & DM1_CHAMPION_ATTR_ACTION_HAND) != 0,
          "F-dirty-hand", "rebirth marks MASK0x8000_ACTION_HAND");
    CHECK((r.dirtyAttributes & DM1_CHAMPION_ATTR_STATUS_BOX) != 0,
          "F-dirty-status", "rebirth marks MASK0x1000_STATUS_BOX");
    CHECK((r.dirtyAttributes & DM1_CHAMPION_ATTR_ICON) != 0,
          "F-dirty-icon", "rebirth marks MASK0x0400_ICON");
}

/* ===== Block G: end-to-end 4-champion death storm ====================== */

/* All four party champions die in sequence, each producing bones at
 * their own cell.  Each subsequent resurrection must pick the correct
 * cell-free fallback and apply the correct health penalty. */
static void block_g_four_champion_storm(void)
{
    static const struct {
        uint16_t idx;
        uint16_t cell;
        int16_t maxHp;
        uint16_t dir;
        uint16_t partyMask;
    } cases[] = {
        /* Champion 0 dies at cell 0, party of four still alive */
        { 0, 0, 80,  0, 0x0Fu },
        /* Champion 1 dies at cell 2, party mask 0..3 minus 2 */
        { 1, 2, 60,  1, 0x0Bu },
        /* Champion 2 dies at cell 1, party mask 0/2/3 minus 1 -> cell 1 free */
        { 2, 1, 100, 2, 0x0Du },
        /* Champion 3 dies at cell 3, only cell 0 occupied -> cell 3 free */
        { 3, 3, 50,  3, 0x01u },
    };
    int i;

    for (i = 0; i < 4; ++i) {
        SimulatedBones bones = simulate_f0319_kill(cases[i].idx, cases[i].cell);
        ViAltarFullCycleInput_Compat in;
        ViAltarFullCycleResult_Compat r;
        int16_t expectedMaxHp = (int16_t)(cases[i].maxHp - (cases[i].maxHp >> 6) - 1);
        if (expectedMaxHp < 25) expectedMaxHp = 25;
        {
            uint16_t probe;
            uint16_t expectedCell = cases[i].cell;
            uint16_t mask = cases[i].partyMask;
            if (mask & (1u << cases[i].cell)) {
                for (probe = 0; probe < 4; ++probe) {
                    if ((mask & (1u << probe)) == 0) { expectedCell = probe; break; }
                }
            }
            memset(&in, 0, sizeof(in));
            in.championIndex = cases[i].idx;
            in.oldChampionCell = cases[i].cell;
            in.occupiedCellMask = cases[i].partyMask;
            in.partyDirection = cases[i].dir;
            in.maximumHealth = cases[i].maxHp;
            in.droppingIntoAlcove = 1;
            in.facingViAltar = 1;
            in.objectIconIndex = DM1_ICON_CHAMPION_BONES;
            in.bonesChargeCount = bones.chargeCount;
            in.bonesCell = bones.cell;
            r = F0868_RESURRECTION_RunViAltarFullCycle_Compat(&in);

            CHECK(r.revived == 1, "G-storm-revived",
                  "storm champion rebirth reached");
            CHECK(r.championIndex == cases[i].idx,
                  "G-storm-idx", "storm champion index preserved");
            CHECK(r.finalCell == expectedCell,
                  "G-storm-cell", "storm cell fallback matches expected");
            CHECK(r.finalMaximumHealth == expectedMaxHp,
                  "G-storm-maxHp", "storm health penalty per F0863");
            CHECK(r.finalCurrentHealth == (int16_t)(expectedMaxHp >> 1),
                  "G-storm-curHp", "storm currentHealth = newMaxHealth / 2");
            CHECK(r.finalDirection == cases[i].dir,
                  "G-storm-dir", "storm direction copied from party");
        }
    }
}

/* ===== Block H: command validation ====================================== */

static void block_h_command_validation(void)
{
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(
              DM1_COMMAND_RESURRECT, 1) == 1,
          "H-resurrect-valid", "C160 valid with party count 1");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(
              DM1_COMMAND_REINCARNATE, 1) == 1,
          "H-reincarnate-valid", "C161 valid with party count 1");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(
              DM1_COMMAND_CANCEL, 1) == 1,
          "H-cancel-valid", "C162 valid with party count 1");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(
              DM1_COMMAND_RESURRECT, 0) == 0,
          "H-empty-party", "C160 invalid with empty party");
    CHECK(F0865_RESURRECTION_IsCommandValid_Compat(99, 1) == 0,
          "H-bad-command", "unknown command invalid");
}

/* ===== Main ============================================================== */

int main(void)
{
    printf("probe=firestaff_dm1_v1_champion_death_resurrection_gate\n");
    printf("scope=F0860..F0868 end-to-end death/bones/altar/resurrection "
           "integration\n");

    block_a_bones_round_trip();
    block_b_vi_altar_trigger();
    block_c_vi_altar_full_cycle();
    block_d_reincarnation();
    block_e_recruit_panel_cycle();
    block_f_full_death_cycle();
    block_g_four_champion_storm();
    block_h_command_validation();

    printf("\nresult=%d/%d pass, %d fail, %d block\n",
           g_pass, g_pass + g_fail, g_fail, g_block);
    printf("sourceEvidence=%s\n", dm1_v1_resurrection_GetEvidence());
    printf("resurrectionInvariantOk=%u\n",
           dm1_v1_resurrection_GetInvariant());

    return (g_fail == 0 && g_block == 0) ? 0 : 1;
}
