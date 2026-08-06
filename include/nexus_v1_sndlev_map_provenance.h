#ifndef NEXUS_V1_SNDLEV_MAP_PROVENANCE_H
#define NEXUS_V1_SNDLEV_MAP_PROVENANCE_H

#include "nexus_v1_slev_sal_asset_discovery.h"

#include <stdint.h>

/* DMWeb DecodeSNDLEVxxMAP: retail MAP records start at byte zero.  There is
 * no 24-byte file header; keep the field/macro for receipt compatibility and
 * make its value explicit so admission cannot mistake a fixture header for
 * retail data. */
#define NEXUS_V1_SNDLEV_MAP_HEADER_BYTES 0U
#define NEXUS_V1_SNDLEV_MAP_RECORD_BYTES 8U
#define NEXUS_V1_SNDLEV_MAP_MAX_RECORDS 16U

typedef struct {
    int valid;
    uint64_t source_fnv1a64;
    uint64_t source_byte_count;
    uint64_t header_offset, header_length, header_fnv1a64;
    uint64_t table_offset, table_length, table_fnv1a64;
    uint32_t record_count, terminator_offset;
    int playback_permitted;
} Nexus_V1_SndlevMapProvenanceReceipt;

/* A row is opaque source evidence. Its selector/event bytes are never
 * interpreted or promoted to a playback route. */
typedef struct {
    int valid;
    uint32_t row_index;
    uint32_t row_offset;
    uint32_t row_length;
    uint64_t table_fnv1a64;
    uint64_t row_fnv1a64;
    int playback_permitted;
} Nexus_V1_SndlevMapRowProvenanceReceipt;

int nexus_v1_sndlev_map_provenance_parse(const uint8_t *, uint64_t, uint64_t,
                                         Nexus_V1_SndlevMapProvenanceReceipt *);
int nexus_v1_sndlev_map_provenance_from_direct_identity(
    const Nexus_V1_SlevSalDirectIdentity *, Nexus_V1_SndlevMapProvenanceReceipt *);
int nexus_v1_sndlev_map_row_provenance_parse(const uint8_t *, uint64_t,
    uint64_t, uint32_t, Nexus_V1_SndlevMapRowProvenanceReceipt *);
int nexus_v1_sndlev_map_row_provenance_from_direct_identity(
    const Nexus_V1_SlevSalDirectIdentity *, uint32_t,
    Nexus_V1_SndlevMapRowProvenanceReceipt *);

#endif
