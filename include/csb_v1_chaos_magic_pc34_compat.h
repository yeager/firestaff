
#ifndef FIRESTAFF_CSB_V1_CHAOS_MAGIC_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHAOS_MAGIC_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Chaos Magic System
 *
 * CSB extends DM1's spell system with the Chaos magic:
 * - DSA (Dungeon Scripting Architecture) scripts triggered by spells
 * - Programmable spell effects per dungeon level
 * - Custom creature behaviors tied to Chaos scripts
 *
 * Source: CSBWin/Chaos.cpp (5336 lines)
 * Source: CSBWin/DSA.cpp (5806 lines)
 */

#define CSB_V1_MAX_DSA_SCRIPTS 256
#define CSB_V1_DSA_STACK_SIZE 64
#define CSB_V1_MAX_DSA_BYTECODE_BYTES 65536

/* CSBWin Data.h:1686-1708, 1947-1984.  These are source DSA word-code
 * values, not the older Firestaff compatibility-bytecode values below. */
#define CSB_V1_CSBWIN_DSACMD_LOAD 6u
#define CSB_V1_CSBWIN_DSACMD_FETCH 7u
#define CSB_V1_CSBWIN_DSACMD_NOOP 3u
#define CSB_V1_CSBWIN_DSACMD_EQUAL 8u
#define CSB_V1_CSBWIN_DSACMD_QUESTION 9u
#define CSB_V1_CSBWIN_DSACMD_GOSUB 5u
#define CSB_V1_CSBWIN_DSACMD_MESSAGE 1u
#define CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER 4u
#define CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER32 14u
#define CSB_V1_CSBWIN_DSACMD_MESSAGE32 15u
#define CSB_V1_CSBWIN_DSACMD_DESSAGE32 22u
#define CSB_V1_CSBWIN_DSACMD_CASE 16u
#define CSB_V1_CSBWIN_DSACMD_OVERRIDE 2u
#define CSB_V1_CSBWIN_DSACMD_AMPERSAND 11u
#define CSB_V1_CSBWIN_DSACMD_JUMP 12u
#define CSB_V1_CSBWIN_DSACMD_AMPERSAND2 21u
#define CSB_V1_CSBWIN_DSACMD_STORE 13u
#define CSB_V1_CSBWIN_DSACMD_VARIABLEFETCH 17u
#define CSB_V1_CSBWIN_DSACMD_VARIABLESTORE 18u
#define CSB_V1_CSBWIN_DSACMD_GLOBALFETCH 19u
#define CSB_V1_CSBWIN_DSACMD_GLOBALSTORE 20u
#define CSB_V1_CSBWIN_DSA_LOAD_INTEGER 26u
#define CSB_V1_CSBWIN_DSA_LOAD_ABS 27u
#define CSB_V1_CSBWIN_DSA_LOAD_DOLLAR 28u
#define CSB_V1_CSBWIN_DSA_LOAD_ABS32 29u
#define CSB_V1_CSBWIN_DSA_LOAD_INTEGER32 30u

typedef enum {
    CSB_DSA_OP_NOP = 0,
    CSB_DSA_OP_SET,
    CSB_DSA_OP_CLEAR,
    CSB_DSA_OP_TOGGLE,
    CSB_DSA_OP_TEST,
    CSB_DSA_OP_JUMP,
    CSB_DSA_OP_CALL,
    CSB_DSA_OP_RETURN,
    CSB_DSA_OP_DELAY,
    CSB_DSA_OP_SOUND,
    CSB_DSA_OP_SPAWN,
    CSB_DSA_OP_MOVE,
    CSB_DSA_OP_DAMAGE,
    CSB_DSA_OP_TELEPORT,
    CSB_DSA_OP_MESSAGE,
    CSB_DSA_OP_END,
    CSB_DSA_OP_COUNT
} CSB_DSA_Opcode;

typedef struct {
    uint16_t *bytecode;
    int bytecode_len;
    int pc;  /* program counter */
    int stack[CSB_V1_DSA_STACK_SIZE];
    int sp;  /* stack pointer */
    int active;
    int delay_ticks;
} CSB_V1_DSAScript;

typedef enum {
    CSB_V1_DSA_DISPATCH_NONE = 0,
    CSB_V1_DSA_DISPATCH_MESSAGE
} CSB_V1_DSADispatchKind;

typedef struct {
    CSB_V1_DSADispatchKind kind;
    int opcode;
    int operand;
    int op_pc;
} CSB_V1_DSADispatchRecord;

/* A CSBWin extended-save DSA action after ReadDSAs()/DSA::Read has
 * authenticated its containing section.  `program_words` is an owned copy
 * of the source little-endian DSAAction program; it is intentionally not
 * interpreted as this compatibility VM's private opcode stream. */
typedef struct {
    uint8_t dsa_id;
    uint32_t state_index;
    uint32_t column;
    uint16_t *program_words;
    int program_word_count;
} CSB_V1_DSAImportedAction;

/* CSBWin DSA.cpp:5637-5672 serializes this DSA header before its state
 * table.  ProcessDSATimer6 later selects state through LocalState(): only
 * local state 0 is the actuator-owned DSAstate nibble.  Keep the complete
 * source header identity so runtime handoffs never invent a state selector. */
typedef struct {
    int valid;
    uint32_t persistent_state;
    uint32_t local_state;
    uint32_t group_id;
    uint32_t state_slot_count;
} CSB_V1_CSBWinDSAImportedHeader;

