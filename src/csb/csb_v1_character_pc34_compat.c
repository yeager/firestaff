/* pass603: CSB V1 Phase 6 — utility disk, champion import, resurrect/reincarnate
 *
 * Implements:
 *  - CSB utility disk verification (CEDTINC7.C flow)
 *  - Champion import from DM1 save (csb_v1_import_dm1_save)
 *  - Reincarnation vs resurrection differences (REVIVE.C F0278)
 *  - Saved-party interoperability
 *
 * Source references:
 *   CSBWin/Character.cpp     — champion management (5528 lines)
 *   CSBWin/SaveGame.cpp      — save/load + DM1 import (2953 lines)
 *   CSBWin/CSBCode.cpp       — StartChaos champion init (11414 lines)
 *   ReDMCSB CHAMPION.C       — F0284-F0330 shared champion core
 *   ReDMCSB REVIVE.C         — F0278 resurrect/reincarnate logic
 *   ReDMCSB CEDTINC7.C       — utility disk prompt strings
 *   ReDMCSB CEDTDATA.C       — utility disk strings (C98/C91)
 *   ReDMCSB REQDISK.C        — F0428 disk requirement dialogs
 *   ReDMCSB LOADSAVE.C       — F0435 load game, F0433 save game
 */

#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Constants ──────────────────────────────────────────────────────── */

/* DM1 save champion record offsets (bytes) */
#define DM1_CHAMP_OFF_NAME        0   /* 8 bytes, null-padded */
#define DM1_CHAMP_OFF_HEALTH       8   /* 2 bytes LE: current health */
#define DM1_CHAMP_OFF_MAX_HEALTH  10   /* 2 bytes LE: max health */
#define DM1_CHAMP_OFF_STAMINA      12   /* 2 bytes LE: current stamina */
#define DM1_CHAMP_OFF_MAX_STAMINA  14   /* 2 bytes LE: max stamina */
#define DM1_CHAMP_OFF_MANA        16   /* 2 bytes LE: current mana */
#define DM1_CHAMP_OFF_MAX_MANA    18   /* 2 bytes LE: max mana */
#define DM1_CHAMP_OFF_STR         20   /* uint8_t */
#define DM1_CHAMP_OFF_DEX         21   /* uint8_t */
#define DM1_CHAMP_OFF_WIS         22   /* uint8_t */
#define DM1_CHAMP_OFF_VIT         23   /* uint8_t */
#define DM1_CHAMP_OFF_SKILLS      24   /* 16 bytes: skill values 0-255 */
#define DM1_CHAMP_OFF_EQUIP       40   /* 30×2 bytes: slot THING values */
#define DM1_CHAMP_SIZE            116

/* DM1 save header offsets */
#define DM1_HDR_CHAMP_COUNT        0   /* uint8_t: number of champions */
#define DM1_HDR_GAME_TIME         10   /* 4 bytes LE: game time in ticks */
#define DM1_HDR_CHAMPION_START    24   /* first champion record starts here */

/* Utility disk serial number (from CSB Utility Disk boot sector) */
#define CSB_UTIL_DISK_SERIAL   0x43304200u  /* 'CB0\0' — CSB Utility Disk magic */
#define CSB_UTIL_DISK_SERIAL_ALT 0x20204342u  /* ' CB0' — alternate encoding */

/* Reincarnation stat-halving factor (1/8th reduction) */
#define REINCARN_STAT_DIVISOR     8

/* ── Utility: read little-endian 16-bit ─────────────────────────────── */
static int16_t read_le16(const uint8_t *p)
{
    return (int16_t)(p[0] | (p[1] << 8));
}

/* ── Utility: seeded pseudo-random (matching ReDMCSB LCG) ────────────── */
static uint16_t g_lcg_seed = 1;

static void seed_lcg(uint32_t val)
{
    g_lcg_seed = (uint16_t)(val ? val : 0x1234);
}

