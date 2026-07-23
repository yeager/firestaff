#include "csb_v1_csbwin_dsa_runtime_admission_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) { hash = (hash ^ bytes[i]) * 16777619u; }
    return hash ? hash : 1u;
}

static uint32_t hash_step(uint32_t hash, uint32_t value)
{
    hash = (hash ^ value) * 16777619u;
    return hash ? hash : 1u;
}

static int csb_v1_csbwin_dsa_handoff_current(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 *out_chain)
{
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain;

    if (!profile || !handoff || !startup_session || !out_chain ||
        !handoff->valid || !handoff->source_admission_consumed ||
        !handoff->save_handoff_consumed || !handoff->dungeon_identity_current ||
        !handoff->startup_session_consumed || !handoff->runtime_chain_consumed ||
        handoff->save_fnv1a == 0u || handoff->gameblock_fnv1a == 0u ||
        handoff->source_admission_hash == 0u || handoff->handoff_hash == 0u ||
        !profile->assets_verified || !profile->dungeon_verified ||
        !profile->runtime.dungeon_handle ||
        strcmp(profile->dungeon_md5, handoff->dungeon_md5) != 0 ||
        !startup_session->valid || !startup_session->real_asset_matched ||
        !startup_session->full_startup_ready ||
        !startup_session->rejects_legacy_wrappers ||
        startup_session->generation != handoff->startup_session_generation ||
        startup_session->source_tick != handoff->startup_source_tick ||
        profile->runtime.game_time != handoff->runtime_game_time ||
        !csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
            &profile->runtime, &chain) ||
        !chain.valid || chain.imported_action_count != handoff->imported_action_count ||
        chain.live_timer_event_count != handoff->live_timer_event_count) {
        return 0;
    }
    *out_chain = chain;
    return 1;
}

static int csb_v1_csbwin_dsa_action_ordinal(
    const CSB_V1_ChaosMagicState *state,
    const CSB_V1_DSAImportedAction *action)
{
    int i;
    if (!state || !action || !state->imported_actions) return -1;
    for (i = 0; i < state->imported_action_count; ++i) {
        if (&state->imported_actions[i] == action) return i;
    }
    return -1;
}

/* Timer.cpp owns the function/action/position fields as one serialized TIMER
 * record. Keep that record separate from the executable DSA body: the body
 * verifier may accept an action only after this source owner still agrees. */
static uint32_t csb_v1_csbwin_dsa_timer_owner_hash(
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_CSBWin512TimerSummary *timer,
    const CSB_V1_DSAFilterLocation *location,
    uint16_t queue_slot, const CSB_V1_DSAImportedAction *action,
    int action_ordinal)
{
    uint32_t hash;

    if (!handoff || !timer || !location || !action ||
        !action->program_words || action->program_word_count <= 0 ||
        handoff->handoff_hash == 0u) return 0u;
    hash = handoff->handoff_hash;
    hash = hash_step(hash, queue_slot);
    hash = hash_step(hash, timer->source_index);
    hash = hash_step(hash, timer->function);
    hash = hash_step(hash, timer->ubyte8);
    hash = hash_step(hash, timer->ubyte9);
    hash = hash_step(hash, timer->level);
    hash = hash_step(hash, timer->ubyte6);
    hash = hash_step(hash, timer->ubyte7);
    hash = hash_step(hash, (uint32_t)location->position);
    hash = hash_step(hash, location->actuator_thing);
    hash = hash_step(hash, action->dsa_id);
    hash = hash_step(hash, action->state_index);
    hash = hash_step(hash, action->column);
    hash = hash_step(hash, (uint32_t)action_ordinal);
    hash = hash_step(hash, (uint32_t)action->program_word_count);
    hash = hash_step(hash, fnv1a32((const uint8_t *)action->program_words,
                                   (size_t)action->program_word_count *
                                       sizeof(*action->program_words)));
    return hash;
}

static int csb_v1_csbwin_dsa_prepare_saved_timer(
    const CSB_V1_RuntimeProfile *runtime,
    const CSB_V1_DSAFilterLocation *location,
    uint16_t queue_slot, const CSB_V1_CSBWin512TimerSummary **out_timer,
    const CSB_V1_DSAImportedAction **out_action, int *out_ordinal)
{
    const CSB_V1_CSBWin512TimerSummary *timer;
    CSB_V1_CSBWinDSAFilterStackRunnerContext runner;
    const CSB_V1_DSAImportedAction *action = NULL;
    uint16_t timer_index;
    int prepared;

    if (!runtime || !location || !out_timer || !out_action || !out_ordinal ||
        queue_slot >= runtime->csbwin_timer_queue_summary_count) return 0;
    timer_index = runtime->csbwin_timer_queue[queue_slot];
    if (timer_index >= runtime->csbwin_timer_summary_count) return 0;
    timer = &runtime->csbwin_timers[timer_index];
    if (!timer->valid || timer->truncated || timer->source_index != timer_index ||
        timer->time != runtime->game_time ||
        timer->level != (uint8_t)location->level ||
        timer->ubyte6 != (uint8_t)location->x ||
        timer->ubyte7 != (uint8_t)location->y) return 0;
    memset(&runner, 0, sizeof(runner));
    switch (timer->function) {
    case 5u: prepared = csb_v1_runtime_prepare_csbwin_openroom_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    case 6u: prepared = csb_v1_runtime_prepare_csbwin_stoneroom_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    case 7u: prepared = csb_v1_runtime_prepare_csbwin_falsewall_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    case 8u: prepared = csb_v1_runtime_prepare_csbwin_teleporter_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    case 9u: prepared = csb_v1_runtime_prepare_csbwin_pitroom_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    case 10u: prepared = csb_v1_runtime_prepare_csbwin_door_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    case 102u: prepared = csb_v1_runtime_prepare_csbwin_dessage_dsa_timer_stack_runner(runtime, runtime->dungeon_handle, location, timer, &runner, &action); break;
    default: return 0;
    }
    *out_ordinal = csb_v1_csbwin_dsa_action_ordinal(
        &runtime->csbwin_extended_dsa_state, action);
    if (!prepared || !action || *out_ordinal < 0) return 0;
    *out_timer = timer;
    *out_action = action;
    return 1;
}

