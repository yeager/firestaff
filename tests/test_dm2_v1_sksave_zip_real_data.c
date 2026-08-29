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

typedef struct {
    const char *path;
    size_t payload_size;
    uint32_t source_file_hash;
} ExpectedRetailSave;

/* Exact receipts for the admitted PC-DOS English archive's original members.
 * Hashes cover the complete source member, including its 42-byte SKSave
 * envelope, so an equally-sized synthetic replacement cannot pass the
 * real-media census. */
static const ExpectedRetailSave k_expected_retail_saves[] = {
    { "data/sksave0.bak", 51571u - 42u, 0x11ff6e43u },
    { "data/sksave0.dat", 51553u - 42u, 0x05f5c3c7u },
    { "data/sksave1.bak", 51557u - 42u, 0x13aafce2u },
    { "data/sksave1.dat", 51557u - 42u, 0xeae51622u },
    { "data/sksave2.bak", 51551u - 42u, 0xaa49805eu },
    { "data/sksave2.dat", 51574u - 42u, 0x06967d98u },
    { "data/sksave3.bak", 51521u - 42u, 0xe24fc512u },
    { "data/sksave3.dat", 51531u - 42u, 0x88af2967u }
};

#define CHECK(condition, message) \
    do { \
        if (condition) { \
            printf("  PASS: %s\n", message); \
        } else { \
            printf("  FAIL: %s\n", message); \
            ++failures; \
        } \
    } while (0)

static int receipt_matches_expected(const DM2_SKSaveCorpusReceipt *corpus,
                                    const ExpectedRetailSave *expected)
{
    uint8_t index;
    if (!corpus || !expected) return 0;
    for (index = 0u; index < corpus->candidate_receipt_count; ++index) {
        const DM2_SKSaveCandidateReceipt *candidate =
            &corpus->candidate_receipts[index];
        const char *member = strstr(candidate->path, "::");
        if (member) member += 2;
        if (member && strcmp(member, expected->path) == 0) {
            return candidate->payload_size == expected->payload_size &&
                   candidate->source_file_hash == expected->source_file_hash;
        }
    }
    return 0;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_SKSAVE_ZIP");
    DM2_SKSaveCorpusReceipt corpus;
    DM2_OriginalSaveStateCorpusReceipt states;
    DM2_SL_State slots;
    uint8_t *payload = NULL;
    uint8_t prefix[64];
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
    {
        size_t index;
        for (index = 0u;
             index < sizeof(k_expected_retail_saves) /
                         sizeof(k_expected_retail_saves[0]);
             ++index) {
            CHECK(receipt_matches_expected(&corpus,
                                           &k_expected_retail_saves[index]),
                  "each original SKSave member retains its source identity");
        }
    }

    dm2_sl_init(&slots, archive);
    CHECK(dm2_sl_scan_slots(&slots) && slots.slot_count == 4u &&
              dm2_sl_slot_occupied(&slots, 0u) &&
              dm2_sl_slot_occupied(&slots, 3u),
          "public slot scan reads source ZIP members without extraction");
    CHECK(dm2_v1_save_has_valid_slot(archive, 0u) &&
              dm2_v1_save_has_valid_slot(archive, 3u),
          "public slot validation accepts virtual source members");

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
        payload_size = 0u;
        CHECK(dm2_sl_load(archive, 0u, payload,
                          corpus.first_importable_payload_size,
                          &payload_size) == 0 && payload_size != 0u,
              "public slot loader reads a retail ZIP member in memory");
        payload_size = 0u;
        CHECK(dm2_sl_load(archive, 0u, prefix, sizeof(prefix),
                          &payload_size) == 0 &&
                  payload_size == sizeof(prefix) &&
                  memcmp(prefix, payload, sizeof(prefix)) == 0,
              "public slot loader preserves its bounded-prefix contract");
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