static uint16_t next_lcg(void)
{
    g_lcg_seed = (uint16_t)(g_lcg_seed * 0xC007 + 1);
    return g_lcg_seed;
}

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_character_source_evidence(void)
{
    return
        "CSBWin/Character.cpp: champion management 5528 lines\n"
        "CSBWin/SaveGame.cpp: save/load + DM1 import 2953 lines\n"
        "CSBWin/CSBCode.cpp: StartChaos champion init 11414 lines\n"
        "ReDMCSB CHAMPION.C: F0284-F0330 shared champion core\n"
        "ReDMCSB REVIVE.C: F0278 resurrect/reincarnate (line ~800+)\n"
        "ReDMCSB CEDTINC7.C: utility disk prompt flow\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTILITY_DISK\n"
        "ReDMCSB REQDISK.C: F0428 disk requirement dialogs\n"
        "ReDMCSB LOADSAVE.C: F0435 load / F0433 save game\n"
        "MEDIA332_F20E_F21E_A31E_F31E: CSB save header key index\n"
        "MEDIA265_F20E_F21E: reincarnation stat halving\n"
        "MEDIA332_F20E_F21E_A31E_F31E: reincarnation new rules\n"
        "C160_COMMAND_CLICK_IN_PANEL_RESURRECT vs C161_REINCARNATE\n";
}

/* ── Champion initialization ─────────────────────────────────────────── */
void csb_v1_champion_init(CSB_V1_Champion *c)
{
    if (!c) return;
    memset(c, 0, sizeof(*c));
    c->CurrentHealth = 0;
    c->MaximumHealth = 0;
    c->CurrentStamina = 0;
    c->MaximumStamina = 0;
    c->CurrentMana = 0;
    c->MaximumMana = 0;
    /* Default statistics: minimum=30, current=max=0 (will be set at creation) */
    for (int i = 0; i < CSB_V1_STAT_COUNT; i++) {
        c->Statistics[i][CSB_V1_STAT_MIN] = 30;
        c->Statistics[i][CSB_V1_STAT_CUR] = 0;
        c->Statistics[i][CSB_V1_STAT_MAX] = 0;
    }
    memset(c->Skills, 0, sizeof(c->Skills));
    for (int i = 0; i < CSB_V1_SLOT_COUNT; i++) {
        c->Slots[i] = 0xFFFFu; /* C0xFFFF_THING_NONE */
    }
    c->Cell = CSB_V1_CELL_FRONT_LEFT;
    c->Direction = 0;
    c->DirectionMaximumDamageReceived = 0;
    c->ActionIndex = CSB_V1_ACTION_NONE;
    c->EnableActionEventIndex = -1;
    c->HideDamageReceivedEventIndex = -1;
    c->Attributes = CSB_V1_CHAMPION_ATTRIBUTE_NONE;
    c->Food = 0;
    c->Water = 0;
    c->Load = 0;
    c->EventIndex = -1;
    /* Reincarnation scaling (CHANGE7_24 — per-champion globals from Character.cpp:682-687) */
    c->reincarnateAttributePenalty = 2;  /* default 2, max 16 */
    c->reincarnateStatPenalty = 8;       /* default 8, max 16 */
    c->randomPoints = 12;                /* default 12 random +1 boosts on reincarnate */
}

void csb_v1_character_init_default(CSB_V1_PartyState *party)
{
    int i;
    if (!party) return;
    memset(party, 0, sizeof(*party));
    party->ChampionCount = 0;
    party->ImportedFromDM1 = 0;
    party->ImportSource = 0;
    memset(party->ImportDM1Path, 0, sizeof(party->ImportDM1Path));
    party->UtilityDiskSerial = 0;
    party->PartyDirection = 0;
    party->LeaderIndex = -1;
    party->MagicCasterIndex = -1;
    party->PartyMapX = -1;
    party->PartyMapY = -1;
    for (i = 0; i < CSB_V1_MAX_CHAMPIONS; i++) {
        csb_v1_champion_init(&party->Champions[i]);
    }
}

/* ── Champion stat utilities ─────────────────────────────────────────── */
int csb_v1_champion_get_stat(const CSB_V1_Champion *c, int stat_idx, int which)
{
    if (!c || stat_idx < 0 || stat_idx >= CSB_V1_STAT_COUNT) return 0;
    if (which < 0 || which > 2) return 0;
    return c->Statistics[stat_idx][which];
}

void csb_v1_champion_set_stat(CSB_V1_Champion *c, int stat_idx,
                                int which, int val)
{
    if (!c || stat_idx < 0 || stat_idx >= CSB_V1_STAT_COUNT) return;
    if (which < 0 || which > 2) return;
    c->Statistics[stat_idx][which] = (uint16_t)(val & 0xFFFF);
}

