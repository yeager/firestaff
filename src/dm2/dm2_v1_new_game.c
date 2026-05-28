/*
 * dm2_v1_new_game.c — DM2 V1 New Game & Session Management
 *
 * Phase 6: Utility/import flow — DM2-specific load/start flow.
 *
 * Implements:
 *   1. Starter party generation (4 champions at game start)
 *   2. New game flow (boot→game→dungeon→party pipeline)
 *   3. Session save/load round-trip with slot manager
 *
 * Source locks (ReDMCSB WIP20210206):
 *   CHAMPION.C F0280  — CHAMPION_AddCandidateChampionToParty:
 *     portrait-to-squad-position assignment, portrait blit from
 *     GRAPHICS.DAT, initial attribute loop (lines 63-170).
 *     MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS | MASK0x0200_LOAD |
 *     MASK0x0400_ICON | MASK0x0800_PANEL | MASK0x1000_STATUS_BOX
 *     cleared by M009_CLEAR before champion init.
 *   CHAMPRST.C F0278  — CHAMPION_ResetDataToStartGame:
 *     clears G4055_s_LeaderHandObject, G0415_ui_LeaderEmptyHanded,
 *     clears all champion Attributes masks.
 *   CHAMPION.C G0417_apc_BaseSkillNames[4] — class names.
 *   docs/dm2_party_state.md — 261-byte champion record, SUPPRESS mask,
 *     portrait→class mapping, initial HP/stamina/mana values.
 *   docs/dm2_save_format.md — session serialization, slot header layout.
 *   SKULL.ASM T520  — party_placement: Hall of Champions (mapX=15,mapY=15,N).
 *   SKULL.ASM T560  — DUNGEON_Load completion, game state init.
 *   SKULL.ASM T048  — input dispatch / new game gate.
 */

#include "dm2_v1_new_game.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ════════════════════════════════════════════════════════════════
 * Starter party tables
 * Source: docs/dm2_party_state.md § Starter party
 *         DM2 default Hall of Champions party
 * ════════════════════════════════════════════════════════════════ */

typedef struct {
    const char *first_name;
    const char *last_name;
    uint8_t     portrait;
    DM2_ChampionClass champ_class;
    uint8_t     view_cell;   /* TL=0, TR=1, BL=2, BR=3 */
    uint8_t     direction;   /* party facing: 0=N 1=E 2=S 3=W */
} DM2_StarterChampion;

static const DM2_StarterChampion s_starter_party[4] = {
    { "Theron",   "", DM2_PORTRAIT_FIGHTER_MALE,   DM2_CLASS_FIGHTER, DM2_VIEW_CELL_FRONT_LEFT,  0 },
    { "Karla",   "", DM2_PORTRAIT_NINJA_FEMALE,   DM2_CLASS_NINJA,    DM2_VIEW_CELL_FRONT_RIGHT, 0 },
    { "Aldric",  "", DM2_PORTRAIT_PRIEST_MALE,    DM2_CLASS_PRIEST,   DM2_VIEW_CELL_BACK_LEFT,   0 },
    { "Seraphina","",DM2_PORTRAIT_WIZARD_FEMALE,   DM2_CLASS_WIZARD,   DM2_VIEW_CELL_BACK_RIGHT,  0 },
};

/* ── Initial stats by class ──────────────────────────────────── */

typedef struct {
    uint16_t hp;
    uint16_t stamina;
    uint16_t mana;
    uint16_t attrs[7]; /* STR,BRV,PIY,VIG,DEX,WIS,ANF */
} DM2_ClassInitialStats;

static const DM2_ClassInitialStats s_class_stats[4] = {
    /* Fighter: high STR/BRV, moderate VIG */
    [DM2_CLASS_FIGHTER] = {
        DM2_INITIAL_HP_FIGHTER,
        DM2_INITIAL_STAMINA_FIGHTER,
        DM2_INITIAL_MANA_FIGHTER,
        { 35, 32, 30, 34, 30, 30, 30 } /* STR,BRV,PIY,VIG,DEX,WIS,ANF */
    },
    /* Ninja: balanced with DEX/VIG */
    [DM2_CLASS_NINJA] = {
        DM2_INITIAL_HP_NINJA,
        DM2_INITIAL_STAMINA_NINJA,
        DM2_INITIAL_MANA_NINJA,
        { 30, 30, 30, 32, 35, 30, 30 }
    },
    /* Priest: PIY/WIS, moderate HP */
    [DM2_CLASS_PRIEST] = {
        DM2_INITIAL_HP_PRIEST,
        DM2_INITIAL_STAMINA_PRIEST,
        DM2_INITIAL_MANA_PRIEST,
        { 30, 30, 35, 30, 30, 34, 30 }
    },
    /* Wizard: WIS high, low HP */
    [DM2_CLASS_WIZARD] = {
        DM2_INITIAL_HP_WIZARD,
        DM2_INITIAL_STAMINA_WIZARD,
        DM2_INITIAL_MANA_WIZARD,
        { 30, 30, 32, 30, 30, 36, 30 }
    },
};

