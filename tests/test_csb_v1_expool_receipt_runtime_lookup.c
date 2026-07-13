/*
 * CSBWin EXPOOL runtime-consumer receipt regression.
 * A stale appended-tail receipt must block record lookup before any save-owned
 * runtime consumer can observe altered bytes.
 */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++g_failures; }
}

static void put_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void put_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    const uint8_t *payload = (const uint8_t *)1;
    size_t payload_size = 99u;
    const uint32_t record_id = (5u << 24) | (5u << 16);
    const uint32_t bucket = 32u + ((record_id * 0xbb40e62du) >> 27);

    csb_v1_runtime_init(&profile, NULL);
    profile.csbwin_appended_tail_valid = 1;
    profile.csbwin_appended_tail_size = 256u;
    profile.csbwin_appended_tail_preserved_size = 256u;
    memset(profile.csbwin_appended_tail, 0, 256u);
    put_le16(profile.csbwin_appended_tail, 2u, 3u);
    put_le32(profile.csbwin_appended_tail, (size_t)bucket * 4u, 1u);
    put_le32(profile.csbwin_appended_tail, 2u * 4u, record_id);
    profile.csbwin_appended_tail[3u * 4u] = 1u;
    profile.csbwin_appended_tail_fnv1a = 0u; /* stale receipt */

    check(csb_v1_runtime_locate_csbwin_appended_expool_record(
              &profile, record_id, &payload, &payload_size) == 0 &&
              payload == NULL && payload_size == 0u,
          "stale EXPOOL receipt blocks runtime record lookup before consumption");

    csb_v1_runtime_cleanup(&profile);
    return g_failures == 0 ? 0 : 1;
}