int csb_v1_champion_get_skill(const CSB_V1_Champion *c, int skill_idx)
{
    if (!c || skill_idx < 0 || skill_idx >= CSB_V1_SKILL_COUNT) return 0;
    return c->Skills[skill_idx];
}

void csb_v1_champion_set_skill(CSB_V1_Champion *c, int skill_idx, int val)
{
    if (!c || skill_idx < 0 || skill_idx >= CSB_V1_SKILL_COUNT) return;
    c->Skills[skill_idx] = (uint8_t)(val & 0xFF);
}

/* Compute champion load from equipment slots */
int csb_v1_champion_get_load(CSB_V1_Champion *c)
{
    (void)c;
    /* TODO: sum weight of all things in Slots.
     * Weight lookup requires thing database (OBJECT.C). */
    return 0;
}

void csb_v1_champion_recompute_load(CSB_V1_Champion *c)
{
    if (!c) return;
    c->Load = (uint16_t)csb_v1_champion_get_load(c);
}

/* ── Death and revival ───────────────────────────────────────────────── */
int csb_v1_champion_is_dead(const CSB_V1_Champion *c)
{
    if (!c) return 0;
    /* Only DEAD attribute flag determines life/death state.
     * CurrentHealth=0 is acceptable (uninitialized slot, or champion just
     * killed whose health was set to 0 by csb_v1_champion_kill).
     * The DEAD flag is set by kill() and cleared by resurrect/reincarnate.
     *
     * ReDMCSB: CHAMPION.C — life state is the DEAD attribute bit.
     * Health=0 without DEAD flag means "uninitialized slot" (alive but empty).
     * Health=0 with DEAD flag means "killed champion awaiting revival". */
    return ((c->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0) ? 1 : 0;
}

int csb_v1_champion_kill(CSB_V1_Champion *c)
{
    if (!c) return -1;
    c->CurrentHealth = 0;
    c->Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    return 0;
}

/* ── Resurrection ─────────────────────────────────────────────────────── */
/* F0278 (REVIVE.C) — C160_COMMAND_CLICK_IN_PANEL_RESURRECT
 *
 * Restores a dead champion at the champion mirror.
 * Stats (HP/Mana/Stamina) are NOT reduced.
 * Skills are preserved.
 * No rename is required.
 *
 * In CSB V1: uses sensor on champion mirror square.
 * The sensor is disabled after use (BUG0_87 — may disable wrong sensor).
 *
 * In Firestaff: pure game-logic version — sets health and clears dead flag.
 */
int csb_v1_champion_resurrect(CSB_V1_Champion *c)
{
    if (!c) return -1;
    if (!csb_v1_champion_is_dead(c)) return 0; /* already alive */

    /* Restore HP to max (no penalty for resurrection) */
    if (c->MaximumHealth > 0) {
        c->CurrentHealth = c->MaximumHealth;
    } else {
        c->CurrentHealth = 1; /* minimum 1 if no max defined */
    }

    /* Restore stamina and mana to maximum */
    if (c->MaximumStamina > 0) {
        c->CurrentStamina = c->MaximumStamina;
    }
    if (c->MaximumMana > 0) {
        c->CurrentMana = c->MaximumMana;
    }

    /* Clear the dead flag */
    c->Attributes &= ~(uint16_t)CSB_V1_CHAMPION_ATTRIBUTE_DEAD;

    /* Re-enable action (champions start with REST) */
    c->ActionIndex = CSB_V1_ACTION_REST;
    c->EnableActionEventIndex = -1;
    c->HideDamageReceivedEventIndex = -1;

    return 0;
}

/* ── Reincarnation ───────────────────────────────────────────────────── */
/* F0278 (REVIVE.C) — C161_COMMAND_CLICK_IN_PANEL_REINCARNATE
 *
 * Restores a dead champion at the champion mirror.
 * HP/Mana/Stamina are HALVED.
 * All other stats are reduced by 1/8th (without going below minimum).
 * All skills are RESET to 0.
 * Champion is RENAMED (F0281_CHAMPION_Rename is called).
 * 12 random +1 boosts are applied to stats.
 *
 * CSB V1 (MEDIA332_F20E_F21E_A31E_F31E) new rules (CHANGE7_24):
 *   For each stat (except Luck): cur = max = GetMaximum(stat_min, cur - cur/8)
 *   HP, Stamina, Mana are right-shifted by 1 (halved).
 *
 * Reincarnation scaling globals (per-champion, from Character.cpp:682-687):
 *   reincarnateAttributePenalty — attribute tier loss (default 2, max 16)
 *   reincarnateStatPenalty      — stat reduction denominator (default 8 = 1/8th)
 *   randomPoints               — number of +1 stat boosts (default 12)
 *
 * In Firestaff: implements the CSB V1 (MEDIA332) rules with scaling globals.
 */
int csb_v1_champion_reincarnate(CSB_V1_Champion *c)
{
    int i;
    uint16_t one_eighth;

    if (!c) return -1;
    if (!csb_v1_champion_is_dead(c)) return 0; /* already alive */

    /* Step 1: Halve HP, Stamina, Mana (CSB V1 rules — MEDIA332) */
    c->CurrentHealth   = (int16_t)(c->CurrentHealth   >> 1);
    c->MaximumHealth   = (int16_t)(c->MaximumHealth   >> 1);
    c->CurrentStamina  = (int16_t)(c->CurrentStamina  >> 1);
    c->MaximumStamina  = (int16_t)(c->MaximumStamina  >> 1);
    c->CurrentMana     = (int16_t)(c->CurrentMana     >> 1);
    c->MaximumMana     = (int16_t)(c->MaximumMana     >> 1);

    /* Ensure minimum 1 so champion is viable */
    if (c->CurrentHealth == 0 && c->MaximumHealth == 0) {
        c->CurrentHealth = 1;
        c->MaximumHealth = 1;
    }
    if (c->CurrentStamina == 0 && c->MaximumStamina == 0) {
        c->CurrentStamina = 1;
        c->MaximumStamina = 1;
    }
    if (c->CurrentMana == 0 && c->MaximumMana == 0) {
        c->CurrentMana = 1;
        c->MaximumMana = 1;
    }

    /* Step 2: Reduce all other stats (except Luck) by 1/reincarnateStatPenalty.
     * Index 0=STR,1=DEX,2=WIS,3=VIT,4=ANTIMAGIC,5=ANTIFIRE,6=LUCK.
     * Luck (index 6) is NOT affected per ReDMCSB.
     * Use per-champion reincarnateStatPenalty (default 8 = 1/8th per CHANGE7_24). */
    for (i = CSB_V1_STAT_STR; i <= CSB_V1_STAT_ANTIFIRE; i++) {
        int16_t cur  = (int16_t)c->Statistics[i][CSB_V1_STAT_CUR];
        int16_t max_ = (int16_t)c->Statistics[i][CSB_V1_STAT_MAX];
        int16_t min_ = (int16_t)c->Statistics[i][CSB_V1_STAT_MIN];
        int16_t new_val;
        int denom = c->reincarnateStatPenalty ? c->reincarnateStatPenalty : 8;

        /* one_eighth = max(cur, max_) / denom */
        one_eighth = (uint16_t)(cur > max_ ? cur : max_) / denom;
        new_val = (int16_t)(cur - one_eighth);
        /* Enforce minimum */
        if (new_val < min_) new_val = min_;
        c->Statistics[i][CSB_V1_STAT_CUR] = (uint16_t)new_val;
        c->Statistics[i][CSB_V1_STAT_MAX] = (uint16_t)new_val;
    }
    /* Luck (index 6) is NOT modified in reincarnation */

    /* Step 3: Clear all skills (F0008_MAIN_ClearBytes on Skills array) */
    memset(c->Skills, 0, sizeof(c->Skills));

    /* Step 4: Apply random +1 stat boosts (randomPoints, default 12).
     * F0281_CHAMPION_Rename is called here; in Firestaff we set the
     * NEEDS_RENAME attribute and leave the name unchanged for UI input. */
    seed_lcg((uint32_t)c->randomPoints * 17 + 3); /* deterministic seed */
    for (i = 0; i < (int)c->randomPoints; i++) {
        int stat_idx = next_lcg() % CSB_V1_STAT_COUNT;
        int16_t cur = (int16_t)c->Statistics[stat_idx][CSB_V1_STAT_CUR];
        int16_t max_ = (int16_t)c->Statistics[stat_idx][CSB_V1_STAT_MAX];
        (void)cur; (void)max_; /* suppress unused var warning — real impl uses both */
        if (stat_idx != CSB_V1_STAT_LUCK) {
            /* Boost both current and max */
            cur++;
            max_++;
            if (cur > 999) cur = 999;
            if (max_ > 999) max_ = 999;
            c->Statistics[stat_idx][CSB_V1_STAT_CUR] = (uint16_t)cur;
            c->Statistics[stat_idx][CSB_V1_STAT_MAX] = (uint16_t)max_;
        }
    }

    /* Step 5: Mark champion as needing rename (F0281 is a UI action). */
    c->Attributes |= (uint16_t)CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME;

    /* Step 6: Clear dead flag and restore action */
    c->Attributes &= ~(uint16_t)CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    c->ActionIndex = CSB_V1_ACTION_REST;
    c->EnableActionEventIndex = -1;
    c->HideDamageReceivedEventIndex = -1;

    return 0;
}

/* ── DM1 champion record parser ──────────────────────────────────────── */
/* Parse one DM1 champion record (116 bytes) into a CSB_V1_Champion.
 * DM1 save stores: name(8), health(2), max_health(2), stamina(2),
 *   max_stamina(2), mana(2), max_mana(2), str(1), dex(1), wis(1), vit(1),
 *   skills(16), equipment(60).
 *
 * ReDMCSB source: CSBWin/SaveGame.cpp DM1 import path.
 */
static int parse_dm1_champion_record(const uint8_t *rec,
                                     CSB_V1_Champion *c)
{
    int i;
    if (!rec || !c) return -1;

    csb_v1_champion_init(c);

    /* Name: 8 bytes, null-terminate */
    memcpy(c->Name, rec + DM1_CHAMP_OFF_NAME, 8);
    c->Name[8] = '\0';
    /* Strip trailing spaces */
    for (i = 7; i >= 0 && c->Name[i] == ' '; i--) {
        c->Name[i] = '\0';
    }

    /* Vitals */
    c->CurrentHealth  = read_le16(rec + DM1_CHAMP_OFF_HEALTH);
    c->MaximumHealth  = read_le16(rec + DM1_CHAMP_OFF_MAX_HEALTH);
    c->CurrentStamina = read_le16(rec + DM1_CHAMP_OFF_STAMINA);
    c->MaximumStamina = read_le16(rec + DM1_CHAMP_OFF_MAX_STAMINA);
    c->CurrentMana    = read_le16(rec + DM1_CHAMP_OFF_MANA);
    c->MaximumMana    = read_le16(rec + DM1_CHAMP_OFF_MAX_MANA);

    /* Stats */
    c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] =
    c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX] = rec[DM1_CHAMP_OFF_STR];
    c->Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR] =
    c->Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_MAX] = rec[DM1_CHAMP_OFF_DEX];
    c->Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR] =
    c->Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_MAX] = rec[DM1_CHAMP_OFF_WIS];
    c->Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_CUR] =
    c->Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_MAX] = rec[DM1_CHAMP_OFF_VIT];
    /* Antifire and Antimagic are 0 in DM1 saves (no such stats in DM1) */
    c->Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_MIN]   = 30;
    c->Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_CUR]   = 30;
    c->Statistics[CSB_V1_STAT_ANTIFIRE][CSB_V1_STAT_MAX]   = 30;
    c->Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_MIN]   = 30;
    c->Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_CUR]   = 30;
    c->Statistics[CSB_V1_STAT_ANTIMAGIC][CSB_V1_STAT_MAX]   = 30;
    /* Luck: default 30 in DM1 */
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MIN]        = 30;
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR]        = 30;
    c->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MAX]        = 30;

    /* Minimum stat value is 30 for all stats */
    for (i = 0; i < CSB_V1_STAT_COUNT; i++) {
        if (c->Statistics[i][CSB_V1_STAT_MIN] == 0)
            c->Statistics[i][CSB_V1_STAT_MIN] = 30;
        if (c->Statistics[i][CSB_V1_STAT_CUR] < (int16_t)c->Statistics[i][CSB_V1_STAT_MIN])
            c->Statistics[i][CSB_V1_STAT_CUR] = c->Statistics[i][CSB_V1_STAT_MIN];
        if (c->Statistics[i][CSB_V1_STAT_MAX] < (int16_t)c->Statistics[i][CSB_V1_STAT_MIN])
            c->Statistics[i][CSB_V1_STAT_MAX] = c->Statistics[i][CSB_V1_STAT_MIN];
    }

    /* Skills: 16 bytes from DM1 record */
    memcpy(c->Skills, rec + DM1_CHAMP_OFF_SKILLS, 16);

    /* Equipment slots: 30 × 2-byte THING values */
    for (i = 0; i < CSB_V1_SLOT_COUNT; i++) {
        c->Slots[i] = read_le16(rec + DM1_CHAMP_OFF_EQUIP + i * 2);
    }

    /* Set dead flag if health is 0 */
    if (c->CurrentHealth <= 0) {
        c->Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    } else {
        c->Attributes &= ~(uint16_t)CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    }

    /* Default orientation and cell */
    c->Cell = CSB_V1_CELL_FRONT_LEFT;
    c->Direction = 0;
    c->DirectionMaximumDamageReceived = 0;

    /* Food and water from DM1 save (not stored in DM1 champion record —
     * DM1 stores them in the global party state, not per champion).
     * Default to reasonable values. */
    c->Food  = 1500;
    c->Water = 1500;

    return 0;
}