typedef enum {
    CSB_V1_CSBWIN_DSA_LOAD_OK = 0,
    CSB_V1_CSBWIN_DSA_LOAD_NOT_LOAD = 1,
    CSB_V1_CSBWIN_DSA_LOAD_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_LOAD_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSALoadResult;

typedef enum {
    CSB_V1_CSBWIN_DSA_LOAD_STORE_OK = 0,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSALoadStoreResult;

typedef enum {
    CSB_V1_CSBWIN_DSA_STACK_OK = 0,
    CSB_V1_CSBWIN_DSA_STACK_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_STACK_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSAStackResult;

typedef enum {
    CSB_V1_CSBWIN_DSA_CORE_OK = 0,
    CSB_V1_CSBWIN_DSA_CORE_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_CORE_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_CORE_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSACoreVerifyResult;

typedef struct {
    int valid;
    int transfer_only;
    int stack_core;
    int requires_runtime_owner;
    int conditional_core;
    int arithmetic_core;
    int comparison_core;
    int variable_core;
    int timer_core;
    int message_core;
    int text_display_core;
    int sound_core;
    int champion_core;
    int object_core;
    int query_core;
    int dungeon_mutation_core;
    uint16_t command_count;
    uint16_t words_consumed;
    uint8_t unsupported_opcode;
    uint16_t unsupported_subcode;
} CSB_V1_CSBWinDSACoreProgramReceipt;

/* The master location is packed exactly as CSBWin LOCATIONREL::Integer():
 * position bits 16-17, level 10-15, X 5-9, Y 0-4.  `parameters` models the
 * ordered A..Z type-47 actuator chain used by EX_LOAD; unavailable entries
 * produce the source's zero value. */
typedef struct {
    uint32_t master_location;
    const uint32_t *parameters;
    int parameter_count;
} CSB_V1_CSBWinDSALoadContext;

typedef struct {
    uint32_t value;
    int next_state;
    uint16_t words_consumed;
    uint8_t selector;
} CSB_V1_CSBWinDSALoadExecution;

/* Mutable A..Z parameter surface used by DSA.cpp EX_LOAD and EX_STORE. It is
 * accepted only through the runtime-owned authenticated lookup below. */
typedef struct {
    uint32_t master_location;
    uint32_t *parameters;
    int parameter_count;
} CSB_V1_CSBWinDSALoadStoreContext;

typedef struct {
    uint32_t value;
    int load_next_state;
    int next_state;
    uint16_t words_consumed;
    uint8_t load_selector;
    uint8_t store_selector;
} CSB_V1_CSBWinDSALoadStoreExecution;

/* The source STACK has 100 signed 32-bit cells (CSBWin DSA.cpp:98-426).
 * Runtime-owned callers may bind the verified EX_AMPERSAND/AMPERSAND2 subset
 * to save, timer, and dungeon callbacks. Unsupported opcodes and unowned
 * callbacks remain outside this authenticated action surface. */
#define CSB_V1_CSBWIN_DSA_STACK_CAPACITY 100
#define CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY 100
#define CSB_V1_CSBWIN_DSA_VARIABLE_COUNT 100

/* CSBWin DSA.cpp:3107-3135 routes the two skin operators through the
 * currently loaded SKIN_CACHE.  The bytecode module deliberately owns no
 * dungeon or EXPOOL storage, so a runtime owner supplies this narrow,
 * transactional bridge.  A missing hook keeps the opcode unsupported. */
typedef int (*CSB_V1_CSBWinDSAGetSkinFn)(void *user,
                                         uint32_t location,
                                         uint8_t *out_skin);
typedef int (*CSB_V1_CSBWinDSASetSkinFn)(void *user,
                                         uint32_t location,
                                         uint8_t skin);
/* CSBWin CHARDESC::GetFromWings reads eight exact EDT_Character EXPOOL
 * records.  The bytecode executor owns neither EXPOOL nor a character
 * layout, so the runtime provides this read-only, fail-closed bridge. */
typedef int (*CSB_V1_CSBWinDSAGetWingTalentsFn)(void *user,
                                                uint16_t fingerprint,
                                                uint32_t *out_talents);
typedef int (*CSB_V1_CSBWinDSAHasWingCharacterFn)(void *user,
                                                   uint16_t fingerprint);
typedef int (*CSB_V1_CSBWinDSASetWingTalentsFn)(void *user,
                                                uint16_t fingerprint,
                                                uint32_t talents);
typedef int (*CSB_V1_CSBWinDSAGetInfoFn)(void *user,
                                         uint16_t thing,
                                         int *out_selector,
                                         int *out_state,
                                         int *out_parameter_a,
                                         int *out_parameter_b);
/* CSBWin DSA.cpp STKOP_FetchExCellFlg reads the eight source ui32 words in
 * one EDT_ExtendedCellFlags EXPOOL record.  The runtime owns the authenticated
 * save tail; a missing hook keeps this source-owned query unavailable. */
typedef int (*CSB_V1_CSBWinDSAGetExCellFlagsFn)(void *user,
                                                uint32_t location,
                                                uint32_t out_words[8]);
typedef int (*CSB_V1_CSBWinDSASetExCellFlagsFn)(void *user,
                                                uint32_t location,
                                                uint32_t flags);
typedef int (*CSB_V1_CSBWinDSAGetGeneratorDelayFn)(void *user,
                                                    uint32_t location,
                                                    int *out_delay);
typedef int (*CSB_V1_CSBWinDSASetGeneratorDelayFn)(void *user,
                                                    uint32_t location,
                                                    int delay);
typedef int (*CSB_V1_CSBWinDSACommitGeneratorDelayFn)(void *user,
                                                       uint32_t location,
                                                       int expected_delay,
                                                       int delay);
/* CSBWin DSA.cpp STKOP_MonsterFetch reads one loaded DB4 record into its
 * eight-word source result. The runtime owns the decoded DB4 layout. */
typedef int (*CSB_V1_CSBWinDSAGetMonsterInfoFn)(void *user,
                                                uint16_t thing,
                                                uint32_t out_values[8]);
typedef int (*CSB_V1_CSBWinDSASetMonsterInfoFn)(void *user,
                                                uint16_t thing,
                                                const uint32_t values[8],
                                                uint8_t write_mask);
typedef int (*CSB_V1_CSBWinDSAGetCellInfoFn)(void *user,
                                             uint32_t location,
                                             uint32_t out_values[5]);
/* Returns 1 for a source-writable cell, 0 for CSBWin's silent source no-op,
 * and -1 when the loaded original cell owner is unavailable or malformed. */
typedef int (*CSB_V1_CSBWinDSAResolveCellStoreFn)(void *user,
                                                  uint32_t location,
                                                  uint32_t expected_room_type);
typedef int (*CSB_V1_CSBWinDSASetCellInfoFn)(void *user,
                                             uint32_t location,
                                             const uint32_t values[5],
                                             uint8_t write_mask);
/* CSBWin DSA.cpp EX_COPYTELEPORTER copies the first source DB1 teleporter
 * record and the source CELLFLAG byte to an existing destination teleporter.
 * The runtime owner must reject missing real cells/records rather than
 * creating a substitute teleporter. */
typedef int (*CSB_V1_CSBWinDSACopyTeleporterFn)(
    void *user, uint32_t source_location, uint32_t destination_location);

/* DSA.cpp's object attribute opcodes address the original DB5/DB6/DB8/DB10
 * record selected by an authenticated Thing integer.  The runtime owns the
 * raw record and exposes only these source-defined fields. */
typedef enum {
    CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE = 0,
    CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN = 1,
    CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED = 2,
    CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES = 3,
    CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE = 4
} CSB_V1_CSBWinDSAObjectProperty;

typedef int (*CSB_V1_CSBWinDSAGetObjectPropertyFn)(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t *out_value);
typedef int (*CSB_V1_CSBWinDSASetObjectPropertyFn)(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t value);
/* CSBWin DSA.cpp STKOP_Copy copies DB3 bytes two through seven between
 * like-typed actuator records. The runtime owns both raw records and must
 * reject a missing or malformed record rather than inventing a payload. */
typedef int (*CSB_V1_CSBWinDSAGetActuatorPayloadFn)(
    void *user, uint16_t thing, uint8_t out_payload[6]);
typedef int (*CSB_V1_CSBWinDSASetActuatorPayloadFn)(
    void *user, uint16_t thing, const uint8_t payload[6]);
/* CSBWin DSA.cpp STKOP_Copy commits one DB3 source/destination pair.  The
 * runtime callback rechecks both source owners against the staged source
 * bytes immediately before writing the destination. */
typedef int (*CSB_V1_CSBWinDSACopyActuatorPayloadFn)(
    void *user, uint16_t source_thing, uint16_t destination_thing,
    const uint8_t source_payload[6]);
typedef int (*CSB_V1_CSBWinDSANormalizeObjectPropertyFn)(
    void *user, uint16_t thing, CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t input_value, uint32_t *out_value);
/* DSA.cpp STKOP_ChPoss/STKOP_MonPoss return a converted Thing integer or
 * -1 for the source empty-list sentinel. */
typedef int (*CSB_V1_CSBWinDSAGetChampionPossessionFn)(
    void *user, int champion_index, uint32_t slot_index, int32_t *out_thing);
typedef int (*CSB_V1_CSBWinDSAGetMonsterPossessionFn)(
    void *user, uint16_t monster_thing, uint32_t possession_index,
    int32_t *out_thing);
/* DSA.cpp ExamineCell powers THISCELL and NEIGHBORS from a loaded CELLFLAG
 * map plus its DB1/DB4 chains. */
typedef int (*CSB_V1_CSBWinDSAInspectCellsFn)(
    void *user, uint32_t location, uint32_t criteria_mask,
    uint32_t first_cell, uint32_t last_cell, uint32_t *out_result);
typedef int (*CSB_V1_CSBWinDSAGetThingTypeFn)(
    void *user, int32_t thing_index, int32_t *out_type);
/* DSA.cpp EX_FETCH walks one original square Thing chain. A zero return is
 * unavailable/malformed ownership; a successful miss is out_thing=-1. */
typedef int (*CSB_V1_CSBWinDSAFetchObjectFn)(
    void *user, uint32_t depth, uint32_t location, uint32_t position_mask,
    uint32_t object_mask, int32_t *out_thing);
typedef int (*CSB_V1_CSBWinDSAIsCarriedFn)(
    void *user, int32_t character_selector, int32_t object_selector,
    int32_t *out_result);
typedef int (*CSB_V1_CSBWinDSAGetLevelMultiplierFn)(
    void *user, int32_t level, int32_t *out_multiplier);
/* DSA.cpp STKOP_MissileInfoFetch/Store access DB14 plus its TIMER word8. */
typedef int (*CSB_V1_CSBWinDSAGetMissileInfoFn)(
    void *user, uint16_t thing, uint32_t out_values[4]);
typedef int (*CSB_V1_CSBWinDSASetMissileInfoFn)(
    void *user, uint16_t thing, const uint32_t values[4]);
typedef int (*CSB_V1_CSBWinDSACommitMissileInfoFn)(
    void *user, uint16_t thing, const uint32_t expected_values[4],
    const uint32_t values[4]);
/* DSA.cpp STKOP_Mastery delegates to the live CHARDESC/skill owner. */
typedef int (*CSB_V1_CSBWinDSAGetMasteryFn)(
    void *user, uint32_t champion_index, uint32_t skill_index,
    uint32_t flags, uint32_t *out_mastery);
/* DSA.cpp STKOP_PartyFetch reads all twelve GAMEBLOCK2 party words as one
 * coherent snapshot. The source owner must decline when any word is absent. */
typedef int (*CSB_V1_CSBWinDSAGetPartyInfoFn)(
    void *user, uint32_t out_values[12]);
/* DSA.cpp STKOP_CharFetch has source-owned hand-character, Wings, pending
 * damage, attribute, and skill rules. A return of zero means the selector is
 * invalid in that exact owner and therefore produces the source zero image. */
typedef int (*CSB_V1_CSBWinDSAGetCharacterInfoFn)(
    void *user, int32_t character_selector, uint32_t out_values[59]);
/* STKOP_CharStore mutates only selected CHARDESC fields. Preparation may
 * clamp the source-visible health DSAVAR, but must not publish a live write;
 * the executor calls set_character_info only after full action acceptance. */
typedef int (*CSB_V1_CSBWinDSAPrepareCharacterStoreFn)(
    void *user, int32_t character_selector, uint32_t values[59],
    uint32_t word_count);
typedef int (*CSB_V1_CSBWinDSASetCharacterInfoFn)(
    void *user, int32_t character_selector, const uint32_t values[59],
    uint32_t word_count);
/* STKOP_ExperiencePlus delegates CSBWin Magic.cpp::AddToSkill to a live
 * CHARDESC/skill candidate. Preparation must not publish XP or level changes. */
typedef int (*CSB_V1_CSBWinDSAPrepareExperiencePlusFn)(
    void *user, int32_t character_selector, int32_t skill_number,
    int32_t experience);
typedef int (*CSB_V1_CSBWinDSAAddExperiencePlusFn)(
    void *user, int32_t character_selector, int32_t skill_number,
    int32_t experience);
/* STKOP_SwapCharacter owns party roster, CHARDESC, and Wings mutation. The
 * prepare callback returns a source SwapCharacter status in out_result and
 * reports whether an accepted action has a candidate roster mutation. */
typedef int (*CSB_V1_CSBWinDSAPrepareCharacterSwapFn)(
    void *user, int32_t party_index, int32_t fingerprint,
    uint32_t *out_result);
typedef int (*CSB_V1_CSBWinDSACommitCharacterSwapFn)(
    void *user, int32_t party_index, int32_t fingerprint);
/* STKOP_CausePoison delegates damage, flags, and TT_75 scheduling to one
 * source-owned candidate; no partial poison state may be published. */
typedef int (*CSB_V1_CSBWinDSAPrepareCausePoisonFn)(
    void *user, int32_t character_selector, int32_t poison_value);
typedef int (*CSB_V1_CSBWinDSACommitCausePoisonFn)(
    void *user, int32_t character_selector, int32_t poison_value);
typedef int (*CSB_V1_CSBWinDSADiscardTextFn)(void *user);
typedef int (*CSB_V1_CSBWinDSAPlaySoundFn)(
    void *user, int32_t sound_number, int32_t volume, int32_t flags);
/* CSBWin DSA.cpp::EX_OVERRIDE writes Override_P/Override_Pos.  The runtime
 * owner supplies the real ProcessTimers scope; no synthetic global state is
 * accepted by the bounded interpreter. */
typedef int (*CSB_V1_CSBWinDSASetOverridePFn)(
    void *user, int enabled, uint32_t position);
/* CSBWin DSA.cpp::STKOP_SetAdjustSkillsParameters updates the five global
 * values consumed by Magic.cpp::AddToSkill.  Their runtime owner must accept
 * the complete replacement after the authenticated action succeeds. */
typedef int (*CSB_V1_CSBWinDSASetAdjustSkillsParametersFn)(
    void *user, const uint32_t values[5]);
/* Character.cpp::ModifyDescription owns DB2 text decoding and the live
 * descriptive phrase state.  It receives only original location/index/color
 * triples after a complete authenticated DSA action. */
typedef int (*CSB_V1_CSBWinDSADescribeFn)(
    void *user, int32_t location, int32_t index, int32_t color);
/* CSBWin DSA.cpp EX_MESSAGE/QueueDSASwitchAction schedules a source TIMER,
 * not an immediate visual/text mutation.  The runtime owner must validate the
 * target cell for M routes and enqueue exactly the selected timer record. */
typedef int (*CSB_V1_CSBWinDSAQueueSwitchActionFn)(
    void *user, uint32_t delay, uint32_t action, uint32_t target_location,
    int message_route, uint8_t *out_event_type);

typedef struct {
    uint32_t master_location;
    uint32_t *parameters;
    int parameter_count;
    /* CSBWin DSA.cpp STKOP_PartyDistance and STKOP_GlobalFetch read this
     * live party pose.  It is supplied by the runtime profile that owns it. */
    int party_location_valid;
    int party_level;
    int party_x;
    int party_y;
    int party_direction;
    int game_time_valid;
    uint32_t game_time;
    /* CSBWin CSBCode.cpp::STRandom owns this GAMEBLOCK2 state. */
    int random_state_valid;
    uint32_t random_state;
    int dsa_slave_thing_valid;
    uint16_t dsa_slave_thing;
    /* DSA.cpp::STKOP_ObjectID reads this live DSA-bank value. */
    int most_recent_interesting_object_valid;
    uint32_t most_recent_interesting_object;
    int party_champions_valid;
    int party_champion_count;
    int party_leader_index;
    uint32_t party_champion_talents[4];
    uint16_t party_champion_fingerprints[4];
    uint16_t party_champion_wounds[4];
    int party_champion_health[4];
    int saves_disabled_valid;
    int saves_disabled;
    /* CSBWin DSA.cpp:4931-4947 keeps this three-entry SET/CLEAR/TOGGLE
     * remap in the current ProcessTimers execution only. The caller owns
     * that timer scope; the DSA core stages all three values until the
     * authenticated action has been fully consumed. */
    int timer_type_modifiers_valid;
    uint8_t timer_type_modifiers[3];
    /* CSBWin DSA.cpp:4898-4929 owns these four transient graphic offsets
     * and the redraw latch in the current DSA/render context. They are not
     * derived from host scaling or synthesized graphics. */
    int jitter_state_valid;
    int32_t x_graphic_jitter;
    int32_t y_graphic_jitter;
    int32_t x_overlay_jitter;
    int32_t y_overlay_jitter;
    int jitter_changed;
    /* CSBWin Monster.cpp resets this four-direction mask before each live
     * movement-filter execution; STKOP_MonBlk then owns its replacement. */
    int monster_move_inhibit_valid;
    uint8_t monster_move_inhibit[4];
    int override_state_valid;
    int override_p;
    uint32_t override_position;
    CSB_V1_CSBWinDSASetOverridePFn set_override_p;
    void *override_user;
    CSB_V1_CSBWinDSASetAdjustSkillsParametersFn set_adjust_skills_parameters;
    CSB_V1_CSBWinDSADescribeFn describe;
    CSB_V1_CSBWinDSAQueueSwitchActionFn queue_switch_action;
    /* CSBWin DSA.cpp EX_GLOBALFETCH/EX_GLOBALSTORE address the source
     * numGlobalVariables/globalVariables bank. The caller owns this narrow
     * runtime surface; the executor stages it and publishes it only after a
     * fully consumed authenticated action succeeds. */
    uint32_t *global_variables;
    int global_variable_count;
    CSB_V1_CSBWinDSAGetSkinFn get_skin;
    CSB_V1_CSBWinDSASetSkinFn set_skin;
    void *skin_user;
    CSB_V1_CSBWinDSAGetWingTalentsFn get_wing_talents;
    CSB_V1_CSBWinDSAHasWingCharacterFn has_wing_character;
    CSB_V1_CSBWinDSASetWingTalentsFn set_wing_talents;
    CSB_V1_CSBWinDSAGetInfoFn get_dsa_info;
    void *wing_user;
    uint16_t wing_talents_store_count;
    uint16_t last_wing_talents_fingerprint;
    uint32_t last_wing_talents_before;
    uint32_t last_wing_talents_after;
    CSB_V1_CSBWinDSAGetExCellFlagsFn get_excell_flags;
    CSB_V1_CSBWinDSASetExCellFlagsFn set_excell_flags;
    void *excell_user;
    CSB_V1_CSBWinDSAGetGeneratorDelayFn get_generator_delay;
    CSB_V1_CSBWinDSASetGeneratorDelayFn set_generator_delay;
    CSB_V1_CSBWinDSACommitGeneratorDelayFn commit_generator_delay;
    CSB_V1_CSBWinDSAGetMonsterInfoFn get_monster_info;
    CSB_V1_CSBWinDSASetMonsterInfoFn set_monster_info;
    int monster_invisible_enabled;
    int monster_size4_enabled;
    CSB_V1_CSBWinDSAGetCellInfoFn get_cell_info;
    CSB_V1_CSBWinDSAResolveCellStoreFn resolve_cell_store;
    CSB_V1_CSBWinDSASetCellInfoFn set_cell_info;
    CSB_V1_CSBWinDSACopyTeleporterFn copy_teleporter;
    CSB_V1_CSBWinDSAGetObjectPropertyFn get_object_property;
    CSB_V1_CSBWinDSASetObjectPropertyFn set_object_property;
    CSB_V1_CSBWinDSANormalizeObjectPropertyFn normalize_object_property;
    CSB_V1_CSBWinDSAGetActuatorPayloadFn get_actuator_payload;
    CSB_V1_CSBWinDSASetActuatorPayloadFn set_actuator_payload;
    CSB_V1_CSBWinDSACopyActuatorPayloadFn copy_actuator_payload;
    CSB_V1_CSBWinDSAGetChampionPossessionFn get_champion_possession;
    CSB_V1_CSBWinDSAGetMonsterPossessionFn get_monster_possession;
    CSB_V1_CSBWinDSAInspectCellsFn inspect_cells;
    CSB_V1_CSBWinDSAGetThingTypeFn get_thing_type;
    CSB_V1_CSBWinDSAFetchObjectFn fetch_object;
    CSB_V1_CSBWinDSAIsCarriedFn is_carried;
    CSB_V1_CSBWinDSAGetLevelMultiplierFn get_level_multiplier;
    CSB_V1_CSBWinDSAGetMissileInfoFn get_missile_info;
    CSB_V1_CSBWinDSASetMissileInfoFn set_missile_info;
    CSB_V1_CSBWinDSACommitMissileInfoFn commit_missile_info;
    CSB_V1_CSBWinDSAGetMasteryFn get_mastery;
    CSB_V1_CSBWinDSAGetPartyInfoFn get_party_info;
    CSB_V1_CSBWinDSAGetCharacterInfoFn get_character_info;
    CSB_V1_CSBWinDSAPrepareCharacterStoreFn prepare_character_store;
    CSB_V1_CSBWinDSASetCharacterInfoFn set_character_info;
    CSB_V1_CSBWinDSAPrepareExperiencePlusFn prepare_experience_plus;
    CSB_V1_CSBWinDSAAddExperiencePlusFn add_experience_plus;
    CSB_V1_CSBWinDSAPrepareCharacterSwapFn prepare_character_swap;
    CSB_V1_CSBWinDSACommitCharacterSwapFn commit_character_swap;
    CSB_V1_CSBWinDSAPrepareCausePoisonFn prepare_cause_poison;
    CSB_V1_CSBWinDSACommitCausePoisonFn commit_cause_poison;
    CSB_V1_CSBWinDSADiscardTextFn discard_text;
    CSB_V1_CSBWinDSAPlaySoundFn play_sound;
    void *dungeon_user;
} CSB_V1_CSBWinDSAStackContext;

typedef struct {
    CSB_V1_DSAScript scripts[CSB_V1_MAX_DSA_SCRIPTS];
    int script_count;
    int flags[256]; /* global DSA flags */
    int dispatch_count;
    CSB_V1_DSADispatchRecord last_dispatch;
    /* A loaded DSA block owns decoded little-endian words here.  Script
     * pointers remain valid after the caller releases the input buffer. */
    uint16_t *loaded_bytecode;
    int loaded_bytecode_words;
    uint32_t loaded_bytecode_magic;
    CSB_V1_CSBWinDSAImportedHeader
        imported_headers[CSB_V1_MAX_DSA_SCRIPTS];
    CSB_V1_DSAImportedAction *imported_actions;
    int imported_action_count;
} CSB_V1_ChaosMagicState;

void csb_v1_chaos_init(CSB_V1_ChaosMagicState *state);
void csb_v1_chaos_cleanup(CSB_V1_ChaosMagicState *state);
int csb_v1_chaos_load_scripts(CSB_V1_ChaosMagicState *state,
    const uint8_t *data, int data_size);
int csb_v1_chaos_trigger(CSB_V1_ChaosMagicState *state, int script_id);
int csb_v1_chaos_tick(CSB_V1_ChaosMagicState *state);
int csb_v1_dsa_execute_step(CSB_V1_DSAScript *script,
    CSB_V1_ChaosMagicState *state);

/* Imports only checksum-verified CSBWin Extended Features DSA records.
 * The imported program words remain opaque until a source-faithful CSBWin
 * ProcessDSAFilter interpreter consumes them.  The operation is transactional:
 * malformed, encrypted, truncated, or checksum-invalid data leaves state
 * unchanged.  Source: SaveGame.cpp ReadDSAs; DSA.cpp DSA::Read/DSAAction::Read. */
int csb_v1_chaos_import_extended_save_dsas(CSB_V1_ChaosMagicState *state,
    const uint8_t *bytes, int size);

const CSB_V1_DSAImportedAction *csb_v1_chaos_find_imported_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal);

/* CSBWin DSAState::Program searches its action table in file order and uses
 * the first action whose column equals the requested message column. */
const CSB_V1_DSAImportedAction *csb_v1_chaos_find_imported_action_column(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column);

/* Resolve the exact ProcessDSATimer6 state/column selected by a type-47
 * master actuator.  CSBWin DSA.cpp:534-575, 5363-5416 uses LocalState 0 and
 * DB3::DSAstate for these filters; LocalState 1/2 and slave DSAs need their
 * distinct original ownership paths and therefore reject here. */
const CSB_V1_DSAImportedAction *
csb_v1_chaos_resolve_imported_master_filter_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint16_t actuator_word2,
    uint32_t input_column, uint32_t *out_state_index, int *out_action_ordinal);

typedef enum {
    CSB_V1_CSBWIN_DSA_JUMP_OK = 0,
    CSB_V1_CSBWIN_DSA_JUMP_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_JUMP_NOT_FOUND = 2,
    CSB_V1_CSBWIN_DSA_JUMP_NOT_JUMP = 3,
    CSB_V1_CSBWIN_DSA_JUMP_MALFORMED = -1
} CSB_V1_CSBWinDSAJumpResult;

/* Source dispatch-only boundary for DSACMD_JUMP. It owns neither a filter
 * activation nor any dungeon mutation: callers receive a value-copy of the
 * selected state/column transfer and decide when execution is source-complete. */
typedef struct {
    uint32_t source_state;
    uint32_t source_column;
    int continuation_state;
    uint32_t target_state;
    uint32_t target_column;
    uint16_t words_consumed;
} CSB_V1_CSBWinDSAJumpDispatch;

CSB_V1_CSBWinDSAJumpResult
csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, CSB_V1_CSBWinDSAJumpDispatch *out_dispatch);

typedef enum {
    CSB_V1_CSBWIN_DSA_GOSUB_OK = 0,
    CSB_V1_CSBWIN_DSA_GOSUB_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_GOSUB_NOT_FOUND = 2,
    CSB_V1_CSBWIN_DSA_GOSUB_NOT_GOSUB = 3,
    CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED = -1
} CSB_V1_CSBWinDSAGosubResult;

/* Source dispatch-only boundary for DSACMD_GOSUB. CSBWin records the outer
 * continuation, then calls Execute() at one greater subroutine depth. This
 * resolver reports those selected values but neither enters the nested frame
 * nor activates a filter or mutates the dungeon. */
typedef struct {
    uint32_t source_state;
    uint32_t source_column;
    int continuation_state;
    uint32_t target_state;
    uint32_t target_column;
    uint8_t subroutine_depth_delta;
    uint16_t words_consumed;
} CSB_V1_CSBWinDSAGosubDispatch;

CSB_V1_CSBWinDSAGosubResult
csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, CSB_V1_CSBWinDSAGosubDispatch *out_dispatch);

