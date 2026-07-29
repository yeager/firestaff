/*
 * csb_v1_runtime_pc34_compat.h — CSB V1 Runtime Profile
 *
 * Chaos Strikes Back V1 runtime profile: separate from DM1.
 *
 * Boot sequence (ReDMCSB ENTRANCE.C F0806_F0806_ENTRANCE_int):
 *   1. Load ENTRANCE.GFX (bootsector or disk-based)
 *   2. Show title screen with game ID / version string
 *   3. F0807_ENTRANCE_DrawAnimationStep: optional door animation
 *   4. Present game selector then run game
 *
 * CSB vs DM1 key runtime differences:
 *   - Different dungeon data (DUNGEON.C hash: 6695d2acebce49f95db1d8f3a5c733de)
 *   - Different default difficulty
 *   - Champion difficulty scale: +25% per champion (CSB harder vs DM1 flat)
 *   - Save namespace: csb_save_%d.fsav (LOADSAVE.C F0433/F0435)
 *   - Asset names: GRAPHICS.DAT + CSBGRAPH.DAT (or CSB.DAT on floppy platforms)
 *   - Entry point: ENTRANCE (same as DM1, but C28_ENTRANCE_CSB palette)
 *   - Chaos Magic system initialized at boot (CASTER.C F0211)
 *   - Champion import from DM1 save before dungeon load
 *
 * ReDMCSB references:
 *   ENTRANCE.C: F0806Entrance_int  (game boot)
 *   ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)
 *   ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics init)
 *   SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful
 *   SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful
 *   LOADSAVE.C: F0435_STARTEND_LoadGame
 *   LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame
 *   DUNGEON.C:  F0237_DUNGEON_DungeonLoad (hash-verified dungeon load)
 *   CASTER.C:   F0211_CASTER_ClearSpellEffects  (CSB-wide spell reset)
 *   CEDTINC7.C: G3764_THAT_S_THE_CSB_UTILITY_DISK  (utility disk prompt)
 *   CEDTDATA.C: G3921 PLEASE_INSERT_UTIL_DISK string
 *   BugsAndChanges.htm: CHANGE7_29  (new header format)
 *   MEDIA529_F20E_F20J: F20E/F21E save path decision
 *   MEDIA332_F20E_F21E_A31E_F31E: CSB uses C29 key index
 *
 * CSBWin save path references:
 *   SaveGame.cpp: LoadGame(), SaveGame() (2953 lines)
 *   Character.cpp: Character::import_dm1_save()
 */

#ifndef FIRESTAFF_CSB_V1_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_RUNTIME_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "asset_find_by_hash.h"
#include "csb_v1_game_state_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_csbwin_512_xor_pad_classify.h"
#include "csb_v1_csbwin_graphics_signature_gate.h"
#include "csb_v1_chaos_magic_pc34_compat.h"
#include "csb_v1_monster_pc34_compat.h"
#include "csb_v1_skin_cache_pc34_compat.h"
#include "csb_v1_audio_runtime_pc34_compat.h"
#include "csb_v1_utility_flow_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

struct CSB_V1_StartupRuntimePlan_PC34;
struct CSB_V1_CSBGraphicsRuntimePlan;