int csb_v1_csbwin_dsa_runtime_admission_from_corpus_pc34(
    const CSB_V1_BootProfile *profile, const uint8_t *save_bytes,
    size_t save_size, const CSB_V1_CSBWinDSASaveCorpusReceipt *corpus,
    CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 receipt;
    uint32_t hash = 2166136261u;
    size_t gameblock_size;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!profile || !save_bytes || save_size == 0u || !corpus ||
        !corpus->valid || !corpus->corpus_positive ||
        !corpus->runtime_handoff_ready || !corpus->extended_tail_valid ||
        !corpus->dsa_section_valid || !corpus->dsa_has_runtime_actions ||
        !corpus->gameblock1_valid || !corpus->gameblock1_body_valid ||
        !profile->assets_verified || !profile->dungeon_verified ||
        !profile->dungeon_path[0] || strlen(profile->dungeon_md5) != 32u ||
        corpus->save_bytes_fnv1a == 0u || corpus->gameblock1_body_fnv1a == 0u ||
        corpus->dsa.dsa_payload_size == 0u ||
        corpus->dsa.stored_checksum != corpus->dsa.computed_checksum ||
        corpus->dsa.next_payload_offset < corpus->dsa.dsa_payload_offset ||
        corpus->gameblock1_offset >= save_size) return 0;

    gameblock_size = save_size - corpus->gameblock1_offset;
    if (corpus->dsa.next_payload_offset > save_size ||
        corpus->dsa.dsa_payload_offset > corpus->dsa.next_payload_offset ||
        corpus->dsa.dsa_payload_size >
            corpus->dsa.next_payload_offset - corpus->dsa.dsa_payload_offset ||
        fnv1a32(save_bytes, save_size) != corpus->save_bytes_fnv1a ||
        fnv1a32(save_bytes + corpus->gameblock1_offset, gameblock_size) !=
            corpus->gameblock1_body_fnv1a) return 0;

    receipt.save_size = save_size;
    receipt.save_fnv1a = corpus->save_bytes_fnv1a;
    receipt.dsa_offset = corpus->dsa.dsa_payload_offset;
    receipt.dsa_size = corpus->dsa.dsa_payload_size;
    receipt.dsa_stored_checksum = corpus->dsa.stored_checksum;
    receipt.dsa_computed_checksum = corpus->dsa.computed_checksum;
    receipt.gameblock_offset = corpus->gameblock1_offset;
    receipt.gameblock_size = gameblock_size;
    receipt.gameblock_fnv1a = corpus->gameblock1_body_fnv1a;
    memcpy(receipt.dungeon_md5, profile->dungeon_md5, 33u);
    receipt.save_identity_bound = 1;
    receipt.dungeon_identity_bound = 1;
    receipt.dsa_span_bound = 1;
    receipt.gameblock_span_bound = 1;
    receipt.immutable_source_bound = 1;
    hash = hash_step(hash, receipt.save_fnv1a);
    hash = hash_step(hash, (uint32_t)receipt.dsa_offset);
    hash = hash_step(hash, (uint32_t)receipt.dsa_size);
    hash = hash_step(hash, receipt.dsa_stored_checksum);
    hash = hash_step(hash, (uint32_t)receipt.gameblock_offset);
    hash = hash_step(hash, receipt.gameblock_fnv1a);
    hash = hash_step(hash, fnv1a32((const uint8_t *)receipt.dungeon_md5, 32u));
    receipt.admission_hash = hash;
    receipt.source_evidence = "CSBWin SaveGame.cpp ReadExtendedFeatures/ReadDSAs/ReadGameInfo; ReDMCSB LOADSAVE.C F0435";
    receipt.valid = receipt.admission_hash != 0u;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_csbwin_save_corpus_candidate_admission_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinSaveCorpusCandidate_PC34 *candidate,
    const CSB_V1_CSBWinDSASaveCorpusReceipt *corpus,
    CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 receipt;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!profile || !candidate || !corpus || !candidate->source_path ||
        !candidate->source_path[0] || !candidate->save_bytes ||
        candidate->save_size == 0u || candidate->declared_save_fnv1a == 0u ||
        !candidate->declared_save_md5 ||
        !candidate->declared_dungeon_md5 ||
        strlen(candidate->declared_save_md5) != 32u ||
        strlen(candidate->declared_dungeon_md5) != 32u ||
        strcmp(candidate->declared_dungeon_md5, profile->dungeon_md5) != 0 ||
        (candidate->source_kind != CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_DIRECT_PC34 &&
         candidate->source_kind != CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_VIRTUAL_CONTAINER_PC34) ||
        fnv1a32(candidate->save_bytes, candidate->save_size) !=
            candidate->declared_save_fnv1a ||
        !csb_v1_csbwin_dsa_runtime_admission_from_corpus_pc34(
            profile, candidate->save_bytes, candidate->save_size, corpus,
            &receipt.dsa_admission) ||
        receipt.dsa_admission.save_fnv1a != candidate->declared_save_fnv1a) {
        return 0;
    }
    receipt.direct_file = candidate->source_kind ==
        CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_DIRECT_PC34;
    receipt.virtual_container = candidate->source_kind ==
        CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_VIRTUAL_CONTAINER_PC34;
    receipt.no_write_or_extract = 1;
    receipt.save_fnv1a = receipt.dsa_admission.save_fnv1a;
    receipt.gameblock_fnv1a = receipt.dsa_admission.gameblock_fnv1a;
    snprintf(receipt.source_path, sizeof(receipt.source_path), "%s",
             candidate->source_path);
    memcpy(receipt.save_md5, candidate->declared_save_md5, 33u);
    memcpy(receipt.dungeon_md5, candidate->declared_dungeon_md5, 33u);
    hash = hash_step(hash, receipt.save_fnv1a);
    hash = hash_step(hash, receipt.gameblock_fnv1a);
    hash = hash_step(hash, fnv1a32((const uint8_t *)receipt.source_path,
                                   strlen(receipt.source_path)));
    hash = hash_step(hash, fnv1a32((const uint8_t *)receipt.save_md5, 32u));
    hash = hash_step(hash, fnv1a32((const uint8_t *)receipt.dungeon_md5, 32u));
    receipt.candidate_hash = hash;
    receipt.valid = hash != 0u;
    *out_receipt = receipt;
    return receipt.valid;
}