/* Bounded, receipt-only CSBWin Execute() transfer subset. This admits only
 * complete authenticated JUMP and GOSUB action programs. JUMP changes the
 * current state/column in its existing Execute frame; GOSUB pushes the source
 * continuation and enters its selected target one subroutine level deeper.
 * Missing state/column programs terminate a frame just as DSAState::Program
 * returning NULL does in CSBWin. Filter, world, stack, and other opcode paths
 * remain outside this boundary. */
#define CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH 64
#define CSB_V1_CSBWIN_DSA_EXECUTE_MAX_TRANSFERS 256

typedef enum {
    CSB_V1_CSBWIN_DSA_EXECUTE_OK = 0,
    CSB_V1_CSBWIN_DSA_EXECUTE_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_EXECUTE_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_EXECUTE_DEPTH_LIMIT = -2,
    CSB_V1_CSBWIN_DSA_EXECUTE_TRANSFER_LIMIT = -3
} CSB_V1_CSBWinDSAExecuteResult;

typedef struct {
    uint32_t source_state;
    uint32_t source_column;
    int initial_subroutine_depth;
    int final_state;
    uint16_t transfer_count;
    uint16_t return_count;
    uint16_t frame_push_count;
    uint16_t frame_pop_count;
    uint16_t words_consumed;
    uint8_t maximum_subroutine_depth;
    int returned_by_missing_program;
} CSB_V1_CSBWinDSAExecuteReceipt;

