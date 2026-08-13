#include "dm2_v1_timer_dispatch_wiring_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_wiring_count(void)
{
    assert(dm2_v1_timer_dispatch_wiring_count() == 28);
    printf("test_wiring_count OK\n");
}

static void test_init_populates_handlers(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));

    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    static const int wired_types[] = {
        0x01, 0x02, 0x0C, 0x0D, 0x0E, 0x15, 0x19, 0x1D, 0x1E, 0x21, 0x22,
        0x3C, 0x3D, 0x46, 0x47, 0x48, 0x4B, 0x54, 0x55, 0x56, 0x58, 0x59,
        0x5A, 0x5B, 0x5C, 0x5D, 0x5E
    };
    for (size_t i = 0; i < sizeof(wired_types) / sizeof(wired_types[0]); i++) {
        assert(dispatcher.handlers[wired_types[i]] != NULL);
    }

    /* 0x04 ACTUATE_TILE is wired via actuator_tile[]/tile_class_at, not
     * handlers[0x04]. */
    assert(dispatcher.handlers[0x04] == NULL);
    assert(dispatcher.actuator_tile[0] != NULL);
    assert(dispatcher.actuator_tile[1] != NULL);
    assert(dispatcher.actuator_tile[2] != NULL);
    assert(dispatcher.actuator_tile[3] == NULL); /* class 3 is no-op */
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

static int process_3d_move_result;
static int process_3d_move_calls;
static uint16_t process_3d_record;
static int16_t process_3d_level;
static int16_t process_3d_unused;
static int16_t process_3d_x;
static int16_t process_3d_y;
static int process_3d_noise_calls;
static int16_t process_3d_noise_x;
static int16_t process_3d_noise_y;

static int fake_process_3d_move(
    void *ctx __attribute__((unused)), uint16_t record,
    int16_t level, int16_t unused, int16_t x, int16_t y)
{
    ++process_3d_move_calls;
    process_3d_record = record;
    process_3d_level = level;
    process_3d_unused = unused;
    process_3d_x = x;
    process_3d_y = y;
    return process_3d_move_result;
}

static void fake_process_3d_noise(
    void *ctx __attribute__((unused)), int16_t x, int16_t y)
{
    ++process_3d_noise_calls;
    process_3d_noise_x = x;
    process_3d_noise_y = y;
}

static void test_process_3d_payload_and_noise_contract(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    DM2_V1_SourceTimer timer;
    DM2_V1_ProceedTimersReceipt receipt;

    memset(&wctx, 0, sizeof(wctx));
    wctx.move_record_to = fake_process_3d_move;
    wctx.queue_noise = fake_process_3d_noise;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    process_3d_move_result = 1;
    process_3d_move_calls = 0;
    process_3d_noise_calls = 0;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x3c;
    timer.value_a = (int16_t)0x2a19; /* source setxyA: x=0x19, y=0x2a */
    timer.value_b = (int16_t)0x4c21; /* source setxyB/B: record handle */
    memset(&receipt, 0, sizeof(receipt));
    assert(dispatcher.handlers[0x3d] != NULL);
    assert(dispatcher.handlers[0x3c] == dispatcher.handlers[0x3d]);
    assert(dispatcher.handlers[0x3c](&wctx, &timer, 0, &receipt) == 1);
    assert(process_3d_move_calls == 1 &&
           process_3d_record == 0x4c21u && process_3d_level == -3 &&
           process_3d_unused == 0 && process_3d_x == 0x19 &&
           process_3d_y == 0x2a && process_3d_noise_calls == 1 &&
           process_3d_noise_x == 0x19 && process_3d_noise_y == 0x2a);

    process_3d_move_result = -1;
    process_3d_move_calls = 0;
    process_3d_noise_calls = 0;
    timer.type = 0x3d;
    memset(&receipt, 0, sizeof(receipt));
    assert(dispatcher.handlers[0x3d](&wctx, &timer, 0, &receipt) == 1);
    assert(process_3d_move_calls == 1 && process_3d_record == 0x4c21u &&
           process_3d_x == 0x19 && process_3d_y == 0x2a &&
           process_3d_noise_calls == 1);

    printf("test_process_3d_payload_and_noise_contract OK\n");
}

static int rotate_move_calls;
static uint16_t rotate_move_record;
static int16_t rotate_move_level;
static int16_t rotate_move_unused;
static int16_t rotate_move_x;
static int16_t rotate_move_y;
static int rotate_party_calls;
static int16_t rotate_party_direction;

