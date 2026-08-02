/* Test DM2 WRITE_POSSESSION_INDICES. */
#include "dm2_v1_save_write_possession_indices_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int mock_resolve(void *ctx, uint16_t link) {
    (void)ctx;
    int type = (link & 0x3C00) >> 10;
    if (type == 9) return 42;
    if (type == 0xe) return 7;
    return -1;
}

int main(void) {
    DM2_WriteRecordSession session;
    DM2_WritePossessionCallbacks cb;
    uint8_t buf[256];
    int creature_idx[16], container_idx[16];

    cb.resolve_possession_index = mock_resolve;
    cb.ctx = NULL;

    /* Test 1: empty array. */
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    int rc = dm2_v1_write_possession_indices(&session, &cb, NULL, 0);
    assert(rc == 0);

    /* Test 2: single type-9 container link. */
    uint16_t links[3];
    links[0] = (9 << 10) | 5;
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_possession_indices(&session, &cb, links, 1);
    assert(rc == 0);

    /* Test 3: mixed types — only 9 and 0xe should be processed. */
    links[0] = (0 << 10) | 1;
    links[1] = (9 << 10) | 2;
    links[2] = (0xe << 10) | 3;
    dm2_v1_write_record_session_init(&session, buf, sizeof(buf),
        creature_idx, 16, container_idx, 16, NULL, 0);
    rc = dm2_v1_write_possession_indices(&session, &cb, links, 3);
    assert(rc == 0);

    /* Test 4: NULL params. */
    rc = dm2_v1_write_possession_indices(NULL, &cb, links, 3);
    assert(rc == -1);

    printf("PASS: dm2_v1_save_write_possession_indices\n");
    return 0;
}