int csb_v1_csbwin_save_corpus_discover_local_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinSaveCorpusDiscoveryInput_PC34 *inputs,
    size_t input_count,
    CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34 receipt;
    CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 admitted;
    size_t i;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!profile || (!inputs && input_count != 0u)) return 0;
    receipt.inspected_count = input_count;
    if (input_count == 0u) {
        receipt.skipped_no_candidate = 1;
        *out_receipt = receipt;
        return 0;
    }
    for (i = 0u; i < input_count; ++i) {
        memset(&admitted, 0, sizeof(admitted));
        if (!csb_v1_csbwin_save_corpus_candidate_admission_pc34(
                profile, &inputs[i].candidate, inputs[i].corpus, &admitted)) {
            continue;
        }
        ++receipt.admitted_count;
        if (receipt.admitted_count == 1u) receipt.candidate = admitted;
    }
    if (receipt.admitted_count != 1u) {
        receipt.rejected_mixed_candidates = receipt.admitted_count > 1u;
        *out_receipt = receipt;
        return 0;
    }
    receipt.selected_direct_file = receipt.candidate.direct_file;
    receipt.selected_virtual_container = receipt.candidate.virtual_container;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 *source_admission,
    const CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 *save_handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 *runtime_chain,
    CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 receipt;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));

    if (!profile || !source_admission || !save_handoff || !startup_session ||
        !runtime_chain || !source_admission->valid ||
        !source_admission->immutable_source_bound ||
        !source_admission->save_identity_bound ||
        !source_admission->dungeon_identity_bound ||
        !source_admission->dsa_span_bound ||
        !source_admission->gameblock_span_bound ||
        source_admission->admission_hash == 0u ||
        !save_handoff->valid || !save_handoff->save_import_receipt_consumed ||
        !save_handoff->runtime_load_consumed ||
        !save_handoff->dsa_corpus_positive ||
        !save_handoff->dsa_runtime_handoff_ready ||
        !save_handoff->extended_tail_valid ||
        !save_handoff->dsa_section_valid ||
        !save_handoff->dsa_has_runtime_actions ||
        !save_handoff->gameblock1_valid ||
        !save_handoff->gameblock1_body_valid ||
        !startup_session->valid || !startup_session->real_asset_matched ||
        !startup_session->full_startup_ready ||
        !startup_session->rejects_legacy_wrappers ||
        startup_session->generation == 0u || startup_session->source_tick == 0u ||
        !runtime_chain->valid || !runtime_chain->dsa_catalog_valid ||
        !runtime_chain->level_index_valid ||
        !runtime_chain->timer_queue_event_chain_valid ||
        runtime_chain->imported_action_count <= 0 ||
        !profile->assets_verified || !profile->dungeon_verified ||
        !profile->runtime.dungeon_handle ||
        strlen(profile->dungeon_md5) != 32u ||
        strcmp(profile->dungeon_md5, source_admission->dungeon_md5) != 0 ||
        save_handoff->save_bytes_fnv1a == 0u ||
        save_handoff->gameblock1_body_fnv1a == 0u ||
        save_handoff->save_bytes_fnv1a != source_admission->save_fnv1a ||
        save_handoff->gameblock1_body_fnv1a !=
            source_admission->gameblock_fnv1a ||
        save_handoff->runtime_game_time_after != profile->runtime.game_time) {
        return 0;
    }

    receipt.source_admission_consumed = 1;
    receipt.save_handoff_consumed = 1;
    receipt.dungeon_identity_current = 1;
    receipt.startup_session_consumed = 1;
    receipt.runtime_chain_consumed = 1;
    receipt.save_fnv1a = source_admission->save_fnv1a;
    receipt.gameblock_fnv1a = source_admission->gameblock_fnv1a;
    receipt.source_admission_hash = source_admission->admission_hash;
    receipt.startup_session_generation = startup_session->generation;
    receipt.startup_source_tick = startup_session->source_tick;
    receipt.runtime_game_time = save_handoff->runtime_game_time_after;
    receipt.live_timer_event_count = runtime_chain->live_timer_event_count;
    receipt.imported_action_count = runtime_chain->imported_action_count;
    memcpy(receipt.dungeon_md5, source_admission->dungeon_md5,
           sizeof(receipt.dungeon_md5));
    hash = hash_step(hash, receipt.save_fnv1a);
    hash = hash_step(hash, receipt.gameblock_fnv1a);
    hash = hash_step(hash, receipt.source_admission_hash);
    hash = hash_step(hash, receipt.startup_session_generation);
    hash = hash_step(hash, receipt.startup_source_tick);
    hash = hash_step(hash, receipt.runtime_game_time);
    hash = hash_step(hash, receipt.live_timer_event_count);
    hash = hash_step(hash, (uint32_t)receipt.imported_action_count);
    hash = hash_step(hash, fnv1a32((const uint8_t *)receipt.dungeon_md5, 32u));
    receipt.handoff_hash = hash;
    receipt.source_evidence =
        "CSBWin SaveGame.cpp ReadExtendedFeatures/ReadDSAs/ReadGameInfo; "
        "CSBWin DSA.cpp runtime catalog; ReDMCSB LOADSAVE.C F0435";
    receipt.valid = receipt.handoff_hash != 0u;
    if (!receipt.valid) return 0;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_csbwin_dsa_execute_restored_timer_pc34(
    CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_DSAFilterLocation *location,
    uint16_t queue_slot,
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 receipt;
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain;
    CSB_V1_CSBWinDSACoreProgramReceipt core;
    const CSB_V1_CSBWin512TimerSummary *timer;
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 execution;
    CSB_V1_RuntimeProfile runtime_before;
    uint8_t *dungeon_before = NULL;
    size_t dungeon_size = 0u;
    uint32_t hash = 2166136261u;
    int action_ordinal;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    memset(&chain, 0, sizeof(chain));
    memset(&core, 0, sizeof(core));
    if (!location || !csb_v1_csbwin_dsa_handoff_current(
            profile, handoff, startup_session, &chain) ||
        !csb_v1_csbwin_dsa_prepare_saved_timer(
            &profile->runtime, location, queue_slot, &timer, &action,
            &action_ordinal) ||
        csb_v1_csbwin_dsa_verify_authenticated_core_program(
            &profile->runtime.csbwin_extended_dsa_state, action->dsa_id,
            action->state_index, action_ordinal, &core) !=
            CSB_V1_CSBWIN_DSA_CORE_OK || !core.valid ||
        !action->program_words || action->program_word_count <= 0 ||
        action->program_word_count > UINT16_MAX) {
        return 0;
    }
    runtime_before = profile->runtime;
    if (profile->runtime.dungeon_handle &&
        profile->runtime.dungeon_handle->raw_data &&
        profile->runtime.dungeon_handle->raw_size > 0) {
        dungeon_size = (size_t)profile->runtime.dungeon_handle->raw_size;
        dungeon_before = (uint8_t *)malloc(dungeon_size);
        if (!dungeon_before) return 0;
        memcpy(dungeon_before, profile->runtime.dungeon_handle->raw_data,
               dungeon_size);
    }
    if (!csb_v1_runtime_execute_csbwin_saved_queued_timer_dsa_stack_action(
            &profile->runtime, profile->runtime.dungeon_handle, location,
            queue_slot)) {
        if (dungeon_before) {
            memcpy(profile->runtime.dungeon_handle->raw_data, dungeon_before,
                   dungeon_size);
        }
        profile->runtime = runtime_before;
        free(dungeon_before);
        return 0;
    }
    if (!csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
            &profile->runtime, &chain) ||
        !chain.saved_timer_dsa_execution_valid ||
        chain.last_queue_slot != queue_slot ||
        chain.last_timer_index != timer->source_index ||
        chain.last_dsa_id != action->dsa_id ||
        chain.last_state_index != action->state_index ||
        chain.last_input_column != action->column ||
        chain.last_action_ordinal != action_ordinal ||
        !csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
            &profile->runtime, &execution) || !execution.valid) {
        if (dungeon_before) {
            memcpy(profile->runtime.dungeon_handle->raw_data, dungeon_before,
                   dungeon_size);
        }
        profile->runtime = runtime_before;
        free(dungeon_before);
        return 0;
    }
    free(dungeon_before);
    receipt.handoff_consumed = 1;
    receipt.session_current = 1;
    receipt.save_identity_current = 1;
    receipt.tick_order_current = 1;
    receipt.timer_record_consumed = 1;
    receipt.local_state_consumed = 1;
    receipt.opcode_body_admitted = 1;
    receipt.action_executed = 1;
    receipt.runtime_world_mutation_committed = execution.dungeon_changed ? 1 : 0;
    receipt.runtime_dsa_state_committed = execution.dsa_state_changed ? 1 : 0;
    receipt.runtime_teleporter_copy_committed =
        execution.teleporter_copy_count > 0 ? 1 : 0;
    receipt.save_fnv1a = handoff->save_fnv1a;
    receipt.startup_session_generation = handoff->startup_session_generation;
    receipt.startup_source_tick = handoff->startup_source_tick;
    receipt.runtime_game_time = profile->runtime.game_time;
    receipt.queue_slot = queue_slot;
    receipt.timer_index = timer->source_index;
    receipt.timer_function = timer->function;
    receipt.timer_position = timer->ubyte8;
    receipt.timer_action = timer->ubyte9;
    receipt.timer_level = location->level;
    receipt.timer_x = location->x;
    receipt.timer_y = location->y;
    receipt.timer_location_position = location->position;
    receipt.timer_actuator_thing = location->actuator_thing;
    receipt.dsa_id = action->dsa_id;
    receipt.state_index = action->state_index;
    receipt.input_column = action->column;
    receipt.action_ordinal = action_ordinal;
    receipt.action_program_word_count = (uint16_t)action->program_word_count;
    receipt.action_program_fnv1a = fnv1a32((const uint8_t *)action->program_words,
                                            (size_t)action->program_word_count *
                                                sizeof(*action->program_words));
    receipt.comparison_core = core.comparison_core;
    receipt.arithmetic_core = core.arithmetic_core;
    receipt.variable_core = core.variable_core;
    receipt.timer_core = core.timer_core;
    receipt.message_core = core.message_core;
    receipt.text_display_core = core.text_display_core;
    receipt.sound_core = core.sound_core;
    receipt.champion_core = core.champion_core;
    receipt.object_core = core.object_core;
    receipt.query_core = core.query_core;
    receipt.forced_state = execution.forced_state;
    receipt.saved_dsa_state_transition_valid =
        execution.saved_dsa_state_transition_valid;
    receipt.saved_dsa_state_before = execution.saved_dsa_state_before;
    receipt.saved_dsa_state_after = execution.saved_dsa_state_after;
    receipt.saved_dsa_state_tail_fnv1a = execution.saved_dsa_state_tail_fnv1a;
    /* SetNewState is only publishable when the source-owned LocalState=2
     * record was committed against the exact loaded CSBWin PC34 tail. */
    if ((receipt.forced_state >= 0 &&
         !receipt.saved_dsa_state_transition_valid) ||
        (receipt.saved_dsa_state_transition_valid &&
         (!profile->runtime.csbwin_appended_tail_valid ||
          receipt.saved_dsa_state_tail_fnv1a == 0u ||
          receipt.saved_dsa_state_tail_fnv1a !=
              profile->runtime.csbwin_appended_tail_fnv1a)) ||
        (!receipt.saved_dsa_state_transition_valid &&
         receipt.saved_dsa_state_tail_fnv1a != 0u)) {
        return 0;
    }
    receipt.dungeon_mutation_core = core.dungeon_mutation_core;
    receipt.runtime_dungeon_changed = execution.dungeon_changed ? 1 : 0;
    if (!profile->runtime.dungeon_handle ||
        !profile->runtime.dungeon_handle->raw_data ||
        profile->runtime.dungeon_handle->raw_size <= 0) {
        return 0;
    }
    receipt.dungeon_raw_fnv1a = fnv1a32(
        profile->runtime.dungeon_handle->raw_data,
        (size_t)profile->runtime.dungeon_handle->raw_size);
    if (receipt.dungeon_mutation_core) {
        receipt.cell_store_count = execution.cell_store_count;
        receipt.last_cell_store_location = execution.last_cell_store_location;
        receipt.last_cell_store_write_mask = execution.last_cell_store_write_mask;
        receipt.false_pit_count = execution.false_pit_count;
        receipt.last_false_pit_location = execution.last_false_pit_location;
        receipt.teleporter_copy_count = execution.teleporter_copy_count;
        receipt.last_teleporter_copy_source_location =
            execution.last_teleporter_copy_source_location;
        receipt.last_teleporter_copy_destination_location =
            execution.last_teleporter_copy_destination_location;
    }
    if ((receipt.false_pit_count != 0u &&
         (receipt.false_pit_count != 1u || receipt.cell_store_count != 1u ||
          receipt.last_false_pit_location != receipt.last_cell_store_location))) {
        return 0;
    }
    receipt.timer_scheduled_count = execution.timer_scheduled_count;
    receipt.last_scheduled_event_type = execution.last_scheduled_event_type;
    receipt.last_scheduled_target_location =
        execution.last_scheduled_target_location;
    receipt.message_scheduled_count = execution.message_scheduled_count;
    receipt.last_message_route = execution.last_message_route;
    receipt.text_discard_count = execution.text_discard_count;
    receipt.sound_notification_count = execution.sound_notification_count;
    receipt.last_sound_number = execution.last_sound_number;
    receipt.last_sound_volume = execution.last_sound_volume;
    receipt.last_sound_flags = execution.last_sound_flags;
    receipt.party_talents_changed = execution.party_talents_changed;
    if (receipt.party_talents_changed) {
        receipt.party_talents_champion_count =
            execution.party_talents_champion_count;
        memcpy(receipt.party_talents_fingerprints,
               execution.party_talents_fingerprints,
               sizeof(receipt.party_talents_fingerprints));
        memcpy(receipt.party_talents_after, execution.party_talents_after,
               sizeof(receipt.party_talents_after));
        if (!receipt.champion_core || receipt.party_talents_champion_count < 1 ||
            receipt.party_talents_champion_count > CSB_V1_MAX_CHAMPIONS) return 0;
    }
    if ((!receipt.timer_core && receipt.timer_scheduled_count != 0u) ||
        (!receipt.message_core && receipt.timer_scheduled_count != 0u)) {
        return 0;
    }
    if ((receipt.message_scheduled_count != 0u &&
         (!receipt.message_core || receipt.message_scheduled_count !=
              receipt.timer_scheduled_count ||
          (receipt.last_message_route != (uint8_t)'M' &&
           receipt.last_message_route != (uint8_t)'D'))) ||
        (receipt.text_discard_count != 0u &&
         (!receipt.text_display_core || receipt.text_discard_count != 1u)) ||
        (receipt.sound_notification_count != 0u &&
         (!receipt.sound_core || receipt.sound_notification_count != 1u ||
          !profile->runtime.csbwin_dsa_sound_receipt.valid ||
          profile->runtime.csbwin_dsa_sound_receipt.sound_number !=
              receipt.last_sound_number ||
          profile->runtime.csbwin_dsa_sound_receipt.volume !=
              receipt.last_sound_volume ||
          profile->runtime.csbwin_dsa_sound_receipt.flags !=
              receipt.last_sound_flags ||
          profile->runtime.csbwin_dsa_sound_receipt.source_game_time !=
              profile->runtime.game_time))) {
        return 0;
    }
    if (receipt.action_program_fnv1a == 0u) return 0;
    receipt.dynamic_transfer_count = execution.dynamic_transfer_count;
    if (receipt.dynamic_transfer_count != 0u) {
        if (receipt.dynamic_transfer_count != 1u ||
            execution.dynamic_transfer_final_state < 0) {
            return 0;
        }
        receipt.dynamic_transfer_state = execution.dynamic_transfer_state;
        receipt.dynamic_transfer_column = execution.dynamic_transfer_column;
        receipt.dynamic_transfer_gosub = execution.dynamic_transfer_gosub;
        receipt.dynamic_transfer_final_state =
            execution.dynamic_transfer_final_state;
    }
    receipt.transfer_final_state = -1;
    receipt.transfer_only = execution.transfer_only ? 1 : 0;
    if (receipt.transfer_only) {
        receipt.transfer_count = execution.transfer_count;
        receipt.transfer_return_count = execution.transfer_return_count;
        receipt.transfer_frame_push_count = execution.transfer_frame_push_count;
        receipt.transfer_frame_pop_count = execution.transfer_frame_pop_count;
        receipt.maximum_subroutine_depth = execution.maximum_subroutine_depth;
        receipt.transfer_final_state = execution.transfer_final_state;
        receipt.transfer_returned_by_missing_program =
            execution.transfer_returned_by_missing_program ? 1 : 0;
        if (receipt.transfer_count == 0u || receipt.transfer_final_state < 0 ||
            receipt.transfer_frame_pop_count > receipt.transfer_frame_push_count ||
            receipt.transfer_return_count < receipt.transfer_frame_pop_count) {
            return 0;
        }
    }
    receipt.timer_owner_hash = csb_v1_csbwin_dsa_timer_owner_hash(
        handoff, timer, location, queue_slot, action, action_ordinal);
    if (receipt.timer_owner_hash == 0u) return 0;
    hash = hash_step(hash, receipt.save_fnv1a);
    hash = hash_step(hash, receipt.startup_session_generation);
    hash = hash_step(hash, receipt.startup_source_tick);
    hash = hash_step(hash, receipt.runtime_game_time);
    hash = hash_step(hash, receipt.queue_slot);
    hash = hash_step(hash, receipt.timer_index);
    hash = hash_step(hash, receipt.dsa_id);
    hash = hash_step(hash, receipt.state_index);
    hash = hash_step(hash, receipt.input_column);
    hash = hash_step(hash, (uint32_t)receipt.action_ordinal);
    hash = hash_step(hash, receipt.action_program_word_count);
    hash = hash_step(hash, receipt.action_program_fnv1a);
    hash = hash_step(hash, (uint32_t)receipt.comparison_core);
    hash = hash_step(hash, (uint32_t)receipt.arithmetic_core);
    hash = hash_step(hash, (uint32_t)receipt.variable_core);
    hash = hash_step(hash, (uint32_t)receipt.timer_core);
    hash = hash_step(hash, (uint32_t)receipt.message_core);
    hash = hash_step(hash, (uint32_t)receipt.text_display_core);
    hash = hash_step(hash, (uint32_t)receipt.sound_core);
    hash = hash_step(hash, (uint32_t)receipt.champion_core);
    hash = hash_step(hash, (uint32_t)receipt.object_core);
    hash = hash_step(hash, (uint32_t)receipt.query_core);
    hash = hash_step(hash, (uint32_t)receipt.forced_state);
    hash = hash_step(hash, (uint32_t)receipt.saved_dsa_state_transition_valid);
    hash = hash_step(hash, receipt.saved_dsa_state_before);
    hash = hash_step(hash, receipt.saved_dsa_state_after);
    hash = hash_step(hash, receipt.saved_dsa_state_tail_fnv1a);
    hash = hash_step(hash, (uint32_t)receipt.dungeon_mutation_core);
    hash = hash_step(hash, (uint32_t)receipt.runtime_dungeon_changed);
    hash = hash_step(hash, receipt.dungeon_raw_fnv1a);
    hash = hash_step(hash, receipt.timer_scheduled_count);
    hash = hash_step(hash, receipt.last_scheduled_event_type);
    hash = hash_step(hash, receipt.last_scheduled_target_location);
    hash = hash_step(hash, receipt.message_scheduled_count);
    hash = hash_step(hash, receipt.last_message_route);
    hash = hash_step(hash, receipt.text_discard_count);
    hash = hash_step(hash, receipt.sound_notification_count);
    hash = hash_step(hash, (uint32_t)receipt.last_sound_number);
    hash = hash_step(hash, (uint32_t)receipt.last_sound_volume);
    hash = hash_step(hash, (uint32_t)receipt.last_sound_flags);
    hash = hash_step(hash, (uint32_t)receipt.party_talents_changed);
    hash = hash_step(hash, (uint32_t)receipt.party_talents_champion_count);
    for (int i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
        hash = hash_step(hash, receipt.party_talents_fingerprints[i]);
        hash = hash_step(hash, receipt.party_talents_after[i]);
    }
    hash = hash_step(hash, receipt.cell_store_count);
    hash = hash_step(hash, receipt.last_cell_store_location);
    hash = hash_step(hash, receipt.last_cell_store_write_mask);
    hash = hash_step(hash, receipt.false_pit_count);
    hash = hash_step(hash, receipt.last_false_pit_location);
    hash = hash_step(hash, receipt.teleporter_copy_count);
    hash = hash_step(hash, receipt.last_teleporter_copy_source_location);
    hash = hash_step(hash, receipt.last_teleporter_copy_destination_location);
    hash = hash_step(hash, receipt.dynamic_transfer_count);
    hash = hash_step(hash, receipt.dynamic_transfer_state);
    hash = hash_step(hash, receipt.dynamic_transfer_column);
    hash = hash_step(hash, (uint32_t)receipt.dynamic_transfer_gosub);
    hash = hash_step(hash, (uint32_t)receipt.dynamic_transfer_final_state);
    hash = hash_step(hash, (uint32_t)receipt.transfer_only);
    hash = hash_step(hash, receipt.transfer_count);
    hash = hash_step(hash, receipt.transfer_return_count);
    hash = hash_step(hash, receipt.transfer_frame_push_count);
    hash = hash_step(hash, receipt.transfer_frame_pop_count);
    hash = hash_step(hash, receipt.maximum_subroutine_depth);
    hash = hash_step(hash, (uint32_t)receipt.transfer_final_state);
    hash = hash_step(hash, (uint32_t)receipt.transfer_returned_by_missing_program);
    receipt.bridge_hash = hash;
    receipt.source_evidence =
        "CSBWin SaveGame.cpp TimerQueue/DSA state; Timer.cpp ProcessTT_*; "
        "DSA.cpp ProcessDSATimer6; ReDMCSB LOADSAVE.C F0435";
    receipt.valid = receipt.bridge_hash != 0u;
    *out_receipt = receipt;
    return receipt.valid;
}