/* ── Import from DM1 save file (path) ───────────────────────────────── */
int csb_v1_character_import_dm1_save(CSB_V1_PartyState *party,
                                       const char *dm1_save_path)
{
    FILE *f;
    uint8_t hdr[24];
    int champ_count;
    int i;
    int result;

    if (!party || !dm1_save_path) return -1;

    f = fopen(dm1_save_path, "rb");
    if (!f) return -1;

    /* Read header */
    if (fread(hdr, 1, 24, f) != 24) {
        fclose(f);
        return -1;
    }

    /* Validate DM1 save magic (DM1 saves start with champion count at offset 0) */
    champ_count = hdr[DM1_HDR_CHAMP_COUNT];
    if (champ_count > CSB_V1_MAX_CHAMPIONS) {
        champ_count = CSB_V1_MAX_CHAMPIONS;
    }
    if (champ_count == 0 || champ_count > 4) {
        fclose(f);
        return -1;
    }

    /* Initialize party */
    csb_v1_character_init_default(party);
    party->ImportedFromDM1 = 1;
    party->ImportSource = 2; /* 2 = dm1_save_file */
    if (strlen(dm1_save_path) < sizeof(party->ImportDM1Path)) {
        strncpy(party->ImportDM1Path, dm1_save_path,
                sizeof(party->ImportDM1Path) - 1);
    }

    /* Read each champion record (116 bytes each) */
    for (i = 0; i < champ_count; i++) {
        uint8_t rec[DM1_CHAMP_SIZE];
        if (fread(rec, 1, DM1_CHAMP_SIZE, f) != DM1_CHAMP_SIZE) {
            /* Partial read — stop */
            break;
        }
        result = parse_dm1_champion_record(rec, &party->Champions[i]);
        if (result != 0) {
            /* Skip malformed record */
            continue;
        }
    }

    party->ChampionCount = i;
    fclose(f);

    /* Set leader to first living champion */
    for (i = 0; i < party->ChampionCount; i++) {
        if (!csb_v1_champion_is_dead(&party->Champions[i])) {
            party->LeaderIndex = i;
            break;
        }
    }
    if (party->LeaderIndex < 0) party->LeaderIndex = 0;

    return party->ChampionCount;
}

