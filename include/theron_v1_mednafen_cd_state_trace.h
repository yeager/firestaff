#ifndef THERON_V1_MEDNAFEN_CD_STATE_TRACE_H
#define THERON_V1_MEDNAFEN_CD_STATE_TRACE_H

#include <stddef.h>
#include <stdint.h>

#define THERON_V1_MEDNAFEN_CD_STATE_TRACE_PATH_CAPACITY 512

typedef enum {
    THERON_V1_MEDNAFEN_CD_STATE_TRACE_UNAVAILABLE = 0,
    THERON_V1_MEDNAFEN_CD_STATE_TRACE_REJECTED,
    THERON_V1_MEDNAFEN_CD_STATE_TRACE_READY
} Theron_V1MednafenCdStateTraceStatus;

/* Opaque receipt for the real Mednafen SCSI/CD sidecar. It retains transport
 * identities only; it never publishes a level, object, tile, palette, or
 * runtime handoff. */
typedef struct {
    Theron_V1MednafenCdStateTraceStatus status;
    int source_trace_md5_verified;
    int source_header_verified;
    int raw_mode1_2352_verified;
    int command_sector_binding_verified;
    int semantic_publication_allowed;
    uint32_t source_marker_rows;
    uint32_t scsi_command_count;
    uint32_t requested_sector_count;
    uint32_t raw_sector_count;
    uint32_t sector_binding_count;
    uint32_t cd_irq_count;
    uint32_t register_read_count;
    uint32_t register_write_count;
    uint32_t destination_candidate_count;
    uint32_t first_lba;
    uint32_t last_lba;
    uint32_t first_sector_fnv1a;
    uint32_t last_sector_fnv1a;
    char source_trace_path[THERON_V1_MEDNAFEN_CD_STATE_TRACE_PATH_CAPACITY];
    char source_trace_md5[33];
} Theron_V1MednafenCdStateTraceReceipt;

/* Parse one real Mednafen `*.trace.cd` sidecar. Missing input reports
 * UNAVAILABLE; malformed, reordered, duplicated, or unknown transport rows
 * reject. The parser copies no CD payload bytes. */
int theron_v1_mednafen_cd_state_trace_parse_file(
    const char *path,
    Theron_V1MednafenCdStateTraceReceipt *out);

#endif