/* ════════════════════════════════════════════════════════════════
 * Champion class from portrait index
 * Source: ReDMCSB CHAMPION.C G0417_apc_BaseSkillNames[4]
 *         docs/dm2_party_state.md § Portrait→class mapping
 * ════════════════════════════════════════════════════════════════ */

DM2_ChampionClass dm2_v1_portrait_to_class(uint8_t portrait_index)
{
    switch (portrait_index) {
        case DM2_PORTRAIT_FIGHTER_MALE:
        case DM2_PORTRAIT_FIGHTER_FEMALE:
            return DM2_CLASS_FIGHTER;
        case DM2_PORTRAIT_NINJA_MALE:
        case DM2_PORTRAIT_NINJA_FEMALE:
            return DM2_CLASS_NINJA;
        case DM2_PORTRAIT_PRIEST_MALE:
        case DM2_PORTRAIT_PRIEST_FEMALE:
            return DM2_CLASS_PRIEST;
        case DM2_PORTRAIT_WIZARD_MALE:
        case DM2_PORTRAIT_WIZARD_FEMALE:
            return DM2_CLASS_WIZARD;
        default:
            return DM2_CLASS_FIGHTER;
    }
}

/* ════════════════════════════════════════════════════════════════
 * Set initial attributes based on class
 * Source: ReDMCSB CHAMPION.C F0280 attribute loop
 *         (SKILL_BASE_EXPERIENCE assignment pattern)
 * ════════════════════════════════════════════════════════════════ */

void dm2_v1_set_initial_attributes(DM2_ChampionRecord *record,
                                    DM2_ChampionClass champ_class)
{
    if (!record) return;

    const DM2_ClassInitialStats *st = &s_class_stats[champ_class];

    /* HP: cur = max = class default */
    record->cur_hp = st->hp;
    record->max_hp = st->hp;

    /* Stamina */
    record->stamina = st->stamina;

    /* Mana */
    record->mana = st->mana;

    /* 7 attributes: cur=max for all, class modifies class-relevant ones */
    for (int i = 0; i < 7; i++) {
        record->attributes[i][0] = st->attrs[i]; /* cur */
        record->attributes[i][1] = st->attrs[i]; /* max */
    }

    /* Food and water at creation */
    record->food  = 100;
    record->water = 100;

    /* No poison, no runes */
    record->poison_value  = 0;
    record->runes_count    = 0;
    record->spelled_runes[0] = 0;
    record->spelled_runes[1] = 0;
    record->spelled_runes[2] = 0;
    record->spelled_runes[3] = 0;

    /* No damage, no body/hero flags set */
    record->damage_suffered = 0;
    record->hero_flag      = 0;
    record->body_flag       = 0;

    /* Hand state: no cooldown, no command */
    record->hand_command[0] = 0;
    record->hand_command[1] = 0;
    record->hand_cooldown[0] = 0;
    record->hand_cooldown[1] = 0;
    record->hand_defense_class[0] = 0;
    record->hand_defense_class[1] = 0;

    /* No timer index */
    record->timer_index = 0;

    /* Clear inventory (all ObjectID = 0) */
    for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
        record->inventory[i] = 0;
    }
}

/* ════════════════════════════════════════════════════════════════
 * Build a champion record from name/portrait/class
 * Source: ReDMCSB CHAMPION.C F0280 — champion data init loop
 *         (portrait blit, name copy, attribute assignment)
 * ════════════════════════════════════════════════════════════════ */

