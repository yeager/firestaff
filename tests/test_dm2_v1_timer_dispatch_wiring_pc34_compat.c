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

/* ---- Newly wired: RESURRECTION, PROCESS_CLOUD, STEP_MISSILE,
 * THINK_CREATURE_A/B, ORNATE_NOISE ---- */

static int resurrect_calls = 0;
static void fake_bring_to_life(void *ctx __attribute__((unused)), int16_t actor __attribute__((unused)))
{
    resurrect_calls++;
}

static void test_resurrection_final_phase_fires(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.bring_champion_to_life = fake_bring_to_life;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    resurrect_calls = 0;
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x0D;
    timer.value_b = 0x0000; /* yB = 0: final phase */
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x0D](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(resurrect_calls == 1);
    printf("test_resurrection_final_phase_fires OK\n");
}

static uint8_t cloud_record[8];
static uint8_t *mock_get_cloud_record(void *ctx __attribute__((unused)), uint16_t rw __attribute__((unused)))
{
    return cloud_record;
}
static uint8_t mock_tile_open(void *ctx __attribute__((unused)), int16_t x __attribute__((unused)),
                              int16_t y __attribute__((unused)))
{
    return 0x40; /* class 2: not a wall, not a door */
}

static void test_process_cloud_decays_and_deallocs(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.get_record_address = mock_get_cloud_record;
    wctx.get_tile_value = mock_tile_open;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(cloud_record, 0, sizeof(cloud_record));
    cloud_record[4] = 7; /* cloud type 7: decay -1 per tick */
    cloud_record[2] = 1; /* decay counter = 1 -> hits zero this tick */

    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x19;
    timer.value_a = 0x0000;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x19](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    printf("test_process_cloud_decays_and_deallocs OK\n");
}

static uint8_t missile_record[8];
static uint8_t *mock_get_missile_record(void *ctx __attribute__((unused)), uint16_t rw __attribute__((unused)))
{
    return missile_record;
}
static uint8_t mock_tile_wall(void *ctx __attribute__((unused)), int16_t x __attribute__((unused)),
                              int16_t y __attribute__((unused)))
{
    return 0x00; /* class 0: wall */
}
static int missile_deleted = 0;
static void fake_delete_missile(void *ctx __attribute__((unused)), uint16_t record __attribute__((unused)),
                                int16_t x __attribute__((unused)), int16_t y __attribute__((unused)))
{
    missile_deleted = 1;
}

static const int16_t missile_dx[4] = { 0, 1, 0, -1 };
static const int16_t missile_dy[4] = { -1, 0, 1, 0 };

static void test_step_missile_bounces_off_wall(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.get_record_address = mock_get_missile_record;
    wctx.get_tile_value = mock_tile_wall;
    wctx.delete_missile_record = fake_delete_missile;
    wctx.missile_dx_table = missile_dx;
    wctx.missile_dy_table = missile_dy;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(missile_record, 0, sizeof(missile_record));
    missile_deleted = 0;
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x1E;
    timer.value_a = 0x0505; /* x=5, y=5 */
    timer.value_b = 0x1234;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x1E](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(missile_deleted == 1);
    printf("test_step_missile_bounces_off_wall OK\n");
}

static int think_creature_seen = 0;
static int fake_think_creature_handler(void *context __attribute__((unused)),
                                       const DM2_V1_SourceTimer *timer __attribute__((unused)),
                                       uint16_t source_index __attribute__((unused)),
                                       DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    think_creature_seen++;
    return 1;
}

static void test_think_creature_delegates_to_bound_handler(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.think_creature_handler = fake_think_creature_handler;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    think_creature_seen = 0;
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    timer.type = 0x21;
    int result = dispatcher.handlers[0x21](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    timer.type = 0x22;
    result = dispatcher.handlers[0x22](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(think_creature_seen == 2);

    /* Without a bound handler, the adapter fails closed. */
    DM2_V1_TimerDispatchWiringContext wctx2;
    memset(&wctx2, 0, sizeof(wctx2));
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx2);
    result = dispatcher.handlers[0x21](&wctx2, &timer, 0, &receipt);
    assert(result == 0);
    printf("test_think_creature_delegates_to_bound_handler OK\n");
}

static uint8_t ornate_record[8];
static uint8_t *mock_get_ornate_record(void *ctx __attribute__((unused)), uint16_t rw __attribute__((unused)))
{
    return ornate_record;
}

static void test_ornate_noise_clears_frame_when_inactive(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.get_record_address = mock_get_ornate_record;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(ornate_record, 0xFF, sizeof(ornate_record));
    ornate_record[4] = 0x00; /* inactive: clears frame counter */
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x5A;
    timer.value_a = 0x0000;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x5A](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(ornate_record[3] == 0x00);
    printf("test_ornate_noise_clears_frame_when_inactive OK\n");
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
    test_resurrection_final_phase_fires();
    test_process_cloud_decays_and_deallocs();
    test_step_missile_bounces_off_wall();
    test_think_creature_delegates_to_bound_handler();
    test_ornate_noise_clears_frame_when_inactive();
    printf("All dm2_v1_timer_dispatch_wiring tests passed.\n");
    return 0;
}
