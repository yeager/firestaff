#ifndef DM1_V1_RESURRECTION_PC34_COMPAT_H
#define DM1_V1_RESURRECTION_PC34_COMPAT_H

/*
 * DM1 V1 Resurrection & Reincarnation System — Source-locked from ReDMCSB.
 *
 * Pure, caller-driven port of:
 *   REVIVE.C — F0280 (AddCandidateChampionToParty),
 *              F0282 (ProcessCommands160To162_ClickInResurrectReincarnatePanel),
 *              F0283 (ViAltarRebirth)
 *   CHAMPION.C — F0319 (Kill → bones creation)
 *   CLIKVIEW.C — F0374 (DropLeaderHandObject → Vi Altar bones detection)
 *
 * ReDMCSB source references:
 *   REVIVE.C lines: F0280 decodes champion text, sets stats/skills/slots;
 *     F0282 processes commands C160_RESURRECT / C161_REINCARNATE / C162_CANCEL;
 *     F0283 rebirth at Vi Altar: cell assignment, health penalty, slot clear.
 *   CHAMPION.C: F0319 creates C05_JUNK_BONES with ChargeCount=championIndex.
 *   CLIKVIEW.C: F0374 detects C147_ICON_JUNK_CHAMPION_BONES on alcove+ViAltar,
 *     creates C13_EVENT_VI_ALTAR_REBIRTH with Priority=championIndex from bones.
 *   DEFS.H: C05_JUNK_BONES, C147_ICON_JUNK_CHAMPION_BONES,
 *     C13_EVENT_VI_ALTAR_REBIRTH, C160/C161/C162 commands,
 *     MASK0x8000_CHAMPION_BONES, C0xFFE4_THING_EXPLOSION_REBIRTH_STEP1/STEP2.
 *
 * Conventions:
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - NO globals, NO UI, NO IO. All state passed explicitly.
 *   - Function numbering: F0860..F0879 exclusively.
 *   - ADDITIVE ONLY: consumes existing Phase interfaces; never edits them.
 */

#include <stdint.h>

/* -------- Constants (from DEFS.H) ---------------------------------- */

#define DM1_JUNK_TYPE_BONES                   5   /* C05_JUNK_BONES */
#define DM1_ICON_CHAMPION_BONES             147   /* C147_ICON_JUNK_CHAMPION_BONES */
#define DM1_EVENT_TYPE_VI_ALTAR_REBIRTH      13   /* C13_EVENT_VI_ALTAR_REBIRTH */
#define DM1_COMMAND_RESURRECT               160   /* C160_COMMAND_CLICK_IN_PANEL_RESURRECT */
#define DM1_COMMAND_REINCARNATE             161   /* C161_COMMAND_CLICK_IN_PANEL_REINCARNATE */
#define DM1_COMMAND_CANCEL                  162   /* C162_COMMAND_CLICK_IN_PANEL_CANCEL */
#define DM1_EXPLOSION_REBIRTH_STEP1       0xFFE4  /* C0xFFE4 */
#define DM1_EXPLOSION_REBIRTH_STEP2       0xFFE5  /* C0xFFE5 */

/* Bones item structure (mirrors JUNK in DEFS.H):
 * Type=C05_JUNK_BONES, DoNotDiscard=1, ChargeCount=championIndex (0..3) */

/* -------- Resurrection state ---------------------------------------- */

/* Compact state for a resurrection/reincarnation operation.
 * Populated by the caller from game state; consumed by pure functions. */
typedef struct {
    uint16_t championIndex;       /* Which champion to resurrect/revive */
    uint16_t partyChampionCount;  /* G0305 */
    int16_t  partyDirection;      /* G0308 */
    int16_t  partyMapX;           /* G0306 */
    int16_t  partyMapY;           /* G0307 */
} ResurrectionContext_Compat;

/* -------- Bones creation (from F0319_CHAMPION_Kill) ----------------- */

/* Result of creating bones when a champion dies.
 * ReDMCSB CHAMPION.C F0319 lines:
 *   L0964_T_Thing = F0166_DUNGEON_GetUnusedThing(MASK0x8000_CHAMPION_BONES | C10_THING_TYPE_JUNK);
 *   L0966_ps_Junk->Type = C05_JUNK_BONES;
 *   L0966_ps_Junk->DoNotDiscard = C1_TRUE;
 *   L0966_ps_Junk->ChargeCount = P0661_ui_ChampionIndex; */
typedef struct {
    uint8_t  junkType;          /* Must be DM1_JUNK_TYPE_BONES (5) */
    uint8_t  doNotDiscard;      /* Must be 1 */
    uint8_t  chargeCount;       /* Champion index (0..3) */
    uint16_t cell;              /* Cell where bones are placed */
    int      valid;             /* 1 if bones were created, 0 if no unused thing */
} BonesCreationResult_Compat;