#ifdef __cplusplus
extern "C" {
#endif

/* ── Game identifier ────────────────────────────────────────────────────── */
#define CSB_V1_GAME_ID_STR  "csb"
#define CSB_V1_SAVE_ID_STR  "csbgame"

#define CSB_V1_MAX_PARTY_X  32
#define CSB_V1_MAX_PARTY_Y  32
#define CSB_V1_RUNTIME_ACTIVE_GROUP_CAP 110
#define CSB_V1_DUNGEON_PACKAGE_MD5_CAP 33
#define CSB_V1_DUNGEON_SAVE_NAMESPACE_CAP 48
/* CSBWin SaveGame.cpp serializes the active overlay palette as twenty-four
 * consecutive EDT_Palette EXPOOL records: 3 channels * 512 entries. */
#define CSB_V1_CSBWIN_OVERLAY_PALETTE_BYTES (3u * 512u)
#define CSB_V1_CSBWIN_TIMER_QUEUE_NONE 0xffffu
#define CSB_V1_RUNTIME_TEXT_MESSAGE_MAX_CHARS 192
#define CSB_V1_RUNTIME_POST_TELEPORT_PROJECTILE_MAX_PC34 8
#define CSB_V1_CSBWIN_SAVE_SOURCE_PATH_CAP 256

/* Receipt for a CSBWin save accepted by the live resume path.  It identifies
 * source bytes, rather than inventing a Firestaff-native provenance token. */
typedef struct {
    int valid;
    size_t source_size;
    size_t core_offset;
    uint32_t source_fnv1a;
    uint32_t core_fnv1a;
    uint32_t appended_fnv1a;
    uint32_t random_game_id;
    uint8_t key_verdict;
    uint8_t format_id;
    char source_path[CSB_V1_CSBWIN_SAVE_SOURCE_PATH_CAP];
} CSB_V1_CSBWinSaveProvenance_PC34;

/* Transient F0219 receipt.  The runtime produces this only after the live
 * C05 chain has committed the C14 to its resolved square.  The boot renderer
 * must still re-check the raw Thing ownership before it can draw anything. */
typedef struct {
    int valid;
    int projectile_slot;
    int map_index;
    int map_x;
    int map_y;
    int cell;
    uint32_t game_time;
} CSB_V1_RuntimePostTeleportProjectileReceiptPc34;

/* A live TEXT.C message is admissible only when CSBWin Timer.cpp's exact
 * TT_OPENROOM DB2 transition made it visible on the party square.  This is
 * intentionally a one-message receipt, not a Firestaff log or queue. */
typedef struct {
    int valid;
    uint16_t text_thing;
    uint32_t source_game_time;
    char text[CSB_V1_RUNTIME_TEXT_MESSAGE_MAX_CHARS];
} CSB_V1_RuntimeTextMessageReceipt;

/* CSBWin DSA.cpp STKOP_Sound delegates to PlayCustomSound after the complete
 * action is accepted. Keep the exact source request as a runtime receipt;
 * an unavailable original sound backend must not be replaced by host audio. */
typedef struct {
    int valid;
    int32_t sound_number;
    int32_t volume;
    int32_t flags;
    uint32_t source_game_time;
} CSB_V1_RuntimeDSASoundReceipt;

typedef struct {
    int valid;
    int transfer_only;
    int stack_core;
    int requires_runtime_owner;
    int conditional_core;
    int arithmetic_core;
    int variable_core;
    int timer_core;
    int message_core;
    int dungeon_mutation_core;
    int query_core;
    int rollback_guarded;
    int parameter_count;
    uint16_t command_count;
    uint16_t words_consumed;
    uint16_t stack_depth;
    uint16_t transfer_count;
    uint16_t transfer_return_count;
    uint16_t transfer_frame_push_count;
    uint16_t transfer_frame_pop_count;
    uint8_t maximum_subroutine_depth;
    int next_state;
    int forced_state;
    int transfer_final_state;
    /* CSBWin DSA.cpp Execute() returns this i32 to its caller. Keep that
     * return value explicit rather than inferring it from a frame summary. */
    int transfer_return_value;
    int transfer_frame_balance_valid;
    int transfer_returned_by_missing_program;
    uint16_t timer_scheduled_count;
    uint8_t last_scheduled_event_type;
    uint32_t last_scheduled_target_location;
    uint32_t last_scheduled_delay;
    uint32_t last_scheduled_action;
    uint16_t message_scheduled_count;
    uint8_t last_message_route;
    uint16_t text_discard_count;
    uint16_t sound_notification_count;
    int32_t last_sound_number;
    int32_t last_sound_volume;
    int32_t last_sound_flags;
    uint16_t teleport_party_count;
    uint32_t last_teleport_party_destination;
    uint16_t teleporter_copy_count;
    uint32_t last_teleporter_copy_source_location;
    uint32_t last_teleporter_copy_destination_location;
    uint32_t last_teleporter_copy_source_before[5];
    uint32_t last_teleporter_copy_destination_before[5];
    uint32_t last_teleporter_copy_destination_after[5];
    uint16_t actuator_copy_count;
    uint16_t last_actuator_copy_source_thing;
    uint16_t last_actuator_copy_destination_thing;
    uint16_t skin_store_count;
    uint32_t last_skin_store_location;
    uint8_t last_skin_store_before;
    uint8_t last_skin_store_after;
    uint16_t wing_talents_store_count;
    uint16_t last_wing_talents_fingerprint;
    uint32_t last_wing_talents_before;
    uint32_t last_wing_talents_after;
    uint32_t wing_talents_tail_fnv1a_before;
    uint32_t wing_talents_tail_fnv1a_after;
    uint16_t experience_plus_count;
    int32_t last_experience_character_selector;
    int32_t last_experience_skill_number;
    int32_t last_experience_basic_skill_number;
    int32_t last_experience_amount;
    uint32_t last_experience_selected_before;
    uint32_t last_experience_selected_after;
    uint32_t last_experience_basic_before;
    uint32_t last_experience_basic_after;
    uint16_t monster_store_count;
    uint16_t last_monster_store_thing;
    uint8_t last_monster_store_write_mask;
    uint32_t last_monster_store_before[8];
    uint32_t last_monster_store_after[8];
    uint16_t cell_store_count;
    uint32_t last_cell_store_location;
    uint8_t last_cell_store_write_mask;
    uint32_t last_cell_store_before[5];
    uint32_t last_cell_store_after[5];
    uint16_t false_pit_count;
    uint32_t last_false_pit_location;
    uint32_t last_false_pit_before[5];
    uint32_t last_false_pit_after[5];
    uint16_t object_property_store_count;
    uint16_t last_object_property_thing;
    uint8_t last_object_property_kind;
    uint32_t last_object_property_before;
    uint32_t last_object_property_after;
    uint16_t missile_info_store_count;
    uint16_t last_missile_info_thing;
    uint32_t last_missile_info_before[4];
    uint32_t last_missile_info_after[4];
    int missile_info_timer_owner_valid;
    uint16_t missile_info_timer_index;
    uint16_t missile_info_timer_queue_slot;
    uint8_t missile_info_timer_function;
    uint8_t missile_info_timer_position_before;
    uint8_t missile_info_timer_position_after;
    uint32_t missile_info_timer_time;
    uint16_t excell_store_count;
    uint32_t last_excell_store_location;
    uint32_t last_excell_store_before[8];
    uint32_t last_excell_store_after[8];
    uint32_t excell_tail_fnv1a_before;
    uint32_t excell_tail_fnv1a_after;
    uint16_t generator_delay_store_count;
    uint32_t last_generator_delay_location;
    int32_t last_generator_delay_before;
    int32_t last_generator_delay_after;
    int generator_delay_has_generator;
    uint16_t cause_poison_count;
    int32_t last_cause_poison_character_selector;
    int32_t last_cause_poison_attack;
    int16_t last_cause_poison_health_before;
    int16_t last_cause_poison_health_after;
    uint16_t last_cause_poison_dose_before;
    uint16_t last_cause_poison_dose_after;
    uint8_t last_cause_poison_event_count_before;
    uint8_t last_cause_poison_event_count_after;
    uint16_t last_cause_poison_timer_event_index;
    uint16_t last_cause_poison_timer_attack;
    uint32_t last_cause_poison_timer_time;
    uint16_t parameter_message_created_count;
    uint16_t last_parameter_message_timer_index;
    uint16_t last_parameter_message_queue_slot;
    uint8_t last_parameter_message_sequence;
    uint32_t last_parameter_message_tail_fnv1a;
    uint8_t dsa_id;
    uint32_t state_index;
    uint32_t column;
    int action_ordinal;
    int globals_changed;
    /* GLOBALSTORE is persisted by CSBWin through the loaded EXPOOL tail.
     * Publish the post-write identity only when that source-owned bank changed. */
    uint32_t globals_tail_fnv1a;
    int saves_disabled_changed;
    int saves_disabled_before;
    int saves_disabled_after;
    int random_state_changed;
    uint32_t random_state_before;
    uint32_t random_state_after;
    int text_message_changed;
    CSB_V1_RuntimeTextMessageReceipt text_message_before;
    CSB_V1_RuntimeTextMessageReceipt text_message_after;
    int party_talents_changed;
    int party_talents_champion_count;
    uint16_t party_talents_fingerprints[4];
    uint32_t party_talents_before[4];
    uint32_t party_talents_after[4];
    int party_skill_experience_changed;
    int timer_type_modifiers_valid;
    uint8_t timer_type_modifiers[3];
    /* STKOP_ModifyMessage belongs to one authenticated ProcessTimers call.
     * Preserve its exact saved TIMER owner with the transient map; neither
     * field is a general queue or a serialized replacement timer. */
    int saved_timer_scope_valid;
    uint16_t saved_timer_queue_slot;
    uint16_t saved_timer_index;
    uint8_t saved_timer_function;
    uint8_t saved_timer_action;
    uint8_t saved_timer_position;
    uint32_t saved_timer_time;
    /* DSA.cpp PutState keeps LocalState=0 in DB3::DSAstate and LocalState=1
     * in the authenticated Extended Features stream.  The transition is
     * published only after the source owner and RCS-protected tail agree. */
    int saved_dsa_state_transition_valid;
    uint8_t saved_dsa_state_storage_kind;
    uint32_t saved_dsa_state_before;
    uint32_t saved_dsa_state_after;
    uint32_t saved_dsa_state_tail_fnv1a;
    /* A dynamic JumpGear/GosubGear target is part of the authenticated DSA
     * execution identity. Keep its exact imported state/column in the
     * runtime receipt so a later save/timer check cannot substitute it. */
    uint16_t dynamic_transfer_count;
    uint32_t dynamic_transfer_state;
    uint32_t dynamic_transfer_column;
    int dynamic_transfer_gosub;
    int dynamic_transfer_final_state;
    int expool_changed;
    int dsa_state_changed;
    int dungeon_changed;
} CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34;

/* Read-only prerequisite for CSBWin Magic.cpp::AddToSkill when its proposed
 * basic-skill mastery changes. The actual LevelUp random/stat/UI transaction
 * remains unavailable; this receipt proves only the source-owned CHARDESC
 * pair that a future complete owner must consume. */
typedef struct {
    int valid;
    int levelup_required;
    int32_t character_selector;
    int32_t selected_skill_number;
    int32_t basic_skill_number;
    uint16_t increment_ui16;
    uint32_t selected_before;
    uint32_t selected_after;
    uint32_t basic_before;
    uint32_t basic_after;
    int basic_mastery_before;
    int basic_mastery_after;
} CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34;

#define CSB_V1_CSBWIN_DSA_STATE_STORAGE_DB3_DSASTATE 0u
#define CSB_V1_CSBWIN_DSA_STATE_STORAGE_SAVED_M_STATE 1u
#define CSB_V1_CSBWIN_DSA_STATE_STORAGE_DB3_PARAMETER_B 2u

typedef struct {
    int valid;
    int expired;
    int event_count;
    int first_event_index;
    uint32_t game_time;
    uint32_t first_event_time;
    uint8_t first_event_type;
    const char *status;
} CSB_V1_F0240_FirstEventExpiredReceipt;

typedef struct {
    int valid;
    int tick_fired;
    int pre_event_count;
    int post_event_count;
    int dispatched_count;
    int first_event_index;
    uint32_t game_time_before;
    uint32_t game_time_after;
    uint32_t first_event_time;
    uint8_t first_event_type;
    uint32_t timeline_dispatch_count_before;
    uint32_t timeline_dispatch_count_after;
    const char *status;
} CSB_V1_F0261_ProcessTickReceipt;

/* ── Deterministic tick config ────────────────────────────────────────── */
/*
 * CSB uses a 55ms per-tick clock (same as DM1).
 * A 32-bit counter rolls over after ~27 hours of play.
 * Using uint64_t for extended precision during probing.
 */
#define CSB_V1_TICK_MS_NOMINAL  55U    /* nominal V1 tick in ms */

/* ── Default start position ──────────────────────────────────────────── */
/*
 * CSB starts in the Hall of Champions on map index 0.
 * Party always begins at coordinates (5, 5) facing North (0).
 * ReDMCSB: ENTRANCE.C line ~430 (G0309_i_PartyMapIndex init)
 */
#define CSB_V1_START_PARTY_X      5
#define CSB_V1_START_PARTY_Y      5
#define CSB_V1_START_PARTY_Z      0    /* ground floor */
#define CSB_V1_START_PARTY_DIR    0    /* 0=North, 1=East, 2=South, 3=West */

#define CSB_V1_OBJECT_NAME_COUNT 199
#define CSB_V1_OBJECT_NAME_MAX_CHARS 31

/* ── Difficulty ───────────────────────────────────────────────────────── */
/*
 * CSB difficulty scale: each champion fight gets +25% effective stats.
 * Base multiplier is 1.0 (same as DM1).  Escalates per champion count.
 * ReDMCSB: PROJEXPL.C projectile + CSBWin Character.cpp difficulty scale
 *
 *   Champions in party | Effective difficulty (x100%)
 *   ------------------+--------------------------------
 *     1                |  1.00 (Easy)
 *     2                |  1.25 (+25% Normal)
 *     3                |  1.50 (+50% Hard — default)
 *     4                |  2.00 (+100% Extreme)
 */
#define CSB_V1_DIFFICULTY_BASE        100   /* base percent, x100 scale */
#define CSB_V1_DIFFICULTY_PER_CHAMP    25   /* extra % x100 per champion */

typedef enum {
    CSB_V1_DIFFICULTY_EASY   = 100,
    CSB_V1_DIFFICULTY_NORMAL = 125,  /* 2 champions */
    CSB_V1_DIFFICULTY_HARD   = 150,  /* 3 champions (default) */
    CSB_V1_DIFFICULTY_EXTREME = 200  /* 4 champions */
} CSB_V1_Difficulty;

/* ── Asset discovery namespace ───────────────────────────────────────── */

/*
 * CSB asset discovery differs from DM1 in two ways:
 *   1. Graphics archive can be CSBGRAPH.DAT, CSB.DAT, or GRAPHICS.DAT
 *      (floppy vs CD/data vs hybrid media).
 *   2. Dungeon data uses a separate hash (6695d2acebce49f95db1d8f3a5c733de).
 *
 * File search order (ReDMCSB DISK.C + CSBWin AssetCache):
 *   GRAPHICS.DAT on any media type (always required)
 *   CSBGRAPH.DAT (CD/data release, optional overlay)
 *   CSB.DAT     (floppy release, complete GRAPHICS replacement)
 *   DUNGEON.DAT  (hash-verified)
 *
 * Data directory discovery:
 *   ~/.firestaff/data/csb/     (canonical)
 *   ~/.firestaff/data/          (shared DM1/CSB/DM2 search fallback)
 *   $PWD                       (dev mode fallback)
 */
typedef enum {
    CSB_V1_ASSET_GFX_ARCHIVE_NONE   = 0,
    CSB_V1_ASSET_GFX_ARCHIVE_CSB    = 1,  /* CSB.DAT */
    CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF = 2, /* CSBGRAPH.DAT */
    CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS = 3, /* GRAPHICS.DAT (floppy fallback) */
} CSB_V1_AssetGfxArchiveType;

typedef struct {
    const char *path;       /* absolute path found */
    CSB_V1_AssetGfxArchiveType kind;  /* archive variant found */
} CSB_V1_AssetResult;

/* ── Variant diagnostics ──────────────────────────────────────────────── */
/*
 * CSB ships on many platforms, each with different media layouts.
 * Runtime variant diagnostics identify platform for bug-for-bug fixes.
 *
 * CSBWin AssetCache + ReDMCSB COMPILE.H MEDIA_* tags:
 *   MEDIA278 / P20JA / P20JB  = PC DOS 3.4 (EN/Multi)
 *   MEDIA332 / S20E / S21E    = Atari ST 2.0 / 2.1 English
 *   MEDIA529 / A35E / A35M    = Amiga 3.5 (EN/Multi)
 *   MEDIA529 / F20J / F20E    = Atari ST TT variant (F20J = 060)
 *
 * These affect only diagnostics and platform-specific quirks.
 * Game logic is entirely source-locked to ReDMCSB.
 */
typedef enum {
    CSB_V1_VARIANT_UNKNOWN           = 0,
    CSB_V1_VARIANT_PC34_EN          = 1,   /* PC DOS 3.4 English       MEDIA278 */
    CSB_V1_VARIANT_PC34_MULTI        = 2,   /* PC DOS 3.4 Multilanguage MEDIA278 */
    CSB_V1_VARIANT_ST20_EN           = 3,   /* Atari ST 2.0 English   MEDIA332 */
    CSB_V1_VARIANT_ST21_EN           = 4,   /* Atari ST 2.1 English   MEDIA332 */
    CSB_V1_VARIANT_AMIGA35_EN         = 5,   /* Amiga 3.5 English       MEDIA529 */
    CSB_V1_VARIANT_AMIGA35_MULTI      = 6,   /* Amiga 3.5 Multilanguage MEDIA529 */
    CSB_V1_VARIANT_ST_F20J           = 7,   /* Atari ST TT (F20J)     MEDIA529 */
    CSB_V1_VARIANT_ST_F20E          = 8,   /* Atari ST (F20E)        MEDIA529 */
    CSB_V1_VARIANT_COUNT
} CSB_V1_VariantId;

typedef struct {
    CSB_V1_VariantId   id;
    const char        *name;       /* human-readable: "Atari ST 2.1 EN" */
    const char        *media_ref;   /* ReDMCSB COMPILE.H media tag */
    const char        *md5_gfx;     /* GRAPHICS.DAT hash for this variant */
    const char        *md5_graf;    /* CSBGRAPH.DAT / CSB.DAT hash */
    const char        *md5_dungeon; /* DUNGEON.DAT hash (shared) */
} CSB_V1_VariantInfo;

/* ── Chaos Magic runtime state ──────────────────────────────────────── */
/*
 * CSB introduces a Chaos Magic system not present in DM1.
 * Initialized at boot (F0211_CASTER_ClearSpellEffects fires at world load).
 * Active during all dungeon exploration.
 * ReDMCSB: CASTER.C F0211 (cleared at world load)
 *          CASTER.C F0213 (per-square invocation slots)
 * CSBWin:  Magic.cpp ChaosMagic namespace
 */
typedef struct {
    uint32_t spell_grid_version;  /* version key for CSB-wide spell grid */
    uint8_t  chaos_level;        /* current ambient chaos level (0-3) */
    int      magic_initialized;   /* 1 = spell grid built */
} CSB_V1_ChaosAmbientState;

typedef struct {
    int valid;
    uint16_t monster_index;
    int live_ai_owned;
    uint16_t live_ai_group_thing;
    int live_ai_map_index;
    int live_ai_map_x;
    int live_ai_map_y;
    int live_ai_c37_queued;
    uint8_t facings;
    uint8_t positions;
    uint8_t last_move_time_lsb;
    uint8_t delay_or_flee_timer;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t previous_x;
    uint8_t previous_y;
    uint8_t current_x;
    uint8_t current_y;
    uint8_t single_monster_status[4];
} CSB_V1_CSBWinRuntimeItem16;

typedef struct {
    int valid;
    uint16_t group_thing;
    int map_index;
    int map_x;
    int map_y;
    uint8_t cells;
    uint16_t directions;
    int prior_map_x;
    int prior_map_y;
    int home_map_x;
    int home_map_y;
    uint32_t last_move_time;
    int target_map_x;
    int target_map_y;
    uint8_t aspect[4];
    uint8_t delay_fleeing_from_target;
} CSB_V1_RuntimeActiveGroupState;

/* ── Runtime profile ─────────────────────────────────────────────────── */
/*
 * CSB V1 runtime profile: everything that distinguishes CSB from DM1.
 *
 * Separated from:
 *   - DM1 profile: different dungeon hash, save namespace, difficulty
 *   - CSB V2 profile: V1 game logic identical; V2 has render changes
 *
 * The profile owns:
 *   - Game mode and state machine
 *   - Party position/direction
 *   - Chaos Magic state
 *   - Deterministic 55ms tick accumulator
 *   - Config/game variant
 *   - Asset path references
 */
typedef struct {
    /* ── Identity ─────────────────────────────── */
    CSB_V1_VariantId        variant_id;
    CSB_V1_Difficulty       difficulty;
    uint32_t                dungeon_seed;   /* from DUNGEON.DAT */
    uint16_t                dungeon_game_id; /* serial from dungeon header */
    CSB_V1_AssetResult      dungeon_asset;
    CSB_V1_AssetResult      graphics_asset;
    /* Every running dungeon owns a hash-pinned save namespace.  Original
     * packages use the CSB registry; custom packages must be registered with
     * their exact bytes before LOADSAVE.C may switch to them. */
    char                    dungeon_package_md5[CSB_V1_DUNGEON_PACKAGE_MD5_CAP];
    char                    dungeon_save_namespace[CSB_V1_DUNGEON_SAVE_NAMESPACE_CAP];
    char                    custom_bonus_dungeon_path[ASSET_PATH_MAX];
    char                    custom_bonus_dungeon_md5[CSB_V1_DUNGEON_PACKAGE_MD5_CAP];
    int                     custom_bonus_dungeon_registered;

    /* ── Dungeon world ──────────────────────────── */
    int                     current_level;   /* 0-based dungeon level */
    int                     current_world;   /* 0-based world index */
    int                     level_count;    /* total dungeon levels */
    int                     world_count;     /* worlds in this campaign */

    /* ── Party state ────────────────────────────── */
    int                     party_x;
    int                     party_y;
    int                     party_z;         /* floor / height level */
    int                     party_dir;       /* 0=North, 1=East, 2=South, 3=West */
    int                     champion_count;  /* champions in party */
    int                     leader_index;    /* G0411_i_LeaderIndex, -1 = none */
    int                     magic_caster_index;
    int                     party_state_valid;
    CSB_V1_PartyState       party_state;

    /* ── State machine ─────────────────────────── */
    int                     state;   /* CSB_STATE_* enum */
    int                     load_bonus_dungeon; /* ReDMCSB G1147_B_LoadBonusDungeon */
    int                     paused;
    int                     victory;
    int                     game_over;

    /* ── Map indices (boot/profile handoff) ───── */
    /* entrance_map_index is C255_MAP_INDEX_ENTRANCE (255) for CSB.
     * start_map_index is 0 for new-game map selection.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 */
    uint32_t                entrance_map_index;
    uint32_t                start_map_index;

    /* ── Timing ────────────────────────────────── */
    uint64_t                game_ticks;      /* ms accumulator */
    uint32_t                game_time;       /* V1 game_time */
    uint64_t                total_play_ms;   /* wall-clock ms */
    uint32_t                tick_count;       /* how many V1 ticks elapsed */
    /* CSBWin Data.h d.PartySleeping is transient runtime state used by
     * Code17818.cpp::DetermineMastery. It is deliberately not a save field. */
    int                     csbwin_party_sleeping;
    int                     csbwin_gameblock2_summary_valid;
    uint32_t                csbwin_random_seed;
    uint16_t                csbwin_object_in_hand;
    uint16_t                csbwin_num_timer;
    uint16_t                csbwin_first_avail_timer;
    uint16_t                csbwin_max_timers;
    uint16_t                csbwin_item16_queue_len;
    uint16_t                csbwin_max_item16;
    uint16_t                csbwin_timer_sequence;
    /* DSA.cpp's parameterMessageSequence is independent of TIMER's heap
     * sequence. It is runtime-only and occupies TIMER::ubyte5 for each
     * source TT_ParameterMessage created by STKOP_Message. */
    uint8_t                 csbwin_parameter_message_sequence;
    uint32_t                csbwin_last_monster_attack_time;
    uint32_t                csbwin_last_party_move_time;
    uint16_t                csbwin_party_move_disable_timer;
    uint16_t                csbwin_word11712;
    uint16_t                csbwin_word11714;
    int                     csbwin_header_tail_valid;
    uint8_t                 csbwin_header_byte22808[132];
    int                     csbwin_appended_tail_valid;
    size_t                  csbwin_appended_tail_size;
    size_t                  csbwin_appended_tail_preserved_size;
    uint32_t                csbwin_appended_tail_fnv1a;
    int                     csbwin_appended_tail_truncated;
    uint8_t                 csbwin_appended_tail[CSB_V1_CSBWIN_MAX_APPENDED_TAIL_BYTES];
    CSB_V1_CSBWinSaveProvenance_PC34 csbwin_save_provenance;
    /* CSBWin SaveGame.cpp ReadExtendedFeatures()/ReadDSAs()/ReadGameInfo()
     * owns this separately from the regular GAMEBLOCK sections. Imported DSA
     * programs remain opaque source words; no compatibility opcode runner
     * consumes them. */
    int                     csbwin_extended_features_valid;
    uint8_t                 csbwin_extended_features_version;
    uint8_t                 csbwin_extended_features_flags;
    uint32_t                csbwin_extended_features_flags32;
    /* Exact EXTENDEDFEATURESBLOCK save header fields. These are retained
     * after the header/map checks but never select a renderer, overlay, or
     * graphics package without that source-owned consumer. */
    uint32_t                csbwin_extended_editing_options;
    uint32_t                csbwin_extended_cell_flag_array_size;
    uint32_t                csbwin_extended_graphics_signature1;
    uint32_t                csbwin_extended_graphics_signature2;
    uint32_t                csbwin_extended_spell_filter_location;
    int32_t                 csbwin_extended_overlay_ordinal;
    int32_t                 csbwin_extended_overlay_p1;
    int32_t                 csbwin_extended_overlay_p2;
    int32_t                 csbwin_extended_overlay_p3;
    int32_t                 csbwin_extended_overlay_p4;
    uint32_t                csbwin_extended_csbgraphics_signature1;
    uint32_t                csbwin_extended_csbgraphics_signature2;
    uint8_t                 csbwin_extended_hint_key[8];
    char                    *csbwin_extended_game_info;
    uint32_t                csbwin_extended_game_info_size;
    uint32_t                csbwin_extended_game_info_fnv1a;
    int                     csbwin_extended_level_index_present;
    uint16_t                csbwin_extended_level_dsa_index[64][32];
    CSB_V1_ChaosMagicState  csbwin_extended_dsa_state;
    int                     csbwin_global_variables_valid;
    uint32_t                csbwin_global_variable_count;
    uint32_t                csbwin_global_variables[CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY];
    int                     csbwin_overlay_palette_valid;
    uint32_t                csbwin_overlay_palette_tail_fnv1a;
    uint8_t                 csbwin_overlay_palette[CSB_V1_CSBWIN_OVERLAY_PALETTE_BYTES];
    int                     csbwin_saves_disabled;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34
                            csbwin_last_dsa_execution_receipt;
    uint32_t                csbwin_delete_duplicate_timers;
    uint32_t                csbwin_debugging_data;
    uint32_t                csbwin_csbgraphics_signature_data;
    uint32_t                csbwin_graphics_signature_data;
    uint32_t                csbwin_version_data;
    CSB_V1_CSBWinDSATracingReport
                            csbwin_dsa_tracing;
    int                     csbwin_body_runtime_summary_valid;
    int16_t                 csbwin_character_tail_brightness;
    uint8_t                 csbwin_character_tail_see_thru_walls;
    uint8_t                 csbwin_character_tail_magic_footprints_active;
    int16_t                 csbwin_character_tail_party_shield;
    int16_t                 csbwin_character_tail_fire_shield;
    int16_t                 csbwin_character_tail_spell_shield;
    uint8_t                 csbwin_character_tail_num_footprint_entries;
    uint8_t                 csbwin_character_tail_freeze_life_timer;
    uint8_t                 csbwin_character_tail_first_magic_footprint;
    uint8_t                 csbwin_character_tail_last_magic_footprint;
    uint16_t                csbwin_character_tail_party_footprints[24];
    uint8_t                 csbwin_character_tail_byte13220[24];
    uint8_t                 csbwin_character_tail_invisible;
    uint16_t                csbwin_item16_summary_count;
    uint16_t                csbwin_item16_summary_total;
    CSB_V1_CSBWin512Item16Summary
                            csbwin_item16[CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES];
    uint16_t                csbwin_runtime_item16_count;
    uint16_t                csbwin_runtime_item16_total;
    CSB_V1_CSBWinRuntimeItem16
                            csbwin_runtime_item16[CSB_V1_CSBWIN_MAX_ITEM16_SUMMARIES];
    uint16_t                csbwin_timer_summary_count;
    uint16_t                csbwin_timer_summary_total;
    CSB_V1_CSBWin512TimerSummary
                            csbwin_timers[CSB_V1_CSBWIN_MAX_TIMER_SUMMARIES];
    uint16_t                csbwin_timer_queue_summary_count;
    uint16_t                csbwin_timer_queue_summary_total;
    uint16_t                csbwin_timer_queue[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES];
    /* Runtime-only receipt from a materialized DM1 timeline event index to
     * its original CSBWin timer-queue slot. It is rebuilt on every verified
     * CSBWin resume and is never a caller-provided timer identity. */
    uint16_t                csbwin_timeline_event_queue_slot[DM1_EVENT_MAX_COUNT];
    /* Receipt for the most recent source-owned ProcessDSATimer5/6 action
     * completed by a restored TimerQueue entry. This is diagnostic state for
     * real-package provenance; it never selects or creates a DSA action. */
    int                     csbwin_last_saved_timer_dsa_valid;
    uint16_t                csbwin_last_saved_timer_dsa_queue_slot;
    uint16_t                csbwin_last_saved_timer_dsa_timer_index;
    uint8_t                 csbwin_last_saved_timer_dsa_id;
    uint32_t                csbwin_last_saved_timer_dsa_state_index;
    uint32_t                csbwin_last_saved_timer_dsa_column;
    int                     csbwin_last_saved_timer_dsa_action_ordinal;
     /* CSBWin TT_75 carries its remaining poison attack in TIMER::timerWord6.
     * The shared V1 EVENT record has only an eight-bit effect field, so a
     * requeued poison event retains its full source word here, keyed by the
     * live heap slot that owns it.  This is runtime-only and is cleared as
     * soon as that exact event is consumed. */
     uint16_t                csbwin_poison_event_attack[DM1_EVENT_MAX_COUNT];
    uint8_t                 csbwin_poison_event_attack_valid[DM1_EVENT_MAX_COUNT];
    CSB_V1_RuntimeTextMessageReceipt
                            csbwin_text_message_receipt;
    CSB_V1_RuntimeDSASoundReceipt
                            csbwin_dsa_sound_receipt;
    uint16_t                active_group_state_count;
    CSB_V1_RuntimeActiveGroupState
                            active_group_state[CSB_V1_RUNTIME_ACTIVE_GROUP_CAP];
    int                     half_square_direction_debounce_valid;
    uint16_t                half_square_direction_debounce_group;
    uint32_t                half_square_direction_debounce_time;
    struct DM1_EventQueue_V1 timeline_queue;  /* ReDMCSB TIMELINE.C heap */
    struct DM1_TickDispatchResult_V1 last_timeline_dispatch;
    uint32_t                timeline_dispatch_count;
    struct ProjectileList_Compat projectiles;
    struct ExplosionList_Compat explosions;
    CSB_V1_RuntimePostTeleportProjectileReceiptPc34
                            post_teleport_projectiles
                                [CSB_V1_RUNTIME_POST_TELEPORT_PROJECTILE_MAX_PC34];
    size_t                  post_teleport_projectile_count;
    CsbV1AudioRuntime audio_runtime;
    CSB_V1_SkinCache skin_cache;
    int                     csbwin_skin_cache_tail_receipt_valid;
    int                     csbwin_skin_cache_tail_valid;
    size_t                  csbwin_skin_cache_tail_size;
    uint32_t                csbwin_skin_cache_tail_fnv1a;
    int                     object_name_table_valid;
    char                    object_names[CSB_V1_OBJECT_NAME_COUNT]
                                            [CSB_V1_OBJECT_NAME_MAX_CHARS + 1];
    struct Dm1V1InputCommandQueuePc34Compat input_command_queue;
    struct Dm1V1InputQueueProcessResultPc34Compat last_input_dispatch;
    uint32_t                input_dispatch_count;

    /* ── Chaos Magic ────────────────────────────── */
    CSB_V1_ChaosAmbientState chaos_magic;

    /* ── Data paths ─────────────────────────────── */
    const char             *data_dir;
    const char             *save_dir;  /* resolved at init via _save_dir_x() */
    const char             *dungeon_path;
    const char             *graphics_path;
    char                    bonus_dungeon_path[ASSET_PATH_MAX];

    /* ── Dungeon data (owned) ─────────────────────── */
    /* Heap-allocated dungeon loaded by csb_v1_runtime_boot().
     * Freed by csb_v1_runtime_cleanup() or during a subsequent boot.
     * Also accessible via csb_v1_dungeon_get_current() for dungeon-layer
     * accessor stubs in csb_v1_dungeon_world_pc34_compat.c. */
    CSB_V1_DungeonData *dungeon_handle;
} CSB_V1_RuntimeProfile;

typedef struct {
    const uint8_t *portrait;
    size_t portrait_byte_count;
    int portrait_width;
    int portrait_height;
    int portrait_byte_width;
    int champion_index;
    int is_leader;
    const char *name;
    const char *title;
} CSB_V1_ChampionPortraitRenderSource;

typedef struct {
    int creature_type;
    int direction;
    int visible_count;
    int cells[4];
} CSB_V1_RuntimeGroupOverlayInfo;

typedef struct {
    int thing_type;
    int relative_cell;
    int icon_index;
    int subtype_index;
} CSB_V1_RuntimeObjectOverlayInfo;

/* Source-only F0141 -> G0209 bridge.  This receipt records the original
 * PC34 DUNGEON.DAT Thing arithmetic only; it deliberately names no graphic,
 * palette, viewport zone, or render route. */
typedef struct {
    int valid;
    uint16_t thing;
    int thing_type;
    int thing_index;
    int subtype;
    int object_info_index;
    int record_offset;
    int record_size;
    uint32_t record_fnv1a;
    const char *source_evidence;
} CSB_V1_F0141G0209ObjectInfoReceiptPc34;

/* ReDMCSB DUNGEON.C F0143: a source-owned armour Thing joins F0141's
 * ObjectInfo arithmetic to the immutable G0239 ArmourInfo row. */
typedef struct {
    int valid;
    CSB_V1_F0141G0209ObjectInfoReceiptPc34 object_info;
    int armour_type;
    int base_defense;
    int sharp_defense_bits;
    int use_sharp_defense;
    int defense;
    int is_shield;
    uint32_t armour_info_fnv1a;
    const char *source_evidence;
} CSB_V1_F0143ArmourDefenseReceiptPc34;

/* ReDMCSB DUNGEON.C F0144: raw C04 GROUP.Type joins the immutable G0243
 * CreatureInfo record.  This has no sprite, palette, or viewport meaning. */
typedef struct {
    int valid;
    uint16_t group_thing;
    int group_index;
    int record_offset;
    int record_size;
    uint32_t group_record_fnv1a;
    int creature_type;
    int base_attack;
    int base_defense;
    int dexterity;
    int attributes;
    int properties;
    uint32_t creature_info_fnv1a;
    const char *source_evidence;
} CSB_V1_F0144CreatureAttributesReceiptPc34;

/* ReDMCSB DUNGEON.C F0158: raw C05 WEAPON.Type joins F0141's ObjectInfo
 * row to immutable G0238 WeaponInfo. No graphics decision is represented. */
typedef struct {
    int valid;
    CSB_V1_F0141G0209ObjectInfoReceiptPc34 object_info;
    int weapon_type;
    int weight;
    int weapon_class;
    int strength;
    int kinetic_energy;
    int shoot_attack;
    uint32_t weapon_info_fnv1a;
    const char *source_evidence;
} CSB_V1_F0158WeaponInfoReceiptPc34;

/* ReDMCSB DUNGEON.C F0167: a loaded C03 sensor supplies the icon index;
 * F0166 supplies one real unused object record.  This receipt records both
 * raw records after the source mutation.  It has no graphics meaning. */
typedef struct {
    int valid;
    uint16_t source_sensor_thing;
    int source_sensor_index;
    int source_sensor_type;
    int source_icon_index;
    int allocated_thing_type;
    int allocated_item_type;
    uint16_t allocated_thing;
    int source_sensor_record_offset;
    int source_sensor_record_size;
    uint32_t source_sensor_record_fnv1a;
    CSB_V1_F0141G0209ObjectInfoReceiptPc34 object_info;
    const char *source_evidence;
} CSB_V1_F0167NewObjectReceiptPc34;

/* ReDMCSB DUNGEON.C F0164 removes one admitted object from its source
 * square Thing chain; F0163 appends that exact Thing to an existing target
 * square chain.  The receipt names both raw list mutations. */
typedef struct {
    int valid;
    CSB_V1_F0141G0209ObjectInfoReceiptPc34 object_info;
    uint16_t thing_before;
    uint16_t source_first_before;
    uint16_t source_previous_thing;
    uint16_t source_next_thing;
    uint16_t destination_first_before;
    uint16_t destination_tail_thing;
    int source_level;
    int source_map_x;
    int source_map_y;
    int destination_level;
    int destination_map_x;
    int destination_map_y;
    uint32_t source_record_fnv1a_before;
    uint32_t source_record_fnv1a_after;
    const char *source_evidence;
} CSB_V1_F0163F0164ObjectMoveReceiptPc34;

/* ReDMCSB GROUP1.C F0175 scans a loaded square Thing chain for C04. This
 * receipt retains that raw group record and its F0144 CreatureInfo join. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t square_first_thing;
    uint16_t group_thing;
    int group_record_offset;
    int group_record_size;
    uint32_t group_record_fnv1a;
    CSB_V1_F0144CreatureAttributesReceiptPc34 creature_attributes;
    const char *source_evidence;
} CSB_V1_F0175GroupThingReceiptPc34;

/* GROUP1.C F0176/F0178 work only from a C04 that is still linked on its
 * loaded PC34 square.  The receipts keep cell selection and packed-cell
 * rewrite separate from the consumers that mutate health or active state. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int creature_count;
    int requested_cell;
    int creature_ordinal;
    uint8_t group_cells;
    uint16_t group_directions;
    CSB_V1_F0144CreatureAttributesReceiptPc34 creature_attributes;
    const char *source_evidence;
} CSB_V1_F0176CreatureOrdinalReceiptPc34;

typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int creature_count;
    int removed_creature_index;
    uint8_t original_group_cells;
    uint8_t compacted_group_cells;
    const char *source_evidence;
} CSB_V1_F0178GroupCellsCompactReceiptPc34;

/* GROUP.C F0183 admits one raw C04 to the bounded current-map ActiveGroup
 * pool. The receipt is produced before the runtime writes any side state. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int creature_count;
    uint8_t group_cells;
    int group_direction;
    int active_group_slot;
    int already_active;
    CSB_V1_F0144CreatureAttributesReceiptPc34 creature_attributes;
    const char *source_evidence;
} CSB_V1_F0183ActiveGroupReceiptPc34;

/* GROUP.C F0184 writes a current-map ActiveGroup back to its linked raw C04
 * before F0194 retires the pool. This receipt carries the whole writeback. */
typedef struct {
    int valid;
    int active_group_slot;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    uint8_t group_cells;
    int group_direction;
    int behavior_before;
    int behavior_after;
    const char *source_evidence;
} CSB_V1_F0184ActiveGroupRemoveReceiptPc34;

/* GROUP1.C F0185 materializes only from a linked C006 generator and an
 * unused raw C04 record. This receipt locks both source records before the
 * existing generator calculation and square-link writeback consume them. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t source_sensor_thing;
    int source_sensor_record_offset;
    int source_sensor_record_size;
    uint32_t source_sensor_record_fnv1a;
    int creature_type;
    uint16_t flags_word;
    uint16_t local_word;
    uint16_t allocated_group_thing;
    int allocated_group_record_offset;
    uint32_t allocated_group_record_fnv1a;
    const char *source_evidence;
} CSB_V1_F0185GeneratedGroupReceiptPc34;

/* GROUP1.C F0189 removes a linked C04 only after the final F0188 drop.
 * The receipt locks the source group and its current-map ActiveGroup owner
 * before timeline cleanup, unlinking, and raw-record retirement. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    uint16_t group_next;
    uint16_t group_slot;
    int active_group_slot;
    const char *source_evidence;
} CSB_V1_F0189GroupDeleteReceiptPc34;

/* GROUP1.C F0191 receives a C04 only after MOVESENS.C F0267 has committed a
 * pit relocation. This receipt locks the destination-square C04 before the
 * all-creature fall-damage lifecycle starts. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int creature_count;
    int attack;
    int random_window;
    const char *source_evidence;
} CSB_V1_F0191GroupFallReceiptPc34;

/* GROUP1.C F0193 consumes one attacking Giggler C04 and one live champion.
 * This receipt binds the raw group identity before its Slot-chain mutation. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int creature_index;
    int champion_index;
    uint16_t group_slot_before;
    const char *source_evidence;
} CSB_V1_F0193GigglerStealReceiptPc34;

/* TIMELINE.C F0249 moves a linked C04 first when an open C08/C09 square
 * releases its occupants. The receipt preserves the source raw identity. */
typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    int square_type;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    const char *source_evidence;
} CSB_V1_F0249OpenSquareGroupReceiptPc34;

