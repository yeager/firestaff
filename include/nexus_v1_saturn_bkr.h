#ifndef NEXUS_V1_SATURN_BKR_H
#define NEXUS_V1_SATURN_BKR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_SATURN_BKR_MAX_IMAGE_BYTES (4U * 1024U * 1024U)
#define NEXUS_V1_SATURN_BKR_MAX_ENTRIES 128U
#define NEXUS_V1_SATURN_BKR_MAX_PARTY 4U

typedef struct {
    char name[12];
    uint8_t language;
    char comment[11];
    uint32_t timestamp_minutes;
    uint32_t payload_bytes;
    uint32_t payload_offset;
    uint32_t first_block;
    uint32_t allocated_block_count;
} Nexus_V1_SaturnBkrEntry;

typedef struct {
    int valid;
    uint32_t image_bytes;
    uint32_t block_bytes;
    uint32_t block_count;
    uint32_t entry_count;
    uint32_t payload_bytes;
} Nexus_V1_SaturnBkrReceipt;

typedef struct {
    int valid;
    uint16_t level;
    uint16_t heading;
    uint16_t x;
    uint16_t y;
} Nexus_V1_SaturnBkrLoaderTupleReceipt;

/* Bounded receipt for the retail DMNEXUS__01 payload. Champion records stay
 * opaque apart from fields supported by retail decompilation and authentic
 * payload/WorkRAM captures. */
typedef struct {
    int valid;
    uint32_t payload_bytes;
    uint32_t champion_count;
    uint32_t party_leader_index;
    /* Retail BKR byte +0x09 maps to champion runtime +0x35. Its domain
     * meaning is unresolved; keep the byte opaque until its consumer is bound. */
    union {
        uint8_t champion_byte_09_runtime_35[NEXUS_V1_SATURN_BKR_MAX_PARTY];
        /* Deprecated source-compatibility alias. The byte remains opaque and
         * is not established as a rune-sequence cursor. */
        uint8_t champion_rune_sequence_cursor[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    };
    int32_t champion_health_current[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_max_health[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_stamina_current[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_stamina_max[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_mana_current[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    int32_t champion_max_mana[NEXUS_V1_SATURN_BKR_MAX_PARTY];
    uint32_t champion_records_offset;
    uint32_t champion_record_stride;
    uint32_t champion_records_bytes;
    uint32_t declared_payload_span;
    /* Retail Continue copies the two payload dwords at +0x10/+0x14 into an
     * eight-byte loader tuple. This receipt preserves those decoded halves;
     * it does not apply them to the Firestaff engine. */
    Nexus_V1_SaturnBkrLoaderTupleReceipt loader_tuple;
} Nexus_V1_SaturnBkrNexusPayloadReceipt;

/* Receipt for one explicitly selected card file. This only validates the
 * container and exposes proven metadata; it does not load or apply game
 * state. */
typedef struct {
    int valid;
    Nexus_V1_SaturnBkrReceipt card;
    Nexus_V1_SaturnBkrEntry entry;
    Nexus_V1_SaturnBkrNexusPayloadReceipt nexus_payload;
} Nexus_V1_SaturnBkrNexusFileReceipt;

/* Parse a Saturn Backup RAM image and copy each game payload, in directory
 * order, to caller-owned storage. The destination must be at least image_size
 * bytes, which bounds the total copied payload for every valid card image.
 * No game-specific payload fields are interpreted. */
int nexus_v1_saturn_bkr_parse(
    const uint8_t *image, size_t image_size,
    Nexus_V1_SaturnBkrEntry *entries, size_t entry_capacity,
    uint8_t *payload_storage, size_t payload_capacity,
    Nexus_V1_SaturnBkrReceipt *out_receipt);

/* Validate the source-relative offsets, roster count, leader index, record
 * stride, and bounded span observed in hash-verified retail payloads. Extract
 * health, stamina, and mana pairs whose payload/runtime mappings are bound by
 * retail SH-2 decompilation. Other champion fields stay opaque. */
int nexus_v1_saturn_bkr_nexus_payload_probe(
    const Nexus_V1_SaturnBkrEntry *entry,
    const uint8_t *payload, size_t payload_size,
    Nexus_V1_SaturnBkrNexusPayloadReceipt *out_receipt);

/* Probe one caller-selected path. The file is read only into bounded memory,
 * and succeeds only when the card contains exactly one valid DMNEXUS__01
 * payload. No card discovery or payload application occurs. */
int nexus_v1_saturn_bkr_probe_nexus_file(
    const char *path,
    Nexus_V1_SaturnBkrNexusFileReceipt *out_receipt);

/* Read the explicitly selected card and copy its single validated
 * DMNEXUS__01 payload to caller-owned storage. The caller supplies capacity;
 * on failure the receipt is cleared and no partial payload is reported.
 * This preserves the retail payload bytes and does not apply them to runtime
 * game state. */
int nexus_v1_saturn_bkr_read_nexus_file(
    const char *path,
    uint8_t *payload_out, size_t payload_capacity,
    Nexus_V1_SaturnBkrNexusFileReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
