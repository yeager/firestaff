/*
 * Real PC-DOS DM2 SKSave ZIP regression.
 *
 * Firestaff must consume source-owned game archives without materialising
 * their members on disk.  This test uses the retail archive selected through
 * FIRESTAFF_DM2_SKSAVE_ZIP and checks the authenticated SKSave corpus path
 * from discovery through receipt-bound payload reread.
 */

#include "dm2_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) \
    do { \
        if (condition) { \
            printf("  PASS: %s\n", message); \
        } else { \
            printf("  FAIL: %s\n", message); \
            ++failures; \
        } \
    } while (0)

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_SKSAVE_ZIP");
    DM2_SKSaveCorpusReceipt corpus;
    DM2_OriginalSaveStateCorpusReceipt states;
    uint8_t *payload = NULL;
    size_t payload_size = 0u;

    printf("DM2 real PC-DOS SKSave ZIP tests:\n\n");
    if (!archive || !archive[0]) {
        printf("SKIP: FIRESTAFF_DM2_SKSAVE_ZIP is not set\n");
        return 77;
    }

    memset(&corpus, 0, sizeof(corpus));
    CHECK(dm2_v1_sksave_corpus_scan(archive, &corpus),
          "scanner accepts a source ZIP as its corpus root");
    CHECK(corpus.valid_slot_count == 4u && corpus.valid_slot_mask == 0x000fu,
          "all four source slot primaries are found in the ZIP");
    CHECK(corpus.valid_slot_backup_count == 4u,
          "all four source slot backups are found in the ZIP");
    CHECK(corpus.candidate_receipt_count == 8u &&
              corpus.importable_candidate_count == 8u &&
              strstr(corpus.first_importable_path, "::data/") != NULL,
          "source members retain virtual archive paths in their receipts");

    if (corpus.first_importable_payload_size != 0u) {
        payload = (uint8_t *)malloc(corpus.first_importable_payload_size);
    }
    CHECK(payload != NULL, "receipt payload buffer allocates");
    if (payload) {
        CHECK(dm2_v1_sksave_corpus_load_receipted_candidate(
                  &corpus.candidate_receipts[0], payload,
                  corpus.first_importable_payload_size, &payload_size) &&
                  payload_size == corpus.candidate_receipts[0].payload_size,
              "receipt-bound payload reread succeeds without extraction");
    }
    free(payload);

    memset(&states, 0, sizeof(states));
    CHECK(dm2_v1_original_save_state_corpus_probe(archive, &states) &&
              states.scan_complete && states.original_candidate_list_complete &&
              states.original_candidate_count == 8u &&
              states.parsed_candidate_count == 8u &&
              states.rejected_candidate_count == 0u,
          "original save-state census reads every ZIP member in memory");

    return failures ? 1 : 0;
}