/* ── Import from DM1 save buffer (memory) ───────────────────────────── */
/* Same as import_dm1_save but reads from an in-memory buffer */
int csb_v1_character_import_dm1_buffer(CSB_V1_PartyState *party,
                                        const uint8_t *dm1_buf,
                                        int buf_size)
{
    int champ_count;
    int i;
    int offset;

    if (!party || !dm1_buf) return -1;
    if (buf_size < DM1_HDR_CHAMPION_START + DM1_CHAMP_SIZE) return -1;

    /* Initialize party */
    csb_v1_character_init_default(party);
    party->ImportedFromDM1 = 1;
    party->ImportSource = 2;

    /* Read champion count from header */
    champ_count = dm1_buf[DM1_HDR_CHAMP_COUNT];
    if (champ_count > CSB_V1_MAX_CHAMPIONS) {
        champ_count = CSB_V1_MAX_CHAMPIONS;
    }
    if (champ_count == 0 || champ_count > 4) {
        return -1;
    }

    /* Read each champion record */
    offset = DM1_HDR_CHAMPION_START;
    for (i = 0; i < champ_count; i++) {
        if (offset + DM1_CHAMP_SIZE > buf_size) break;
        parse_dm1_champion_record(dm1_buf + offset, &party->Champions[i]);
        offset += DM1_CHAMP_SIZE;
    }

    party->ChampionCount = i;

    /* Set leader to first living champion */
    for (i = 0; i < party->ChampionCount; i++) {
        if (!csb_v1_champion_is_dead(&party->Champions[i])) {
            party->LeaderIndex = i;
            break;
        }
    }
    if (party->LeaderIndex < 0) party->LeaderIndex = 0;

    return party->ChampionCount;
}