/* TIMELINE.C F0252 retries C60/C61 only for the same linked raw C04. */
typedef struct {
    int valid;
    int source_map_index;
    int source_map_x;
    int source_map_y;
    int target_map_index;
    int target_map_x;
    int target_map_y;
    int target_square_type;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int audible;
    const char *source_evidence;
} CSB_V1_F0252GroupMoveReceiptPc34;

/* MOVE.C F0265 owns C60/C61 creation from one currently linked raw C04. */
typedef struct {
    int valid;
    int source_map_index;
    int source_map_x;
    int source_map_y;
    int target_map_index;
    int target_map_x;
    int target_map_y;
    int target_square_type;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int audible;
    const char *source_evidence;
} CSB_V1_F0265GroupRetryReceiptPc34;

/* MOVE.C F0266 builds its impact table from the linked, live C04 before a
 * same-map move. This receipt retains only source-owned PC34 facts; C14
 * impact application remains owned by the existing projectile timeline. */
typedef struct {
    int valid;
    int source_map_index;
    int source_map_x;
    int source_map_y;
    int destination_map_x;
    int destination_map_y;
    uint16_t group_thing;
    int group_record_offset;
    uint32_t group_record_fnv1a;
    int creature_count;
    uint8_t live_creature_cell_mask;
    uint8_t intermediary_creature_cell_mask;
    int adjacent_move;
    int source_projectile_count;
    int destination_projectile_count;
    const char *source_evidence;
} CSB_V1_F0266GroupMoveProjectileReceiptPc34;