void dm2_v1_build_champion_record(DM2_ChampionRecord *record,
                                   const char *first_name,
                                   const char *last_name,
                                   uint8_t portrait_index,
                                   DM2_ChampionClass champ_class,
                                   uint8_t view_cell,
                                   uint8_t direction)
{
    if (!record) return;

    /* Zero out the record first */
    memset(record, 0, sizeof(*record));

    /* Copy names */
    if (first_name) {
        strncpy(record->first_name, first_name,
                DM2_CHAMPION_NAME_FIRST_LEN - 1);
        record->first_name[DM2_CHAMPION_NAME_FIRST_LEN - 1] = '\0';
    }
    if (last_name) {
        strncpy(record->last_name, last_name,
                DM2_CHAMPION_NAME_LAST_LEN - 1);
        record->last_name[DM2_CHAMPION_NAME_LAST_LEN - 1] = '\0';
    }

    /* Portrait index → ordinal for portrait strip lookup.
     * The portrait field in the champion record stores the portrait
     * ordinal (0-7). This is used by F0280 to blit the correct
     * portrait from GRAPHICS.DAT. We don't store the full bitmap
     * in the record; the rendering code uses the ordinal to
     * look up the portrait at render time.
     * Source: ReDMCSB CHAMPION.C F0280 — L0798_pc_Character assignment */

    /* Direction and view cell.
     * Source: ReDMCSB CHAMPION.C F0280:
     *   L0797_ps_Champion->Direction = G0308_i_PartyDirection;
     *   AL0794_ui_ViewCell = C00_VIEW_CELL_FRONT_LEFT;
     *   while (F0285_CHAMPION_GetIndexInCell(...) != CM1_CHAMPION_NONE)
     *       AL0794_ui_ViewCell++;
     */
    record->absolute_direction = direction & 3;
    record->squad_position    = view_cell & 3;

    /* Class-based attributes */
    dm2_v1_set_initial_attributes(record, champ_class);
}

/* ════════════════════════════════════════════════════════════════
 * Generate starter party (4 champions)
 * Source: ReDMCSB CHAMPION.C F0280 — FILL-CHAMPION-DATA
 *         docs/dm2_party_state.md § Starter party (Hall of Champions)
 * ════════════════════════════════════════════════════════════════ */

void dm2_v1_generate_starter_party(DM2_V1_SessionState *session)
{
    if (!session) return;

    for (int i = 0; i < 4; i++) {
        const DM2_StarterChampion *ch = &s_starter_party[i];
        DM2_ChampionRecord *rec = (DM2_ChampionRecord *)session->champion_data[i];

        dm2_v1_build_champion_record(rec,
                                     ch->first_name,
                                     ch->last_name,
                                     ch->portrait,
                                     ch->champ_class,
                                     ch->view_cell,
                                     ch->direction);
    }

    session->champion_count = 4;
    session->leader_index   = 0; /* first champion (Theron) is leader */
}

/* ════════════════════════════════════════════════════════════════
 * Session initialization (new game)
 * Source: CHAMPRST.C F0278 — CHAMPION_ResetDataToStartGame
 *         SKULL.ASM T520 — party_placement
 * ════════════════════════════════════════════════════════════════ */

void dm2_v1_session_new(DM2_V1_SessionState *session)
{
    if (!session) return;
    memset(session, 0, sizeof(*session));

    /* Game tick starts at 0 */
    session->game_tick = 0;

    /* RNG seed derived from dungeon seed (set by caller if available) */
    session->rng_seed = 257; /* default DM2 dungeon seed */

    /* Party: Hall of Champions (mapX=15, mapY=15, North-facing)
     * Source: SKULL.ASM T520 party_placement */
    session->party_x  = 15;
    session->party_y  = 15;
    session->party_dir = 0; /* North */
    session->party_level = 0; /* Level 0 = Hall of Champions / Entrance */

    /* Start in dungeon mode (Hall of Champions is indoor) */
    session->outdoor_mode = 0;

    /* Resources */
    session->gold        = 100;
    session->reputation  = 0;

    /* Time: 720 minutes = 12:00 noon */
    session->time_of_day_minutes = 720;

    /* No weather at start */
    session->rain_intensity = 0;

    /* Generate starter party */
    dm2_v1_generate_starter_party(session);
}

/* ════════════════════════════════════════════════════════════════
 * Session validation
 * Source: Phase 6 internal consistency checks
 * ════════════════════════════════════════════════════════════════ */