int csb_v1_csbwin_dsa_restored_timer_receipt_current_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 *handoff,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *startup_session,
    const CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *receipt)
{
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 execution;
    CSB_V1_CSBWinDSACoreProgramReceipt core;
    CSB_V1_DSAFilterLocation location;
    const CSB_V1_CSBWin512TimerSummary *timer;
    const CSB_V1_DSAImportedAction *action;
    int action_ordinal;
    uint32_t hash = 2166136261u;

    memset(&core, 0, sizeof(core));
    if (!receipt || !receipt->valid || !receipt->handoff_consumed ||
        !receipt->session_current || !receipt->save_identity_current ||
        !receipt->tick_order_current || !receipt->timer_record_consumed ||
        !receipt->local_state_consumed || !receipt->opcode_body_admitted ||
        !receipt->action_executed || receipt->timer_owner_hash == 0u ||
        !csb_v1_csbwin_dsa_handoff_current(
            profile, handoff, startup_session, &chain) ||
        receipt->save_fnv1a != handoff->save_fnv1a ||
        receipt->startup_session_generation != handoff->startup_session_generation ||
        receipt->startup_source_tick != handoff->startup_source_tick ||
        receipt->runtime_game_time != profile->runtime.game_time ||
        !csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
            &profile->runtime, &execution) || !execution.valid ||
        execution.dsa_id != receipt->dsa_id ||
        execution.state_index != receipt->state_index ||
        execution.column != receipt->input_column ||
        execution.action_ordinal != receipt->action_ordinal ||
        !chain.saved_timer_dsa_execution_valid ||
        chain.last_queue_slot != receipt->queue_slot ||
        chain.last_timer_index != receipt->timer_index) {
        return 0;
    }
    if (execution.dynamic_transfer_count != receipt->dynamic_transfer_count ||
        execution.dynamic_transfer_state != receipt->dynamic_transfer_state ||
        execution.dynamic_transfer_column != receipt->dynamic_transfer_column ||
        execution.dynamic_transfer_gosub != receipt->dynamic_transfer_gosub ||
        execution.dynamic_transfer_final_state !=
            receipt->dynamic_transfer_final_state) {
        return 0;
    }
    if (execution.transfer_only != receipt->transfer_only ||
        (receipt->transfer_only &&
         (receipt->transfer_count == 0u || receipt->transfer_final_state < 0 ||
          receipt->transfer_frame_pop_count > receipt->transfer_frame_push_count ||
          receipt->transfer_return_count < receipt->transfer_frame_pop_count ||
          execution.transfer_count != receipt->transfer_count ||
          execution.transfer_return_count != receipt->transfer_return_count ||
          execution.transfer_frame_push_count != receipt->transfer_frame_push_count ||
          execution.transfer_frame_pop_count != receipt->transfer_frame_pop_count ||
          execution.maximum_subroutine_depth != receipt->maximum_subroutine_depth ||
          execution.transfer_final_state != receipt->transfer_final_state ||
          execution.transfer_returned_by_missing_program !=
              receipt->transfer_returned_by_missing_program)) ||
        (!receipt->transfer_only &&
         (receipt->transfer_count != 0u || receipt->transfer_return_count != 0u ||
          receipt->transfer_frame_push_count != 0u ||
          receipt->transfer_frame_pop_count != 0u ||
          receipt->maximum_subroutine_depth != 0u ||
          receipt->transfer_final_state != -1 ||
          receipt->transfer_returned_by_missing_program))) {
        return 0;
    }
    memset(&location, 0, sizeof(location));
    location.level = receipt->timer_level;
    location.x = receipt->timer_x;
    location.y = receipt->timer_y;
    location.position = receipt->timer_location_position;
    location.actuator_thing = receipt->timer_actuator_thing;
    if (!csb_v1_csbwin_dsa_prepare_saved_timer(
            &profile->runtime, &location,
            receipt->queue_slot, &timer, &action, &action_ordinal) ||
        timer->source_index != receipt->timer_index ||
        timer->function != receipt->timer_function ||
        timer->ubyte8 != receipt->timer_position ||
        timer->ubyte9 != receipt->timer_action ||
        action->dsa_id != receipt->dsa_id ||
        action->state_index != receipt->state_index ||
        action->column != receipt->input_column ||
        action_ordinal != receipt->action_ordinal ||
        !action->program_words || action->program_word_count <= 0 ||
        action->program_word_count > UINT16_MAX ||
        action->program_word_count != receipt->action_program_word_count ||
        fnv1a32((const uint8_t *)action->program_words,
                (size_t)action->program_word_count *
                    sizeof(*action->program_words)) != receipt->action_program_fnv1a ||
        csb_v1_csbwin_dsa_verify_authenticated_core_program(
            &profile->runtime.csbwin_extended_dsa_state, action->dsa_id,
            action->state_index, action_ordinal, &core) !=
            CSB_V1_CSBWIN_DSA_CORE_OK || !core.valid ||
        core.comparison_core != receipt->comparison_core ||
        core.arithmetic_core != receipt->arithmetic_core ||
        core.variable_core != receipt->variable_core ||
        core.timer_core != receipt->timer_core ||
        core.message_core != receipt->message_core ||
        core.text_display_core != receipt->text_display_core ||
        core.sound_core != receipt->sound_core ||
        core.champion_core != receipt->champion_core ||
        core.object_core != receipt->object_core ||
        core.query_core != receipt->query_core ||
        execution.forced_state != receipt->forced_state ||
        execution.saved_dsa_state_transition_valid !=
            receipt->saved_dsa_state_transition_valid ||
        execution.saved_dsa_state_before != receipt->saved_dsa_state_before ||
        execution.saved_dsa_state_after != receipt->saved_dsa_state_after ||
        execution.saved_dsa_state_tail_fnv1a !=
            receipt->saved_dsa_state_tail_fnv1a ||
        (receipt->forced_state >= 0 &&
         !receipt->saved_dsa_state_transition_valid) ||
        (receipt->saved_dsa_state_transition_valid &&
         (!profile->runtime.csbwin_appended_tail_valid ||
          receipt->saved_dsa_state_tail_fnv1a == 0u ||
          receipt->saved_dsa_state_tail_fnv1a !=
              profile->runtime.csbwin_appended_tail_fnv1a)) ||
        (!receipt->saved_dsa_state_transition_valid &&
         receipt->saved_dsa_state_tail_fnv1a != 0u) ||
        core.dungeon_mutation_core != receipt->dungeon_mutation_core ||
        execution.variable_core != receipt->variable_core ||
        execution.timer_core != receipt->timer_core ||
        execution.message_core != receipt->message_core ||
        (execution.dungeon_changed ? 1 : 0) != receipt->runtime_dungeon_changed ||
        (receipt->dungeon_mutation_core &&
         (execution.cell_store_count != receipt->cell_store_count ||
          execution.last_cell_store_location != receipt->last_cell_store_location ||
          execution.last_cell_store_write_mask != receipt->last_cell_store_write_mask ||
          execution.false_pit_count != receipt->false_pit_count ||
          execution.last_false_pit_location != receipt->last_false_pit_location ||
          execution.teleporter_copy_count != receipt->teleporter_copy_count ||
          execution.last_teleporter_copy_source_location !=
              receipt->last_teleporter_copy_source_location ||
          execution.last_teleporter_copy_destination_location !=
              receipt->last_teleporter_copy_destination_location)) ||
        !profile->runtime.dungeon_handle ||
        !profile->runtime.dungeon_handle->raw_data ||
        profile->runtime.dungeon_handle->raw_size <= 0 ||
        fnv1a32(profile->runtime.dungeon_handle->raw_data,
                (size_t)profile->runtime.dungeon_handle->raw_size) !=
            receipt->dungeon_raw_fnv1a ||
        (receipt->false_pit_count != 0u &&
         (receipt->false_pit_count != 1u || receipt->cell_store_count != 1u ||
          receipt->last_false_pit_location != receipt->last_cell_store_location)) ||
        execution.timer_scheduled_count != receipt->timer_scheduled_count ||
        execution.last_scheduled_event_type != receipt->last_scheduled_event_type ||
        execution.last_scheduled_target_location !=
            receipt->last_scheduled_target_location ||
        execution.message_scheduled_count != receipt->message_scheduled_count ||
        execution.last_message_route != receipt->last_message_route ||
        execution.text_discard_count != receipt->text_discard_count ||
        execution.sound_notification_count != receipt->sound_notification_count ||
        execution.last_sound_number != receipt->last_sound_number ||
        execution.last_sound_volume != receipt->last_sound_volume ||
        execution.last_sound_flags != receipt->last_sound_flags ||
        execution.party_talents_changed != receipt->party_talents_changed ||
        execution.query_core != receipt->query_core ||
        (receipt->party_talents_changed &&
         (!receipt->champion_core ||
          receipt->party_talents_champion_count < 1 ||
          receipt->party_talents_champion_count > CSB_V1_MAX_CHAMPIONS ||
          execution.party_talents_champion_count !=
              receipt->party_talents_champion_count ||
          memcmp(execution.party_talents_fingerprints,
                 receipt->party_talents_fingerprints,
                 sizeof(receipt->party_talents_fingerprints)) != 0 ||
          memcmp(execution.party_talents_after, receipt->party_talents_after,
                 sizeof(receipt->party_talents_after)) != 0)) ||
        ((!receipt->timer_core || !receipt->message_core) &&
         receipt->timer_scheduled_count != 0u) ||
        (receipt->message_scheduled_count != 0u &&
         (!receipt->message_core || receipt->message_scheduled_count !=
              receipt->timer_scheduled_count ||
          (receipt->last_message_route != (uint8_t)'M' &&
           receipt->last_message_route != (uint8_t)'D'))) ||
        (receipt->text_discard_count != 0u &&
         (!receipt->text_display_core || receipt->text_discard_count != 1u)) ||
        (receipt->sound_notification_count != 0u &&
         (!receipt->sound_core || receipt->sound_notification_count != 1u ||
          !profile->runtime.csbwin_dsa_sound_receipt.valid ||
          profile->runtime.csbwin_dsa_sound_receipt.sound_number !=
              receipt->last_sound_number ||
          profile->runtime.csbwin_dsa_sound_receipt.volume !=
              receipt->last_sound_volume ||
          profile->runtime.csbwin_dsa_sound_receipt.flags !=
              receipt->last_sound_flags ||
          profile->runtime.csbwin_dsa_sound_receipt.source_game_time !=
              profile->runtime.game_time)) ||
        csb_v1_csbwin_dsa_timer_owner_hash(
            handoff, timer, &location, receipt->queue_slot, action,
            action_ordinal) != receipt->timer_owner_hash) {
        return 0;
    }
    hash = hash_step(hash, receipt->save_fnv1a);
    hash = hash_step(hash, receipt->startup_session_generation);
    hash = hash_step(hash, receipt->startup_source_tick);
    hash = hash_step(hash, receipt->runtime_game_time);
    hash = hash_step(hash, receipt->queue_slot);
    hash = hash_step(hash, receipt->timer_index);
    hash = hash_step(hash, receipt->dsa_id);
    hash = hash_step(hash, receipt->state_index);
    hash = hash_step(hash, receipt->input_column);
    hash = hash_step(hash, (uint32_t)receipt->action_ordinal);
    hash = hash_step(hash, receipt->action_program_word_count);
    hash = hash_step(hash, receipt->action_program_fnv1a);
    hash = hash_step(hash, (uint32_t)receipt->comparison_core);
    hash = hash_step(hash, (uint32_t)receipt->arithmetic_core);
    hash = hash_step(hash, (uint32_t)receipt->variable_core);
    hash = hash_step(hash, (uint32_t)receipt->timer_core);
    hash = hash_step(hash, (uint32_t)receipt->message_core);
    hash = hash_step(hash, (uint32_t)receipt->text_display_core);
    hash = hash_step(hash, (uint32_t)receipt->sound_core);
    hash = hash_step(hash, (uint32_t)receipt->champion_core);
    hash = hash_step(hash, (uint32_t)receipt->object_core);
    hash = hash_step(hash, (uint32_t)receipt->query_core);
    hash = hash_step(hash, (uint32_t)receipt->forced_state);
    hash = hash_step(hash, (uint32_t)receipt->saved_dsa_state_transition_valid);
    hash = hash_step(hash, receipt->saved_dsa_state_before);
    hash = hash_step(hash, receipt->saved_dsa_state_after);
    hash = hash_step(hash, receipt->saved_dsa_state_tail_fnv1a);
    hash = hash_step(hash, (uint32_t)receipt->dungeon_mutation_core);
    hash = hash_step(hash, (uint32_t)receipt->runtime_dungeon_changed);
    hash = hash_step(hash, receipt->dungeon_raw_fnv1a);
    hash = hash_step(hash, receipt->timer_scheduled_count);
    hash = hash_step(hash, receipt->last_scheduled_event_type);
    hash = hash_step(hash, receipt->last_scheduled_target_location);
    hash = hash_step(hash, receipt->message_scheduled_count);
    hash = hash_step(hash, receipt->last_message_route);
    hash = hash_step(hash, receipt->text_discard_count);
    hash = hash_step(hash, receipt->sound_notification_count);
    hash = hash_step(hash, (uint32_t)receipt->last_sound_number);
    hash = hash_step(hash, (uint32_t)receipt->last_sound_volume);
    hash = hash_step(hash, (uint32_t)receipt->last_sound_flags);
    hash = hash_step(hash, (uint32_t)receipt->party_talents_changed);
    hash = hash_step(hash, (uint32_t)receipt->party_talents_champion_count);
    for (int i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
        hash = hash_step(hash, receipt->party_talents_fingerprints[i]);
        hash = hash_step(hash, receipt->party_talents_after[i]);
    }
    hash = hash_step(hash, receipt->cell_store_count);
    hash = hash_step(hash, receipt->last_cell_store_location);
    hash = hash_step(hash, receipt->last_cell_store_write_mask);
    hash = hash_step(hash, receipt->false_pit_count);
    hash = hash_step(hash, receipt->last_false_pit_location);
    hash = hash_step(hash, receipt->teleporter_copy_count);
    hash = hash_step(hash, receipt->last_teleporter_copy_source_location);
    hash = hash_step(hash, receipt->last_teleporter_copy_destination_location);
    hash = hash_step(hash, receipt->dynamic_transfer_count);
    hash = hash_step(hash, receipt->dynamic_transfer_state);
    hash = hash_step(hash, receipt->dynamic_transfer_column);
    hash = hash_step(hash, (uint32_t)receipt->dynamic_transfer_gosub);
    hash = hash_step(hash, (uint32_t)receipt->dynamic_transfer_final_state);
    hash = hash_step(hash, (uint32_t)receipt->transfer_only);
    hash = hash_step(hash, receipt->transfer_count);
    hash = hash_step(hash, receipt->transfer_return_count);
    hash = hash_step(hash, receipt->transfer_frame_push_count);
    hash = hash_step(hash, receipt->transfer_frame_pop_count);
    hash = hash_step(hash, receipt->maximum_subroutine_depth);
    hash = hash_step(hash, (uint32_t)receipt->transfer_final_state);
    hash = hash_step(hash, (uint32_t)receipt->transfer_returned_by_missing_program);
    return hash == receipt->bridge_hash;
}

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
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinDSARestoredTimerExecutionReceipt_PC34 receipt;
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain;
    CSB_V1_CSBWinDSACoreProgramReceipt core;
    const CSB_V1_DSAImportedAction *action;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    memset(&chain, 0, sizeof(chain));
    memset(&core, 0, sizeof(core));
    if (!binding || !out_filter || !out_adapter ||
        !csb_v1_csbwin_dsa_handoff_current(
            profile, handoff, startup_session, &chain) ||
        !binding->actuator_identity_valid ||
        binding->location.level != loaded_level || loaded_level < 0 ||
        loaded_level >= 64 || binding->dsa_selector >= 32u ||
        profile->runtime.csbwin_extended_level_dsa_index[loaded_level]
            [binding->dsa_selector] != binding->dsa_id ||
        action_ordinal < 0 ||
        !(action = csb_v1_chaos_find_imported_action(
              &profile->runtime.csbwin_extended_dsa_state, binding->dsa_id,
              state_index, action_ordinal)) ||
        csb_v1_csbwin_dsa_verify_authenticated_core_program(
            &profile->runtime.csbwin_extended_dsa_state, binding->dsa_id,
            state_index, action_ordinal, &core) != CSB_V1_CSBWIN_DSA_CORE_OK ||
        !core.valid ||
        !csb_v1_runtime_bind_csbwin_movement_filter_stack_runtime(
            &profile->runtime, binding, state_index, action_ordinal,
            master_location, loaded_level, out_filter, out_adapter)) {
        return 0;
    }
    receipt.handoff_consumed = 1;
    receipt.session_current = 1;
    receipt.save_identity_current = 1;
    receipt.tick_order_current = 1;
    receipt.local_state_consumed = 1;
    receipt.opcode_body_admitted = 1;
    receipt.save_fnv1a = handoff->save_fnv1a;
    receipt.startup_session_generation = handoff->startup_session_generation;
    receipt.startup_source_tick = handoff->startup_source_tick;
    receipt.runtime_game_time = profile->runtime.game_time;
    receipt.dsa_id = action->dsa_id;
    receipt.state_index = action->state_index;
    receipt.input_column = action->column;
    receipt.action_ordinal = action_ordinal;
    receipt.bridge_hash = hash_step(handoff->handoff_hash, action->dsa_id);
    receipt.bridge_hash = hash_step(receipt.bridge_hash, state_index);
    receipt.bridge_hash = hash_step(receipt.bridge_hash, (uint32_t)action_ordinal);
    receipt.source_evidence =
        "CSBWin Monster.cpp movement filter; DSA.cpp ProcessDSATimer6; "
        "source-bound restored save receipt";
    receipt.valid = receipt.bridge_hash != 0u;
    *out_receipt = receipt;
    return receipt.valid;
}