/* TIMELINE.C F0252 and MOVE.C F0266 share one source transaction.  The
 * receipt ties the scheduled C60/C61 record to the linked C04 and its live
 * C14 census before a move mutates either Thing chain.  A rejected late
 * mutation restores both the raw dungeon bytes and the runtime timeline. */
typedef struct {
    int valid;
    int committed;
    int retry_scheduled;
    int rolled_back;
    int group_destroyed_by_consequence;
    uint32_t raw_dungeon_fnv1a_before;
    uint32_t raw_dungeon_fnv1a_after;
    uint32_t timeline_dispatch_count_before;
    uint32_t timeline_dispatch_count_after;
    CSB_V1_F0252GroupMoveReceiptPc34 move;
    CSB_V1_F0266GroupMoveProjectileReceiptPc34 projectile;
    const char *source_evidence;
} CSB_V1_F0252F0266GroupMoveTransactionReceiptPc34;

typedef struct {
    struct Dm1V1InputQueueProcessResultPc34Compat queue_result;
    int old_party_x;
    int old_party_y;
    int old_party_dir;
    int new_party_x;
    int new_party_y;
    int new_party_dir;
    int runtime_state_changed;
    int unsupported_runtime_command;
    int movement_command_handled;
    int movement_step_attempted;
    int movement_step_applied;
    int movement_blocked_by_wall;
    int movement_blocked_by_door;
    int movement_blocked_by_fakewall;
    int movement_blocked_by_group;
    int movement_group_reaction_scheduled;
    int movement_destination_x;
    int movement_destination_y;
    int movement_destination_square_type;
    int movement_destination_raw_square;
    int movement_destination_door_state;
    int disabled_movement_ticks_after;
    int stair_transition_applied;
    int stair_up;
    int pit_fall_applied;
    int pit_open;
    int pit_fall_damaged_champion_count;
    int pit_fall_total_damage;
    int pit_fall_wound_mask;
    int teleporter_transition_applied;
    int teleporter_open;
    int teleporter_scope;
    int teleporter_absolute_rotation;
    int teleporter_rotation;
    int teleporter_audible;
    int teleporter_target_x;
    int teleporter_target_y;
    int teleporter_target_level;
    int chained_move_count;
    int chained_move_limit_hit;
    int teleporter_chain_count;
    int pit_chain_count;
    int sensor_source_remove_checked;
    int sensor_destination_add_checked;
    int sensor_trigger_count;
    int sensor_event_count;
    int sensor_audible_count;
    int sensor_last_type;
    int sensor_last_data;
    int sensor_last_effect;
    int sensor_last_target_x;
    int sensor_last_target_y;
    int sensor_last_target_cell;
    int sensor_last_event_type;
    int old_party_level;
    int new_party_level;
    int deferred_new_party_map_index_valid;
    int deferred_new_party_map_index;
} CSB_V1_InputCommandRuntimeResult;

/* ── Runtime profile API ─────────────────────────────────────────────── */

/* Initialize a fresh runtime profile with CSB defaults.
 * Sets difficulty, start position, and NULL paths.
 * Does NOT boot the dungeon or initialize Chaos Magic. */
void csb_v1_runtime_init(CSB_V1_RuntimeProfile *profile, const char *data_dir);

/* Clean up runtime resources.
 * Unloads the dungeon data loaded by csb_v1_runtime_boot().
 * Idempotent: safe to call when no dungeon is loaded.
 * After this call, dungeon-layer accessors return ENDOF until
 * csb_v1_runtime_boot() is called again with a valid dungeon. */
void csb_v1_runtime_cleanup(CSB_V1_RuntimeProfile *profile);

/* Copy the imported/loaded CSB party into the runtime profile.
 * This is intentionally a snapshot: UI/utility flow owns the source state,
 * while the runtime owns the state after verified CSB boot handoff.
 * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 initializes the party
 * map globals during new-game load; CLIKCHAM.C F0368 lines 38-73 mutates
 * G0411_i_LeaderIndex after champion-status-box selection. */
int csb_v1_runtime_set_party_state(CSB_V1_RuntimeProfile *profile,
                                   const CSB_V1_PartyState *party);
int csb_v1_runtime_get_party_state(const CSB_V1_RuntimeProfile *profile,
                                   CSB_V1_PartyState *out_party);

/* ReDMCSB DUNGEON.C F0140: compute one live Thing's inventory weight from
 * the loaded original dungeon data.  This includes waterskin charges and
 * recursive container contents.  Returns zero for an invalid/unloaded Thing.
 */
int csb_v1_runtime_get_object_weight_pc34_compat(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing);

/* ReDMCSB DUNGEON.C F0140 consumed by CHAMPION.C's Load field.  Rebuild a
 * champion's cached Load from C00..C29 only after a live dungeon is present.
 * The single-champion form returns the calculated load, or -1 for invalid
 * arguments/no loaded dungeon; the party form returns recomputed champions.
 */
int csb_v1_runtime_recompute_champion_load_pc34_compat(
    CSB_V1_RuntimeProfile *profile,
    int champion_index);
int csb_v1_runtime_recompute_party_loads_pc34_compat(
    CSB_V1_RuntimeProfile *profile);
int csb_v1_runtime_set_load_bonus_dungeon(CSB_V1_RuntimeProfile *profile,
                                          int enabled);
int csb_v1_runtime_get_load_bonus_dungeon(
    const CSB_V1_RuntimeProfile *profile);
/* Expansion filenames are only discovery hints. The live package owner and
 * a candidate must both match the CSB dungeon hash registry before a bonus
 * load can replace the current dungeon. */
int csb_v1_runtime_bonus_dungeon_candidate_admitted(const char *path);
int csb_v1_runtime_bonus_dungeon_active_owner_admitted(
    const CSB_V1_RuntimeProfile *profile);
/* Register one explicit custom expansion from a caller-selected path.  The
 * candidate remains rejected until its live bytes match expected_md5; merely
 * naming a file DUNGEONB.DAT never admits it. */
int csb_v1_runtime_register_custom_bonus_dungeon(
    CSB_V1_RuntimeProfile *profile, const char *path,
    const char *expected_md5);
int csb_v1_runtime_try_load_bonus_dungeon(CSB_V1_RuntimeProfile *profile);
const char *csb_v1_runtime_get_bonus_dungeon_path(
    const CSB_V1_RuntimeProfile *profile);
const char *csb_v1_runtime_get_dungeon_save_namespace(
    const CSB_V1_RuntimeProfile *profile);
int csb_v1_runtime_get_champion_skill_level(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int skill_index);
int csb_v1_runtime_apply_csbwin_gameblock2_summary(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary);
int csb_v1_runtime_apply_csbwin_champion_summaries(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary);
int csb_v1_runtime_export_csbwin_champion_summaries(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWin512BodyReport *out_summary);
int csb_v1_runtime_apply_csbwin_body_runtime_summaries(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary);
int csb_v1_runtime_materialize_csbwin_item16_summaries(
    CSB_V1_RuntimeProfile *profile);
int csb_v1_runtime_claim_csbwin_item16_ai_ownership(
    CSB_V1_RuntimeProfile *profile);
int csb_v1_runtime_materialize_csbwin_timer_queue(
    CSB_V1_RuntimeProfile *profile);
int csb_v1_runtime_apply_csbwin_resume_report(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWin512BodyReport *summary);
int csb_v1_runtime_apply_csbwin_resume_file(
    CSB_V1_RuntimeProfile *profile,
    const char *path,
    size_t max_size);
int csb_v1_runtime_get_csbwin_save_provenance(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinSaveProvenance_PC34 *out);
int csb_v1_runtime_export_csbwin_core_save_to_memory(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t *out,
    size_t out_capacity,
    size_t *out_size);
int csb_v1_runtime_export_csbwin_core_save_to_path(
    const CSB_V1_RuntimeProfile *profile,
    const char *path);
int csb_v1_runtime_locate_csbwin_appended_expool_record(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t record_id,
    const uint8_t **out_bytes,
    size_t *out_size);

/* Recover one CSBWin Statistics.cpp monster-name variant from the current
 * EDT_Database|EDBT_MonsterNames EXPOOL record. The record must have exactly
 * one live DB11 owner in the loaded, FNV-authenticated tail; absent,
 * duplicate, malformed, stale, or overlong names do not fall back to host
 * text. `graphic` is CSBWin's zero-based `|`-separated variant ordinal. */
int csb_v1_runtime_recover_csbwin_monster_name(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t monster_type,
    int graphic,
    char *out_name,
    size_t out_name_size);

/* Recover the raw CSBWin Mouse.cpp chest base weight. Unlike the live source
 * routine, this evidence accessor never returns its absent-record default;
 * exactly one current, authenticated EDBT_ObjectWeights word is required. */
int csb_v1_runtime_recover_csbwin_chest_base_weight(
    const CSB_V1_RuntimeProfile *profile,
    int32_t *out_weight);

/* Recover the complete three-word EDBT_RuntimeFileSignatures bundle from
 * SaveGame.cpp. Every raw PC34 record must have one current authenticated
 * owner; missing, duplicate, malformed, or stale records have no defaults. */
int csb_v1_runtime_recover_csbwin_runtime_file_signatures(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_csbgraphics_signature,
    uint32_t *out_graphics_signature,
    uint32_t *out_version);

/* Recover the raw SaveGame.cpp EDBT_Debuging word. This evidence accessor
 * requires exactly one current authenticated PC34 owner and never supplies
 * the source's absent-record zero default or enables debugging behavior. */
int csb_v1_runtime_recover_csbwin_debugging_data(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_debugging_data);

/* Recover the raw SaveGame.cpp EDBT_DeleteDuplicateTimers word. This is
 * evidence only: exactly one current authenticated PC34 owner is required;
 * absent, duplicate, malformed, or stale data has no policy default or
 * timer/runtime side effect. */
int csb_v1_runtime_recover_csbwin_delete_duplicate_timers(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_delete_duplicate_timers);

/* Prove the raw SaveGame.cpp EDBT_DisableSaves zero-payload marker. Exactly
 * one current authenticated PC34 DB11 owner with the original zero payload is
 * required; this read-only result neither changes save policy nor defaults. */
int csb_v1_runtime_recover_csbwin_disable_saves_marker(
    const CSB_V1_RuntimeProfile *profile);

/* Recover one complete CSBWin data.cpp GetExtendedCellFlag source record.
 * This evidence-only accessor requires exactly one current authenticated
 * eight-word EDT_ExtendedCellFlags DB11 owner; unlike the DSA compatibility
 * reader it never turns an absent record into zero flags or changes state. */
int csb_v1_runtime_recover_csbwin_extended_cell_flags(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t level,
    uint8_t x,
    uint32_t out_words[8]);
/* Recover the CSBWin Sound.cpp ESL_SOUNDFILTER location word from its one
 * current authenticated EDT_SpecialLocations DB11 owner. Evidence only: this
 * neither invokes the filter DSA nor changes audio or runtime state. */
int csb_v1_runtime_recover_csbwin_sound_filter_location(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_location);
/* Recover the CSBWin Character.cpp ESL_FEEDINGFILTER location word from one
 * current authenticated DB11 owner. It never invokes feeding DSA or changes
 * champions, items, timers, or runtime state. */
int csb_v1_runtime_recover_csbwin_feeding_filter_location(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_location);
/* Recover one CSBWin DSA.cpp MESSAGE payload from its current authenticated
 * EDT_MessageParameters DB11 owner. The source writer limits the record to
 * 29 words; this evidence accessor never queues a timer or runs a DSA. */
int csb_v1_runtime_recover_csbwin_message_parameters(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t timer_id,
    uint32_t *out_words,
    size_t out_capacity_words,
    size_t *out_word_count);
/* Recover the raw CSBWin SKIN_CACHE default-skin record. Unlike GetDefaultSkin
 * this accessor never zero-fills an absent or short record, and has no cache,
 * render, DSA, or runtime-state side effect. */
int csb_v1_runtime_recover_csbwin_default_skins(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t *out_bytes,
    size_t out_capacity,
    size_t *out_size);
/* Recover one raw 16-word SaveGame.cpp EDBT_GlobalVariables record. This is
 * detached from the global-bank import/writeback path and has no DSA or
 * runtime-state effect. */
int csb_v1_runtime_recover_csbwin_global_variables_record(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t record_index,
    uint32_t out_words[16]);
/* Recover one raw 16-word SaveGame.cpp EDT_Palette record. This is detached
 * from palette staging, caching, rendering, and all runtime behavior. */
int csb_v1_runtime_recover_csbwin_palette_record(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t record_index,
    uint32_t out_words[16]);
/* Recover one raw 25-word Character.cpp EDT_Character wing record. This does
 * not assemble a wing, create a party member, or invoke UI/DSA/runtime paths. */
int csb_v1_runtime_recover_csbwin_wing_record(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint,
    uint8_t record_index,
    uint32_t out_words[25]);
/* Recover one CSBWin Code51a4.cpp::AltGraphicMapping value from an exact
 * four-word EDT_Database|EDBT_AltMonGraphics record. This is read-only
 * mapping evidence; no derived graphic, cache entry, or host fallback is
 * created when the source record is absent, duplicate, malformed, or stale. */
int csb_v1_runtime_recover_csbwin_alt_mon_graphic(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t level,
    uint8_t monster_type,
    uint8_t alternate_graphic,
    int32_t *out_graphic_id);