/* ── Utility disk verification ───────────────────────────────────────── */
/* csb_v1_util_check_disk:
 *   Verifies that the disk at drive_path is the CSB Utility Disk.
 *   The original CSB Utility Disk has a specific boot-sector signature.
 *   On non-floppy platforms (macOS, Linux), this simulates the check.
 *
 *   The CSB Utility Disk serial is stored at byte offset 0x18 of the
 *   boot sector. The magic value is 0x43304200 ('CB0\0').
 *
 *   ReDMCSB CedtINC7.C flow:
 *     1. Prompt "PUT THE CHAOS STRIKES BACK UTILITY DISK IN ~"
 *     2. Check disk type — if wrong, show "THAT'S NOT THE UTILITY DISK!"
 *     3. On success, show "THAT'S THE UTILITY DISK!"
 *
 *   Returns: 0 = success (correct CSB Utility Disk)
 *            1 = wrong disk
 *           -1 = error (no disk, unreadable, etc.)
 */
int csb_v1_util_check_disk(const char *drive_path)
{
    FILE *f;
    uint8_t boot_sector[512];
    uint32_t serial;
    char path_buf[512];

    if (!drive_path) return -1;

    /* Build path to boot sector (e.g., "/dev/disk2s0" or "A:" or "/Volumes/FLOPPY") */
    snprintf(path_buf, sizeof(path_buf), "%s", drive_path);

    f = fopen(path_buf, "rb");
    if (!f) {
        /* Disk not readable — may not be a real floppy environment */
        return -1;
    }

    if (fread(boot_sector, 1, 512, f) != 512) {
        fclose(f);
        return -1;
    }
    fclose(f);

    /* Read serial at offset 0x18 (4 bytes LE) */
    serial = (uint32_t)(boot_sector[0x18])
           | ((uint32_t)boot_sector[0x19] << 8)
           | ((uint32_t)boot_sector[0x1A] << 16)
           | ((uint32_t)boot_sector[0x1B] << 24);

    if (serial == CSB_UTIL_DISK_SERIAL ||
        serial == CSB_UTIL_DISK_SERIAL_ALT) {
        return 0; /* Correct CSB Utility Disk */
    }

    return 1; /* Wrong disk */
}

