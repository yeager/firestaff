/*
 * dm2_v1_new_game.h — DM2 V1 New Game & Session Management API
 *
 * Phase 6: Utility/import flow — DM2-specific load/start flow.
 *
 * Implements:
 *   1. Starter party generation — creates 4 initial champions at game start
 *   2. New game flow — full boot→game→dungeon→party pipeline
 *   3. Session save/load — save/load round-trip with slot manager
 *
 * Source locks (ReDMCSB WIP20210206):
 *   CHAMPION.C F0280 — CHAMPION_AddCandidateChampionToParty: portrait index
 *     to squad position assignment, initial attribute setting.
 *     SKULL.ASM T520 — party placement and start position (Hall of Champions).
 *     SKULL.ASM T560 — dungeon load completion and party state init.
 *     CHAMPRST.C F0278 — CHAMPION_ResetDataToStartGame: clears hand, clears
 *       champion load/HP/name/title masks on new game.
 *   REQDISK.C F0428 — DIALOG_RequireGameDiskInDrive_NoDialogDrawn: floppy
 *     disk check gate (N/A for modern file-based loading).
 *   docs/dm2_party_state.md — champion record (261 bytes), SUPPRESS mask,
 *     portrait→class mapping, HP/stamina/mana initial values.
 *   docs/dm2_save_format.md — slot header layout (42 bytes), SUPPRESS
 *     game-state block (56 bytes), party state encoding.
 *
 * DM2 starter party conventions:
 *   - 4 champions, positions TL/TR/BL/BR in party view
 *   - Portrait index selects portrait from GRAPHICS.DAT portrait strip
 *   - Class is derived from portrait (portrait range → class)
 *   - Initial HP = 50, Stamina = 50, Mana = 20 (varies by class)
 *   - All champions start in the Hall of Champions (mapX=15,mapY=15,N)
 *   - Gold = 100, Time = 720 (noon), no outdoor mode at start
 */

#ifndef FIRESTAFF_DM2_V1_NEW_GAME_H
#define FIRESTAFF_DM2_V1_NEW_GAME_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* dm2_v1_boot.h defines DM2_V1_BootProfile */
#include "dm2_v1_boot.h"

/* dm2_v1_save_load.h defines DM2_ChampionRecord, DM2_GameStateBlock,
 * SUPPRESS codec, and slot manager */
#include "dm2_v1_save_load.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ════════════════════════════════════════════════════════════════
 * Champion class IDs (DM2 character classes)
 * Source: ReDMCSB CHAMPION.C G0417_apc_BaseSkillNames
 * ════════════════════════════════════════════════════════════════ */

typedef enum {
    DM2_CLASS_FIGHTER = 0,
    DM2_CLASS_NINJA    = 1,
    DM2_CLASS_PRIEST   = 2,
    DM2_CLASS_WIZARD   = 3,
} DM2_ChampionClass;

/* Champion portrait indices (into GRAPHICS.DAT portrait strip).
 * These are the 8 portrait slots used by DM2 for character creation.
 * Source: ReDMCSB CHAMPION.C portrait index enumeration */
typedef enum {
    DM2_PORTRAIT_FIGHTER_MALE   = 0,
    DM2_PORTRAIT_FIGHTER_FEMALE = 1,
    DM2_PORTRAIT_NINJA_MALE     = 2,
    DM2_PORTRAIT_NINJA_FEMALE   = 3,
    DM2_PORTRAIT_PRIEST_MALE    = 4,
    DM2_PORTRAIT_PRIEST_FEMALE  = 5,
    DM2_PORTRAIT_WIZARD_MALE    = 6,
    DM2_PORTRAIT_WIZARD_FEMALE  = 7,
    DM2_PORTRAIT_COUNT          = 8,
} DM2_PortraitIndex;

/* Champion view-cell positions in party display.
 * Source: ReDMCSB CHAMPION.C F0280 — cell assignment by direction */
typedef enum {
    DM2_VIEW_CELL_FRONT_LEFT  = 0,
    DM2_VIEW_CELL_FRONT_RIGHT = 1,
    DM2_VIEW_CELL_BACK_LEFT   = 2,
    DM2_VIEW_CELL_BACK_RIGHT  = 3,
} DM2_ViewCell;

/* ════════════════════════════════════════════════════════════════
 * Champion initial attribute tables
 * Source: ReDMCSB CHAMPION.C F0280 — base stat initialization
 *         docs/dm2_party_state.md — champion record initial values
 * ════════════════════════════════════════════════════════════════ */

/* Initial HP by class (cur_hp = max_hp at creation) */
#define DM2_INITIAL_HP_FIGHTER  60
#define DM2_INITIAL_HP_NINJA    45
#define DM2_INITIAL_HP_PRIEST   40
#define DM2_INITIAL_HP_WIZARD   30