/* Recover one CSBWin Code11f52.cpp/Statistics.cpp monster-kill counter from
 * an exact four-word EDT_Statistics|ESTAT_NumMonsterKilled record. The saved
 * DB11 node is evidence only: it does not update campaign progress, create a
 * counter, or substitute a missing statistic. */
int csb_v1_runtime_recover_csbwin_monster_kill_count(
    const CSB_V1_RuntimeProfile *profile,
    uint8_t death_reason,
    uint8_t monster_type,
    uint8_t alternate_graphic,
    uint32_t *out_count);

/* CSBWin Code11f52.cpp::ProcessMonsterDeleteFilter updates the selected
 * ESTAT_NumMonsterKilled word in place after EXPOOL::Locate. This bounded
 * runtime path accepts only one current, exact four-word DB11 record. It
 * never creates the source's absent-record zero counter or grows EXPOOL. */
int csb_v1_runtime_increment_csbwin_monster_kill_count(
    CSB_V1_RuntimeProfile *profile,
    uint8_t death_reason,
    uint8_t monster_type,
    uint8_t alternate_graphic);

/* Recover one CSBWin CSBCode.cpp::SubstituteGlobalText saved value. The
 * original writer limits text to 99 bytes plus its in-record NUL. This
 * accessor exposes only that complete authenticated source value; it does
 * not execute TEXT@/TEXTSAY/GLOBALTEXT!, substitute text, or invent text. */
int csb_v1_runtime_recover_csbwin_global_text(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t index,
    char *out_text,
    size_t out_text_size);

/* Read one recovered CSBCode.cpp::SubstituteGlobalText value into a bounded
 * caller buffer. `bcd` follows the source's byte-minus-'A' transform. This is
 * text materialization only: it never expands DSA text macros or displays
 * text, and it refuses an unavailable, non-unique, malformed, or stale DB11
 * owner instead of substituting a host string. */
int csb_v1_runtime_substitute_csbwin_global_text(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t index,
    int bcd,
    char *out_text,
    size_t out_text_size);

/* Recover the text identity stored by Character.cpp::GetFromWings. All eight
 * raw EDT_Character records must have one current, authenticated PC34 owner.
 * This is a read-only DB11 accessor: it has no DSA, party, or UI path. */
int csb_v1_runtime_recover_csbwin_wing_identity(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint,
    char *out_name,
    size_t out_name_size,
    char *out_title,
    size_t out_title_size);

/* Recover the one-word Character.cpp/Timer.cpp ChampionBones identity from
 * exactly one current PC34 DB11 owner. This read-only accessor does not enter
 * Vi Altar, DSA, party, UI, or the old no-EXPOOL DB10 fallback path. */
int csb_v1_runtime_recover_csbwin_champion_bones_fingerprint(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t bones_thing,
    uint16_t *out_fingerprint);

/* CSBWin Character.cpp CHARDESC::GetFromWings serializes a CHARDESC as eight
 * consecutive 25-word EDT_Character records.  Return one for a complete,
 * receipt-authenticated match, zero for an authenticated absent character,
 * and minus one for unavailable or partial source data. */
int csb_v1_runtime_read_csbwin_wing_talents(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint,
    uint32_t *out_talents);
/* CSBWin DSA.cpp STKOP_WhereIsChar tests the first 25-word EDT_Character
 * EXPOOL record only. Return one when that original record is present, zero
 * when absent, and minus one when its saved-tail owner is unavailable. */
int csb_v1_runtime_has_csbwin_wing_character(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint);
/* CSBWin Character.cpp CHARDESC::SaveToWings rewrites all eight existing
 * 25-word EDT_Character records. This routine follows that order without
 * EXPOOL growth or invented records. */
int csb_v1_runtime_set_csbwin_wing_talents(
    CSB_V1_RuntimeProfile *profile,
    uint16_t fingerprint,
    uint32_t talents);
/* CSBWin DSA.cpp STKOP_FetchExCellFlg reads an authenticated eight-word
 * EDT_ExtendedCellFlags record. One means an available tail (including an
 * absent/short source record, which yields eight zero words); minus one means
 * unavailable or altered source data. */
int csb_v1_runtime_read_csbwin_extended_cell_flags(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t location,
    uint32_t out_words[8]);
/* Source-shaped EXPOOL::Read/Write update for STKOP_StoreExCellFlg. It uses
 * only an existing authenticated DB11 free-list node; lacking one fails
 * rather than expanding a save tail or inventing storage. */
int csb_v1_runtime_set_csbwin_extended_cell_flags(
    CSB_V1_RuntimeProfile *profile,
    uint32_t location,
    uint32_t flags);
int csb_v1_runtime_get_csbwin_dsa_tracing(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSATracingReport *out_report);
/* Rebuild the CSBWin save-owned global-variable bank from the complete
 * appended EXPOOL tail. A missing first record is a valid empty bank; a
 * malformed present record leaves the prior profile bank unchanged. */
int csb_v1_runtime_restore_csbwin_expool_global_variables(
    CSB_V1_RuntimeProfile *profile);
/* Restore CSBWin's complete 24-record EDT_Palette bundle from a verified
 * appended EXPOOL tail. A partial, altered, or absent bundle is not exposed
 * as a renderer palette. */
int csb_v1_runtime_restore_csbwin_expool_overlay_palette(
    CSB_V1_RuntimeProfile *profile);
/* Return the byte-exact CSBWin overlay palette only when the complete source
 * bundle was restored from the current appended-tail receipt. */
int csb_v1_runtime_get_csbwin_expool_overlay_palette(
    const CSB_V1_RuntimeProfile *profile,
    const uint8_t **out_palette,
    size_t *out_size);
/* Persist a complete CSBWin SaveGame.cpp EDT_Palette bundle. This writes only
 * the existing 24 source-owned 16-word records in a complete,
 * FNV-authenticated EXPOOL tail; it never allocates a DB11 node or creates a
 * palette record. */
int csb_v1_runtime_set_csbwin_expool_overlay_palette(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *palette,
    size_t palette_size);
int csb_v1_runtime_csbwin_saves_disabled(
    const CSB_V1_RuntimeProfile *profile);
int csb_v1_runtime_restore_csbwin_save_policy(
    CSB_V1_RuntimeProfile *profile);
/* Read the post-palette CSBWin SaveGame.cpp EXPOOL policy records. The
 * values are restored only from a complete, receipt-authenticated save tail. */
int csb_v1_runtime_get_csbwin_save_policy(
    const CSB_V1_RuntimeProfile *profile,
    uint32_t *out_delete_duplicate_timers,
    uint32_t *out_debugging_data,
    uint32_t *out_csbgraphics_signature,
    uint32_t *out_graphics_signature,
    uint32_t *out_version);

/* Admit a hash-owned CSBgraphics.dat runtime plan only when the complete
 * CSBWin save's Extended Features and EXPOOL signature gates accept its real
 * cached MD5. This is an admission check, not a graphics loader: missing,
 * unready, or mismatched plans remain unavailable to startup/HUD consumers. */
int csb_v1_runtime_admit_csbwin_csbgraphics_plan(
    const CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_CSBGraphicsRuntimePlan *plan,
    CSB_V1_CSBWinGraphicsSignatureReceipt *out_receipt);

/* CSBWin Monster.cpp resolves a type-47 filter actuator from Expool, then
 * obtains its DSAselector from DB3::word2 bits 7..11 and maps that slot
 * through the save-owned DSALevelIndex[level][selector] table.  This receipt
 * binds those already-decoded source values without executing an opcode. */
typedef struct {
    CSB_V1_DSAFilterLocation location;
    uint8_t dsa_selector;
    uint8_t dsa_id;
    int actuator_identity_valid;
} CSB_V1_RuntimeDSAFilterBinding;

/* Exact, bounded receipt for CSBWin DSA.cpp ProcessDSATimer6.  The current
 * CSBWin FindMaster implementation returns the slave itself only when its
 * DSA is a master (LocalState != 3); it explicitly reports slave DSAs as not
 * implemented.  Keep both identities here so a later original slave route
 * cannot silently be mistaken for the self-master route. */
typedef struct {
    CSB_V1_RuntimeDSAFilterBinding slave;
    CSB_V1_RuntimeDSAFilterBinding master;
    uint32_t master_location;
    uint32_t state_index;
    uint32_t input_column;
    int action_ordinal;
} CSB_V1_RuntimeCSBWinDSATimer6Resolution;

/* Complete runtime receipt for a CSBWin Extended-DSA resume package.  This
 * does not select an action or construct a save; it proves that the live
 * profile still owns the authenticated DSA catalog, level index, materialized
 * TimerQueue events, and, when a saved timer has fired, the exact source
 * action executed by that timer. */
typedef struct {
    int valid;
    int dsa_catalog_valid;
    int level_index_valid;
    int timer_queue_event_chain_valid;
    int saved_timer_dsa_execution_valid;
    uint16_t live_timer_event_count;
    int imported_action_count;
    uint8_t last_dsa_id;
    uint32_t last_state_index;
    uint32_t last_input_column;
    int last_action_ordinal;
    uint16_t last_queue_slot;
    uint16_t last_timer_index;
} CSB_V1_CSBWinDSARuntimeChainReceipt_PC34;

int csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 *out_receipt);
int csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 *out_receipt);
int csb_v1_runtime_csbwin_dsa_levelup_prerequisite_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int32_t character_selector,
    int32_t skill_number, int32_t experience,
    CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34 *out_receipt);
int csb_v1_runtime_csbwin_dsa_levelup_prerequisite_current_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBWinDSALevelUpPrerequisiteReceipt_PC34 *receipt);

int csb_v1_runtime_resolve_csbwin_dsa_filter_binding(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *location,
    CSB_V1_RuntimeDSAFilterBinding *out_binding);

/* Resolve one authenticated ProcessDSATimer6 dispatch without executing it.
 * CSBWin DSA.cpp:534-575,5363-5416 maps the slave selector, applies the
 * implemented self-master FindMaster branch, gets saved state, and selects
 * column `3 * timerPosition + timerFunction`.  LocalState 0 uses the DB3
 * DSAstate nibble and LocalState 1 uses serialized DSA::m_state. LocalState
 * 2 reads compact DB3 ParameterB after DB3::MakeBig masks word6 to fourteen
 * bits; writing its widened word8 form remains unavailable. LocalState 3 is
 * an explicitly unimplemented source slave route. */
int csb_v1_runtime_resolve_csbwin_dsa_timer6_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    int timer_function,
    int timer_position,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

/* Bind one restored CSBWin TT_STONEROOM (function 6) timer to the exact
 * ProcessDSATimer6 receipt for a concrete type-47 actuator on its target
 * square.  Timer.cpp ProcessTT_STONEROOM passes timerUByte9 through the
 * configured SET/CLEAR/TOGGLE map and uses timerUByte8 as the input position;
 * Firestaff's source save summary preserves the default 0/1/2 mapping and
 * validates the timer level and coordinates before selecting a DSA action.
 * TT_ParameterMessage and non-stone-room functions remain outside this
 * bounded route because their EXPOOL parameter payload has not been wired. */
int csb_v1_runtime_resolve_csbwin_stoneroom_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

int csb_v1_runtime_resolve_csbwin_falsewall_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

/* Prepare only the source-selected pure-stack action for a restored
 * TT_FALSEWALL -> ProcessDSATimer7 timer.  This mirrors the source timer
 * handoff without promoting fake-wall state mutation, parameter payloads,
 * or the unproven LocalState 3 path or LocalState-2 widened-state writes. */
int csb_v1_runtime_prepare_csbwin_falsewall_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

/* Consume one restored TT_STONEROOM receipt into the existing authenticated
 * pure-stack runner.  The returned action is the exact source-owned item
 * selected by ProcessDSATimer6; callers cannot substitute compatible-looking
 * DSA words.  This deliberately does not persist a resulting master state or
 * execute world/filter opcodes.  LocalState 2/3 remain rejected by the
 * receipt resolver. */
int csb_v1_runtime_prepare_csbwin_stoneroom_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

/* Bind one restored CSBWin TT_OPENROOM (function 5) timer to the exact
 * ProcessDSATimer5 -> ProcessDSATimer6 receipt for a concrete type-47
 * actuator on its target square. Timer.cpp does not alter a normal
 * TT_OPENROOM timer before calling ProcessDSATimer5, so the saved
 * SET/CLEAR/TOGGLE action, position, level, and coordinates are the direct
 * source inputs. TT_ParameterMessage remains excluded until its EXPOOL
 * parameter record has an authenticated runtime owner. */
int csb_v1_runtime_resolve_csbwin_openroom_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

/* Prepare the existing authenticated pure-stack runner only from the exact
 * action selected through TT_OPENROOM -> ProcessDSATimer5. This bridge does
 * not execute world opcodes, text updates, or unknown LocalState 2/3 paths. */
int csb_v1_runtime_prepare_csbwin_openroom_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

/* CSBWin CSBCode.cpp dispatches TT_DESSAGE (102) through
 * ProcessTT_OPENROOM. That path deliberately skips text, counter, and
 * generator work, but still invokes ProcessDSATimer5 for type-47 actuators.
 * Bind only this saved DSA handoff; parameter-message payloads and world
 * mutation remain outside this receipt. */
int csb_v1_runtime_resolve_csbwin_dessage_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

int csb_v1_runtime_prepare_csbwin_dessage_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

/* CSBWin TT_ParameterMessage (101) restores its parameter vector from the
 * timer-indexed EDT_MessageParameters EXPOOL record, then dispatches through
 * the room's native OPENROOM or STONEROOM DSA route. */
int csb_v1_runtime_prepare_csbwin_parameter_message_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action,
    int out_parameters[26],
    size_t *out_parameter_count,
    uint16_t *out_queue_slot,
    uint32_t *out_parameter_payload_fnv1a);

/* CSBWin Timer.cpp::ProcessTT_DOOR invokes ActivateDSA with the decoded
 * SET/CLEAR/TOGGLE action, which constructs a source-shaped timer and calls
 * ProcessDSATimer5 for every type-47 actuator on the door square.  This
 * receipt covers only that saved function-10 -> DSA path; door cell flags,
 * TT_1 rescheduling, and unsupported LocalState 2/3 remain outside it. */
int csb_v1_runtime_resolve_csbwin_door_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

int csb_v1_runtime_prepare_csbwin_door_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

/* CSBWin Timer.cpp::ProcessTT_TELEPORTER and ProcessTT_PITROOM each invoke
 * ActivateDSA before changing the square's bit-3 state. ActivateDSA creates
 * the same type-47 ProcessDSATimer5 input as TT_DOOR. These receipts bind
 * only that original DSA handoff: the later teleporter/pit state mutation,
 * WiggleEverything, parameter messages, and LocalState 2/3 stay out of this
 * bounded saved-timer path. */
int csb_v1_runtime_resolve_csbwin_teleporter_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

int csb_v1_runtime_prepare_csbwin_teleporter_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

int csb_v1_runtime_resolve_csbwin_pitroom_dsa_timer_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_RuntimeCSBWinDSATimer6Resolution *out_resolution);