/* F0860: Compute bones creation parameters for a dying champion.
 * Source: CHAMPION.C F0319 lines creating bones thing. */
BonesCreationResult_Compat F0860_RESURRECTION_ComputeBonesCreation_Compat(
    uint16_t championIndex,
    uint16_t championCell);

/* -------- Vi Altar detection (from F0374 / CLIKVIEW.C) -------------- */

/* F0861: Check if dropping an object on an alcove triggers Vi Altar rebirth.
 * Source: CLIKVIEW.C F0374:
 *   if (L1146_B_DroppingIntoAnAlcove && G0287_B_FacingViAltar &&
 *       (F0033_OBJECT_GetIconIndex(L1142_T_Thing) == C147_ICON_JUNK_CHAMPION_BONES))
 * Returns 1 if rebirth event should be created, 0 otherwise. */
int F0861_RESURRECTION_ShouldTriggerViAltarRebirth_Compat(
    int droppingIntoAlcove,
    int facingViAltar,
    int objectIconIndex);

/* F0862: Get champion index from bones object.
 * Source: CLIKVIEW.C F0374:
 *   L1143_ps_Junk = (JUNK*)F0156_DUNGEON_GetThingData(L1142_T_Thing);
 *   L1147_s_Event.A.A.Priority = L1143_ps_Junk->ChargeCount; */
uint8_t F0862_RESURRECTION_GetChampionIndexFromBones_Compat(
    uint8_t bonesChargeCount);

/* -------- Vi Altar Rebirth (from F0283 / REVIVE.C) ------------------ */

/* Result of the rebirth health penalty computation.
 * Source: REVIVE.C F0283:
 *   AL0831_ui_MaximumHealth = L0832_ps_Champion->MaximumHealth;
 *   L0832_ps_Champion->CurrentHealth =
 *     (L0832_ps_Champion->MaximumHealth =
 *       F0025_MAIN_GetMaximumValue(25,
 *         AL0831_ui_MaximumHealth - (AL0831_ui_MaximumHealth >> 6) - 1)) >> 1; */
typedef struct {
    int16_t newMaxHealth;     /* max(25, oldMax - oldMax/64 - 1) */
    int16_t newCurrentHealth; /* newMaxHealth / 2 */
} RebirthHealthResult_Compat;

/* F0863: Compute health penalty for Vi Altar rebirth.
 * Source: REVIVE.C F0283 health calculation. */
RebirthHealthResult_Compat F0863_RESURRECTION_ComputeRebirthHealth_Compat(
    int16_t currentMaxHealth);

/* -------- Resurrect/Reincarnate panel (from F0282 / REVIVE.C) ------- */

/* Reincarnation stat changes result.
 * Source: REVIVE.C F0282 (MEDIA265_S20E version — DM1 V1):
 *   Skills cleared; Health/Stamina/Mana halved;
 *   12 random stat increments distributed. */
typedef struct {
    int16_t newMaxHealth;
    int16_t newCurrentHealth;
    int16_t newMaxStamina;
    int16_t newCurrentStamina;
    int16_t newMaxMana;
    int16_t newCurrentMana;
    /* Statistics changes: +1 applied randomly to 12 random stats (0..6) */
    uint8_t statIncrements[7]; /* Count of +1 increments per stat index */
} ReincarnationResult_Compat;

/* F0864: Compute reincarnation stat changes (DM1 V1 / MEDIA265_S20E rules).
 * Source: REVIVE.C F0282, MEDIA265 branch:
 *   CurrentHealth >>= 1; MaximumHealth >>= 1; (same for Stamina, Mana)
 *   Skills cleared; 12 random increments to random stats.
 * rngValues: array of 12 values, each pre-rolled as RANDOM(7) (0..6). */
ReincarnationResult_Compat F0864_RESURRECTION_ComputeReincarnation_Compat(
    int16_t maxHealth, int16_t currentHealth,
    int16_t maxStamina, int16_t currentStamina,
    int16_t maxMana, int16_t currentMana,
    const uint8_t rngValues[12]);

/* F0865: Check if resurrect/reincarnate/cancel command is valid.
 * Source: REVIVE.C F0282 entry checks. */
int F0865_RESURRECTION_IsCommandValid_Compat(
    int16_t command,
    uint16_t partyChampionCount);

/* -------- Evidence / invariant (project convention) ------------------ */

const char* dm1_v1_resurrection_GetEvidence(void);
unsigned int dm1_v1_resurrection_GetInvariant(void);

#endif /* DM1_V1_RESURRECTION_PC34_COMPAT_H */
