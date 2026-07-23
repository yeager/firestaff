#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL %s (line %d): %s\\n", \
                    message, __LINE__, #condition); \
            return 1; \
        } \
    } while (0)

static int receipt_is_runtime_admitted(
    const DM1OriginalSavePC34CorpusReceipt *receipt)
{
    return receipt && receipt->external_original &&
           receipt->roundtrip_receipts_committed &&
           receipt->roundtrip_result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
           receipt->core_state_matches &&
           receipt->source_f7057_envelope_end_offset >
               SAVEGAME_PC34_DM_SAVE_HEADER_SIZE &&
           receipt->source_f7057_envelope_end_offset +
                   receipt->source_f7057_trailing_byte_count ==
               receipt->source_byte_count &&
           receipt->source_hash != 0u && receipt->exported_hash != 0u &&
           receipt->header_identity_preservation_ok &&
           receipt->part_byte_count_preservation_ok &&
           receipt->active_group_record_byte_receipt_available &&
           receipt->active_group_record_byte_preservation_ok &&
           receipt->source_active_group_record_byte_count ==
               receipt->source_part_byte_counts[SAVEGAME_PC34_PART_ACTIVE_GROUP] &&
           receipt->exported_active_group_record_byte_count ==
               receipt->exported_part_byte_counts[SAVEGAME_PC34_PART_ACTIVE_GROUP] &&
           receipt->source_active_group_record_fingerprint != 0u &&
           receipt->exported_active_group_record_fingerprint != 0u &&
           receipt->m516_champion_record_receipt_available &&
           receipt->m516_champion_record_byte_preservation_ok &&
           receipt->source_m516_champion_record_count == CHAMPION_MAX_PARTY &&
           receipt->exported_m516_champion_record_count == CHAMPION_MAX_PARTY &&
           receipt->source_m516_champion_record_byte_count > 0u &&
           receipt->exported_m516_champion_record_byte_count > 0u &&
           receipt->source_m516_champion_record_fingerprint != 0u &&
           receipt->exported_m516_champion_record_fingerprint != 0u &&
           receipt->party_info_byte_preservation_ok &&
           receipt->source_party_info_byte_count > 0u &&
           receipt->exported_party_info_byte_count ==
               receipt->source_party_info_byte_count &&
           receipt->source_party_info_fingerprint != 0u &&
           receipt->exported_party_info_fingerprint ==
               receipt->source_party_info_fingerprint &&
           receipt->c3_event_layout_receipt_available &&
           receipt->c3_event_byte_preservation_ok &&
           receipt->source_c3_event_byte_count ==
               receipt->source_part_byte_counts[SAVEGAME_PC34_PART_EVENTS] &&
           receipt->exported_c3_event_byte_count ==
               receipt->exported_part_byte_counts[SAVEGAME_PC34_PART_EVENTS] &&
           receipt->source_c3_event_fingerprint != 0u &&
           receipt->exported_c3_event_fingerprint != 0u &&
           receipt->c4_timeline_layout_receipt_available &&
           receipt->c4_timeline_byte_preservation_ok &&
           receipt->source_c4_timeline_byte_count ==
               receipt->source_part_byte_counts[SAVEGAME_PC34_PART_TIMELINE] &&
           receipt->exported_c4_timeline_byte_count ==
               receipt->exported_part_byte_counts[SAVEGAME_PC34_PART_TIMELINE] &&
           receipt->dungeon_tail_byte_preservation_ok &&
           receipt->source_dungeon_tail_byte_count > 0u &&
           receipt->source_runtime_stage_attempted &&
           receipt->source_runtime_stage_result ==
               DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
           receipt->source_runtime_stage_committed &&
           receipt->source_runtime_stage_owns_dungeon &&
           receipt->source_runtime_adopt_attempted &&
           receipt->source_runtime_adopt_result ==
               DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK &&
           receipt->source_runtime_adopted &&
           receipt->source_runtime_adopt_owns_dungeon &&
           receipt->source_runtime_adopt_event_count ==
               receipt->source_runtime_stage_event_count &&
           receipt->source_runtime_adopt_timeline_count ==
               receipt->source_runtime_stage_timeline_count &&
           receipt->source_runtime_adopt_queue_committed &&
           receipt->source_runtime_adopt_queue_event_count ==
               receipt->source_runtime_stage_timeline_count &&
           receipt->source_runtime_adopt_queue_first_unused_index >=
               receipt->source_runtime_adopt_queue_event_count &&
           receipt->source_runtime_stage_active_group_count >= 0 &&
           receipt->source_runtime_stage_active_group_count <=
               (int)receipt->source_active_group_record_count &&
           receipt->source_runtime_stage_active_group_fingerprint != 0u &&
           receipt->source_runtime_stage_global_map_fingerprint != 0u &&
           receipt->source_runtime_adopt_active_group_count ==
               receipt->source_runtime_stage_active_group_count &&
           receipt->source_runtime_adopt_active_group_fingerprint ==
               receipt->source_runtime_stage_active_group_fingerprint &&
           receipt->source_runtime_adopt_global_map_fingerprint ==
               receipt->source_runtime_stage_global_map_fingerprint &&
           receipt->c2_m516_runtime_adoption_receipt_available &&
           receipt->c2_m516_runtime_adoption_valid &&
           receipt->c2_m516_runtime_adoption_fingerprint != 0u &&
           receipt->c13_c24_c25_runtime_adoption_receipt_available &&
           receipt->c13_c24_c25_runtime_adoption_valid &&
           receipt->c13_c24_c25_runtime_adoption_fingerprint != 0u &&
           receipt->c13_c24_c25_runtime_stale_fence_receipt_available &&
           receipt->c13_c24_c25_runtime_stale_fence_valid &&
           !receipt->c13_c24_c25_runtime_stale_fence_revoked &&
           receipt->c13_c24_c25_runtime_stale_fence_fingerprint != 0u &&
           receipt->c03_c04_runtime_adoption_receipt_available &&
           receipt->c03_c04_runtime_adoption_valid &&
           receipt->c03_c04_runtime_adoption_fingerprint != 0u &&
           receipt->source_runtime_stage_c03_c04_receipt_valid &&
           receipt->source_runtime_adopt_c03_c04_receipt_valid &&
           receipt->source_runtime_stage_c03_fingerprint ==
               receipt->source_runtime_adopt_c03_fingerprint &&
           receipt->source_runtime_stage_c04_fingerprint ==
               receipt->source_runtime_adopt_c04_fingerprint &&
           receipt->c13_runtime_identity_receipt_available &&
           receipt->c13_runtime_identity_valid &&
           receipt->c13_runtime_identity_fingerprint != 0u &&
           receipt->c13_runtime_stale_fence_receipt_available &&
           receipt->c13_runtime_stale_fence_valid &&
           !receipt->c13_runtime_stale_fence_revoked &&
           receipt->c13_runtime_stale_fence_revoke_reason ==
               DM1_ORIGINAL_SAVE_PC34_C13_RUNTIME_FENCE_REVOKE_NONE &&
           receipt->c13_runtime_stale_fence_fingerprint != 0u;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM1_PC34_SAVE_CORPUS");
    DM1OriginalSavePC34CorpusRoundtripReport report;
    int result;
    int i;

    /* This target deliberately has no fixture path. ReDMCSB LOADSAVE.C F0435
     * only admits user-supplied PC34 bytes through the existing classifier,
     * five-part envelope, and transient F0433 reload route. */
    if (!root || !root[0]) {
        puts("SKIP external PC34 corpus admission: FIRESTAFF_DM1_PC34_SAVE_CORPUS unset");
        return 0;
    }

    memset(&report, 0, sizeof(report));
    result = dm1_v1_original_save_pc34_roundtrip_corpus_root(root, &report);
    CHECK(result == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
          "external corpus scan completes");
    CHECK(report.scan_succeeded && !report.discovery_root_error,
          "external corpus discovery is complete");
    CHECK(report.discovery_receipt_count == report.scanned_file_count,
          "external corpus records one discovery receipt per scanned file");
    CHECK(report.pc34_candidate_count > 0,
          "external corpus contains at least one PC34 candidate");
    CHECK(report.roundtrip_failed_count == 0 &&
              report.roundtrip_succeeded_count == report.pc34_candidate_count &&
              report.roundtrip_succeeded_count == report.receipt_count,
          "every external PC34 candidate completes F0435 to F0433 to F0435");
    CHECK(report.runtime_stage_attempted_count == report.pc34_candidate_count &&
              report.runtime_stage_succeeded_count == report.pc34_candidate_count &&
              report.runtime_stage_unavailable_count == 0 &&
              report.runtime_stage_failed_count == 0,
          "every external PC34 candidate stages its own F0435 dungeon");
    CHECK(report.runtime_adopt_attempted_count == report.pc34_candidate_count &&
              report.runtime_adopt_succeeded_count == report.pc34_candidate_count &&
              report.runtime_adopt_failed_count == 0,
          "every external PC34 candidate adopts its owned runtime state");
    CHECK(report.firestaff_manifest_rejected_count == 0 &&
              report.nonoriginal_envelope_rejected_count == 0,
          "external corpus contains no Firestaff or nonoriginal envelope");

    for (i = 0; i < report.receipt_count; ++i) {
        const DM1OriginalSavePC34CorpusReceipt *receipt = &report.receipts[i];
        int discovery_seen = 0;
        int j;

        CHECK(receipt_is_runtime_admitted(receipt),
              "external PC34 receipt is runtime-admitted without fallback");
        for (j = 0; j < report.discovery_receipt_count; ++j) {
            const DM1OriginalSavePC34CorpusDiscoveryReceipt *discovery =
                &report.discovery_receipts[j];
            if (strcmp(discovery->path, receipt->path) == 0) {
                discovery_seen = 1;
                CHECK(discovery->pc34_loader_part_envelope_candidate &&
                          discovery->external_original &&
                          discovery->roundtrip_eligible &&
                          discovery->f7057_envelope_end_offset ==
                              receipt->source_f7057_envelope_end_offset &&
                          discovery->f7057_trailing_byte_count ==
                              receipt->source_f7057_trailing_byte_count &&
                          discovery->save_game_id == receipt->game_id,
                      "discovery receipt binds loader/F7057 facts to roundtrip receipt");
            }
        }
        CHECK(discovery_seen,
              "roundtrip receipt has a matching discovery receipt");
        printf("ADMITTED path=%s source_bytes=%u source_hash=%08x "
               "f7057_end=%u tail_bytes=%u exported_bytes=%u "
               "exported_hash=%08x groups=%u/%u champions=%u/%u "
               "runtime_stage=%d runtime_adopt=%d\\n",
               receipt->path, receipt->source_byte_count, receipt->source_hash,
               receipt->source_f7057_envelope_end_offset,
               receipt->source_f7057_trailing_byte_count,
               receipt->exported_byte_count, receipt->exported_hash,
               receipt->source_active_group_record_count,
               receipt->exported_active_group_record_count,
               receipt->source_m516_champion_record_count,
               receipt->exported_m516_champion_record_count,
               receipt->source_runtime_stage_result,
               receipt->source_runtime_adopt_result);
    }
    printf("ADMISSION_SUMMARY root=%s candidates=%d admitted=%d "
           "roundtrip_hash=%08x runtime_stage=%d/%d/%d runtime_adopt=%d/%d\\n",
           root, report.pc34_candidate_count, report.roundtrip_succeeded_count,
           report.roundtrip_hash, report.runtime_stage_succeeded_count,
           report.runtime_stage_unavailable_count, report.runtime_stage_failed_count,
           report.runtime_adopt_succeeded_count, report.runtime_adopt_attempted_count);
    return 0;
}