int csb_v1_runtime_prepare_csbwin_pitroom_dsa_timer_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner,
    const CSB_V1_DSAImportedAction **out_action);

/* Execute the fully proven, parameter-free ProcessDSATimer5 action for one
 * restored TT_DESSAGE, TT_DOOR, TT_TELEPORTER, or TT_PITROOM timer.  CSBWin
 * Timer.cpp's ActivateDSA constructs NEWDSAPARAMETERS with count zero before
 * entering ProcessDSATimer5.  Only the existing authenticated pure-stack
 * subset runs here; LocalState 2/3, master-state writes, cell mutation, and
 * all unsupported world opcodes fail closed. */
int csb_v1_runtime_execute_csbwin_saved_timer_dsa_stack_action(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer);

/* Execute one CSBWin TT_ParameterMessage (101) saved DSA route. The timer
 * must be the exact serialized TIMER slot and one unique live TimerQueue
 * entry retained by this profile; its EDT_MessageParameters EXPOOL record is
 * FNV-authenticated, source-sized, and limited to the runner's 26-word ABI
 * before ProcessTimers' source stone/open-room dispatch is reproduced.
 * Missing, malformed, stale, unqueued, duplicate-queued, or over-cap records
 * fail closed with no global or EXPOOL publication.
 * Source: CSBWin CSBCode.cpp ProcessTimers:6436-6454; Timer.cpp
 * ProcessTT_OPENROOM:1641-1711 / ProcessTT_STONEROOM:2118-2185;
 * data.cpp EXPOOL::Read:1542-1568. */
int csb_v1_runtime_execute_csbwin_saved_parameter_message_dsa_stack_action(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    const CSB_V1_CSBWin512TimerSummary *timer);

/* Execute one normal saved CSBWin timer selected by its authenticated timer
 * queue entry. This admits only TT_OPENROOM (5), TT_STONEROOM (6), and
 * TT_FALSEWALL (7), after the queue/TIMER-array identity and the selected
 * type-47 DSA receipt are proven. It deliberately excludes parameter
 * messages, ActivateDSA families, world/cell effects, and unsupported DSA
 * state/opcode surfaces. Source: CSBWin SaveGame.cpp:1844-1858,
 * CSBCode.cpp ProcessTimers:6430-6470, Timer.cpp:1343-1405,1641-1711,
 * 2118-2185. */
int csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    const CSB_V1_DSAFilterLocation *slave_location,
    uint16_t queue_index);

/* Resolve the source's complete Monster.cpp attack-filter handoff: the
 * verified SpecialLocations actuator, saved level selector, serialized DSA
 * LocalState, actuator DSAstate, and timer column 0.  It only returns an
 * already authenticated action and never synthesizes a DSA program. */
int csb_v1_runtime_resolve_csbwin_attack_filter_stack_action(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    CSB_V1_RuntimeDSAFilterBinding *out_binding,
    uint32_t *out_state_index,
    int *out_action_ordinal,
    uint32_t *out_master_location);

/* CSBWin Character.cpp::KillCharacter checks EDT_SpecialLocations /
 * ESL_CHARDEATHFILTER before changing the CHARDESC.  Execute only that exact
 * authenticated type-47 / ProcessDSATimer6 action with source parameters
 * `{ 1, championIndex }`.  Missing, altered, unsupported, or non-FNV-owned
 * DSA data remains a no-op; this never invents a location or callback. */
int csb_v1_runtime_execute_csbwin_character_death_filter(
    CSB_V1_RuntimeProfile *profile, int champion_index);

/* CSBWin CHARDESC::SetPossession invokes the optional EquipFilter before it
 * writes a slot: timer function 1 for the removed RN, then 0 for the added
 * RN, each with `{ 4, championIndex, slot, thing, 0 }`. This dispatch admits
 * only authenticated pure-stack actions and never fabricates an EquipFilter. */
int csb_v1_runtime_execute_csbwin_equip_filter(
    CSB_V1_RuntimeProfile *profile, int champion_index, int slot_index,
    uint16_t old_thing, uint16_t new_thing);

/* CSBWin Character.cpp::DamageCharacter routes its post-defense damage word
 * through ESL_DAMAGECHARFILTER. The callback receives
 * `{ championIndex, fingerprint, requestedDamage, finalDamage, woundMask,
 * attackType, 0 }`; only its updated finalDamage is returned. */
int csb_v1_runtime_execute_csbwin_damage_character_filter(
    CSB_V1_RuntimeProfile *profile, int champion_index, int requested_damage,
    uint16_t wound_mask, uint16_t attack_type, int *out_final_damage);

/* CSBWin SaveGame.cpp calls CursorFilter with CURSORFILTER_ReadGame after
 * restoring GAMEBLOCK2.objectInHand.  This is that exact six-word packet:
 * `{ object, 1, 0, 0, 0, 0 }`.  ReadGame is notification-only in the source;
 * any values written back by the DSA are deliberately not used to replace the
 * restored cursor object. */
int csb_v1_runtime_execute_csbwin_cursor_read_game_filter(
    CSB_V1_RuntimeProfile *profile, uint16_t object_thing);

/* CSBWin CSBCode.cpp TAG0138ec sends CursorFilter with
 * CURSORFILTER_ResumeSavedGame immediately before ObjectToCursor restores a
 * non-empty saved hand. Like the source path, this is a notification only:
 * the callback cannot cancel or replace the restored object. */
int csb_v1_runtime_execute_csbwin_cursor_resume_saved_game_filter(
    CSB_V1_RuntimeProfile *profile, uint16_t object_thing);

/* Prepare the source-authenticated pure-stack runner only after a concrete
 * imported action was selected. World opcodes, DSA master-state persistence,
 * and movement post-filter flags remain outside this bounded bridge. */
int csb_v1_runtime_prepare_csbwin_dsa_filter_stack_runner(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *out_runner);
/* Execute one prepared authenticated pure-stack action against the profile's
 * save-owned global bank. Publication and the existing global EXPOOL records
 * update only after complete execution; world/filter opcodes remain outside. */
int csb_v1_runtime_run_csbwin_dsa_filter_stack_action(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_CSBWinDSAFilterStackRunnerContext *runner,
    const CSB_V1_DSAImportedAction *action,
    int *parameters,
    int parameter_count,
    int flgs_inout[2]);

/* Source-owned callback bridge for CSBWin Monster.cpp's ProcessDSAFilter
 * call shape. The adapter may execute only a runner prepared from the same
 * runtime profile and authenticated DSA action; it adds no opcode support or
 * world/filter side effects. */
typedef struct {
    CSB_V1_RuntimeProfile *profile;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
} CSB_V1_RuntimeDSAFilterStackAdapter;

#define CSB_V1_RUNTIME_CSBWIN_MOVEMENT_FILTER_CAP 12

typedef struct {
    CSB_V1_RuntimeDSAFilterBinding binding;
    uint32_t state_index;
    int action_ordinal;
    uint32_t master_location;
} CSB_V1_RuntimeDSAMovementFilterRequest;

/* A single Monster.cpp movement callback can service multiple source levels
 * only when every selected DSAAction was independently authenticated. */
typedef struct {
    CSB_V1_RuntimeProfile *profile;
    int runner_count;
    CSB_V1_CSBWinDSAFilterStackRunnerContext
        runners[CSB_V1_RUNTIME_CSBWIN_MOVEMENT_FILTER_CAP];
} CSB_V1_RuntimeDSAMovementFilterStackAdapter;

int csb_v1_runtime_prepare_csbwin_dsa_filter_stack_adapter(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter);

int csb_v1_runtime_csbwin_dsa_filter_stack_runner_callback(
    const CSB_V1_DSAImportedAction *action,
    int *parameters,
    int parameter_count,
    int flgs_inout[2],
    void *user);

/* Install the authenticated callback in the concrete Monster.cpp attack
 * filter runtime. This is deliberately limited to one already-resolved
 * type-47 attack filter and one imported action; it neither creates DSA
 * bytecode nor expands the admitted opcode subset. */
int csb_v1_runtime_bind_csbwin_attack_filter_stack_runtime(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter);

/* Install one resolved CSBWin Monster.cpp movement filter in its source level
 * slot. Unbound levels remain explicitly disabled; this admits only the
 * existing authenticated pure-stack/transfer action subset. */
int csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter);

int csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime_multi(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeDSAMovementFilterRequest *requests,
    size_t request_count,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAMovementFilterStackAdapter *out_adapter);

int csb_v1_runtime_csbwin_movement_filter_stack_runner_callback(
    const CSB_V1_DSAImportedAction *action,
    int *parameters,
    int parameter_count,
    int flgs_inout[2],
    void *user);

int csb_v1_runtime_set_leader(CSB_V1_RuntimeProfile *profile,
                              int champion_index);
int csb_v1_runtime_select_champion_portrait_render_source(
    const CSB_V1_RuntimeProfile *profile,
    int champion_index,
    CSB_V1_ChampionPortraitRenderSource *out_source);

/* Rotate the party to a new direction.
 * Mirrors ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines 117-130:
 * for every champion in the imported party, applies (target_dir - party_dir)
 * to that champion's Cell and Direction (both modulo 4), then commits the
 * new party_dir.  When target_dir == party_dir the call is a deterministic
 * no-op (returns 0, no champion state is touched) and matches the F0284
 * "if (P0600_i_Direction == G0308_i_PartyDirection) return;" guard.
 *
 * The Cell field encodes the champion's normalized view position
 * (0=front-left, 1=front, 2=right, 3=back) and the Direction field encodes
 * the per-champion facing direction. Both fields are part of the
 * source-locked CHAMPION.C invariant and rotate together so that the
 * relative geometry between party_dir and each champion is preserved
 * across a party turn.
 *
 * This is intentionally a runtime boundary that is independent of full
 * CSB playability: it operates only on the imported party snapshot stored
 * by csb_v1_runtime_set_party_state() and does not depend on dungeon
 * geometry, hand objects, or any F0292 redraw stack.  It is the
 * narrow runtime equivalent of the F0284 assembly loop, exposed for the
 * boot-handoff regression.
 *
 * Returns 0 on success, -1 if profile is NULL, no party is loaded, or
 * target_dir is outside [0,3].
 * Source: ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines 117-130. */
int csb_v1_runtime_rotate_party(CSB_V1_RuntimeProfile *profile,
                                 int target_dir);

/* Consume one queued V1 input command and apply the narrow CSB V1 runtime
 * boundary that is currently source-locked: C001/C002 turn commands dispatch
 * through the shared V1 queue and mutate the CSB party direction via
 * csb_v1_runtime_rotate_party(); C003..C006 movement commands apply the
 * bounded one-cell CSB movement-step helper with the live dungeon wall
 * probe. Inventory, action, and panel commands are deliberately reported as
 * unsupported_runtime_command until their CSB runtime state boundaries are
 * source-locked separately.
 *
 * Returns 1 when a queue item was dequeued, 0 when the queue was empty or a
 * movement-disabled gate kept the command queued, and -1 on invalid input.
 * Source: ReDMCSB COMMAND.C F0380 lines 2045-2156 dispatches C001/C002 to
 * F0365_COMMAND_ProcessTypes1To2_TurnParty; CLIKMENU.C F0366 lines
 * 224-351 applies one movement step; CHAMPION.C F0284 lines 117-130
 * applies the party-direction delta to every champion Cell/Direction. */
int csb_v1_runtime_process_input_queue(
    CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_InputCommandRuntimeResult *out_result);

/* Boot the CSB dungeon and initialize Chaos Magic.
 * Finds DUNGEON.DAT by hash (csb_v1_runtime_find_dungeon), loads the
 * dungeon data into the current dungeon context
 * (csb_v1_dungeon_load_from_file + csb_v1_dungeon_set_current), sets
 * dungeon_seed/game_id from the dungeon header, and initializes the spell
 * grid (F0211_CASTER_ClearSpellEffects).
 *
 * On success, the dungeon is accessible via csb_v1_dungeon_get_current()
 * and dungeon-layer accessors in csb_v1_dungeon_world_pc34_compat.h are
 * live.  Call csb_v1_runtime_cleanup() to unload before shutdown.
 *
 * Returns 0 on success, -1 if dungeon not found or load fails.
 * On success, profile->dungeon_path and ->graphics_path are set. */
int csb_v1_runtime_boot(CSB_V1_RuntimeProfile *profile,
                          const char *data_dir,
                          const char *version_hint);

/* Advance the runtime clock by dt_ms milliseconds.
 * Accumulates sub-55ms frame deltas and fires one V1 tick for each full
 * quantum reached by total_play_ms. Advances chaos_magic state each tick. */
void csb_v1_runtime_tick(CSB_V1_RuntimeProfile *profile, uint32_t dt_ms);

/* Advance exactly one V1 tick (55ms nominal).
 * Deterministic stepping function.  Returns 1 if a tick fired, 0 if paused,
 * game-over, victorious, or profile is NULL. */
int csb_v1_runtime_tick_v1(CSB_V1_RuntimeProfile *profile);

/* CSB-owned ReDMCSB TIMELINE.C F0240 receipt. It inspects only the live
 * runtime's source event heap and compares the first event's low 24-bit time
 * to G0313_ul_GameTime; malformed or empty heaps fail closed. */
int csb_v1_runtime_f0240_is_first_event_expired(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_F0240_FirstEventExpiredReceipt *out_receipt);

/* CSB-owned ReDMCSB TIMELINE.C F0261 receipt. It advances exactly one live
 * CSB V1 tick through csb_v1_runtime_tick_v1(), records the source heap before
 * and after processing, and never constructs replacement EVENT records. */
int csb_v1_runtime_f0261_process_tick(
    CSB_V1_RuntimeProfile *profile,
    CSB_V1_F0261_ProcessTickReceipt *out_receipt);

/* Check if a V1 tick is due at accumulated wall-clock time now_ms.
 * Pass 0 to use profile->total_play_ms. */
int csb_v1_runtime_tick_due(const CSB_V1_RuntimeProfile *profile, uint32_t now_ms);

/* CSBWin DSA.cpp:3122-3135 + data.cpp:2130-2167 SETSKIN saved-skin
 * writeback: change one byte in an existing EDT_Skins DB11 EXPOOL
 * record, preserving its source word-aligned payload extent.
 * Returns 1 on success (including source-equivalent no-op), 0 when
 * the write would require allocator/free-list semantics Firestaff
 * has not source-proven. */
int csb_v1_runtime_set_csbwin_saved_skin(
    CSB_V1_RuntimeProfile *profile,
    int level,
    int x,
    int y,
    uint8_t skin_num);

int csb_v1_runtime_custom_background_skin_grid(
    CSB_V1_RuntimeProfile *profile,
    uint8_t *out_cell_skins,
    int out_cell_skin_capacity,
    int *out_width,
    int *out_height,
    int *out_loaded_level,
    int *out_default_skin);