static int fake_rotate_move(
    void *ctx __attribute__((unused)), uint16_t record,
    int16_t level, int16_t unused, int16_t x, int16_t y)
{
    ++rotate_move_calls;
    rotate_move_record = record;
    rotate_move_level = level;
    rotate_move_unused = unused;
    rotate_move_x = x;
    rotate_move_y = y;
    return 1;
}

static void fake_party_rotate(
    void *ctx __attribute__((unused)), int16_t direction)
{
    ++rotate_party_calls;
    rotate_party_direction = direction;
}

static void test_move_record_rotate_source_map_and_party_owner(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    DM2_V1_SourceTimer timer;
    DM2_V1_ProceedTimersReceipt receipt;

    memset(&wctx, 0, sizeof(wctx));
    wctx.current_map = 1; /* must not override the timer's source map */
    wctx.party_map = 5;
    wctx.party_x = 7;
    wctx.party_y = 8;
    wctx.move_record_to = fake_rotate_move;
    wctx.party_rotate = fake_party_rotate;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(&timer, 0, sizeof(timer));
    timer.type = 0x5d;
    timer.ticks_and_map = (5u << 24) | 100u;
    timer.value_a = (int16_t)0x0123;
    rotate_move_calls = 0;
    rotate_party_calls = 0;
    memset(&receipt, 0, sizeof(receipt));
    assert(dispatcher.handlers[0x5d](&wctx, &timer, 0, &receipt) == 1);
    assert(rotate_move_calls == 1 && rotate_move_record == 0xffffu &&
           rotate_move_level == 7 && rotate_move_unused == 8 &&
           rotate_move_x == (0x0123 & 0x1f) &&
           rotate_move_y == ((0x0123 << 6) >> 11) &&
           rotate_party_calls == 1 &&
           rotate_party_direction == ((0x0123 << 4) >> 14));

    memset(&wctx, 0, sizeof(wctx));
    wctx.party_map = 5;
    wctx.party_rotate = fake_party_rotate;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);
    assert(dispatcher.handlers[0x5d](&wctx, &timer, 0, &receipt) == 0);
    printf("test_move_record_rotate_source_map_and_party_owner OK\n");
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
static uint16_t cloud_record_handle;
static int cloud_owner_queued;
static uint8_t *mock_get_cloud_record(void *ctx __attribute__((unused)), uint16_t rw)
{
    cloud_record_handle = rw;
    return cloud_record;
}
static uint8_t mock_tile_open(void *ctx __attribute__((unused)), int16_t x __attribute__((unused)),
                              int16_t y __attribute__((unused)))
{
    return 0x40; /* class 2: not a wall, not a door */
}

