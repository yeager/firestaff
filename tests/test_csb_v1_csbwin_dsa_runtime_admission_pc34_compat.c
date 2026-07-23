#include "csb_v1_csbwin_dsa_runtime_admission_pc34_compat.h"
#include <stdio.h>
#include <string.h>

static uint32_t fnv1a32(const uint8_t *p, size_t n) { uint32_t h = 2166136261u; size_t i; for (i=0u;i<n;++i) h=(h^p[i])*16777619u; return h ? h : 1u; }
static int failures;
#define CHECK(x) do { if (!(x)) { ++failures; fprintf(stderr, "FAIL: %s\n", #x); } } while (0)
int main(void) {
    CSB_V1_BootProfile profile; CSB_V1_CSBWinDSASaveCorpusReceipt corpus;
    CSB_V1_CSBWinDSARuntimeAdmissionReceipt_PC34 receipt;
    CSB_V1_CSBWinSaveCorpusCandidate_PC34 candidate;
    CSB_V1_CSBWinSaveCorpusCandidateReceipt_PC34 candidate_receipt;
    CSB_V1_CSBWinSaveCorpusDiscoveryInput_PC34 discovery_inputs[2];
    CSB_V1_CSBWinSaveCorpusDiscoveryReceipt_PC34 discovery_receipt;
    CSB_V1_CSBWinDSASaveRuntimeHandoffReceipt_PC34 handoff;
    CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 save_handoff;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 runtime_chain;
    CSB_V1_DungeonData dungeon;
    uint8_t save[96];
    memset(&profile, 0, sizeof(profile)); memset(&corpus, 0, sizeof(corpus));
    memset(save, 0x5a, sizeof(save));
    profile.assets_verified = 1; profile.dungeon_verified = 1;
    snprintf(profile.dungeon_path, sizeof(profile.dungeon_path), "%s", "Dungeon.dat");
    snprintf(profile.dungeon_md5, sizeof(profile.dungeon_md5), "%s", "6695d2acebce49f95db1d8f3a5c733de");
    corpus.valid = corpus.corpus_positive = corpus.runtime_handoff_ready = 1;
    corpus.extended_tail_valid = corpus.dsa_section_valid = corpus.dsa_has_runtime_actions = 1;
    corpus.gameblock1_valid = corpus.gameblock1_body_valid = 1;
    corpus.dsa.dsa_payload_offset = 8u; corpus.dsa.dsa_payload_size = 24u;
    corpus.dsa.next_payload_offset = 40u; corpus.dsa.stored_checksum = corpus.dsa.computed_checksum = 7u;
    corpus.gameblock1_offset = 48u; corpus.save_bytes_fnv1a = fnv1a32(save, sizeof(save));
    corpus.gameblock1_body_fnv1a = fnv1a32(save + 48u, sizeof(save) - 48u);
    CHECK(csb_v1_csbwin_dsa_runtime_admission_from_corpus_pc34(&profile, save, sizeof(save), &corpus, &receipt));
    CHECK(receipt.valid && receipt.immutable_source_bound && receipt.dsa_offset == 8u && receipt.gameblock_size == 48u);
    memset(&candidate, 0, sizeof(candidate));
    candidate.source_kind = CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_DIRECT_PC34;
    candidate.source_path = "CSBGAME.DAT";
    candidate.save_bytes = save;
    candidate.save_size = sizeof(save);
    candidate.declared_save_fnv1a = corpus.save_bytes_fnv1a;
    candidate.declared_save_md5 = "11111111111111111111111111111111";
    candidate.declared_dungeon_md5 = profile.dungeon_md5;
    CHECK(csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt) &&
        candidate_receipt.direct_file && candidate_receipt.no_write_or_extract &&
        candidate_receipt.dsa_admission.valid);
    candidate.source_kind = CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_VIRTUAL_CONTAINER_PC34;
    candidate.source_path = "archive.zip:CSBGAME.DAT";
    CHECK(csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt) &&
        candidate_receipt.virtual_container && candidate_receipt.no_write_or_extract);
    candidate.declared_save_fnv1a ^= 1u;
    CHECK(!csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt));
    candidate.declared_save_fnv1a = corpus.save_bytes_fnv1a;
    candidate.declared_dungeon_md5 = "00000000000000000000000000000000";
    CHECK(!csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt));
    candidate.declared_dungeon_md5 = profile.dungeon_md5;
    candidate.source_path = NULL;
    CHECK(!csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt));
    candidate.source_path = "archive.zip:CSBGAME.DAT";
    candidate.declared_save_md5 = "";
    CHECK(!csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt));
    candidate.declared_save_md5 = "11111111111111111111111111111111";
    corpus.gameblock1_body_fnv1a ^= 1u;
    CHECK(!csb_v1_csbwin_save_corpus_candidate_admission_pc34(
        &profile, &candidate, &corpus, &candidate_receipt));
    corpus.gameblock1_body_fnv1a = fnv1a32(save + 48u, sizeof(save) - 48u);
    memset(discovery_inputs, 0, sizeof(discovery_inputs));
    candidate.source_kind = CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_DIRECT_PC34;
    candidate.source_path = "CSBGAME.DAT";
    discovery_inputs[0].candidate = candidate;
    discovery_inputs[0].corpus = &corpus;
    CHECK(csb_v1_csbwin_save_corpus_discover_local_pc34(
        &profile, discovery_inputs, 1u, &discovery_receipt) &&
        discovery_receipt.selected_direct_file &&
        !discovery_receipt.selected_virtual_container &&
        discovery_receipt.admitted_count == 1u);
    discovery_inputs[0].candidate.source_kind =
        CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_VIRTUAL_CONTAINER_PC34;
    discovery_inputs[0].candidate.source_path = "local.zip:CSBGAME.DAT";
    CHECK(csb_v1_csbwin_save_corpus_discover_local_pc34(
        &profile, discovery_inputs, 1u, &discovery_receipt) &&
        discovery_receipt.selected_virtual_container &&
        discovery_receipt.candidate.no_write_or_extract);
    discovery_inputs[1] = discovery_inputs[0];
    discovery_inputs[1].candidate.source_kind =
        CSB_V1_CSBWIN_SAVE_CORPUS_SOURCE_DIRECT_PC34;
    discovery_inputs[1].candidate.source_path = "CSBGAME.BAK";
    CHECK(!csb_v1_csbwin_save_corpus_discover_local_pc34(
        &profile, discovery_inputs, 2u, &discovery_receipt) &&
        discovery_receipt.rejected_mixed_candidates &&
        discovery_receipt.admitted_count == 2u);
    CHECK(!csb_v1_csbwin_save_corpus_discover_local_pc34(
        &profile, NULL, 0u, &discovery_receipt) &&
        discovery_receipt.skipped_no_candidate);

    memset(&dungeon, 0, sizeof(dungeon));
    profile.runtime.dungeon_handle = &dungeon;
    memset(&save_handoff, 0, sizeof(save_handoff));
    save_handoff.valid = save_handoff.save_import_receipt_consumed = 1;
    save_handoff.runtime_load_consumed = save_handoff.dsa_corpus_positive = 1;
    save_handoff.dsa_runtime_handoff_ready = save_handoff.extended_tail_valid = 1;
    save_handoff.dsa_section_valid = save_handoff.dsa_has_runtime_actions = 1;
    save_handoff.gameblock1_valid = save_handoff.gameblock1_body_valid = 1;
    save_handoff.save_bytes_fnv1a = receipt.save_fnv1a;
    save_handoff.gameblock1_body_fnv1a = receipt.gameblock_fnv1a;
    memset(&session, 0, sizeof(session));
    session.valid = session.real_asset_matched = session.full_startup_ready = 1;
    session.rejects_legacy_wrappers = 1; session.generation = 17u;
    session.source_tick = 91u;
    memset(&runtime_chain, 0, sizeof(runtime_chain));
    runtime_chain.valid = runtime_chain.dsa_catalog_valid = 1;
    runtime_chain.level_index_valid = runtime_chain.timer_queue_event_chain_valid = 1;
    runtime_chain.live_timer_event_count = 3u; runtime_chain.imported_action_count = 2;
    save_handoff.runtime_game_time_after = 19u;
    profile.runtime.game_time = 19u;
    CHECK(csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    CHECK(handoff.valid && handoff.dungeon_identity_current &&
          handoff.startup_session_generation == 17u &&
          handoff.runtime_game_time == 19u && handoff.imported_action_count == 2);
    receipt.admission_hash = 0u;
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    receipt.admission_hash = 1u;
    save_handoff.runtime_game_time_after = 20u;
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    save_handoff.runtime_game_time_after = 19u;
    session.source_tick = 0u;
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    session.source_tick = 91u;
    save_handoff.save_bytes_fnv1a ^= 1u;
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    save_handoff.save_bytes_fnv1a = receipt.save_fnv1a;
    session.generation = 0u;
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    session.generation = 17u;
    runtime_chain.timer_queue_event_chain_valid = 0;
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    runtime_chain.timer_queue_event_chain_valid = 1;
    snprintf(profile.dungeon_md5, sizeof(profile.dungeon_md5), "%s",
             "00000000000000000000000000000000");
    CHECK(!csb_v1_csbwin_dsa_save_runtime_handoff_pc34(
        &profile, &receipt, &save_handoff, &session, &runtime_chain, &handoff));
    snprintf(profile.dungeon_md5, sizeof(profile.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    corpus.dsa.dsa_payload_size = 33u;
    CHECK(!csb_v1_csbwin_dsa_runtime_admission_from_corpus_pc34(&profile, save, sizeof(save), &corpus, &receipt));
    corpus.dsa.dsa_payload_size = 24u;
    save[0] ^= 1u;
    CHECK(!csb_v1_csbwin_dsa_runtime_admission_from_corpus_pc34(&profile, save, sizeof(save), &corpus, &receipt));
    return failures ? 1 : 0;
}