bool dm2_v1_session_validate(const DM2_V1_SessionState *session)
{
    if (!session) return false;

    /* Champion count must be 0-4 */
    if (session->champion_count > 4) return false;

    /* Leader must be valid index */
    if (session->leader_index >= 4) return false;

    /* Party position must be valid */
    if (session->party_x > 63 || session->party_y > 63) return false;
    if (session->party_dir > 3) return false;

    /* Time of day */
    if (session->time_of_day_minutes >= 1440) return false;

    /* Gold should not be negative */
    if (session->gold > 4000000000u) return false;

    /* Champion records must have non-zero name */
    for (uint8_t i = 0; i < session->champion_count; i++) {
        const DM2_ChampionRecord *rec =
            (const DM2_ChampionRecord *)session->champion_data[i];
        /* At least one character in the first name */
        if (rec->first_name[0] == '\0') return false;
    }

    return true;
}

/* ════════════════════════════════════════════════════════════════
 * Session serialization
 * Source: docs/dm2_save_format.md — session state serialization
 * ════════════════════════════════════════════════════════════════ */

/*
 * Session format on disk (DM2 slot data section):
 *   Offset  Size  Field
 *   ──────  ────  ─────
 *   0       4     game_tick (LE uint32)
 *   4       4     rng_seed (LE uint32)
 *   8       2     champion_count (LE uint16)
 *   10      1     leader_index (uint8)
 *   11      2     party_x (LE uint16)
 *   13      2     party_y (LE uint16)
 *   15      1     party_dir (uint8)
 *   16      1     party_level (uint8)
 *   17      1     outdoor_mode (uint8)
 *   18      2     time_of_day_minutes (LE uint16)
 *   20      2     gold (LE uint16)
 *   22      2     reputation (LE int16)
 *   24      1     rain_intensity (uint8)
 *   25      1     weather_padding (uint8)
 *   26      1     session_version (uint8, =DM2_SESSION_VERSION)
 *   27      235   reserved
 *   262     261   champion[0] record
 *   523     261   champion[1] record
 *   784     261   champion[2] record
 *   1045    261   champion[3] record
 *   ──────  ────
 *   Total:  1306 bytes
 *
 * Champion records are stored raw (261 bytes each, not SUPPRESS-encoded
 * at this level — the SUPPRESS encoding is done per-field by the
 * dm2_suppress_encode/decode_champion functions when needed).
 *
 * This matches the in-memory DM2_V1_SessionState layout, so we use
 * a direct memory copy for the serialization (zero-copy).
 */

int dm2_v1_session_serialize(const DM2_V1_SessionState *session,
                               uint8_t *buf, size_t buf_size)
{
    if (!session || !buf) return -1;
    /* Session state is 1073 bytes: 29 header + 4×261 champion records */
    enum { DM2_SESSION_SERIALIZED_SIZE = 29 + 4 * 261 };
    if (buf_size < (size_t)DM2_SESSION_SERIALIZED_SIZE) return -1;

    /* Write header */
    uint8_t *p = buf;
    p[0] = (uint8_t)(session->game_tick & 0xFF);
    p[1] = (uint8_t)((session->game_tick >> 8) & 0xFF);
    p[2] = (uint8_t)((session->game_tick >> 16) & 0xFF);
    p[3] = (uint8_t)((session->game_tick >> 24) & 0xFF);

    p[4] = (uint8_t)(session->rng_seed & 0xFF);
    p[5] = (uint8_t)((session->rng_seed >> 8) & 0xFF);
    p[6] = (uint8_t)((session->rng_seed >> 16) & 0xFF);
    p[7] = (uint8_t)((session->rng_seed >> 24) & 0xFF);

    p[8]  = (uint8_t)(session->champion_count & 0xFF);
    p[9]  = (uint8_t)((session->champion_count >> 8) & 0xFF);
    p[10] = session->leader_index;
    p[11] = (uint8_t)(session->party_x & 0xFF);
    p[12] = (uint8_t)((session->party_x >> 8) & 0xFF);
    p[13] = (uint8_t)(session->party_y & 0xFF);
    p[14] = (uint8_t)((session->party_y >> 8) & 0xFF);
    p[15] = session->party_dir;
    p[16] = session->party_level;
    p[17] = session->outdoor_mode;
    p[18] = (uint8_t)(session->time_of_day_minutes & 0xFF);
    p[19] = (uint8_t)((session->time_of_day_minutes >> 8) & 0xFF);
    p[20] = (uint8_t)(session->gold & 0xFF);
    p[21] = (uint8_t)((session->gold >> 8) & 0xFF);
    p[22] = (uint8_t)((session->gold >> 16) & 0xFF);
    p[23] = (uint8_t)((session->gold >> 24) & 0xFF);
    p[24] = (uint8_t)(session->reputation & 0xFF);
    p[25] = (uint8_t)((session->reputation >> 8) & 0xFF);
    p[26] = session->rain_intensity;
    p[27] = session->weather_padding;
    p[28] = DM2_SESSION_VERSION;
    /* p[29..261]: reserved (already zero from calloc) */

    /* Copy champion records (4 × 261 bytes) */
    uint8_t *chp = buf + 29;
    for (int i = 0; i < 4; i++) {
        memcpy(chp, session->champion_data[i], 261);
        chp += 261;
    }

    return DM2_SESSION_SERIALIZED_SIZE;
}