/* Initial Stamina by class */
#define DM2_INITIAL_STAMINA_FIGHTER  70
#define DM2_INITIAL_STAMINA_NINJA    65
#define DM2_INITIAL_STAMINA_PRIEST   55
#define DM2_INITIAL_STAMINA_WIZARD   40

/* Initial Mana by class (Priests/Wizards start with some mana) */
#define DM2_INITIAL_MANA_FIGHTER  0
#define DM2_INITIAL_MANA_NINJA    10
#define DM2_INITIAL_MANA_PRIEST   30
#define DM2_INITIAL_MANA_WIZARD   45

/* Attribute index order (matching ReDMCSB C1_STATISTIC_STRENGTH et al.) */
#define DM2_STAT_STRENGTH     0
#define DM2_STAT_BRAVERY      1
#define DM2_STAT_PIETY        2
#define DM2_STAT_VIGOR        3
#define DM2_STAT_DEXTERITY    4
#define DM2_STAT_WISDOM       5
#define DM2_STAT_ANTIFIRE     6

/* Base attribute value at champion creation (minimum 30 per ReDMCSB) */
#define DM2_BASE_ATTRIBUTE    30

/* ════════════════════════════════════════════════════════════════
 * Session state — encapsulates all DM2 runtime state for save/load
 * This is what gets serialized to a save slot.
 * Source: docs/dm2_save_format.md — full save file layout
 * ════════════════════════════════════════════════════════════════ */

/* Maximum serialized session size (conservative estimate) */
#define DM2_SESSION_MAX_SIZE  (2 * 1024 * 1024)

/* Session version marker — written in slot header extension */
#define DM2_SESSION_VERSION   1

typedef struct {
    /* Game tick counter — SUPPRESS-encoded 4-byte field */
    uint32_t game_tick;

    /* RNG seed for this session */
    uint32_t rng_seed;

    /* Party state */
    uint8_t  champion_count;
    uint8_t  leader_index;       /* 0-3, index into champions[] */
    uint16_t party_x;
    uint16_t party_y;
    uint8_t  party_dir;          /* 0=N 1=E 2=S 3=W */
    uint8_t  party_level;        /* current dungeon/outdoor level */
    uint8_t  outdoor_mode;       /* 0=dungeon 1=outdoor 2=building */

    /* Time-of-day (0-1439 minutes) */
    uint16_t time_of_day_minutes;

    /* Resources */
    uint32_t gold;             /* up to ~4 billion gold pieces */
    int16_t  reputation;

    /* Weather (outdoor) */
    uint8_t  rain_intensity;     /* 0-100 */
    uint8_t  weather_padding;

    /* Champion records (4 × 261 bytes, SUPPRESS-encoded) */
    uint8_t  champion_data[4][261];

    /* Dungeon state (variable — level data) */
    /* Note: full dungeon state saved separately via dungeon_serialize() */

    /* Companion state (4 companions × name+stats) */
    /* Note: companion state serialized as fixed-size records */

    /* Minion table */
    /* Note: minion table serialized as count + entries */

    /* Padding to max size */
    uint8_t  reserved[256];
} DM2_V1_SessionState;

/* ════════════════════════════════════════════════════════════════
 * New game API
 * ════════════════════════════════════════════════════════════════ */

/* Generate a starter party of 4 champions.
 * Populates the 4 champion records in session with predefined
 * names, portraits, classes, and initial stats.
 *
 * Starter party (DM2 default, Hall of Champions):
 *   Champion 0: "Theron"  — Fighter, Portrait 0, TL position
 *   Champion 1: "Karla"   — Ninja,   Portrait 3, TR position
 *   Champion 2: "Aldric"  — Priest,  Portrait 4, BL position
 *   Champion 3: "Seraphina" — Wizard, Portrait 7, BR position
 *
 * Source: ReDMCSB F0280 FILL-CHAMPION-DATA (portrait→class assignment)
 *         docs/dm2_party_state.md § Starter party definition
 */
void dm2_v1_generate_starter_party(DM2_V1_SessionState *session);

/* Champion class from portrait index.
 * DM2 portrait ranges map to character classes:
 *   Portraits 0-1 → Fighter
 *   Portraits 2-3 → Ninja
 *   Portraits 4-5 → Priest
 *   Portraits 6-7 → Wizard
 * Source: ReDMCSB CHAMPION.C G0417_apc_BaseSkillNames */
DM2_ChampionClass dm2_v1_portrait_to_class(uint8_t portrait_index);

/* Build a single champion record from portrait + class.
 * Fills the 261-byte champion record using SUPPRESS encoding.
 * Source: ReDMCSB CHAMPION.C F0280 — champion data initialization */