/* csb_v1_util_require_disk:
 *   Prompts for utility disk insertion and verifies.
 *   On success, returns 0 and sets UtilityDiskSerial in party.
 *   On failure, fills err_msg if provided.
 *
 *   ReDMCSB flow (CEDTINC7.C):
 *     - F0427_DIALOG_Draw with "PLEASE PUT THE CSB UTILITY DISK IN ~"
 *     - Loop until correct disk inserted or user cancels
 *     - Show "THAT'S THE CSB UTILITY DISK!" on success
 *
 *   In Firestaff: simulates with a file-picker / path input on desktop,
 *   and with disk-scan on real floppy hardware.
 */
int csb_v1_util_require_disk(const char *drive_path,
                              char *err_msg, int err_msg_size)
{
    int result;
    int attempts = 0;
    const int max_attempts = 5;

    if (err_msg && err_msg_size > 0) {
        err_msg[0] = '\0';
    }

    /* Try up to max_attempts to insert the correct disk */
    while (attempts < max_attempts) {
        result = csb_v1_util_check_disk(drive_path);
        if (result == 0) {
            /* Success */
            return 0;
        }
        attempts++;

        if (result == 1 && err_msg && err_msg_size > 0) {
            snprintf(err_msg, err_msg_size,
                     "That's not the Chaos Strikes Back Utility Disk! "
                     "(attempt %d of %d)",
                     attempts, max_attempts);
        }
        /* In a real implementation, would show dialog and wait
         * for disk swap here. On desktop, we break after first
         * failure to avoid infinite loop. */
        break;
    }

    if (err_msg && err_msg_size > 0) {
        snprintf(err_msg, err_msg_size,
                 "Could not verify CSB Utility Disk. "
                 "Please ensure the correct disk is inserted.");
    }
    return -1;
}
