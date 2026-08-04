#include "dm2_v1_timer_dispatch_wiring_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_wiring_count(void)
{
    assert(dm2_v1_timer_dispatch_wiring_count() == 7);
    printf("test_wiring_count OK\n");
}

static void test_init_populates_handlers(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));

    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    assert(dispatcher.handlers[0x02] != NULL);
    assert(dispatcher.handlers[0x0E] != NULL);
    assert(dispatcher.handlers[0x3D] != NULL);
    assert(dispatcher.handlers[0x46] != NULL);
    assert(dispatcher.handlers[0x55] != NULL);
    assert(dispatcher.handlers[0x56] != NULL);
    assert(dispatcher.handlers[0x58] != NULL);

    assert(dispatcher.handlers[0x01] == NULL);
    assert(dispatcher.handlers[0x04] == NULL);
    assert(dispatcher.handlers[0x15] == NULL);
    assert(dispatcher.handlers[0x1E] == NULL);
    assert(dispatcher.handlers[0x21] == NULL);
    assert(dispatcher.handlers[0x54] == NULL);

    assert(dispatcher.context == &wctx);
    printf("test_init_populates_handlers OK\n");
}

static void test_null_safety(void)
{
    dm2_v1_timer_dispatch_wiring_init(NULL, NULL);
    DM2_V1_TimerDispatcher dispatcher;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, NULL);
    printf("test_null_safety OK\n");
}

static void test_handler_rejects_missing_callbacks(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x46;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x46](&wctx, &timer, 0, &receipt);
    assert(result == 0);

    timer.type = 0x02;
    result = dispatcher.handlers[0x02](&wctx, &timer, 0, &receipt);
    assert(result == 0);

    timer.type = 0x58;
    result = dispatcher.handlers[0x58](&wctx, &timer, 0, &receipt);
    assert(result == 0);

    printf("test_handler_rejects_missing_callbacks OK\n");
}

static uint8_t mock_record[8];
static uint8_t *mock_get_record(void *ctx __attribute__((unused)), uint16_t rw __attribute__((unused)))
{
    return mock_record;
}

static void test_release_door_button_fires(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.get_record_address = mock_get_record;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(mock_record, 0xFF, sizeof(mock_record));
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x58;
    timer.value_a = 0x1234;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x58](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert((mock_record[3] & 0x08) == 0);
    printf("test_release_door_button_fires OK\n");
}

int main(void)
{
    test_wiring_count();
    test_init_populates_handlers();
    test_null_safety();
    test_handler_rejects_missing_callbacks();
    test_release_door_button_fires();
    printf("All dm2_v1_timer_dispatch_wiring tests passed.\n");
    return 0;
}
