#include "dm2_v1_timer_dispatch_wiring_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_wiring_count(void)
{
    assert(dm2_v1_timer_dispatch_wiring_count() == 26);
    printf("test_wiring_count OK\n");
}

static void test_init_populates_handlers(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));

    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    static const int wired_types[] = {
        0x01, 0x02, 0x0C, 0x0D, 0x0E, 0x15, 0x19, 0x1E, 0x21, 0x22,
        0x3D, 0x46, 0x47, 0x48, 0x4B, 0x54, 0x55, 0x56, 0x58, 0x59,
        0x5A, 0x5B, 0x5C, 0x5D, 0x5E
    };
    for (size_t i = 0; i < sizeof(wired_types) / sizeof(wired_types[0]); i++) {
        assert(dispatcher.handlers[wired_types[i]] != NULL);
    }

    /* 0x04 ACTUATE_TILE is wired via actuator_tile[]/tile_class_at, not
     * handlers[0x04]. */
    assert(dispatcher.handlers[0x04] == NULL);
    assert(dispatcher.actuator_tile[2] != NULL);
    assert(dispatcher.actuator_tile[4] != NULL);
    assert(dispatcher.actuator_tile[5] != NULL);
    assert(dispatcher.actuator_tile[6] != NULL);

    assert(dispatcher.handlers[0x03] == NULL);
    assert(dispatcher.handlers[0x05] == NULL);

    assert(dispatcher.context == &wctx);
    printf("test_init_populates_handlers OK\n");
}

static uint8_t mock_record[8];
static uint8_t *mock_get_record(void *ctx __attribute__((unused)), uint16_t rw __attribute__((unused)))
{
    return mock_record;
}

static void test_5b_record_clear_fires(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.get_record_address = mock_get_record;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(mock_record, 0xFF, sizeof(mock_record));
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x5B;
    timer.value_a = 0x1234;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x5B](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert((mock_record[4] & 0x01) == 0);
    printf("test_5b_record_clear_fires OK\n");
}

static int fake_tile_class_at(void *ctx __attribute__((unused)),
                              int map __attribute__((unused)),
                              int x __attribute__((unused)),
                              int y __attribute__((unused)))
{
    return 4; /* door class */
}

static int door_fired = 0;
static void fake_actuate_door(int16_t x __attribute__((unused)),
                              int16_t y __attribute__((unused)),
                              int16_t param __attribute__((unused)),
                              const DM2_V1_TimerActuatorCallbacks *cb __attribute__((unused)),
                              void *ctx __attribute__((unused)))
{
    door_fired = 1;
}

static void test_actuate_tile_dispatch(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.tile_class_at = fake_tile_class_at;
    wctx.actuate_door_fn = fake_actuate_door;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    assert(dispatcher.tile_class_at == fake_tile_class_at);
    door_fired = 0;
    int result = dispatcher.actuator_tile[4](&wctx, &(DM2_V1_SourceTimer){0}, 0, NULL);
    assert(result == 1);
    assert(door_fired == 1);
    printf("test_actuate_tile_dispatch OK\n");
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
    test_5b_record_clear_fires();
    test_actuate_tile_dispatch();
    test_null_safety();
    test_handler_rejects_missing_callbacks();
    test_release_door_button_fires();
    printf("All dm2_v1_timer_dispatch_wiring tests passed.\n");
    return 0;
}
