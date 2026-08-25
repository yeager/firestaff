#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(14695981039346656037);
    size_t index;
    for (index = 0; index < size; ++index) {
        hash ^= (uint64_t)bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int main(int argc, char **argv)
{
    Nexus_ISOReader iso;
    Nexus_V1_Engine engine;
    Nexus_V1_StabgSourceConsumerReceipt receipt;
    const Nexus_ISOFile *member;
    uint8_t *raw;
    uint64_t raw_hash;
    uint32_t member_size;
    int rc = 1;

    if (argc != 2) return 2;
    memset(&iso, 0, sizeof(iso));
    if (nexus_iso_open_cue(&iso, argv[1]) <= 0 ||
        !(member = nexus_iso_find(&iso, "STABG.BIN")) || member->size == 0U) {
        fprintf(stderr, "FAIL: authentic CUE does not expose STABG.BIN\n");
        nexus_iso_close(&iso);
        return 1;
    }
    member_size = member->size;
    raw = (uint8_t *)malloc(member_size);
    if (!raw || nexus_iso_read_file(&iso, member, raw, (int)member_size) !=
                    (int)member_size) {
        fprintf(stderr, "FAIL: cannot read STABG.BIN from authentic CUE\n");
        free(raw);
        nexus_iso_close(&iso);
        return 1;
    }
    raw_hash = fnv1a64(raw, member_size);
    free(raw);
    nexus_iso_close(&iso);

    memset(&engine, 0, sizeof(engine));
    if (nexus_v1_init(&engine, argv[1]) != 0) {
        fprintf(stderr, "FAIL: native Nexus CUE engine initialization\n");
        return 1;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (nexus_v1_stabg_cue_source_consumer_receipt(&engine, &receipt) != 1 ||
        !receipt.source_is_iso || !receipt.source_member_bound ||
        !receipt.source.canonical_hash_verified || !receipt.stmp_framing_bound ||
        !receipt.dmweb_first_map_bound || !receipt.native_surface_consumer_bound ||
        receipt.source_byte_count != member_size ||
        receipt.source_bytes_fnv1a64 != raw_hash || receipt.width != 320 ||
        receipt.height != 168 || !receipt.no_draw_only ||
        receipt.renderer_permitted || receipt.fallback_visuals_permitted) {
        fprintf(stderr, "FAIL: authentic CUE STABG source-consumer receipt "
                "iso=%d member=%d canonical=%d stmp=%d dmweb=%d consumer=%d "
                "size=%u hash=%016llx w=%d h=%d\n",
                receipt.source_is_iso, receipt.source_member_bound,
                receipt.source.canonical_hash_verified, receipt.stmp_framing_bound,
                receipt.dmweb_first_map_bound, receipt.native_surface_consumer_bound,
                receipt.source_byte_count,
                (unsigned long long)receipt.source_bytes_fnv1a64,
                receipt.width, receipt.height);
        goto done;
    }
    puts("PASS: authentic CUE STABG reaches native source-bound STMP consumer");
    rc = 0;
done:
    nexus_v1_shutdown(&engine);
    return rc;
}
