#ifndef THERON_V1_STARTUP_RUNTIME_ENTRY_H
#define THERON_V1_STARTUP_RUNTIME_ENTRY_H

#include "theron_v1_dungeon_progression.h"
#include "theron_v1_startup_flow.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_world.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int theron_v1_startup_runtime_load_initial_level(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    char *receipt,
    size_t receipt_cap);

/* Production runtime entry.  Unlike the legacy fixture-compatible helper
 * above, this route refuses to synthesize a first room when Track 02 has not
 * been hash-verified and semantically handed off. */
int theron_v1_startup_runtime_load_initial_level_verified_only(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    char *receipt,
    size_t receipt_cap);
typedef struct {
    const uint8_t *hucard_rom;
    size_t hucard_rom_size;
    const char *md5_hex;
    const char *const *roster_names;
    int roster_name_count;
} Theron_V1StartupRuntimeEntryRequest;

/* Runtime proof for the opaque original `$e009` payload retained by boot.
 * It carries no inferred dungeon, object, bitmap, palette, or transition
 * semantics. */
typedef struct {
    int consumed;
    int no_fallback;
    uint32_t record;
    uint32_t destination;
    uint32_t payload_bytes;
    uint32_t payload_checksum;
    uint64_t raw_track02_offset;
    const char *status;
} Theron_V1StartupRuntimeInitialPayloadReceipt;

int theron_v1_startup_runtime_consume_boot_profile_initial_payload(
    const void *boot_profile,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    Theron_V1StartupRuntimeInitialPayloadReceipt *out_receipt);

/* Receives the one source-locked AKUTUBA level-0 route retained in a
 * completed boot handoff. It re-derives the route from the supplied original
 * Track 02 bytes before publishing it to a candidate world. Object-tail,
 * bitmap, palette, and broader transition semantics stay unavailable. */
typedef struct {
    int received;
    int no_fallback;
    int loader_record_received;
    uint32_t loader_record;
    uint64_t loader_record_raw_user_data_offset;
    uint32_t loader_record_payload_checksum;
    uint32_t level_envelope_offset;
    uint32_t level_envelope_bytes;
    uint32_t level_envelope_checksum;
    int dungeon_id;
    int sub_level_index;
    uint32_t route_hash;
    uint32_t payload_checksum;
    uint32_t envelope_checksum;
    const char *status;
} Theron_V1StartupRuntimeInitialRouteReceipt;

int theron_v1_startup_runtime_receive_boot_profile_initial_route(
    const void *boot_profile,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    Theron_DungeonID dungeon_id,
    Theron_V1StartupRuntimeInitialRouteReceipt *out_receipt);