static int16_t mock_cloud_creature_none(void *ctx __attribute__((unused)),
                                        int16_t x __attribute__((unused)),
                                        int16_t y __attribute__((unused)))
{
    return -1;
}
static uint16_t mock_cloud_tile_record_link(void *ctx __attribute__((unused)),
                                            int16_t x __attribute__((unused)),
                                            int16_t y __attribute__((unused)))
{
    return 0xFFFEu;
}
static void mock_cloud_owner_attack_door(void *ctx __attribute__((unused)),
                                         int16_t x __attribute__((unused)),
                                         int16_t y __attribute__((unused)),
                                         int16_t damage __attribute__((unused)),
                                         int16_t mode __attribute__((unused)),
                                         int16_t extra __attribute__((unused)))
{
}
static void mock_cloud_owner_cut(void *ctx __attribute__((unused)),
                                 uint16_t record __attribute__((unused)),
                                 int16_t x __attribute__((unused)),
                                 int16_t y __attribute__((unused)))
{
}
static void mock_cloud_owner_dealloc(void *ctx __attribute__((unused)),
                                     uint16_t record __attribute__((unused)))
{
}
static void mock_cloud_owner_noise(void *ctx __attribute__((unused)),
                                   uint8_t type __attribute__((unused)),
                                   uint8_t cloud_type __attribute__((unused)),
                                   uint8_t a __attribute__((unused)),
                                   uint8_t b __attribute__((unused)),
                                   int16_t x __attribute__((unused)),
                                   int16_t y __attribute__((unused)),
                                   int16_t c __attribute__((unused)),
                                   int16_t d __attribute__((unused)),
                                   int16_t intensity __attribute__((unused)))
{
}
static void mock_cloud_owner_queue(void *ctx __attribute__((unused)),
                                   DM2_V1_CloudTimer *timer __attribute__((unused)))
{
    cloud_owner_queued = 1;
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
    timer.value_b = 0x2c01;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.handlers[0x19](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(cloud_record_handle == 0x2c01);
    printf("test_process_cloud_decays_and_deallocs OK\n");
}

static void test_process_cloud_full_owner_wiring(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    DM2_V1_CloudCallbacks owner;
    memset(&wctx, 0, sizeof(wctx));
    memset(&owner, 0, sizeof(owner));
    owner.get_address_of_record = mock_get_cloud_record;
    owner.get_tile_value = mock_tile_open;
    owner.get_tile_record_link = mock_cloud_tile_record_link;
    owner.attack_door = mock_cloud_owner_attack_door;
    owner.get_creature_at = mock_cloud_creature_none;
    owner.cut_record_from = mock_cloud_owner_cut;
    owner.dealloc_record = mock_cloud_owner_dealloc;
    owner.queue_timer = mock_cloud_owner_queue;
    owner.queue_noise_gen2 = mock_cloud_owner_noise;
    owner.current_map = 1;
    owner.party_map = 2;
    wctx.cloud_owner = &owner;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    memset(cloud_record, 0, sizeof(cloud_record));
    cloud_record[2] = 0x07;
    cloud_record[3] = 0x06;
    cloud_owner_queued = 0;
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.type = 0x19;
    timer.value_a = 0x0403;
    timer.value_b = 0x2c01;
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    assert(dispatcher.handlers[0x19](&wctx, &timer, 0, &receipt) == 1);
    assert(cloud_record_handle == 0x2c01);
    assert(cloud_owner_queued == 1);
    assert((uint16_t)(cloud_record[2] | (cloud_record[3] << 8)) == 0x0307);
    printf("test_process_cloud_full_owner_wiring OK\n");
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

static int wall_mecha_seen = 0;
static int fake_wall_mecha_handler(void *context __attribute__((unused)),
                                   const DM2_V1_SourceTimer *timer __attribute__((unused)),
                                   uint16_t source_index __attribute__((unused)),
                                   DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    wall_mecha_seen++;
    return 1;
}

static int floor_mecha_seen = 0;
static int fake_floor_mecha_handler(void *context __attribute__((unused)),
                                    const DM2_V1_SourceTimer *timer __attribute__((unused)),
                                    uint16_t source_index __attribute__((unused)),
                                    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    floor_mecha_seen++;
    return 1;
}

static int fake_tile_class_wall(void *ctx __attribute__((unused)),
                                int map __attribute__((unused)),
                                int x __attribute__((unused)),
                                int y __attribute__((unused)))
{
    return 0;
}

static void test_wall_floor_mecha_delegates_to_bound_handler(void)
{
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_TimerDispatchWiringContext wctx;
    memset(&wctx, 0, sizeof(wctx));
    wctx.wall_mecha_handler = fake_wall_mecha_handler;
    wctx.floor_mecha_handler = fake_floor_mecha_handler;
    wctx.tile_class_at = fake_tile_class_wall;
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx);

    wall_mecha_seen = 0;
    floor_mecha_seen = 0;
    DM2_V1_SourceTimer timer;
    memset(&timer, 0, sizeof(timer));
    DM2_V1_ProceedTimersReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int result = dispatcher.actuator_tile[0](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(wall_mecha_seen == 1);

    result = dispatcher.actuator_tile[1](&wctx, &timer, 0, &receipt);
    assert(result == 1);
    assert(floor_mecha_seen == 1);

    /* Without bound handlers, fail closed */
    DM2_V1_TimerDispatchWiringContext wctx2;
    memset(&wctx2, 0, sizeof(wctx2));
    dm2_v1_timer_dispatch_wiring_init(&dispatcher, &wctx2);
    result = dispatcher.actuator_tile[0](&wctx2, &timer, 0, &receipt);
    assert(result == 0);
    result = dispatcher.actuator_tile[1](&wctx2, &timer, 0, &receipt);
    assert(result == 0);

    printf("test_wall_floor_mecha_delegates_to_bound_handler OK\n");
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
    test_process_3d_payload_and_noise_contract();
    test_move_record_rotate_source_map_and_party_owner();
    test_resurrection_final_phase_fires();
    test_process_cloud_decays_and_deallocs();
    test_process_cloud_full_owner_wiring();
    test_step_missile_bounces_off_wall();
    test_think_creature_delegates_to_bound_handler();
    test_ornate_noise_clears_frame_when_inactive();
    test_wall_floor_mecha_delegates_to_bound_handler();
    printf("All dm2_v1_timer_dispatch_wiring tests passed.\n");
    return 0;
}