typedef struct {
    int next_state;
    int forced_state;
    int transfer_executed;
    uint16_t words_consumed;
    uint16_t command_count;
    uint16_t stack_depth;
    CSB_V1_CSBWinDSAExecuteReceipt transfer;
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
    uint16_t experience_plus_count;
    int32_t last_experience_character_selector;
    int32_t last_experience_skill_number;
    int32_t last_experience_amount;
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
    uint16_t excell_store_count;
    uint32_t last_excell_store_location;
    uint32_t last_excell_store_before[8];
    uint32_t last_excell_store_after[8];
    uint16_t generator_delay_store_count;
    uint32_t last_generator_delay_location;
    int32_t last_generator_delay_before;
    int32_t last_generator_delay_after;
    int last_generator_delay_has_generator;
    /* DSA.cpp STKOP_CausePoison commits through the runtime-owned
     * PoisonCharacter/DamageCharacter candidate after full bytecode
     * acceptance.  The runner records only source operands here; the live
     * runtime receipt binds them to CHARDESC and TT_75 state. */
    uint16_t cause_poison_count;
    int32_t last_cause_poison_character_selector;
    int32_t last_cause_poison_attack;
    uint16_t override_p_count;
    uint32_t last_override_position;
    /* DSA.cpp STKOP_JumpGear/GosubGear use stack-selected state/column
     * transfers. The receipt records the exact imported target rather than
     * inventing a loop target from the host runtime. */
    uint16_t dynamic_transfer_count;
    uint32_t last_dynamic_transfer_state;
    uint32_t last_dynamic_transfer_column;
    int last_dynamic_transfer_gosub;
} CSB_V1_CSBWinDSAStackExecution;

