#include "redmcsb_f0450_floppy_force_media_change_detection_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

typedef struct F0450Sentinel {
    uint16_t before;
    uint16_t after;
    uint32_t checksum;
} F0450Sentinel;

static uint32_t sentinel_hash(const F0450Sentinel *sentinel)
{
    return ((uint32_t)sentinel->before << 16) ^
        ((uint32_t)sentinel->after << 1) ^
        sentinel->checksum;
}

static int test_pc34_force_media_change_detection_is_noop(void)
{
    F0450Sentinel sentinel = {0x1234u, 0xabcdU, 0x77889900u};
    const uint32_t before_hash = sentinel_hash(&sentinel);

    F0450_FLOPPY_ForceMediaChangeDetection(0u);
    F0450_FLOPPY_ForceMediaChangeDetection(1u);
    F0450_FLOPPY_ForceMediaChangeDetection(0xffffu);
    CHECK(sentinel_hash(&sentinel) == before_hash);
    CHECK(sentinel.before == 0x1234u);
    CHECK(sentinel.after == 0xabcdu);
    CHECK(sentinel.checksum == 0x77889900u);
    return 0;
}

static int test_pc34_compat_entrypoint_matches_source_named_symbol(void)
{
    F0450Sentinel source_named = {0x5555u, 0xaaaau, 0x13572468u};
    F0450Sentinel pc34_named = source_named;

    F0450_FLOPPY_ForceMediaChangeDetection(2u);
    redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(2u);
    CHECK(memcmp(&source_named, &pc34_named, sizeof(source_named)) == 0);
    return 0;
}

static int test_source_evidence_names_pc34_noop_boundary(void)
{
    const char *evidence =
        redmcsb_f0450_floppy_force_media_change_detection_source_evidence_pc34();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F0450") != 0);
    CHECK(strstr(evidence, "PC 3.4") != 0);
    CHECK(strstr(evidence, "no F0450 state mutation") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_pc34_force_media_change_detection_is_noop() == 0);
    CHECK(test_pc34_compat_entrypoint_matches_source_named_symbol() == 0);
    CHECK(test_source_evidence_names_pc34_noop_boundary() == 0);
    return 0;
}