/* Queue one source-locked timeline event for the CSB V1 runtime.
 * The underlying event heap is the shared V1 ReDMCSB TIMELINE.C model used
 * by DM1/CSB.  csb_v1_runtime_tick_v1() processes expired events at the
 * pre-increment game_time boundary, matching GAMELOOP.C F0002 lines 69-124:
 * F0261_TIMELINE_Process_CPSEF() runs before G0313_ul_GameTime++.
 * Returns the event slot index or -1 on invalid input/full queue.
 * Source: ReDMCSB TIMELINE.C F0238/F0240/F0261 lines 565-690,
 * 702-708, 1833-1850; GAMELOOP.C F0002 lines 69-124. */
int csb_v1_runtime_add_timeline_event(CSB_V1_RuntimeProfile *profile,
                                      const struct DM1_Event_V1 *event);

/* ReDMCSB MOVESENS.C F0274.  Consults only the loaded PC34 party and
 * dungeon identities; missing, stale, or non-PC34 state is not possession. */
int csb_v1_runtime_f0274_is_object_in_party_possession_pc34(
    const CSB_V1_RuntimeProfile *profile, int object_type);

/* Copy the dispatch records produced by the most recent fired V1 tick.
 * Returns the number of records copied or -1 on invalid input. */
int csb_v1_runtime_get_last_timeline_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct DM1_TickDispatchResult_V1 *out_result);

/* Queue one source command into the CSB runtime's V1 input command queue.
 * CSB shares the DM1/CSB ReDMCSB command queue ids and queue mechanics.
 * This entrypoint intentionally does not claim broad movement/playability;
 * csb_v1_runtime_process_one_input_command() currently applies the
 * source-locked turn boundary, bounded one-cell movement step, and bounded
 * movement consequences for stairs, pits, teleporters, doors/fake walls, and
 * party floor sensors. It still does not claim full inventory or action
 * handling.
 * Source: ReDMCSB COMMAND.C F0380 lines 2075-2127 and 2150-2156. */
int csb_v1_runtime_enqueue_input_command(CSB_V1_RuntimeProfile *profile,
                                         int command,
                                         int x,
                                         int y);

/* Process one queued CSB V1 input command.
 * TURN_LEFT/TURN_RIGHT dispatch through csb_v1_runtime_rotate_party(), except
 * on a current stairs square where CLIKMENU.C F0365 lines 164-168 routes them
 * through F0364_COMMAND_TakeStairs.  MOVE_* commands dispatch through the
 * bounded one-cell runtime movement helper with a live dungeon wall probe,
 * except MOVE_BACKWARD on current stairs follows CLIKMENU.C F0366 lines
 * 264-266. This gate still does not claim full CSB movement/runtime
 * playability: inventory and action side effects remain separate
 * source-locked boundaries.
 * Returns 1 when a command was processed/dequeued, 0 when the queue was
 * empty or movement cooldown blocked dequeue, and -1 on invalid input. */
int csb_v1_runtime_process_one_input_command(CSB_V1_RuntimeProfile *profile,
                                             int disabled_movement_ticks,
                                             int projectile_disabled_movement_ticks,
                                             int last_projectile_disabled_movement_direction);

int csb_v1_runtime_get_last_input_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputQueueProcessResultPc34Compat *out_result);

/* Trigger bounded CSB wall-ornament click sensors on a real-format wall
 * square.  object_type is the F0032 object type in hand, or -1 for an
 * empty-hand click.  Returns the number of queued remote square events.
 * Source: ReDMCSB MOVESENS.C F0276 lines 1737-1785. */
int csb_v1_runtime_trigger_wall_ornament_click(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    int object_type);

/* Extended wall-ornament click path for source sensors that move an
 * actual leader-hand thing, such as C013 object storage + sensor
 * rotation.  Pass NULL to keep the legacy object_type-only behavior. */
int csb_v1_runtime_trigger_wall_ornament_click_ex(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell,
    uint16_t *leader_hand_thing);

/* Runtime-owned variant: reads/writes CSB_V1_PartyState.LeaderHandThing
 * instead of requiring the caller to carry a transient hand copy. */
int csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
    CSB_V1_RuntimeProfile *profile,
    int map_x,
    int map_y,
    int cell);

/* Move one existing ordinary object through ReDMCSB MOVESENS.C F0267 using
 * only the loaded PC34 DUNGEON.DAT image.  The thing must already be linked
 * at the supplied source square; legacy fixture maps, detached records,
 * groups, and projectiles are rejected.  The route performs the source
 * F0276 removal observation, destination insertion/addition observation,
 * then the normal teleporter/pit/stairs chain.  Returns 1 only after the
 * source-owned move was admitted. */
int csb_v1_runtime_f0267_move_original_object(
    CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    int source_level,
    int source_map_x,
    int source_map_y,
    int destination_level,
    int destination_map_x,
    int destination_map_y);

/* Resolve a CSB runtime object thing to the ReDMCSB OBJECT.C F0033 icon
 * index using the CSB dungeon handle, not DM1 M11 world tables. */
int csb_v1_runtime_object_icon_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing);

/* Resolve the object subtype index used by the current V1 item sprite bridge.
 * This is the source object subtype, not the dynamic OBJECT.C F0033 icon. */
int csb_v1_runtime_object_subtype_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing);

/* Resolve a CSB runtime object thing to the ReDMCSB DUNGLOB.C ObjectInfo
 * ActionSetIndex used by MENU.C F0386/F0389, without using DM1 M11
 * world thing arrays. */
int csb_v1_runtime_object_action_set_index(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing);

/* Resolve a CSB runtime object thing to the ReDMCSB DUNGLOB.C ObjectInfo
 * AllowedSlots mask used by CHAMPION.C F0302 inventory placement. */
uint16_t csb_v1_runtime_object_allowed_slots(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing);

/* Admit one loaded PC34 Thing through ReDMCSB DUNGEON.C F0141's exact
 * type/subtype -> G0237 ObjectInfo arithmetic.  This is intentionally not a
 * GRAPHICS.DAT resolver and must not be used to select or render an image. */
int csb_v1_runtime_f0141_g0209_object_info_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    CSB_V1_F0141G0209ObjectInfoReceiptPc34 *out_receipt);

/* F0143 and F0144 are runtime-data receipts only. They do not select
 * GRAPHICS.DAT content, do not call M11, and fail closed on missing raw data. */
int csb_v1_runtime_f0143_armour_defense_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t armour_thing,
    int use_sharp_defense,
    CSB_V1_F0143ArmourDefenseReceiptPc34 *out_receipt);

int csb_v1_runtime_f0144_creature_attributes_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    CSB_V1_F0144CreatureAttributesReceiptPc34 *out_receipt);

int csb_v1_runtime_f0158_weapon_info_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t weapon_thing,
    CSB_V1_F0158WeaponInfoReceiptPc34 *out_receipt);

/* Admit and materialize a C007/C009/C012 object only from its loaded raw
 * PC34 C03 sensor record.  Missing source, unsupported icon, or no unused
 * F0166 record fails closed. */
int csb_v1_runtime_f0167_new_object_receipt_pc34(
    CSB_V1_DungeonData *dungeon,
    uint16_t source_sensor_thing,
    CSB_V1_F0167NewObjectReceiptPc34 *out_receipt);

/* Move a loaded PC34 C05-C10 object only when its exact raw source and
 * destination Thing chains are admissible.  Both chain writes commit as one
 * transaction; malformed lists and absent objects leave raw data untouched. */
int csb_v1_runtime_f0163_f0164_object_move_receipt_pc34(
    CSB_V1_DungeonData *dungeon,
    uint16_t thing,
    int source_level,
    int source_map_x,
    int source_map_y,
    int destination_level,
    int destination_map_x,
    int destination_map_y,
    CSB_V1_F0163F0164ObjectMoveReceiptPc34 *out_receipt);

/* Resolve one loaded PC34 C04 through F0175's full Thing chain and join it
 * to F0144. Missing or malformed source data fails closed. */
int csb_v1_runtime_f0175_group_thing_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int map_x,
    int map_y,
    CSB_V1_F0175GroupThingReceiptPc34 *out_receipt);

/* Resolve the F0176 one-based ordinal for a linked raw C04.  Missing,
 * malformed, or off-square group records fail closed. */
int csb_v1_runtime_f0176_creature_ordinal_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y,
    int requested_cell,
    CSB_V1_F0176CreatureOrdinalReceiptPc34 *out_receipt);

/* Compute F0178's packed-cell compaction for one F0190 partial group kill.
 * This only computes the authenticated raw rewrite; its runtime owner commits
 * the byte after all other F0190 data is ready. */
int csb_v1_runtime_f0178_group_cells_compact_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y,
    int creature_count,
    int removed_creature_index,
    CSB_V1_F0178GroupCellsCompactReceiptPc34 *out_receipt);

/* Admit a linked PC34 C04 to F0183's current-map active-group pool. Missing
 * source data or an exhausted pool rejects before active state changes. */
int csb_v1_runtime_f0183_active_group_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y,
    CSB_V1_F0183ActiveGroupReceiptPc34 *out_receipt);

/* Derive one F0184 C04 writeback from an authenticated current-map active
 * slot. The receipt does not mutate raw data. */
int csb_v1_runtime_f0184_active_group_remove_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    int active_group_slot,
    CSB_V1_F0184ActiveGroupRemoveReceiptPc34 *out_receipt);

/* Run GROUP.C F0194's all-active-group retirement transaction. It commits
 * no writeback unless every active current-map slot remains source-valid. */
int csb_v1_runtime_f0194_remove_all_active_groups_pc34(
    CSB_V1_RuntimeProfile *profile);

/* Authenticate a C006 generator at its loaded PC34 square plus the exact
 * unused C04 slot F0185 will materialize. The receipt does not mutate data. */
int csb_v1_runtime_f0185_generated_group_receipt_pc34(
    CSB_V1_DungeonData *dungeon,
    uint16_t source_sensor_thing,
    int map_index,
    int map_x,
    int map_y,
    CSB_V1_F0185GeneratedGroupReceiptPc34 *out_receipt);

/* Authenticate the final raw C04 delete transaction. A present current-map
 * ActiveGroup owner must match exactly; missing or drifting raw data is a
 * no-mutation rejection. */
int csb_v1_runtime_f0189_group_delete_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y,
    CSB_V1_F0189GroupDeleteReceiptPc34 *out_receipt);

/* Admit F0267's already-relocated C04 to the F0191 all-creature pit-fall
 * route. Unlinked, stale, or non-positive attack inputs fail closed. */
int csb_v1_runtime_f0191_group_fall_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y,
    int attack,
    CSB_V1_F0191GroupFallReceiptPc34 *out_receipt);

/* Admit a raw attacking Giggler C04 to F0193's champion-slot theft route. */
int csb_v1_runtime_f0193_giggler_steal_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int map_index,
    int map_x,
    int map_y,
    int creature_index,
    int champion_index,
    CSB_V1_F0193GigglerStealReceiptPc34 *out_receipt);

/* Admit F0249's C04-first move after a loaded C08/C09 becomes open. */
int csb_v1_runtime_f0249_open_square_group_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    int square_type,
    int map_index,
    int map_x,
    int map_y,
    CSB_V1_F0249OpenSquareGroupReceiptPc34 *out_receipt);

/* Admit a raw C60/C61 C04 retry before F0252 mutates its Thing chain. */
int csb_v1_runtime_f0252_group_move_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    CSB_V1_F0252GroupMoveReceiptPc34 *out_receipt);

/* Admit F0265 C60/C61 construction from an authenticated C04 owner. */
int csb_v1_runtime_f0265_group_retry_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t group_thing,
    int target_map_index,
    int target_map_x,
    int target_map_y,
    int audible,
    CSB_V1_F0265GroupRetryReceiptPc34 *out_receipt);

/* Admit the raw C04 facts required by MOVE.C F0266 before an adjacent move.
 * Malformed, unlinked, dead, cross-map, or non-adjacent inputs fail closed. */
int csb_v1_runtime_f0266_group_move_projectile_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    int source_map_index,
    int source_map_x,
    int source_map_y,
    int destination_map_x,
    int destination_map_y,
    CSB_V1_F0266GroupMoveProjectileReceiptPc34 *out_receipt);

/* Execute one already-dispatched C60/C61 group move.  Unlike the old
 * side-effect-only bridge this validates both F0252 and F0266 before any
 * write, then rolls the C04/Thing chains and timeline state back if the
 * actual source move cannot commit.  A blocked party/group destination is a
 * successful source retry schedule, not a partial move. */
int csb_v1_runtime_f0252_f0266_group_move_transaction_pc34(
    CSB_V1_RuntimeProfile *profile,
    const struct DM1_DispatchRecord_V1 *record,
    CSB_V1_F0252F0266GroupMoveTransactionReceiptPc34 *out_receipt);

/* ReDMCSB CHEST.C F0333/F0334 container bridge for M11: read the first
 * eight visible chest slots from CONTAINER.Slot and write those slots back as
 * a compact Next chain, without using DM1 GameWorld thing arrays. */
int csb_v1_runtime_read_container_slots(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t container_thing,
    uint16_t out_slots[8]);

int csb_v1_runtime_write_container_slots(
    CSB_V1_RuntimeProfile *profile,
    uint16_t container_thing,
    const uint16_t slots[8]);

int csb_v1_runtime_set_thing_next(
    CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    uint16_t next_thing);

/* Runtime render/interaction helpers for CSB thing chains.  These decode
 * CSB dungeon records directly so M11 overlay code does not duplicate C04
 * GROUP or object-chain layout knowledge from ReDMCSB DEFS.H/DUNGEON.C. */
uint16_t csb_v1_runtime_next_thing(
    const CSB_V1_DungeonData *dungeon,
    uint16_t thing);

int csb_v1_runtime_thing_type_is_floor_object(int thing_type);

int csb_v1_runtime_object_overlay_info(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t object_thing,
    CSB_V1_RuntimeObjectOverlayInfo *out_info);

int csb_v1_runtime_group_record_creature_type(
    const uint8_t *record,
    int size);

int csb_v1_runtime_group_record_direction(
    const uint8_t *record,
    int size);

int csb_v1_runtime_group_record_visible_count(
    const uint8_t *record,
    int size);

int csb_v1_runtime_group_record_creature_cell(
    const uint8_t *record,
    int size,
    int creature_index);

/* ReDMCSB GROUP.C F0195: materialize C04 groups on the current map in the
 * runtime active-group table. Returns the number newly activated, or -1 when
 * the current dungeon data cannot be traversed safely. */
int csb_v1_runtime_f0195_group_add_all_active_groups(
    CSB_V1_RuntimeProfile *profile);

int csb_v1_runtime_group_overlay_info(
    const CSB_V1_DungeonData *dungeon,
    uint16_t group_thing,
    CSB_V1_RuntimeGroupOverlayInfo *out_info);

/* Bounded champion action-hand THROW boundary for M11 CSB playability.
 * Uses the CSB runtime party snapshot and projectile list directly, without
 * DM1 GameWorld thing arrays. On success the champion action-hand slot is
 * cleared and a kinetic F0810 projectile is scheduled in the CSB runtime
 * timeline. */
int csb_v1_runtime_throw_action_hand(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int *out_projectile_slot);

/* Bounded champion ready-hand SHOOT boundary for M11 CSB playability.
 * Resolves action-hand bow/sling and ready-hand ammunition from the CSB
 * dungeon object records, removes the ready-hand object on success, and
 * schedules a champion-owned kinetic projectile in the CSB runtime. */
