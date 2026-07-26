#include "nexus_v1_sndlev_map_provenance.h"

#include <stdio.h>
#include <string.h>

static uint64_t fnv(uint64_t h, const uint8_t *p, size_t n) { size_t i; for(i=0;i<n;++i){h^=p[i];h*=UINT64_C(1099511628211);} return h; }

int nexus_v1_sndlev_map_provenance_parse(const uint8_t *bytes, uint64_t size,
    uint64_t expected, Nexus_V1_SndlevMapProvenanceReceipt *out)
{
    Nexus_V1_SndlevMapProvenanceReceipt r; uint64_t h; uint64_t off;
    if (!out) return 0;
    memset(&r,0,sizeof(r));
    if (!bytes || size > SIZE_MAX || size < NEXUS_V1_SNDLEV_MAP_HEADER_BYTES + 2U || !expected) { *out=r; return 0; }
    h=fnv(UINT64_C(1469598103934665603),bytes,(size_t)size);
    if (h!=expected) { *out=r; return 0; }
    off=NEXUS_V1_SNDLEV_MAP_HEADER_BYTES;
    while (off+2U<=size) {
        if (bytes[off]==0xffU && bytes[off+1U]==0xffU) break;
        if (off+NEXUS_V1_SNDLEV_MAP_RECORD_BYTES>size ||
            (off-NEXUS_V1_SNDLEV_MAP_HEADER_BYTES)/NEXUS_V1_SNDLEV_MAP_RECORD_BYTES >= NEXUS_V1_SNDLEV_MAP_MAX_RECORDS) { *out=r; return 0; }
        off+=NEXUS_V1_SNDLEV_MAP_RECORD_BYTES;
    }
    if (off+2U>size || off==NEXUS_V1_SNDLEV_MAP_HEADER_BYTES) { *out=r; return 0; }
    r.valid=1; r.source_fnv1a64=h; r.source_byte_count=size;
    r.header_length=NEXUS_V1_SNDLEV_MAP_HEADER_BYTES;
    r.header_fnv1a64=fnv(UINT64_C(1469598103934665603),bytes,(size_t)r.header_length);
    r.table_offset=NEXUS_V1_SNDLEV_MAP_HEADER_BYTES; r.table_length=off-r.table_offset;
    r.table_fnv1a64=fnv(UINT64_C(1469598103934665603),bytes+r.table_offset,(size_t)r.table_length);
    r.record_count=(uint32_t)(r.table_length/NEXUS_V1_SNDLEV_MAP_RECORD_BYTES); r.terminator_offset=(uint32_t)off;
    *out=r; return 1;
}

int nexus_v1_sndlev_map_provenance_from_direct_identity(const Nexus_V1_SlevSalDirectIdentity *id, Nexus_V1_SndlevMapProvenanceReceipt *out)
{
    uint8_t bytes[NEXUS_V1_SNDLEV_MAP_HEADER_BYTES+NEXUS_V1_SNDLEV_MAP_MAX_RECORDS*NEXUS_V1_SNDLEV_MAP_RECORD_BYTES+2U]; FILE *f; size_t n;
    if (!out) return 0; if (!id || !id->valid || id->byte_count>sizeof(bytes) || !nexus_v1_slev_sal_direct_identity_still_matches(id) || !(f=fopen(id->direct_path,"rb"))) { memset(out,0,sizeof(*out)); return 0; }
    n=fread(bytes,1U,(size_t)id->byte_count,f);
    if (ferror(f) || fclose(f)!=0 || n!=(size_t)id->byte_count) { memset(out,0,sizeof(*out)); return 0; }
    return nexus_v1_sndlev_map_provenance_parse(bytes,id->byte_count,id->fnv1a64,out);
}

int nexus_v1_sndlev_map_row_provenance_parse(const uint8_t *bytes, uint64_t size,
    uint64_t expected, uint32_t row, Nexus_V1_SndlevMapRowProvenanceReceipt *out)
{
    Nexus_V1_SndlevMapProvenanceReceipt table;
    Nexus_V1_SndlevMapRowProvenanceReceipt receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_sndlev_map_provenance_parse(bytes, size, expected, &table) ||
        row >= table.record_count) {
        *out = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.row_index = row;
    receipt.row_offset = (uint32_t)(
        table.table_offset + row * NEXUS_V1_SNDLEV_MAP_RECORD_BYTES);
    receipt.row_length = NEXUS_V1_SNDLEV_MAP_RECORD_BYTES;
    receipt.table_fnv1a64 = table.table_fnv1a64;
    receipt.row_fnv1a64 = fnv(UINT64_C(1469598103934665603),
                              bytes + receipt.row_offset,
                              receipt.row_length);
    *out = receipt;
    return 1;
}

int nexus_v1_sndlev_map_row_provenance_from_direct_identity(
    const Nexus_V1_SlevSalDirectIdentity *identity, uint32_t row,
    Nexus_V1_SndlevMapRowProvenanceReceipt *out)
{
    uint8_t bytes[NEXUS_V1_SNDLEV_MAP_HEADER_BYTES +
                  NEXUS_V1_SNDLEV_MAP_MAX_RECORDS *
                  NEXUS_V1_SNDLEV_MAP_RECORD_BYTES + 2U];
    FILE *file;
    size_t count;

    if (!out) return 0;
    if (!identity || !identity->valid ||
        identity->byte_count > sizeof(bytes) ||
        !nexus_v1_slev_sal_direct_identity_still_matches(identity) ||
        !(file = fopen(identity->direct_path, "rb"))) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    count = fread(bytes, 1U, (size_t)identity->byte_count, file);
    if (ferror(file) || fclose(file) != 0 ||
        count != (size_t)identity->byte_count) {
        memset(out, 0, sizeof(*out));
        return 0;
    }
    return nexus_v1_sndlev_map_row_provenance_parse(
        bytes, identity->byte_count, identity->fnv1a64, row, out);
}
