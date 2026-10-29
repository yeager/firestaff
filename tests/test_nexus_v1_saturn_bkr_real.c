#include "nexus_v1_saturn_bkr.h"
#include "firestaff_x68k_media_receipt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *filename;
    const char *sha256;
    const char *payload_sha256;
    uint32_t champion_count;
    uint32_t party_leader_index;
    int32_t champion_stamina_current;
    int32_t champion_stamina_max;
} CorpusFile;

static const CorpusFile corpus[] = {
    {"nexus-two-champion-retail-written-20260926.bkr",
     "3d60856119df55ffa4937d13b5dc7487157c7c6b131a999a2e61683316679634",
     "9c969745745acc7f843148abf78f114ab56e82b39b7f01251b6407a0d9fdf4eb",
     2U, 0U, 66, 67},
    {"nexus-three-champion-retail-written-20260926.bkr",
     "d9c86de3b0c668f9961a16529b1d5fd72f3e095823f8f0021cf8db8bfba48e9b",
     "1e7e20ee72cdc57c50557ec6bf6325be22c299558a9b8f2731a78877d89725a3",
     3U, 0U, 66, 67},
    {"nexus-four-champion-retail-written-20260926.bkr",
     "87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035",
     "1c460c65cc6156843ec4e0c1c8ca4ae3667241cc46a04151624b578e478c47e5",
     4U, 0U, 66, 67},
    {"nexus-four-champion-leader-3-retail-written-20260926.bkr",
     "319ed84080a90eaeafb1bfa85ca4405a0153f4c85fda76ee7c557b34ac276867",
     "b77461232e7cf3485be0e179b5feab777c91efa187d0ad6bdcc7bb9867058e3e",
     4U, 3U, 67, 67}
};

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long end;
    uint8_t *bytes;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (end = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)end);
    if (!bytes || fread(bytes, 1U, (size_t)end, file) != (size_t)end) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)end;
    return 1;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    uint8_t *payloads[sizeof(corpus) / sizeof(corpus[0])] = {0};
    size_t payload_sizes[sizeof(corpus) / sizeof(corpus[0])] = {0};
    size_t file_index;
    int failed = 0;
    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not mounted");
        return 77;
    }

    for (file_index = 0U;
         file_index < sizeof(corpus) / sizeof(corpus[0]); ++file_index) {
        char path[1024];
        FILE *file;
        int path_length = snprintf(path, sizeof(path), "%s/%s", root,
                                   corpus[file_index].filename);
        if (path_length < 0 || (size_t)path_length >= sizeof(path) ||
            !(file = fopen(path, "rb"))) {
            printf("SKIP: authentic BKR corpus is incomplete: %s\n",
                   corpus[file_index].filename);
            return 77;
        }
        fclose(file);
    }

    for (file_index = 0U;
         file_index < sizeof(corpus) / sizeof(corpus[0]); ++file_index) {
        char path[1024];
        FILE *file;
        int path_length = snprintf(path, sizeof(path), "%s/%s", root,
                                   corpus[file_index].filename);
        if (path_length < 0 || (size_t)path_length >= sizeof(path) ||
            !(file = fopen(path, "rb"))) {
            printf("SKIP: authentic BKR corpus is incomplete: %s\n",
                   corpus[file_index].filename);
            return 77;
        }
        fclose(file);
    }

    for (file_index = 0U;
         file_index < sizeof(corpus) / sizeof(corpus[0]); ++file_index) {
        char path[1024];
        char digest[65];
        char payload_digest[65];
        uint8_t *image = NULL;
        uint8_t *payload = NULL;
        size_t image_size = 0U;
        Nexus_V1_SaturnBkrEntry entries[NEXUS_V1_SATURN_BKR_MAX_ENTRIES];
        Nexus_V1_SaturnBkrReceipt receipt;
        Nexus_V1_SaturnBkrNexusPayloadReceipt payload_receipt;
        int image_hash_valid;
        int parse_valid;
        int payload_hash_valid = 0;
        int malformed_payloads_rejected = 0;
        int path_length = snprintf(path, sizeof(path), "%s/%s", root,
                                   corpus[file_index].filename);
        if (path_length < 0 || (size_t)path_length >= sizeof(path) ||
            !read_file(path, &image, &image_size)) {
            fprintf(stderr, "FAIL: authentic BKR is unavailable: %s\n",
                    corpus[file_index].filename);
            failed = 1;
            continue;
        }
        payload = (uint8_t *)calloc(image_size, 1U);
        memset(entries, 0, sizeof(entries));
        memset(&receipt, 0, sizeof(receipt));
        memset(&payload_receipt, 0, sizeof(payload_receipt));
        image_hash_valid = payload &&
            firestaff_x68k_media_receipt_sha256_hex(
                image, image_size, digest, sizeof(digest)) == 0 &&
            strcmp(digest, corpus[file_index].sha256) == 0;
        parse_valid = payload && nexus_v1_saturn_bkr_parse(
            image, image_size, entries, NEXUS_V1_SATURN_BKR_MAX_ENTRIES,
            payload, image_size, &receipt);
        if (parse_valid && receipt.entry_count == 1U &&
            entries[0].payload_offset + entries[0].payload_bytes <= image_size) {
            payload_hash_valid =
                firestaff_x68k_media_receipt_sha256_hex(
                    payload + entries[0].payload_offset,
                    entries[0].payload_bytes, payload_digest,
                    sizeof(payload_digest)) == 0 &&
                strcmp(payload_digest, corpus[file_index].payload_sha256) == 0;
        }
        if (parse_valid && receipt.entry_count == 1U &&
            entries[0].payload_offset + entries[0].payload_bytes <= image_size) {
            (void)nexus_v1_saturn_bkr_nexus_payload_probe(
                &entries[0], payload + entries[0].payload_offset,
                entries[0].payload_bytes, &payload_receipt);
        }
        if (payload_receipt.valid) {
            uint8_t changed_payload[20480];
            Nexus_V1_SaturnBkrNexusPayloadReceipt rejected_receipt;
            const uint8_t *authentic_payload =
                payload + entries[0].payload_offset;
            uint32_t champion;

            for (champion = 0U; champion < payload_receipt.champion_count;
                 ++champion) {
                if (payload_receipt.champion_stamina_current[champion] !=
                        corpus[file_index].champion_stamina_current ||
                    payload_receipt.champion_stamina_max[champion] !=
                        corpus[file_index].champion_stamina_max) {
                    fprintf(stderr,
                            "FAIL: authentic stamina mismatch: %s champion=%u current=%d max=%d\n",
                            corpus[file_index].filename, champion,
                            payload_receipt.champion_stamina_current[champion],
                            payload_receipt.champion_stamina_max[champion]);
                    failed = 1;
                }
            }

            /* Derive negative cases from the real payload so the regression
             * tests rejection without adding a synthetic Nexus save fixture. */
            memcpy(changed_payload, authentic_payload,
                   sizeof(changed_payload));
            changed_payload[0x5BU] ^= 1U;
            memset(&rejected_receipt, 0, sizeof(rejected_receipt));
            if (!nexus_v1_saturn_bkr_nexus_payload_probe(
                    &entries[0], changed_payload,
                    sizeof(changed_payload), &rejected_receipt) &&
                !rejected_receipt.valid) {
                memcpy(changed_payload, authentic_payload,
                       sizeof(changed_payload));
                changed_payload[0x62U] = 5U;
                memset(&rejected_receipt, 0, sizeof(rejected_receipt));
                if (!nexus_v1_saturn_bkr_nexus_payload_probe(
                        &entries[0], changed_payload,
                        sizeof(changed_payload), &rejected_receipt) &&
                    !rejected_receipt.valid) {
                    memcpy(changed_payload, authentic_payload,
                           sizeof(changed_payload));
                    changed_payload[0x61U] = (uint8_t)corpus[file_index].champion_count;
                    memset(&rejected_receipt, 0, sizeof(rejected_receipt));
                    if (!nexus_v1_saturn_bkr_nexus_payload_probe(
                            &entries[0], changed_payload,
                            sizeof(changed_payload), &rejected_receipt) &&
                        !rejected_receipt.valid) {
                        memcpy(changed_payload, authentic_payload,
                               sizeof(changed_payload));
                        changed_payload[0x61U] = 0xFFU;
                        memset(&rejected_receipt, 0, sizeof(rejected_receipt));
                        if (!nexus_v1_saturn_bkr_nexus_payload_probe(
                                &entries[0], changed_payload,
                                sizeof(changed_payload), &rejected_receipt) &&
                            !rejected_receipt.valid) {
                            malformed_payloads_rejected = 1;
                        }
                    }
                }
            }
        }
        if (!image_hash_valid || !parse_valid ||
            !receipt.valid || receipt.image_bytes != 32768U ||
            receipt.block_bytes != 64U || receipt.block_count != 512U ||
            receipt.entry_count != 1U || receipt.payload_bytes != 20480U ||
            strcmp(entries[0].name, "DMNEXUS__01") != 0 ||
            entries[0].payload_bytes != 20480U ||
            entries[0].allocated_block_count != 354U ||
            entries[0].payload_offset != 0U ||
            !payload_hash_valid || !payload_receipt.valid ||
            payload_receipt.champion_count != corpus[file_index].champion_count ||
            payload_receipt.party_leader_index != corpus[file_index].party_leader_index ||
            payload_receipt.champion_records_offset != 0x11CU ||
            payload_receipt.champion_record_stride != 0xE0U ||
            payload_receipt.champion_records_bytes !=
                corpus[file_index].champion_count * 0xE0U ||
            payload_receipt.declared_payload_span !=
                0x274U + corpus[file_index].champion_count * 0xE0U ||
            !malformed_payloads_rejected || failed) {
            fprintf(stderr,
                    "FAIL: authentic BKR parse/hash mismatch: %s "
                    "image_hash=%d parse=%d valid=%d geometry=%u/%u "
                    "entries=%u payload=%u name=%s allocated=%u "
                    "payload_hash=%d roster=%u/%u leader=%u/%u records=%u+%u span=%u bad_payloads=%d\n",
                    corpus[file_index].filename, image_hash_valid, parse_valid,
                    receipt.valid, receipt.block_bytes, receipt.block_count,
                    receipt.entry_count, receipt.payload_bytes, entries[0].name,
                    entries[0].allocated_block_count, payload_hash_valid,
                    payload_receipt.champion_count,
                    corpus[file_index].champion_count,
                    payload_receipt.party_leader_index,
                    corpus[file_index].party_leader_index,
                    payload_receipt.champion_records_offset,
                    payload_receipt.champion_record_stride,
                    payload_receipt.declared_payload_span,
                    malformed_payloads_rejected);
            failed = 1;
        } else {
            payloads[file_index] = payload;
            payload_sizes[file_index] = entries[0].payload_bytes;
            printf("PASS: %s (%u-byte image, %u-byte payload, %u champions, stamina %d/%d)\n",
                   corpus[file_index].filename, receipt.image_bytes,
                   entries[0].payload_bytes, payload_receipt.champion_count,
                   payload_receipt.champion_stamina_current[0],
                   payload_receipt.champion_stamina_max[0]);
            payload = NULL;
        }
        free(payload);
        free(image);
    }

    for (file_index = 0U;
         file_index < sizeof(corpus) / sizeof(corpus[0]); ++file_index) {
        size_t other;
        if (!payloads[file_index]) continue;
        for (other = file_index + 1U;
             other < sizeof(corpus) / sizeof(corpus[0]); ++other) {
            if (payloads[other] &&
                (payload_sizes[file_index] != payload_sizes[other] ||
                 memcmp(payloads[file_index], payloads[other],
                        payload_sizes[file_index]) == 0)) {
                fprintf(stderr, "FAIL: authentic BKR payloads were not distinct\n");
                failed = 1;
            }
        }
    }
    for (file_index = 0U;
         file_index < sizeof(corpus) / sizeof(corpus[0]); ++file_index) {
        free(payloads[file_index]);
    }
    if (failed) return 1;
    puts("Nexus Saturn BKR real-data parser: PASS (4 authenticated images)");
    return 0;
}