CSB_V1_CSBWinDSAExecuteResult
csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, int initial_subroutine_depth,
    CSB_V1_CSBWinDSAExecuteReceipt *out_receipt);

/* Executes precisely one complete CSBWin DSACMD_LOAD action program from an
 * authenticated DSAAction owner. It does not dispatch another opcode, mutate
 * dungeon data, or substitute behavior for source-illegal LOAD_ABS32. */
CSB_V1_CSBWinDSALoadResult csb_v1_csbwin_dsa_execute_load_action(
    const CSB_V1_DSAImportedAction *action,
    const CSB_V1_CSBWinDSALoadContext *context,
    CSB_V1_CSBWinDSALoadExecution *out_execution);

/* Executes only a complete source LOAD -> STORE action. `state` must own the
 * action after checksum-authenticated CSBWin Extended Features import; other
 * DSA command streams remain explicitly unsupported. */
CSB_V1_CSBWinDSALoadStoreResult
csb_v1_csbwin_dsa_execute_authenticated_load_store_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSALoadStoreContext *context,
    CSB_V1_CSBWinDSALoadStoreExecution *out_execution);

/* Executes a complete authenticated action composed solely of CSBWin LOAD,
 * STORE, local VARIABLEFETCH/VARIABLESTORE, global GLOBALFETCH/GLOBALSTORE,
 * conditionals, and verified EX_AMPERSAND/AMPERSAND2 STKOP families. The source
 * creates a fresh 100-cell DSAVARS bank for each ProcessDSATimer6 invocation;
 * this boundary does the same and does not expose it after a successful
 * execution. Global access requires the caller-owned source-sized bank above,
 * and both parameter and global writes commit together only on success.
 * Admitted AMPERSAND2 GETSKIN/SETSKIN work against the runtime-owned,
 * save-backed SKIN_CACHE; SETSKIN callbacks are deferred until the complete
 * action has consumed every source word, while later GETSKIN observes the
 * action-local pending value. Every source word must be consumed.
 * Unsupported DSA words, unowned filter/world paths, malformed extensions,
 * stack faults, and source-illegal LOAD_ABS32 reject without changing the
 * caller's parameter surface. */
