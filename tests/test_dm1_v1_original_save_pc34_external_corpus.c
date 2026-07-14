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

static int receipt_is_admitted(const DM1OriginalSavePC34CorpusReceipt *receipt)
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
           receipt->c3_event_byte_preservation_ok &&
           receipt->c4_timeline_byte_preservation_ok &&
           receipt->dungeon_tail_byte_preservation_ok;
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
    CHECK(report.pc34_candidate_count > 0,
          "external corpus contains at least one PC34 candidate");
    CHECK(report.roundtrip_failed_count == 0 &&
              report.roundtrip_succeeded_count == report.pc34_candidate_count &&
              report.roundtrip_succeeded_count == report.receipt_count,
          "every external PC34 candidate completes F0435 to F0433 to F0435");
    CHECK(report.firestaff_manifest_rejected_count == 0 &&
              report.nonoriginal_envelope_rejected_count == 0,
          "external corpus contains no Firestaff or nonoriginal envelope");

    for (i = 0; i < report.receipt_count; ++i) {
        const DM1OriginalSavePC34CorpusReceipt *receipt = &report.receipts[i];

        CHECK(receipt_is_admitted(receipt),
              "external PC34 receipt is fully admitted");
        printf("ADMITTED path=%s source_bytes=%u source_hash=%08x "
               "f7057_end=%u tail_bytes=%u exported_bytes=%u "
               "exported_hash=%08x runtime_stage=%d runtime_adopt=%d\\n",
               receipt->path, receipt->source_byte_count, receipt->source_hash,
               receipt->source_f7057_envelope_end_offset,
               receipt->source_f7057_trailing_byte_count,
               receipt->exported_byte_count, receipt->exported_hash,
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