int csb_v1_runtime_shoot_ready_hand(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int *out_projectile_slot);

/* Refill an empty C00 ready hand after a successful SHOOT enable event.
 * Mirrors ReDMCSB TIMELINE.C F0253 lines ~1597-1607 by scanning the
 * source quiver slots for ammunition compatible with the C01 bow/sling. */
int csb_v1_runtime_refill_ready_hand_after_shoot(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int *out_source_slot,
    uint16_t *out_thing);

/* Create a champion-owned projectile from CSB runtime party pose/champion cell.
 * This is the CSB counterpart to the shared F0810 projectile create path used
 * by M11 action rows, without allocating into DM1 GameWorld.projectiles. */
int csb_v1_runtime_spawn_champion_projectile(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int action_index,
    int projectile_subtype,
    int projectile_category,
    int kinetic_energy,
    int attack,
    int attack_type_code,
    int step_energy,
    uint16_t associated_thing,
    int poison_attack,
    int potion_power,
    int *out_projectile_slot);

/* Record the currently selected F0407 champion action in the CSB runtime party
 * snapshot.  This is the M11 action-row bridge for actions whose exact combat
 * effects are still bounded elsewhere; it avoids dispatching CSB actions
 * through DM1 GameWorld attack state. */
int csb_v1_runtime_record_champion_action(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int action_index);

typedef struct {
    int action_index;
    int performed;
    int target_map_index;
    int target_map_x;
    int target_map_y;
    int target_square_type;
    int hit_group;
    int creature_index;
    int damage;
    int killed_group;
    int hit_door;
    int door_destroyed;
    int door_event_scheduled;
} CSB_V1_RuntimeMeleeActionResult;

/* Runtime-owned CSB F0402/F0231 melee-contact bridge. It records the chosen
 * champion action and applies source-style damage to a real-format C04 group
 * in the square in front of the party, without routing CSB actions through
 * DM1 GameWorld thing tables. */
int csb_v1_runtime_perform_melee_action(
    CSB_V1_RuntimeProfile *profile,
    int champion_index,
    int action_index,
    CSB_V1_RuntimeMeleeActionResult *out_result);

/* Load ReDMCSB OBJECT.C F0031 PC object names from M564_GRAPHIC_OBJECT_NAMES.
 * The PC stream terminates each name by setting bit 7 on the final byte. */
int csb_v1_runtime_load_object_names_m564(
    CSB_V1_RuntimeProfile *profile,
    const uint8_t *bytes,
    size_t byte_count);

/* Resolve a CSB runtime object thing to the leader-hand object name from
 * CSB dungeon records, without using DM1 M11 world tables. */
int csb_v1_runtime_object_name(
    const CSB_V1_RuntimeProfile *profile,
    uint16_t thing,
    char *out,
    size_t out_size);

/* ── Variant diagnostics ─────────────────────────────────────────────── */
const char *csb_v1_runtime_variant_name(CSB_V1_VariantId id);
const CSB_V1_VariantInfo *csb_v1_runtime_get_variant_info(CSB_V1_VariantId id);

/* Detect variant by matching gfx + dungeon MD5 hashes.
 * Returns the best-matching VariantId, or CSB_V1_VARIANT_UNKNOWN. */
int csb_v1_runtime_detect_variant(const char *gfx_path,
                                    const char *dungeon_path,
                                    const char *md5_gfx,
                                    const char *md5_dungeon);

/* ── Difficulty helpers ─────────────────────────────────────────────── */
int csb_v1_runtime_calc_difficulty(int champion_count);
const char *csb_v1_runtime_difficulty_str(int difficulty_x100);

/* ── Asset discovery ────────────────────────────────────────────────── */

/* Find CSB DUNGEON.DAT by hash (6695d2acebce49f95db1d8f3a5c733de).
 * Searches data_dir/csb/ and data_dir/ recursively.
 * Returns absolute path, or NULL if not found. */
const char *csb_v1_runtime_find_dungeon(const char *data_dir,
                                         CSB_V1_AssetResult *out_result);

/* Find CSB graphics archive.
 * Searches csb.dat / CSBGRAPH.DAT / GRAPHICS.DAT in order.
 * Returns absolute path, or NULL if none found. */
const char *csb_v1_runtime_find_graphics(const char *data_dir,
                                           const char *version_hint,
                                           CSB_V1_AssetResult *out_result);

/* ── Save namespace ──────────────────────────────────────────────── */
const char *csb_v1_runtime_save_dir(void);
const char *csb_v1_runtime_save_path(int slot);

/* Save/reload the current CSB runtime profile through the CSB save header
 * path.  The loaded profile must already be initialized/booted enough to
 * own its asset paths/dungeon handle; the save payload restores the live
 * runtime state that ReDMCSB LOADSAVE.C F0435 copies from GLOBAL_DATA,
 * PARTY, EVENTS, and TIMELINE into the running game. */
int csb_v1_runtime_save_game_to_path(const CSB_V1_RuntimeProfile *profile,
                                     const char *path);
int csb_v1_runtime_load_game_from_path(CSB_V1_RuntimeProfile *profile,
                                       const char *path);
int csb_v1_runtime_can_load_resume_path(const char *path);
int csb_v1_runtime_import_dm1_party_path(CSB_V1_RuntimeProfile *profile,
                                         const char *path,
                                         int *out_count,
                                         int *out_utility_state,
                                         char *out_utility_prompt,
                                         size_t out_utility_prompt_size);
enum {
    CSB_V1_RUNTIME_STARTUP_PROMPT_CAP_PC34 = 192
};
typedef enum {
    CSB_V1_RUNTIME_STARTUP_HANDOFF_NONE_PC34 = 0,
    CSB_V1_RUNTIME_STARTUP_HANDOFF_RESUME_PC34 = 1,
    CSB_V1_RUNTIME_STARTUP_HANDOFF_IMPORT_DM1_PC34 = 2
} CSB_V1_RuntimeStartupHandoffKind_PC34;
typedef struct {
    CSB_V1_RuntimeStartupHandoffKind_PC34 kind;
    int direct_resume_loaded;
    int import_attempted;
    int import_succeeded;
    int import_champion_count;
    int import_utility_state;
    char import_utility_prompt[CSB_V1_RUNTIME_STARTUP_PROMPT_CAP_PC34];
    const char *status_scope;
    const char *status;
} CSB_V1_RuntimeStartupHandoffReceipt_PC34;

#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

typedef enum {
    CSB_V1_RUNTIME_STARTUP_PLAN_NONE_PC34 = 0,
    CSB_V1_RUNTIME_STARTUP_PLAN_ENTER_DUNGEON_PC34 = 1,
    CSB_V1_RUNTIME_STARTUP_PLAN_ENTER_BONUS_DUNGEON_PC34 = 2,
    CSB_V1_RUNTIME_STARTUP_PLAN_RESUME_PC34 = 3
} CSB_V1_RuntimeStartupRuntimePlanKind_PC34;
typedef struct {
    CSB_V1_RuntimeStartupRuntimePlanKind_PC34 kind;
    int set_bonus_dungeon;
    int bonus_dungeon;
    int requires_resume_load;
} CSB_V1_RuntimeStartupRuntimePlan_PC34;
typedef struct {
    int resume_available;
    int resume_loaded;
    int bonus_requested_changed;
    int bonus_requested;
    int bonus_dungeon_loaded;
    int sync_profile_state;
    int sync_leader_hand;
} CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34;
typedef struct {
    int level_loaded;
    int current_level;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
} CSB_V1_RuntimeViewStateReceipt_PC34;
typedef struct {
    int valid;
    struct PartyState_Compat party;
} CSB_V1_RuntimePartyMirrorReceipt_PC34;
typedef struct {
    int valid;
    CSB_V1_RuntimeViewStateReceipt_PC34 view;
    CSB_V1_RuntimePartyMirrorReceipt_PC34 party;
    int leader_hand_present;
    uint16_t leader_hand_thing;
    int leader_hand_icon_index;
    char leader_hand_object_name[32];
} CSB_V1_RuntimeM11MirrorReceipt_PC34;
typedef struct {
    int entrance_resume_available;
    char entrance_resume_path[CSB_V1_STARTUP_PATH_CAP_PC34];
    int import_available;
    int import_champion_count;
    int import_selected_action_index;
    int import_preview_active;
    int import_utility_state;
    char import_dm1_save_path[CSB_V1_STARTUP_PATH_CAP_PC34];
    char import_utility_prompt[CSB_V1_STARTUP_PROMPT_CAP_PC34];
} CSB_V1_RuntimeStartupSessionStateReceipt_PC34;
void csb_v1_runtime_startup_handoff_receipt_init_pc34(
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 *receipt);
int csb_v1_runtime_apply_startup_handoff_pc34(
    CSB_V1_RuntimeProfile *profile,
    const char *save_path,
    const char *import_dm1_save_path,
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 *out_receipt);
int csb_v1_runtime_build_startup_session_options_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeStartupHandoffReceipt_PC34 *handoff,
    const char *import_dm1_save_path,
    const char *entrance_resume_save_path,
    CSB_V1_StartupSessionOptions_PC34 *out_options);
void csb_v1_runtime_startup_session_state_receipt_init_pc34(
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *receipt);
int csb_v1_runtime_startup_session_state_receipt_from_options_pc34(
    const CSB_V1_StartupSessionOptions_PC34 *options,
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *out_receipt);
int csb_v1_runtime_build_startup_session_state_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeStartupHandoffReceipt_PC34 *handoff,
    const char *import_dm1_save_path,
    const char *entrance_resume_save_path,
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *out_receipt);
void csb_v1_runtime_startup_runtime_plan_receipt_init_pc34(
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *receipt);
int csb_v1_runtime_apply_startup_runtime_plan_pc34(
    CSB_V1_RuntimeProfile *profile,
    const CSB_V1_RuntimeStartupRuntimePlan_PC34 *runtime_plan,
    const char *resume_path,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_receipt);
int csb_v1_runtime_apply_startup_sequence_plan_pc34(
    CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_receipt);
int csb_v1_runtime_apply_startup_sequence_plan_from_state_facts_with_receipts_pc34(
    CSB_V1_RuntimeProfile *profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_runtime_exec_receipt,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_apply_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt);
int csb_v1_runtime_apply_startup_sequence_plan_from_boot_profile_facts_with_receipts_pc34(
    void *boot_profile,
    const struct CSB_V1_StartupRuntimePlan_PC34 *startup_plan,
    const char *resume_path,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    CSB_V1_RuntimeStartupRuntimePlanReceipt_PC34 *out_runtime_exec_receipt,
    CSB_V1_StartupEntranceInputOutcome_PC34 *out_outcome,
    CSB_V1_StartupRuntimeApplyReceipt_PC34 *out_runtime_apply_receipt,
    CSB_V1_StartupCommandStateReceipt_PC34 *out_state_receipt);
void csb_v1_runtime_view_state_receipt_init_pc34(
    CSB_V1_RuntimeViewStateReceipt_PC34 *receipt);
int csb_v1_runtime_view_state_receipt_from_profile_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimeViewStateReceipt_PC34 *out_receipt);
void csb_v1_runtime_party_mirror_receipt_init_pc34(
    CSB_V1_RuntimePartyMirrorReceipt_PC34 *receipt);
int csb_v1_runtime_party_mirror_receipt_from_profile_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimePartyMirrorReceipt_PC34 *out_receipt);
void csb_v1_runtime_m11_mirror_receipt_init_pc34(
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *receipt);
int csb_v1_runtime_m11_mirror_receipt_from_profile_pc34(
    const CSB_V1_RuntimeProfile *profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt);
int csb_v1_runtime_m11_mirror_receipt_from_boot_profile_pc34(
    const void *boot_profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt);
int csb_v1_runtime_util_render_plan_from_boot_profile_facts_pc34(
    int selected_action_index,
    int imported_champion_count,
    const void *boot_profile,
    const char *prompt_override,
    int preview_active,
    CSB_V1_UtilRenderPlan *out_plan);
typedef struct CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 {
    CSB_V1_UtilApplyReceipt util_receipt;
    CSB_V1_UtilStateReceipt util_state_receipt;
    int entrance_receipt_valid;
    CSB_V1_StartupEntranceHostActionReceipt_PC34 entrance_receipt;
} CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34;
void csb_v1_runtime_util_startup_host_action_receipt_init_pc34(
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *receipt);
int csb_v1_runtime_save_game_to_path_from_boot_profile_pc34(
    const void *boot_profile,
    const char *path,
    uint32_t *out_game_time);
int csb_v1_runtime_load_game_from_path_from_boot_profile_pc34(
    void *boot_profile,
    const char *path,
    uint32_t *out_game_time);
int csb_v1_runtime_tick_from_boot_profile_pc34(
    void *boot_profile,
    uint32_t *out_game_time);
int csb_v1_runtime_object_icon_index_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing);
int csb_v1_runtime_object_action_set_index_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing);
uint16_t csb_v1_runtime_object_allowed_slots_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing);
int csb_v1_runtime_object_name_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short thing,
    char *out,
    size_t out_size);
int csb_v1_runtime_read_container_slots_from_boot_profile_pc34(
    const void *boot_profile,
    unsigned short container_thing,
    unsigned short out_slots[8]);
int csb_v1_runtime_write_container_slots_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short container_thing,
    const unsigned short slots[8]);
int csb_v1_runtime_set_thing_next_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short thing,
    unsigned short next_thing);
int csb_v1_runtime_write_inventory_slot_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int csb_slot,
    unsigned short thing);
int csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short thing);
int csb_v1_runtime_throw_leader_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    unsigned short leader_thing,
    unsigned short *out_restored_action_hand,
    int *out_projectile_slot);
int csb_v1_runtime_write_champion_vitals_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int current_health,
    int current_stamina,
    int current_mana);
int csb_v1_runtime_throw_action_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_projectile_slot);
int csb_v1_runtime_shoot_ready_hand_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_projectile_slot);
int csb_v1_runtime_refill_ready_hand_after_shoot_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int *out_source_slot,
    unsigned short *out_thing);
int csb_v1_runtime_spawn_champion_projectile_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int action_index,
    int projectile_subtype,
    int projectile_category,
    int kinetic_energy,
    int attack,
    int attack_type_code,
    int step_energy,
    unsigned short associated_thing,
    int poison_attack,
    int potion_power,
    int *out_projectile_slot);
int csb_v1_runtime_perform_melee_action_from_boot_profile_pc34(
    void *boot_profile,
    int champion_index,
    int action_index);
int csb_v1_runtime_trigger_front_wall_ornament_click_from_boot_profile_pc34(
    void *boot_profile,
    unsigned short leader_hand_thing,
    unsigned short *out_leader_hand_thing);
int csb_v1_runtime_import_csbgame_roster_from_path(
    CSB_V1_RuntimeProfile *profile,
    const char *path);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *csb_v1_runtime_source_evidence(void);

#ifdef __cplusplus
}
#endif



#endif /* FIRESTAFF_CSB_V1_RUNTIME_PC34_COMPAT_H */