CSB_V1_CSBWinDSAStackResult
csb_v1_csbwin_dsa_execute_authenticated_stack_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSAStackContext *context,
    CSB_V1_CSBWinDSAStackExecution *out_execution);

/* Decode-only admission for the production CSBWin DSA interpreter. It accepts
 * only a checksum-imported action from the current DSA catalog, consumes the
 * exact source word grammar, and reports whether the action belongs to the
 * implemented transfer or stack-core subset before any runtime mutation is
 * attempted. This is a verifier, not a fixture runner: unsupported opcodes
 * and unproved subcodes fail closed. */
CSB_V1_CSBWinDSACoreVerifyResult
csb_v1_csbwin_dsa_verify_authenticated_core_program(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSACoreProgramReceipt *out_receipt);

/* Runtime-owned adapter for the already admitted pure ProcessDSAFilter action
 * subset.  It has the same callback shape as the CSB monster-filter boundary,
 * but remains independent of monster code: the caller supplies a source-owned
 * action pointer and this context proves that it is the exact authenticated
 * `(dsa,state,ordinal)` record before execution.  The global bank and receipt
 * belong to the context; unsupported, malformed, or forged actions leave all
 * caller parameters and that bank unchanged.  `flgs_inout` is deliberately
 * not interpreted here: pre-move filters carry seven source parameters, while
 * post-move flag parameters require their separate CSBWin call boundary.
 *
 * Source: CSBWin DSA.cpp ProcessDSAFilter/ProcessDSATimer6 lines 5315-5460,
 * Execute lines 5053-5293, and Monster.cpp lines 1125-1167. */
