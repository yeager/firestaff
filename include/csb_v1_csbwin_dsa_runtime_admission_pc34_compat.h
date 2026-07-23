#ifndef FIRESTAFF_CSB_V1_CSBWIN_DSA_RUNTIME_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CSBWIN_DSA_RUNTIME_ADMISSION_PC34_COMPAT_H

#include "csb_v1_boot.h"

typedef struct CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 {
    int valid;
    int immutable_source_bound;
    int save_identity_bound;
    int dungeon_identity_bound;
    int dsa_span_bound;
    int gameblock_span_bound;
    size_t save_size;
    uint32_t save_fnv1a;
    size_t dsa_offset;
    size_t dsa_size;
    uint32_t dsa_stored_checksum;
    uint32_t dsa_computed_checksum;
    size_t gameblock_offset;
    size_t gameblock_size;
    uint32_t gameblock_fnv1a;
    char dungeon_md5[33];
    uint32_t admission_hash;
    const char *source_evidence;
} CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34;

typedef enum CSB_V1_CSBWinSaveCorpusSourceKind_PC34 {
    CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_DIRECT_PC34 = 1,
    CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_VIRTUAL_CONTAINER_PC34 = 2
} CSB_V1_CSBWinSaveCorpusSourceKind_PC34;

/* A candidate retains source evidence only. For a container row `save_bytes`
 * is an already bounded virtual member view; this API never extracts or
 * materializes it to disk. */
typedef struct CSB_V1_CSBWinSaveCorpusCandidate_PC34 {
    CSB_V1_CSBWinSaveCorpusSourceKind_PC34 source_kind;
    const char *source_path;
    const uint8_t *save_bytes;
    size_t save_size;
    uint32_t declared_save_fnv1a;
    const char *declared_save_md5;
    const char *declared_dungeon_md5;
} CSB_V1_CSBWinSaveCorpusCandidate_PC34;

typedef struct CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 {
    int valid;
    int direct_file;
    int virtual_container;
    int no_write_or_extract;
    uint32_t save_fnv1a;
    uint32_t gameblock_fnv1a;
    uint32_t candidate_hash;
    char source_path[512];
    char save_md5[33];
    char dungeon_md5[33];
    CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 dsa_admission;
} CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34;

/* Local discovery is intentionally view-only. Direct files and supported
 * container readers supply bounded byte views; discovery neither owns,
 * extracts, nor writes those payloads. Exactly one cohesive candidate may
 * be selected for later runtime handoff. */
typedef struct CSB_V1_CSBWinSaveCorpusDiscoveryInput_PC34 {
    CSB_V1_CSBWinSaveCorpusCandidate_PC34 candidate;
    const CSB_V1_CSBWinDSASaveCorpusReceipt *corpus;
} CSB_V1_CSBWinSaveCorpusDiscoveryInput_PC34;

typedef struct CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34 {
    int valid;
    int skipped_no_candidate;
    int rejected_mixed_candidates;
    int selected_direct_file;
    int selected_virtual_container;
    size_t inspected_count;
    size_t admitted_count;
    CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 candidate;
} CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34;

/* The immutable byte admission above deliberately ends before live state is
 * considered.  This receipt is the separate publication boundary: it binds
 * that source proof to one already-loaded CSBWin runtime chain and one
 * source-owned startup session.  It does not expose DSA program data or
 * execute a DSA action. */
typedef struct CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 {
    int valid;
    int source_admission_consumed;
    int save_handoff_consumed;
    int dungeon_identity_current;
    int startup_session_consumed;
    int runtime_chain_consumed;
    uint32_t save_fnv1a;
    uint32_t gameblock_fnv1a;
    uint32_t source_admission_hash;
    uint32_t startup_session_generation;
    uint32_t startup_source_tick;
    uint32_t runtime_game_time;
    uint16_t live_timer_event_count;
    int imported_action_count;
    char dungeon_md5[33];
    uint32_t handoff_hash;
    const char *source_evidence;
} CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34;

/* This is a consumption receipt, not a second DSA interpreter. It names one
 * saved TimerQueue entry and the exact imported action selected by the
 * existing CSBWin runtime before that runtime is allowed to execute it. */
typedef struct CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 {
    int valid;
    int handoff_consumed;
    int session_current;
    int save_identity_current;
    int tick_order_current;
    int timer_record_consumed;
    int local_state_consumed;
    int opcode_body_admitted;
    int action_executed;
    int runtime_world_mutation_committed;
    int runtime_dsa_state_committed;
    int runtime_teleporter_copy_committed;
    uint32_t save_fnv1a;
    uint32_t startup_session_generation;
    uint32_t startup_source_tick;
    uint32_t runtime_game_time;
    uint16_t queue_slot;
    uint16_t timer_index;
    uint8_t timer_function;
    uint8_t timer_position;
    uint8_t timer_action;
    int timer_level;
    int timer_x;
    int timer_y;
    int timer_location_position;
    uint16_t timer_actuator_thing;
    uint8_t dsa_id;
    uint32_t state_index;
    uint32_t input_column;
    int action_ordinal;
    uint16_t dynamic_transfer_count;
    uint32_t dynamic_transfer_state;
    uint32_t dynamic_transfer_column;
    int dynamic_transfer_gosub;
    int dynamic_transfer_final_state;
    uint32_t timer_owner_hash;
    uint32_t bridge_hash;
    const char *source_evidence;
} CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34;

int csb_v1_csbwin_dsa_runtime_admission_from_corpus_pc34(
    const CSB_V1_BootProfile *profile,
    const uint8_t *save_bytes,
    size_t save_size,
    const CSB_V1_CSBWinDSASaveCorpusReceipt *corpus,
    CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 *out_receipt);
int csb_v1_csbwin_save_corpus_candidate_admission_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinSaveCorpusCandidate_PC34 *candidate,
    const CSB_V1_CSBWinDSASaveCorpusReceipt *corpus,
    CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 *out_receipt);
int csb_v1_csbwin_save_corpus_discover_local_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinSaveCorpusDiscoveryInput_PC34 *inputs,
    size_t input_count,
    CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34 *out_receipt);

/* Publish a DSA/save runtime receipt only when all four existing owners
 * agree: checksum-bound save bytes, the current verified Dungeon.dat,
 * a complete source-owned startup session, and the imported runtime DSA
 * catalog/index/timer chain.  Callers must treat a zero receipt as no DSA
 * runtime handoff. */
int csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 *source_admission,
    const CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 *save_handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 *runtime_chain,
    CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *out_receipt);

/* Consume one due, source-restored TimerQueue entry through the existing
 * CSBWin timer action runner. Save/session/tick drift, a non-source timer,
 * unowned LocalState, and unsupported opcode bodies all reject before the
 * runner is called. */
int csb_v1_csbwin_dsa_execute_restored_timer_pc34(
    CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_DSAFilterLocation *location,
    uint16_t queue_slot,
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *out_receipt);

/* Revalidate a published outcome without executing it. M11 uses this before
 * it commits a presentation transaction. */
int csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *receipt);

/* The same current admission boundary for a resolved Monster.cpp movement
 * filter. It only installs the existing runtime callback; it does not create
 * a filter, map a selector, or interpret an additional opcode body. */
int csb_v1_csbwin_dsa_bind_restored_movement_filter_pc34(
    CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_RuntimeDSAFilterBinding *binding,
    uint32_t state_index,
    int action_ordinal,
    uint32_t master_location,
    int loaded_level,
    CSB_V1_DSAFilterRuntime *out_filter,
    CSB_V1_RuntimeDSAFilterStackAdapter *out_adapter,
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *out_receipt);

#endif