void dm2_v1_build_champion_record(DM2_ChampionRecord *record,
                                   const char *first_name,
                                   const char *last_name,
                                   uint8_t portrait_index,
                                   DM2_ChampionClass champ_class,
                                   uint8_t view_cell,
                                   uint8_t direction);

/* Set initial attributes for a champion based on class.
 * Rolls 7 attributes (STR, BRV, PIY, VIG, DEX, WIS, ANF) each ≥ 30.
 * Class modifies the class-derived attributes (STR/BRV for Fighter,
 * DEX/VIG for Ninja, PIY/WIS for Priest, WIS/PIY for Wizard).
 * Source: ReDMCSB CHAMPION.C F0280 attribute assignment loop */
void dm2_v1_set_initial_attributes(DM2_ChampionRecord *record,
                                    DM2_ChampionClass champ_class);

/* ════════════════════════════════════════════════════════════════
 * Session management API
 * ════════════════════════════════════════════════════════════════ */

/* Initialize a session state with a new game.
 * Sets game_tick=0, party to Hall of Champions, generates starter party.
 * Does NOT load dungeon data — caller must do that separately. */
void dm2_v1_session_new(DM2_V1_SessionState *session);

/* Serialize session state to a flat byte buffer.
 * Returns bytes written, or -1 on error (buffer too small).
 * Format: see DM2_V1_SessionState layout above.
 * Source: docs/dm2_save_format.md — session serialization */
int dm2_v1_session_serialize(const DM2_V1_SessionState *session,
                               uint8_t *buf, size_t buf_size);

/* Deserialize a byte buffer into a session state.
 * Returns 0 on success, -1 on error (invalid data).
 * Source: docs/dm2_save_format.md — session deserialization */
int dm2_v1_session_deserialize(DM2_V1_SessionState *session,
                                 const uint8_t *buf, size_t buf_size);

/* Save session to slot N using the slot manager.
 * Combines serialize + dm2_sl_save.
 * Returns 0 on success.
 * Source: dm2_v1_save_load.h dm2_sl_save() */
int dm2_v1_session_save_slot(const char *save_base, uint8_t slot,
                               const char *name,
                               const DM2_V1_SessionState *session);

/* Load session from slot N using the slot manager.
 * Combines dm2_sl_load + deserialize.
 * Returns 0 on success.
 * Source: dm2_v1_save_load.h dm2_sl_load() */
int dm2_v1_session_load_slot(const char *save_base, uint8_t slot,
                               DM2_V1_SessionState *session);

/* Delete a saved session in slot N.
 * Returns 0 on success. */
int dm2_v1_session_delete_slot(const char *save_base, uint8_t slot);

/* Verify session state is internally consistent.
 * Checks: champion_count ≤ 4, leader_index < champion_count,
 * champion records have non-zero names, game_tick ≥ 0.
 * Returns true if valid. */
bool dm2_v1_session_validate(const DM2_V1_SessionState *session);

/* ════════════════════════════════════════════════════════════════
 * New game flow — full boot→game pipeline
 * ════════════════════════════════════════════════════════════════ */

/* Result of a new game or load game operation */
typedef enum {
    DM2_FLOW_OK             = 0,
    DM2_FLOW_NO_ASSETS      = -1,
    DM2_FLOW_NO_DUNGEON     = -2,
    DM2_FLOW_BAD_SESSION    = -3,
    DM2_FLOW_SLOT_ERROR     = -4,
    DM2_FLOW_ALLOC_ERROR    = -5,
} DM2_FlowResult;

/* Initialize a new DM2 game from the boot profile.
 * This is the full new-game flow:
 *   1. Scan and verify DM2 assets (uses boot profile's scan)
 *   2. Load dungeon data
 *   3. Generate starter party (4 champions)
 *   4. Set initial party position (Hall of Champions)
 *
 * After this function, the session is ready for the game loop.
 *
 * Source: SKULL.ASM T520 — party_placement (Hall of Champions, N)
 *         SKULL.ASM T560 — DUNGEON_Load completion
 *         CHAMPION.C F0280 — starter party generation
 *         CHAMPRST.C F0278 — CHAMPION_ResetDataToStartGame
 */
DM2_FlowResult dm2_v1_new_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot);

/* Load a saved game from slot N.
 * Deserializes the session state from the save slot.
 * Caller is responsible for loading dungeon data separately
 * (dungeon is shared across saves and loaded once at boot).
 *
 * Source: dm2_v1_save_load.h dm2_sl_load()
 *         docs/dm2_save_format.md — save slot layout */
DM2_FlowResult dm2_v1_load_game_flow(DM2_V1_SessionState *session,
                                      const DM2_V1_BootProfile *boot,
                                      uint8_t slot);

/* ════════════════════════════════════════════════════════════════
 * Source evidence
 * ════════════════════════════════════════════════════════════════ */

const char *dm2_v1_new_game_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_NEW_GAME_H */