typedef struct {
    const CSB_V1_ChaosMagicState *programs;
    int dsa_id;
    uint32_t state_index;
    int action_ordinal;
    uint32_t master_location;
    int party_location_valid;
    int party_level;
    int party_x;
    int party_y;
    int party_direction;
    int game_time_valid;
    uint32_t game_time;
    int random_state_valid;
    uint32_t random_state;
    int dsa_slave_thing_valid;
    uint16_t dsa_slave_thing;
    int most_recent_interesting_object_valid;
    uint32_t most_recent_interesting_object;
    int party_champions_valid;
    int party_champion_count;
    int party_leader_index;
    uint32_t party_champion_talents[4];
    uint16_t party_champion_fingerprints[4];
    uint16_t party_champion_wounds[4];
    int party_champion_health[4];
    int saves_disabled_valid;
    int saves_disabled;
    int timer_type_modifiers_valid;
    uint8_t timer_type_modifiers[3];
    int jitter_state_valid;
    int32_t x_graphic_jitter;
    int32_t y_graphic_jitter;
    int32_t x_overlay_jitter;
    int32_t y_overlay_jitter;
    int jitter_changed;
    int monster_move_inhibit_valid;
    uint8_t monster_move_inhibit[4];
    int override_state_valid;
    int override_p;
    uint32_t override_position;
    CSB_V1_CSBWinDSASetOverridePFn set_override_p;
    void *override_user;
    CSB_V1_CSBWinDSASetAdjustSkillsParametersFn set_adjust_skills_parameters;
    CSB_V1_CSBWinDSADescribeFn describe;
    CSB_V1_CSBWinDSAQueueSwitchActionFn queue_switch_action;
    uint32_t global_variables[CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY];
    int global_variable_count;
    CSB_V1_CSBWinDSAGetSkinFn get_skin;
    CSB_V1_CSBWinDSASetSkinFn set_skin;
    void *skin_user;
    CSB_V1_CSBWinDSAGetWingTalentsFn get_wing_talents;
    CSB_V1_CSBWinDSAHasWingCharacterFn has_wing_character;
    CSB_V1_CSBWinDSASetWingTalentsFn set_wing_talents;
    CSB_V1_CSBWinDSAGetInfoFn get_dsa_info;
    void *wing_user;
    CSB_V1_CSBWinDSAGetExCellFlagsFn get_excell_flags;
    CSB_V1_CSBWinDSASetExCellFlagsFn set_excell_flags;
    void *excell_user;
    CSB_V1_CSBWinDSAGetGeneratorDelayFn get_generator_delay;
    CSB_V1_CSBWinDSASetGeneratorDelayFn set_generator_delay;
    CSB_V1_CSBWinDSACommitGeneratorDelayFn commit_generator_delay;
    CSB_V1_CSBWinDSAGetMonsterInfoFn get_monster_info;
    CSB_V1_CSBWinDSASetMonsterInfoFn set_monster_info;
    int monster_invisible_enabled;
    int monster_size4_enabled;
    CSB_V1_CSBWinDSAGetCellInfoFn get_cell_info;
    CSB_V1_CSBWinDSAResolveCellStoreFn resolve_cell_store;
    CSB_V1_CSBWinDSASetCellInfoFn set_cell_info;
    CSB_V1_CSBWinDSACopyTeleporterFn copy_teleporter;
    CSB_V1_CSBWinDSAGetObjectPropertyFn get_object_property;
    CSB_V1_CSBWinDSASetObjectPropertyFn set_object_property;
    CSB_V1_CSBWinDSANormalizeObjectPropertyFn normalize_object_property;
    CSB_V1_CSBWinDSAGetActuatorPayloadFn get_actuator_payload;
    CSB_V1_CSBWinDSASetActuatorPayloadFn set_actuator_payload;
    CSB_V1_CSBWinDSACopyActuatorPayloadFn copy_actuator_payload;
    CSB_V1_CSBWinDSAGetChampionPossessionFn get_champion_possession;
    CSB_V1_CSBWinDSAGetMonsterPossessionFn get_monster_possession;
    CSB_V1_CSBWinDSAInspectCellsFn inspect_cells;
    CSB_V1_CSBWinDSAGetThingTypeFn get_thing_type;
    CSB_V1_CSBWinDSAFetchObjectFn fetch_object;
    CSB_V1_CSBWinDSAIsCarriedFn is_carried;
    CSB_V1_CSBWinDSAGetLevelMultiplierFn get_level_multiplier;
    CSB_V1_CSBWinDSAGetMissileInfoFn get_missile_info;
    CSB_V1_CSBWinDSASetMissileInfoFn set_missile_info;
    CSB_V1_CSBWinDSACommitMissileInfoFn commit_missile_info;
    CSB_V1_CSBWinDSAGetMasteryFn get_mastery;
    CSB_V1_CSBWinDSAGetPartyInfoFn get_party_info;
    CSB_V1_CSBWinDSAGetCharacterInfoFn get_character_info;
    CSB_V1_CSBWinDSAPrepareCharacterStoreFn prepare_character_store;
    CSB_V1_CSBWinDSASetCharacterInfoFn set_character_info;
    CSB_V1_CSBWinDSAPrepareExperiencePlusFn prepare_experience_plus;
    CSB_V1_CSBWinDSAAddExperiencePlusFn add_experience_plus;
    CSB_V1_CSBWinDSAPrepareCharacterSwapFn prepare_character_swap;
    CSB_V1_CSBWinDSACommitCharacterSwapFn commit_character_swap;
    CSB_V1_CSBWinDSAPrepareCausePoisonFn prepare_cause_poison;
    CSB_V1_CSBWinDSACommitCausePoisonFn commit_cause_poison;
    CSB_V1_CSBWinDSADiscardTextFn discard_text;
    CSB_V1_CSBWinDSAPlaySoundFn play_sound;
    void *dungeon_user;
    CSB_V1_CSBWinDSAStackExecution last_execution;
    CSB_V1_CSBWinDSAExecuteReceipt last_transfer;
    int execution_count;
    int transfer_execution_count;
} CSB_V1_CSBWinDSAFilterStackRunnerContext;

int csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
    const CSB_V1_DSAImportedAction *action, int *parameters,
    int parameter_count, int flgs_inout[2], void *user);
const char *csb_v1_chaos_source_evidence(void);



#endif
