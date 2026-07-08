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

typedef struct {
    const uint8_t *hucard_rom;
    size_t hucard_rom_size;
    const char *md5_hex;
    const char *const *roster_names;
    int roster_name_count;
} Theron_V1StartupRuntimeEntryRequest;

typedef struct {
    Theron_StartupResult result;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
} Theron_V1StartupRuntimeEntryResult;

typedef struct {
    Theron_StartupInputResult input_result;
    const char *status_scope;
    const char *status;
    const char *inspect_scope;
    char inspect_detail[320];
    const char *log_first_line;
    int log_receipt;
} Theron_V1StartupRuntimeEntryApplyReceipt;

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

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_STARTUP_RUNTIME_ENTRY_H */
