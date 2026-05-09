#ifndef DM1_V1_RESURRECTION_PC34_COMPAT_H
#define DM1_V1_RESURRECTION_PC34_COMPAT_H

/*
 * DM1 V1 Resurrection & Reincarnation System — Source-locked from ReDMCSB.
 *
 * Pure, caller-driven port of:
 *   COMMAND.C/CLIKVIEW.C/MOVESENS.C — C080 viewport portrait click route
 *              to C127 wall champion portrait sensor and F0280 candidate add,
 *   REVIVE.C — F0280 (AddCandidateChampionToParty),
 *              F0282 (ProcessCommands160To162_ClickInResurrectReincarnatePanel),
 *              F0283 (ViAltarRebirth)
 *   CHAMPION.C — F0319 (Kill → bones creation)
 *   CLIKVIEW.C — F0374 (DropLeaderHandObject → Vi Altar bones detection)
 *
 * ReDMCSB source references:
 *   COMMAND.C:228-233 panel button boxes; 2318-2324 dispatches C080 to F0377.
 *   CLIKVIEW.C:5-25 touches front-square/opposite-wall sensors; 311-349
 *     normalizes viewport coordinates; 406-431 sends front wall ornament C05 to F0372.
 *   MOVESENS.C:1309-1395 checks wall sensor cell and permits C127 with no leader;
 *     1501-1503 calls F0280 for C127 with sensor data.
 *   DUNGEON.C:2608-2612 displays the visible portrait from the same C127 data.
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
#define DM1_COMMAND_CLICK_IN_DUNGEON_VIEW    80   /* C080_COMMAND_CLICK_IN_DUNGEON_VIEW */
#define DM1_SENSOR_DISABLED                   0   /* C000_SENSOR_DISABLED */
#define DM1_SENSOR_WALL_CHAMPION_PORTRAIT   127   /* C127_SENSOR_WALL_CHAMPION_PORTRAIT */
#define DM1_THING_TYPE_TEXTSTRING             2   /* C02_THING_TYPE_TEXTSTRING */
#define DM1_THING_TYPE_SENSOR                 3   /* C03_THING_TYPE_SENSOR */
#define DM1_VIEW_CELL_DOOR_BUTTON_OR_ORNAMENT 5   /* C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT */
#define DM1_CHAMPION_NONE                    -1   /* CM1_CHAMPION_NONE */
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

/* -------- Hall of Champions portrait route (C080 -> C127 -> F0280/F0282) */

typedef struct {
    int16_t command;                 /* C080 from COMMAND.C */
    int leaderEmptyHanded;           /* G0415_ui_LeaderEmptyHanded */
    int16_t leaderIndex;             /* G0411_i_LeaderIndex; -1 allowed for C127 */
    int frontWallOrnamentHit;        /* C05 click box hit in F0377 */
    int facingAlcove;                /* G0286_B_FacingAlcove blocks F0372 */
    int frontSquareInBounds;         /* F0372 map bounds check */
    uint16_t sensorType;             /* M039_TYPE(sensor) */
    uint16_t sensorData;             /* M040_DATA(sensor): champion portrait index */
    uint16_t sensorCell;             /* M011_CELL(sensor) */
    uint16_t clickedWallCell;        /* M018_OPPOSITE(G0308_i_PartyDirection) */
    uint16_t partyChampionCount;     /* G0305 before F0280 */
} ChampionPortraitClickInput_Compat;

typedef struct {
    int triggersCandidateAdd;        /* C127/F0280 is reached */
    uint16_t championPortraitIndex;  /* argument to F0280 */
    uint16_t nextPartyChampionCount; /* F0280 result if applied */
    uint16_t candidateChampionOrdinal; /* G0299, ordinal == previous count + 1 */
    uint16_t candidateChampionIndex; /* inserted at previous party count */
    int setLeaderToFirstChampion;    /* F0280 when first champion enters party */
} CandidateChampionAddResult_Compat;

/* F0866: Source-faithful route predicate for Hall portrait clicks.
 * Source: COMMAND.C:2318-2324; CLIKVIEW.C:5-25,311-349,406-431;
 * MOVESENS.C:1309-1395,1501-1503; REVIVE.C:124-132,272-276. */
CandidateChampionAddResult_Compat F0866_RESURRECTION_RouteChampionPortraitClick_Compat(
    const ChampionPortraitClickInput_Compat* in);

typedef struct {
    uint16_t partyChampionCount;      /* G0305 with candidate already appended */
    uint16_t candidateChampionOrdinal;/* G0299 non-zero while panel is open */
} CandidatePanelState_Compat;

typedef struct {
    int valid;
    int cancelled;
    int resurrected;
    int reincarnated;
    int disablesMirrorSensor;         /* REVIVE.C:794-799 for non-cancel */
    uint16_t candidateChampionIndex;  /* G0305 - 1 */
    uint16_t nextPartyChampionCount;  /* cancel decrements; finalize keeps */
    uint16_t nextCandidateChampionOrdinal; /* cleared to 0 */
} CandidatePanelResult_Compat;

/* Minimal thing-list model for REVIVE.C:794-799 mirror finalization.
 * ReDMCSB BUG0_87 deliberately disables the first C03 sensor thing found on
 * the mirror square; it does not search for C127, and text strings are skipped. */
typedef struct {
    uint16_t thingType;      /* M012_TYPE(thing), e.g. C02 textstring/C03 sensor */
    uint16_t sensorType;     /* M039_TYPE(sensor), meaningful only for C03 sensor */
} MirrorThing_Compat;

typedef struct {
    int foundSensor;
    int disabledThingIndex;      /* -1 if no C03 sensor appears in bounded input */
    uint16_t disabledOldSensorType;
    uint16_t disabledNewSensorType;
} MirrorSensorDisableResult_Compat;

/* F0867a: Source-faithful mirror sensor disable helper.
 * Source: REVIVE.C:794-799 walks F0161/F0159 thing-list and disables the first
 * C03 sensor thing with M044_SET_TYPE_DISABLED, regardless of its sensor type.
 * DUNGEON.C:2568-2583 shows C02 textstrings share wall-square traversal but are
 * not sensor things; MOVESENS.C:1501-1503 is only the click-time C127 route. */
MirrorSensorDisableResult_Compat F0867a_RESURRECTION_DisableFirstMirrorSensor_Compat(
    const MirrorThing_Compat* things,
    uint16_t thingCount);

/* F0867: Finalize the candidate champion decision panel.
 * Source: REVIVE.C:744-785 cancel clears candidate and decrements party count;
 * REVIVE.C:785-806 non-cancel clears candidate and disables a mirror sensor;
 * REVIVE.C:806-835 applies reincarnation-specific changes. */
CandidatePanelResult_Compat F0867_RESURRECTION_ProcessCandidatePanelCommand_Compat(
    CandidatePanelState_Compat state,
    int16_t command);

/* -------- Evidence / invariant (project convention) ------------------ */

const char* dm1_v1_resurrection_GetEvidence(void);
unsigned int dm1_v1_resurrection_GetInvariant(void);

#endif /* DM1_V1_RESURRECTION_PC34_COMPAT_H */
