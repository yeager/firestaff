#include "nexus_v1_sal_corpus_receipt.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        printf("FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static void test_complete_shared_prefix_is_opaque(void) {
    static const uint8_t bank0[] = {0x64, 0x73, 0x70, 0x30, 1, 2, 3, 4};
    static const uint8_t bank1[] = {0x64, 0x73, 0x70, 0x30, 1, 2, 3, 5};
    const uint8_t *banks[NEXUS_V1_AUDIO_LEVEL_COUNT];
    uint32_t sizes[NEXUS_V1_AUDIO_LEVEL_COUNT];
    Nexus_V1_SalSharedPrefixReceipt receipt;
    int i;

    for (i = 0; i < NEXUS_V1_AUDIO_LEVEL_COUNT; ++i) {
        banks[i] = i == 1 ? bank1 : bank0;
        sizes[i] = sizeof(bank0);
    }
    CHECK(nexus_v1_audio_sal_shared_prefix_receipt(banks, sizes, &receipt) ==
              NEXUS_V1_AUDIO_OK,
          "complete SAL bank set emits a receipt");
    CHECK(receipt.expected_bank_count == NEXUS_V1_AUDIO_LEVEL_COUNT &&
              receipt.supplied_bank_count == NEXUS_V1_AUDIO_LEVEL_COUNT &&
              receipt.present_bank_count == NEXUS_V1_AUDIO_LEVEL_COUNT &&
              receipt.missing_bank_count == 0 &&
              receipt.shortest_bank_size == sizeof(bank0) &&
              receipt.shared_prefix_byte_count == 7 &&
              receipt.first_divergent_bank_index == 1 &&
              receipt.first_divergent_offset == 7 && receipt.complete == 1,
          "receipt records the exact shared byte span and first difference");
    CHECK(receipt.codec_or_playback_authorized == 0,
          "a shared SAL byte span does not authorize codec or playback");
}

static void test_short_or_missing_bank_fails_closed(void) {
    static const uint8_t bank[] = {0x64, 0x73, 0x70, 0x30};
    const uint8_t *banks[NEXUS_V1_AUDIO_LEVEL_COUNT];
    uint32_t sizes[NEXUS_V1_AUDIO_LEVEL_COUNT];
    Nexus_V1_SalSharedPrefixReceipt receipt;
    int i;

    for (i = 0; i < NEXUS_V1_AUDIO_LEVEL_COUNT; ++i) {
        banks[i] = bank;
        sizes[i] = sizeof(bank);
    }
    banks[5] = NULL;
    sizes[5] = 0;
    CHECK(nexus_v1_audio_sal_shared_prefix_receipt(banks, sizes, &receipt) ==
              NEXUS_V1_AUDIO_OK,
          "missing bank still emits a negative receipt");
    CHECK(receipt.present_bank_count == 15 && receipt.missing_bank_count == 1 &&
              receipt.shared_prefix_byte_count == 0 && receipt.complete == 0 &&
              receipt.codec_or_playback_authorized == 0,
          "missing bank cannot produce a partial shared-prefix claim");
}

int main(void) {
    test_complete_shared_prefix_is_opaque();
    test_short_or_missing_bank_fails_closed();
    if (failures) {
        printf("test_nexus_v1_sal_shared_prefix_receipt: %d failure(s)\n",
               failures);
        return 1;
    }
    puts("test_nexus_v1_sal_shared_prefix_receipt: PASS");
    return 0;
}
