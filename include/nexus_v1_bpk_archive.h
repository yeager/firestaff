#ifndef NEXUS_V1_BPK_ARCHIVE_H
#define NEXUS_V1_BPK_ARCHIVE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_BPK_MAGIC_BPPK 0x4250504BU
#define NEXUS_V1_BPK_MAGIC_BMPD 0x424D5044U
#define NEXUS_V1_BPK_MAGIC_PRS3 0x50525333U
#define NEXUS_V1_BPK_ENTRY_PREFIX_BYTES 20U

typedef struct {
    uint32_t outer_size;
    uint32_t bmpd_size;
    uint32_t entry_count_hint;
    uint32_t candidate_offset_count;
    uint32_t first_candidate_offset;
    uint32_t last_candidate_offset;
    uint32_t prs3_payload_count;
    uint32_t raw_payload_count;
} Nexus_V1_BpkArchiveInfo;

typedef struct {
    uint32_t offset;
    uint32_t next_offset;
    uint32_t stored_size;
    uint32_t payload_offset;
    uint32_t payload_size;
    int has_prs3;
} Nexus_V1_BpkEntry;

/*
 * Parse the DM Nexus MENU.BPK BPPK/BMPD directory shape.
 *
 * Scope: this is an archive/directory validator only. It records candidate
 * payload spans and detects PRS3-tagged payloads, but it deliberately does
 * not decompress PRS3 yet. ReDMCSB has no Saturn/Nexus implementation; this
 * is source-locked to the observed Nexus MENU.BPK file and the documented
 * "MENU.BPK packed graphics not analyzed" gap in
 * docs/nexus_v1_phase2_data_formats_H2321.md.
 */
int nexus_v1_bpk_archive_parse(const uint8_t *data,
                               size_t data_size,
                               Nexus_V1_BpkArchiveInfo *out_info);

int nexus_v1_bpk_archive_get_entry(const uint8_t *data,
                                   size_t data_size,
                                   uint32_t index,
                                   Nexus_V1_BpkEntry *out_entry);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_BPK_ARCHIVE_H */
