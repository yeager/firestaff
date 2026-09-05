#ifndef NEXUS_V1_TEST_RETAIL_MEMBER_H
#define NEXUS_V1_TEST_RETAIL_MEMBER_H

#include <stddef.h>
#include <stdint.h>

/* Read cue::MEMBER directly from the original data track into RAM and attest
 * the exact member bytes. No game data is materialized on disk. */
uint8_t *nexus_v1_test_read_retail_member(const char *locator,
                                          size_t *out_size,
                                          char out_sha256[65]);

#endif
