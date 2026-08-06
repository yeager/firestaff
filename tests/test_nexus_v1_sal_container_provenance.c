#include "nexus_v1_sal_container_provenance.h"
#include "nexus_v1_slev_sal_asset_discovery.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr, "FAIL: %s\n", message); ++failures; } \
} while (0)

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0U; i < size; ++i) { hash ^= bytes[i]; hash *= UINT64_C(1099511628211); }
    return hash;
}

static void verify_real_european_corpus(void)
{
    const char *directory = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_SlevSalAssetDiscoveryReceipt discovery;
    unsigned level;

    /* User-supplied Saturn data is optional for this focused test. */
    if (!directory || !directory[0]) return;
    CHECK(nexus_v1_slev_sal_assets_discover_direct(directory, &discovery) == 1 &&
          discovery.valid && discovery.direct_files_only &&
          !discovery.payload_materialized,
          "European SLEV/SAL/MAP corpus admits as direct identities only");
    if (!discovery.valid) return;
    for (level = 0U; level < NEXUS_V1_SLEV_SAL_ASSET_LEVEL_COUNT; ++level) {
        Nexus_V1_SalContainerProvenanceReceipt receipt;
        const Nexus_V1_SlevSalDirectLevelIdentity *entry =
            &discovery.levels[level];
        char message[96];

        snprintf(message, sizeof(message),
                 "SNDLEV%02u.SAL real header/hash yields opaque provenance",
                 level);
        CHECK(entry->valid &&
              nexus_v1_sal_container_provenance_from_direct_identity(
                  &entry->sal, &receipt) == 1 && receipt.valid &&
              receipt.source_fnv1a64 == entry->sal.fnv1a64 &&
              receipt.source_byte_count == entry->sal.byte_count &&
              receipt.descriptor_offset ==
                  NEXUS_V1_SAL_CONTAINER_HEADER_BYTES &&
              receipt.descriptor_length ==
                  entry->sal.byte_count - NEXUS_V1_SAL_CONTAINER_HEADER_BYTES &&
              !receipt.codec_proven && !receipt.playback_permitted,
              message);
    }
}

int main(void)
{
    uint8_t bytes[] = { 'd','s','p','0','1','.','E','X', 1,2,3,4,5,6,7,8 };
    Nexus_V1_SalContainerProvenanceReceipt receipt;
    uint64_t hash = fnv1a64(bytes, sizeof(bytes));

    CHECK(nexus_v1_sal_container_provenance_parse(bytes, sizeof(bytes), hash,
                                                   &receipt) == 1 &&
          receipt.valid && receipt.source_fnv1a64 == hash &&
          receipt.source_byte_count == sizeof(bytes) &&
          receipt.descriptor_offset == NEXUS_V1_SAL_CONTAINER_HEADER_BYTES &&
          receipt.descriptor_length == 8U && receipt.descriptor_fnv1a64 ==
              fnv1a64(bytes + 8, 8U) && !receipt.codec_proven &&
          !receipt.playback_permitted,
          "bounded dsp01.EX SAL header produces opaque descriptor provenance");
    bytes[0] = 'x';
    CHECK(nexus_v1_sal_container_provenance_parse(bytes, sizeof(bytes),
                                                   fnv1a64(bytes, sizeof(bytes)),
                                                   &receipt) == 0 && !receipt.valid,
          "unknown SAL magic is rejected");
    bytes[0] = 'd';
    CHECK(nexus_v1_sal_container_provenance_parse(bytes, sizeof(bytes), hash ^ 1U,
                                                   &receipt) == 0 && !receipt.valid,
          "source FNV drift is rejected");
    CHECK(nexus_v1_sal_container_provenance_parse(bytes, 8U, fnv1a64(bytes, 8U),
                                                   &receipt) == 0 && !receipt.valid,
          "header-only SAL has no bounded descriptor interval");
    verify_real_european_corpus();
    if (failures) return 1;
    puts("test_nexus_v1_sal_container_provenance: PASS");
    return 0;
}
