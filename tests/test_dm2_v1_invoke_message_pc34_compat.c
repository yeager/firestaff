/* Test DM2_INVOKE_MESSAGE timer queuing.
 * Validates actor mapping, coordinate packing, and queue integration. */

#include "dm2_v1_invoke_message_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_null_safety(void)
{
    DM2_V1_InvokeMessageReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    assert(dm2_v1_invoke_message(NULL, 0, 0, 0, 0, 0, 0, &receipt) == 0);
    assert(dm2_v1_invoke_message(NULL, 0, 0, 0, 0, 0, 0, NULL) == 0);
    printf("  PASS: null_safety\n");
}

static void test_invalid_map(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_InvokeMessageReceipt receipt;

    dm2_v1_source_timer_queue_init(&queue);
    memset(&receipt, 0, sizeof(receipt));

    assert(dm2_v1_invoke_message(&queue, -1, 0, 0, 0, 0, 0, &receipt) == 0);
    assert(dm2_v1_invoke_message(&queue, 256, 0, 0, 0, 0, 0, &receipt) == 0);

    (void)queue;
    printf("  PASS: invalid_map\n");
}

static void test_actor_mapping(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_InvokeMessageReceipt receipt;

    dm2_v1_source_timer_queue_init(&queue);

    /* sel=0 -> actor=1 */
    memset(&receipt, 0, sizeof(receipt));
    assert(dm2_v1_invoke_message(&queue, 0, 5, 3, 7, 0, 100, &receipt) == 1);
    assert(receipt.valid == 1);
    assert(receipt.actor == 1);
    assert(receipt.type == 4);

    /* sel=1 -> actor=3 */
    memset(&receipt, 0, sizeof(receipt));
    assert(dm2_v1_invoke_message(&queue, 0, 5, 3, 7, 1, 200, &receipt) == 1);
    assert(receipt.actor == 3);

    /* sel=2 -> actor=2 */
    memset(&receipt, 0, sizeof(receipt));
    assert(dm2_v1_invoke_message(&queue, 0, 5, 3, 7, 2, 300, &receipt) == 1);
    assert(receipt.actor == 2);

    /* sel=3 -> actor=0 (default) */
    memset(&receipt, 0, sizeof(receipt));
    assert(dm2_v1_invoke_message(&queue, 0, 5, 3, 7, 3, 400, &receipt) == 1);
    assert(receipt.actor == 0);

    (void)queue;
    printf("  PASS: actor_mapping\n");
}

static void test_coordinate_packing(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_InvokeMessageReceipt receipt;

    dm2_v1_source_timer_queue_init(&queue);
    memset(&receipt, 0, sizeof(receipt));

    assert(dm2_v1_invoke_message(&queue, 3, 0x12, 0x34, 0x56, 0, 500, &receipt) == 1);
    assert(receipt.valid == 1);
    /* value_a = (xa & 0xff) | ((ya & 0xff) << 8) */
    assert((receipt.value_a & 0xff) == 0x12);
    assert(((receipt.value_a >> 8) & 0xff) == 0x34);
    /* value_b = (xb & 0xff) | ((sel & 0xff) << 8) */
    assert((receipt.value_b & 0xff) == 0x56);

    /* Map in ticks_and_map high byte */
    assert((receipt.ticks_and_map >> 24) == 3);

    (void)queue;
    printf("  PASS: coordinate_packing\n");
}

static void test_source_evidence(void)
{
    const char *ev = dm2_v1_invoke_message_source_evidence();
    assert(ev != NULL);
    assert(ev[0] != '\0');
    printf("  PASS: source_evidence\n");
}

int main(void)
{
    printf("test_dm2_v1_invoke_message_pc34_compat:\n");
    test_null_safety();
    test_invalid_map();
    test_actor_mapping();
    test_coordinate_packing();
    test_source_evidence();
    printf("All invoke message tests passed.\n");
    return 0;
}
