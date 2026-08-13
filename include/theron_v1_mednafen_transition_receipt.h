#ifndef THERON_V1_MEDNAFEN_TRANSITION_RECEIPT_H
#define THERON_V1_MEDNAFEN_TRANSITION_RECEIPT_H

#include <stdint.h>

#define THERON_V1_MEDNAFEN_TRANSITION_PATH_CAPACITY 512

typedef enum {
    THERON_V1_MEDNAFEN_TRANSITION_UNAVAILABLE = 0,
    THERON_V1_MEDNAFEN_TRANSITION_REJECTED,
    THERON_V1_MEDNAFEN_TRANSITION_READY
} Theron_V1MednafenTransitionStatus;

/* Same-session transport admission for an explicitly captured original run.
 * The receipt proves authenticated media/loader transport only. Runtime
 * target/spawn/RNG counters are retained as observations, but this type
 * cannot authorize a level, object, tile, creature, RNG, T700 or T900
 * interpretation. */
typedef struct {
    Theron_V1MednafenTransitionStatus status;
    int source_header_verified;
    int pce_module_verified;
    int mode_verified;
    int track02_md5_verified;
    int system_card_md5_verified;
    int transition_observed;
    int transport_verified;
    int semantic_publication_allowed;
    uint64_t input_transactions;
    uint64_t cd_irq_callbacks;
    uint64_t raw_sector_spans;
    uint64_t scsi_read_commands;
    uint64_t scsi_read_sector_bindings;
    uint64_t byte_exact_origin_ram_receipts;
    uint64_t authenticated_cd_ram_receipts;
    uint64_t game_main_ram_e009_dispatches;
    uint64_t main_ram_consumer_reads;
    uint64_t main_ram_target_reads;
    uint64_t main_ram_target_writes;
    uint64_t spawn_consumer_reads;
    uint64_t spawn_entry_b0e5_samples;
    uint64_t rng_consumer_samples;
    uint64_t vdc_vram_snapshot_bytes;
    uint64_t vce_palette_snapshot_bytes;
    uint64_t vdc_io_writes;
    char track02_md5[33];
    char system_card_md5[33];
    char source_trace_path[THERON_V1_MEDNAFEN_TRANSITION_PATH_CAPACITY];
} Theron_V1MednafenTransitionReceipt;

int theron_v1_mednafen_transition_receipt_parse_file(
    const char *path, Theron_V1MednafenTransitionReceipt *out);

#endif