typedef struct {
    Theron_StartupResult result;
    int level_loaded;
    int runtime_level_source;
    int track02_semantic_handoff;
    int track02_media_route;
    Theron_StartupMediaStateReceipt track02_media;
    int fallback_visuals_blocked;
    int structured_runtime_route;
    int runtime_receipt_text_route;
    Theron_RuntimeLevelBankSelection track02_level_bank;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    int object_table_anchor_binding_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_anchor_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint64_t startup_level_anchor_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint64_t startup_level_anchor_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_width[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_height[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t startup_level_anchor_seed[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_level_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
} Theron_V1StartupRuntimeEntryResult;

typedef struct {
    int valid;
    int real_data_capture_ready;
    int capture_count;
    unsigned int dungeon_mask;
    int semantic_level_count;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int object_capture_count;
    unsigned int object_capture_mask;
    int object_count_total;
    uint32_t object_route_hash;
    unsigned int no_fallback_semantic_role_mask;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    int object_table_anchor_binding_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_anchor_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint64_t startup_level_anchor_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint64_t startup_level_anchor_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_width[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_height[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t startup_level_anchor_seed[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_level_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    int object_counts[THERON_DUNGEON_COUNT];
    uint32_t route_hash;
    Theron_RuntimeLevelBankSelection level_banks[THERON_DUNGEON_COUNT];
} Theron_V1StartupAllDungeonRouteReceipt;

typedef struct {
    Theron_StartupInputResult input_result;
    const char *status_scope;
    const char *status;
    const char *inspect_scope;
    char inspect_detail[320];
    int runtime_level_source;
    int track02_semantic_handoff;
    int track02_media_route;
    unsigned int track02_media_route_mask;
    uint32_t track02_media_checksum;
    uint64_t track02_media_title_first_raw_offset;
    uint64_t track02_media_title_last_raw_offset;
    uint64_t track02_media_title_first_user_data_offset;
    uint64_t track02_media_title_last_user_data_offset;
    uint64_t track02_media_stage_first_raw_offset;
    uint64_t track02_media_stage_last_raw_offset;
    uint64_t track02_media_stage_first_user_data_offset;
    uint64_t track02_media_stage_last_user_data_offset;
    uint64_t track02_media_soul_room_first_raw_offset;
    uint64_t track02_media_soul_room_last_raw_offset;
    uint64_t track02_media_soul_room_first_user_data_offset;
    uint64_t track02_media_soul_room_last_user_data_offset;
    uint64_t track02_media_forcefield_first_raw_offset;
    uint64_t track02_media_forcefield_last_raw_offset;
    uint64_t track02_media_forcefield_first_user_data_offset;
    uint64_t track02_media_forcefield_last_user_data_offset;
    int fallback_visuals_blocked;
    int structured_runtime_route;
    int runtime_receipt_text_route;
    Theron_RuntimeLevelBankSelection track02_level_bank;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    int object_table_anchor_binding_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_anchor_hash[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_status[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint64_t startup_level_anchor_raw_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint64_t startup_level_anchor_user_data_offsets[THERON_TRACK02_MAX_BANK_ANCHORS];
    int startup_level_anchor_user_data_valid[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_width[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_height[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t startup_level_anchor_seed[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint16_t startup_level_anchor_level_index[THERON_TRACK02_MAX_BANK_ANCHORS];
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    const char *log_first_line;
    int log_receipt;
} Theron_V1StartupRuntimeEntryApplyReceipt;

typedef enum {
    THERON_V1_STARTUP_RUNTIME_LEVEL_NONE = 0,
    THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM = 1,
    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC = 2,
    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED = 3,
    THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME = 4,
    /* A later dungeon or Continue/SRM entry has a verified Track 02 bitmap
     * atlas but no broader level-record claim. */
    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_MEDIA = 5
} Theron_V1StartupRuntimeLevelSource;

int theron_v1_startup_runtime_load_initial_level_with_host_receipts(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_runtime_load_initial_level_with_receipts(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
/* Builds bounded Track 02 receipts in a private staging world. It never
 * publishes a level or bypasses the direct runtime Stage 2/3 admission. */
int theron_v1_startup_runtime_inspect_initial_level_with_receipts(
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    Theron_DungeonID dungeon_id,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);

void theron_v1_startup_runtime_entry_request_init(
    Theron_V1StartupRuntimeEntryRequest *request);
void theron_v1_startup_runtime_entry_result_init(
    Theron_V1StartupRuntimeEntryResult *result);
void theron_v1_startup_runtime_entry_apply_receipt_init(
    Theron_V1StartupRuntimeEntryApplyReceipt *receipt);
int theron_v1_startup_host_receipt_from_runtime_entry_apply(
    const Theron_V1StartupRuntimeEntryApplyReceipt *apply_receipt,
    Theron_StartupHostReceipt *out_receipt);
int theron_v1_startup_runtime_enter_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const Theron_V1StartupRuntimeEntryRequest *request,
    Theron_V1StartupRuntimeEntryResult *out_result,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_runtime_entry_apply_receipt(
    const Theron_StartupActionPlan *plan,
    const Theron_V1StartupRuntimeEntryResult *result,
    const char *runtime_receipt,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_receipt);
int theron_v1_startup_runtime_entry_state_receipt_from_result(
    const Theron_StartupFlow *flow,
    const Theron_V1StartupRuntimeEntryResult *result,
    Theron_StartupStateReceipt *out_receipt);
int theron_v1_startup_runtime_enter_from_forcefield_with_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const Theron_V1StartupRuntimeEntryRequest *request,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_runtime_enter_from_forcefield_facts_with_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    int startup_roster_name_count,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_V1StartupRuntimeEntryApplyReceipt *out_apply_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_runtime_enter_from_forcefield_facts_with_host_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    int startup_roster_name_count,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
int theron_v1_startup_runtime_enter_from_forcefield_boot_profile_with_host_receipts(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const void *boot_profile,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    int startup_roster_name_count,
    const Theron_StartupActionPlan *plan,
    Theron_V1StartupRuntimeEntryResult *out_result,
    Theron_StartupHostReceipt *out_host_receipt,
    Theron_StartupStateReceipt *out_state_receipt,
    char *receipt,
    size_t receipt_cap);
void theron_v1_startup_all_dungeon_route_receipt_init(
    Theron_V1StartupAllDungeonRouteReceipt *receipt);
int theron_v1_startup_runtime_capture_all_dungeon_routes(
    const uint8_t *hucard_rom,
    size_t hucard_rom_size,
    const char *md5_hex,
    const Theron_StartupMediaStateReceipt *media_receipt,
    Theron_V1StartupAllDungeonRouteReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_RUNTIME_ENTRY_H */