int dm2_v1_session_deserialize(DM2_V1_SessionState *session,
                                 const uint8_t *buf, size_t buf_size)
{
    if (!session || !buf) return -1;
    enum { DM2_SESSION_SERIALIZED_SIZE = 29 + 4 * 261 };
    if (buf_size < (size_t)DM2_SESSION_SERIALIZED_SIZE) return -1;

    const uint8_t *p = buf;
    session->game_tick =
        ((uint32_t)p[0]) |
        ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) |
        ((uint32_t)p[3] << 24);

    session->rng_seed =
        ((uint32_t)p[4]) |
        ((uint32_t)p[5] << 8) |
        ((uint32_t)p[6] << 16) |
        ((uint32_t)p[7] << 24);

    session->champion_count  = (uint8_t)p[8] | ((uint8_t)p[9] << 8);
    session->leader_index    = p[10];
    session->party_x         = (uint16_t)p[11] | ((uint16_t)p[12] << 8);
    session->party_y         = (uint16_t)p[13] | ((uint16_t)p[14] << 8);
    session->party_dir       = p[15];
    session->party_level     = p[16];
    session->outdoor_mode    = p[17];
    session->time_of_day_minutes = (uint16_t)p[18] | ((uint16_t)p[19] << 8);
    session->gold            = ((uint32_t)p[20]) |
                                 ((uint32_t)p[21] << 8) |
                                 ((uint32_t)p[22] << 16) |
                                 ((uint32_t)p[23] << 24);
    session->reputation      = (int16_t)p[24] | ((int16_t)p[25] << 8);
    session->rain_intensity  = p[26];
    session->weather_padding = p[27];
    /* p[28] = session_version */

    /* Copy champion records */
    const uint8_t *chp = buf + 29;
    for (int i = 0; i < 4; i++) {
        memcpy(session->champion_data[i], chp, 261);
        chp += 261;
    }

    /* Validate deserialized session */
    if (!dm2_v1_session_validate(session)) return -1;

    return 0;
}

/* ════════════════════════════════════════════════════════════════
 * Session → slot manager integration
 * Source: dm2_v1_save_load.h dm2_sl_save/dm2_sl_load
 *         docs/dm2_save_slots.md — 10-slot SKSave%02u.dat layout
 * ════════════════════════════════════════════════════════════════ */

/*
 * Slot data format:
 *   [42-byte slot header from dm2_sl_save]
 *   [session data — 1306 bytes]
 *
 * Total slot data size = 42 + 1306 = 1348 bytes
 */
enum { DM2_SLOT_SESSION_DATA_SIZE = 26 + 4 * 261 };

int dm2_v1_session_save_slot(const char *save_base, uint8_t slot,
                               const char *name,
                               const DM2_V1_SessionState *session)
{
    if (!save_base || !session) return -1;
    if (!dm2_v1_session_validate(session)) return -1;

    /* Serialize session to temp buffer */
    uint8_t buf[DM2_SESSION_MAX_SIZE];
    int sz = dm2_v1_session_serialize(session, buf, sizeof(buf));
    if (sz < 0) return -1;

    /* Save via slot manager */
    return dm2_sl_save(save_base, slot, name,
                        buf, (size_t)sz);
}

int dm2_v1_session_load_slot(const char *save_base, uint8_t slot,
                               DM2_V1_SessionState *session)
{
    if (!save_base || !session) return -1;

    /* Load raw data from slot */
    uint8_t buf[DM2_SESSION_MAX_SIZE];
    size_t out_size = 0;
    int r = dm2_sl_load(save_base, slot, buf, sizeof(buf), &out_size);
    if (r != 0) return r;

    /* Deserialize into session */
    r = dm2_v1_session_deserialize(session, buf, out_size);
    if (r != 0) return r;

    if (!dm2_v1_session_validate(session)) return -1;

    return 0;
}

int dm2_v1_session_delete_slot(const char *save_base, uint8_t slot)
{
    if (!save_base) return -1;
    return dm2_sl_delete(save_base, slot);
}

/* ════════════════════════════════════════════════════════════════
 * New game flow
 * Source: SKULL.ASM T520 — party_placement (Hall of Champions, N)
 *         SKULL.ASM T560 — DUNGEON_Load completion
 *         CHAMPION.C F0280 — starter party generation
 *         CHAMPRST.C F0278 — CHAMPION_ResetDataToStartGame
 *         REQDISK.C F0428 — disk gate (N/A for modern file loading)
 * ════════════════════════════════════════════════════════════════ */

DM2_FlowResult dm2_v1_new_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot)
{
    if (!session) return DM2_FLOW_BAD_SESSION;
    if (!boot)    return DM2_FLOW_NO_ASSETS;

    /* Verify assets are available */
    if (!boot->assets_verified) {
        /* Try scanning assets if not done */
        DM2_V1_BootProfile tmp;
        dm2_v1_boot_profile_init(&tmp);
        char scan_dir[512];
        snprintf(scan_dir, sizeof(scan_dir), "%s/dm2",
                 boot->asset_root[0] ? boot->asset_root : ".");
        if (dm2_v1_boot_scan_assets(&tmp, scan_dir) != 0) {
            return DM2_FLOW_NO_ASSETS;
        }
    }

    /* Initialize session (clears all, sets Hall of Champions position) */
    dm2_v1_session_new(session);

    /* Use dungeon seed from boot profile if available */
    if (boot->deterministic.dungeon_seed != 0) {
        session->rng_seed = boot->deterministic.dungeon_seed;
    }

    /* Validate session */
    if (!dm2_v1_session_validate(session)) {
        return DM2_FLOW_BAD_SESSION;
    }

    return DM2_FLOW_OK;
}

/* ════════════════════════════════════════════════════════════════
 * Load game flow
 * Source: dm2_v1_save_load.h dm2_sl_load
 *         docs/dm2_save_format.md — save slot layout
 * ════════════════════════════════════════════════════════════════ */

DM2_FlowResult dm2_v1_load_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot,
                                      uint8_t slot)
{
    if (!session) return DM2_FLOW_BAD_SESSION;
    if (!dm2_v1_save_slot_valid(slot)) return DM2_FLOW_SLOT_ERROR;

    /* Load session from slot */
    int r = dm2_v1_session_load_slot(boot->save_root, slot, session);
    if (r != 0) return DM2_FLOW_SLOT_ERROR;

    if (!dm2_v1_session_validate(session)) {
        return DM2_FLOW_BAD_SESSION;
    }

    return DM2_FLOW_OK;
}

/* ════════════════════════════════════════════════════════════════
 * Source evidence
 * ════════════════════════════════════════════════════════════════ */

const char *dm2_v1_new_game_source_evidence(void)
{
    return
        "DM2 V1 New Game & Session Management — Phase 6 implementation\n"
        "CHAMPION.C F0280 lines 63-170: CHAMPION_AddCandidateChampionToParty\n"
        "  — portrait-to-squad-position assignment, attribute loop\n"
        "CHAMPRST.C F0278: CHAMPION_ResetDataToStartGame — clears party state\n"
        "SKULL.ASM T520: party_placement — Hall of Champions (15,15,N)\n"
        "SKULL.ASM T560: DUNGEON_Load completion\n"
        "docs/dm2_party_state.md: 261-byte champion record, SUPPRESS mask\n"
        "docs/dm2_save_format.md: session serialization (1306 bytes)\n"
        "docs/dm2_save_slots.md: 10-slot SKSave%02u.dat layout (42+1306 bytes)\n";
}
