/*
 * test_dm2_v1_tim_proc_pc34_compat.c — unit tests for DM2 timer
 * processing dispatcher (c_tim_proc.cpp port).
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_tim_proc_pc34_compat.h"

/* ── Test context ───────────────────────────────────────────────── */
typedef struct {
    int timer_queued;
    int map_changed;
    int last_map;
    int hero_hp;
    int champion_brought;
    int record_cut;
    int record_dealloced;
    int cloud_created;
    int sound_processed;
    int weather_updated;
    int poison_processed;
    int creature_thought;
    int noise_queued;
    int light_recalced;
    int record_moved;
    int render_flag;
    int gametick;

    /* Timer queue simulation */
    DM2_V1_TimerRecord pending_timers[8];
    int num_pending;
    int timer_index;
} TestCtx;

static void reset_ctx(TestCtx *c)
{
    memset(c, 0, sizeof(*c));
    c->hero_hp = 100;
    c->gametick = 1000;
}

/* ── Stub callbacks ─────────────────────────────────────────────── */
static bool stub_is_timer_to_proceed(void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    return c->timer_index < c->num_pending;
}

static void stub_get_and_delete_next_timer(DM2_V1_TimerRecord *out, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    if (c->timer_index < c->num_pending)
        *out = c->pending_timers[c->timer_index++];
    else
        memset(out, 0, sizeof(*out));
}

static int16_t stub_queue_timer(const DM2_V1_TimerRecord *tim, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->timer_queued++;
    (void)tim;
    return 0;
}

static void stub_change_current_map_to(int32_t map_id, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->map_changed++;
    c->last_map = map_id;
}

static int16_t stub_get_hero_curHP(int32_t index, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    (void)index;
    return (int16_t)c->hero_hp;
}

static void stub_bring_champion_to_life(int16_t index, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->champion_brought = index + 1;
}

static void stub_cut_record_from(int32_t record, void *unused,
    int16_t x, int16_t y, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->record_cut++;
    (void)record; (void)unused; (void)x; (void)y;
}

static void stub_dealloc_record(int32_t record, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->record_dealloced++;
    (void)record;
}

static int32_t stub_create_cloud(int32_t type, int32_t param, int32_t x,
    int32_t y, int32_t dir, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->cloud_created++;
    (void)type; (void)param; (void)x; (void)y; (void)dir;
    return 0;
}

static void stub_process_sound(int16_t value, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->sound_processed++;
    (void)value;
}

static void stub_update_weather(int32_t param, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->weather_updated++;
    (void)param;
}

static void stub_process_poison(int32_t hero, int32_t amount, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->poison_processed++;
    (void)hero; (void)amount;
}

static void stub_think_creature(int32_t x, int32_t y, int32_t type, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->creature_thought++;
    (void)x; (void)y; (void)type;
}

static void stub_queue_noise_gen1(int32_t a, int8_t b, int8_t c, int16_t d,
    int16_t e, int16_t f, int16_t g, int32_t h, void *ctx)
{
    TestCtx *tc = (TestCtx *)ctx;
    tc->noise_queued++;
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g; (void)h;
}

static void stub_recalc_light_level(void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->light_recalced++;
}

static int32_t stub_move_record_to(int32_t record, int32_t x, int32_t y,
    int32_t dx, int16_t dy, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->record_moved++;
    (void)record; (void)x; (void)y; (void)dx; (void)dy;
    return 0;
}

static void stub_set_render_flag(int32_t value, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->render_flag = value;
}

static int16_t stub_get_current_map(void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    return (int16_t)c->last_map;
}

static int16_t stub_get_party_map(void *ctx)
{
    (void)ctx;
    return 0;
}

static int32_t stub_get_gametick(void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    return c->gametick;
}

static int16_t stub_get_light_table_entry(int32_t index, void *ctx)
{
    (void)ctx;
    /* Simple linear table for testing */
    return (int16_t)(index * 10);
}

static uint8_t stub_get_tile_value(int32_t x, int32_t y, void *ctx)
{
    (void)ctx; (void)x; (void)y;
    /* Return floor tile by default (type 1 = 0x20) */
    return 0x20;
}

static void stub_process_cloud(DM2_V1_TimerRecord *tim, void *ctx)
{
    TestCtx *c = (TestCtx *)ctx;
    c->cloud_created++;
    (void)tim;
}

static void stub_ibmio(void *ctx) { (void)ctx; }

static DM2_V1_TimProcCallbacks make_callbacks(void)
{
    DM2_V1_TimProcCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.is_timer_to_proceed = stub_is_timer_to_proceed;
    cb.get_and_delete_next_timer = stub_get_and_delete_next_timer;
    cb.queue_timer = stub_queue_timer;
    cb.change_current_map_to = stub_change_current_map_to;
    cb.get_hero_curHP = stub_get_hero_curHP;
    cb.bring_champion_to_life = stub_bring_champion_to_life;
    cb.cut_record_from = stub_cut_record_from;
    cb.dealloc_record = stub_dealloc_record;
    cb.create_cloud = stub_create_cloud;
    cb.process_sound = stub_process_sound;
    cb.update_weather = stub_update_weather;
    cb.process_poison = stub_process_poison;
    cb.think_creature = stub_think_creature;
    cb.queue_noise_gen1 = stub_queue_noise_gen1;
    cb.recalc_light_level = stub_recalc_light_level;
    cb.set_render_flag = stub_set_render_flag;
    cb.get_current_map = stub_get_current_map;
    cb.get_party_map = stub_get_party_map;
    cb.get_gametick = stub_get_gametick;
    cb.get_light_table_entry = stub_get_light_table_entry;
    cb.get_tile_value = stub_get_tile_value;
    cb.process_cloud = stub_process_cloud;
    cb.ibmio_user_input_check = stub_ibmio;
    cb.move_record_to = (void *)stub_move_record_to;
    return cb;
}

/* ── Tests ──────────────────────────────────────────────────────── */

static void test_null_safety(void)
{
    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(NULL, NULL, &r);
    assert(r.fail_closed == 1);

    dm2_v1_proceed_timers(NULL, NULL, NULL);

    DM2_V1_ProcessTimer0CReceipt cr;
    dm2_v1_process_timer_0c(0, NULL, NULL, &cr);
    assert(cr.fail_closed == 1);

    DM2_V1_ProcessTimerResurrectionReceipt rr;
    dm2_v1_process_timer_resurrection(NULL, NULL, NULL, &rr);
    /* NULL tim -> early return, no crash */

    DM2_V1_ProcessTimerDestroyDoorReceipt dr;
    dm2_v1_process_timer_destroy_door(NULL, NULL, NULL, &dr);

    DM2_V1_StepDoorReceipt sd;
    dm2_v1_step_door(NULL, NULL, NULL, &sd);

    DM2_V1_StepMissileReceipt mr;
    dm2_v1_step_missile(NULL, NULL, NULL, &mr);

    DM2_V1_ProcessTimer3DReceipt tr;
    dm2_v1_process_timer_3d(NULL, NULL, NULL, &tr);

    DM2_V1_ProcessTimerLightReceipt lr;
    dm2_v1_process_timer_light(NULL, NULL, NULL, &lr);

    DM2_V1_InvokeMessageReceipt imr;
    dm2_v1_invoke_message(0, 0, 0, 0, 0, NULL, NULL, &imr);
    assert(imr.fail_closed == 1);

    DM2_V1_InvokeActuatorReceipt iar;
    dm2_v1_invoke_actuator(NULL, 0, 0, NULL, NULL, &iar);
    assert(iar.fail_closed == 1);

    printf("  PASS: null_safety\n");
}

static void test_compute_flag(void)
{
    DM2_V1_TimProc1DA8Receipt r;

    /* yB=0 -> result=1 (activate) */
    int32_t v = dm2_v1_timproc_compute_flag(0, 0, &r);
    assert(r.valid == 1);
    assert(v == 1);

    /* yB=1 -> result=0 (deactivate) */
    v = dm2_v1_timproc_compute_flag(1, 0, &r);
    assert(v == 0);

    /* yB=2 -> toggle: current=0 -> 1 */
    v = dm2_v1_timproc_compute_flag(2, 0, &r);
    assert(v == 1);

    /* yB=2 -> toggle: current=1 -> 0 */
    v = dm2_v1_timproc_compute_flag(2, 1, &r);
    assert(v == 0);

    /* yB=3 -> unknown -> 0 */
    v = dm2_v1_timproc_compute_flag(3, 0, &r);
    assert(v == 0);

    /* NULL receipt - no crash */
    v = dm2_v1_timproc_compute_flag(0, 0, NULL);
    assert(v == 1);

    printf("  PASS: compute_flag\n");
}

static void test_process_timer_0c_alive(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);
    ctx.hero_hp = 50;

    DM2_V1_ProcessTimer0CReceipt r;
    dm2_v1_process_timer_0c(2, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.hero_flag_set == 1);
    assert(r.hero_index == 2);

    printf("  PASS: process_timer_0c_alive\n");
}

static void test_process_timer_0c_dead(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);
    ctx.hero_hp = 0;

    DM2_V1_ProcessTimer0CReceipt r;
    dm2_v1_process_timer_0c(1, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.hero_flag_set == 0);

    printf("  PASS: process_timer_0c_dead\n");
}

static void test_resurrection_phase0(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.xA = 5;
    tim.yA = 3;
    tim.actor = 2;
    tim.yB = 0; /* phase 0 */

    DM2_V1_ProcessTimerResurrectionReceipt r;
    dm2_v1_process_timer_resurrection(&tim, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.champion_brought_to_life == 1);
    assert(ctx.champion_brought == 3); /* actor + 1 */

    printf("  PASS: resurrection_phase0\n");
}

static void test_resurrection_phase2_cloud(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.xA = 5;
    tim.yA = 3;
    tim.xB = 1;
    tim.actor = 0;
    tim.yB = 2; /* phase 2: create cloud */
    tim.data = 0;

    DM2_V1_ProcessTimerResurrectionReceipt r;
    dm2_v1_process_timer_resurrection(&tim, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.cloud_created == 1);
    assert(r.requeued == 1);
    assert(tim.data == 5);
    assert(tim.yB == 1);
    assert(ctx.cloud_created == 1);
    assert(ctx.timer_queued == 1);

    printf("  PASS: resurrection_phase2_cloud\n");
}

static void test_destroy_door(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.xA = 3;
    tim.yA = 7;

    DM2_V1_ProcessTimerDestroyDoorReceipt r;
    dm2_v1_process_timer_destroy_door(&tim, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.door_destroyed == 1);

    printf("  PASS: destroy_door\n");
}

static void test_process_timer_3d(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_3D;
    tim.xA = 4;
    tim.yA = 6;
    tim.valueB = 0x1234;

    DM2_V1_ProcessTimer3DReceipt r;
    dm2_v1_process_timer_3d(&tim, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.noise_queued == 1);

    printf("  PASS: process_timer_3d\n");
}

static void test_process_timer_light(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.valueA = 3; /* positive light value with steps remaining */

    DM2_V1_ProcessTimerLightReceipt r;
    dm2_v1_process_timer_light(&tim, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.light_changed == 1);
    assert(r.requeued == 1);
    assert(ctx.timer_queued == 1);

    printf("  PASS: process_timer_light\n");
}

static void test_process_timer_light_zero(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.valueA = 0; /* zero -> immediate return */

    DM2_V1_ProcessTimerLightReceipt r;
    dm2_v1_process_timer_light(&tim, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.light_changed == 0);
    assert(ctx.timer_queued == 0);

    printf("  PASS: process_timer_light_zero\n");
}

static void test_invoke_message(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_InvokeMessageReceipt r;
    dm2_v1_invoke_message(5, 3, 2, 1, 500, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.message_invoked == 1);
    assert(ctx.timer_queued == 1);

    printf("  PASS: invoke_message\n");
}

static void test_invoke_actuator(void)
{
    /* Build a minimal actuator record */
    uint8_t addr[8];
    memset(addr, 0, sizeof(addr));
    /* byte 4-5: delay field */
    addr[4] = 0x20; /* some delay value */
    addr[5] = 0x00;
    /* byte 6-7: target x=5, y=3, dir=1 */
    /* word6 = (dir << 10) | (y << 5) | x = (1 << 10) | (3 << 5) | 5 = 0x465 */
    addr[6] = 0x65;
    addr[7] = 0x04;

    DM2_V1_TimProcCallbacks cb = make_callbacks();
    /* invoke_message callback not wired (different signature);
     * test only that actuator_invoked is set when invoke_message is NULL */

    TestCtx ctx;
    reset_ctx(&ctx);

    /* Without invoke_message callback, actuator still parses but does not invoke */
    DM2_V1_InvokeActuatorReceipt r;
    dm2_v1_invoke_actuator(addr, 0, 0, &cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.actuator_invoked == 1);

    printf("  PASS: invoke_actuator\n");
}

static void test_proceed_timers_empty(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);
    ctx.num_pending = 0;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.timers_processed == 0);

    printf("  PASS: proceed_timers_empty\n");
}

static void test_proceed_timers_sound(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_SOUND;
    tim.valueA = 42;

    ctx.pending_timers[0] = tim;
    ctx.num_pending = 1;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.timers_processed == 1);
    assert(ctx.sound_processed == 1);

    printf("  PASS: proceed_timers_sound\n");
}

static void test_proceed_timers_weather(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_WEATHER;

    ctx.pending_timers[0] = tim;
    ctx.num_pending = 1;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.timers_processed == 1);
    assert(ctx.weather_updated == 1);

    printf("  PASS: proceed_timers_weather\n");
}

static void test_proceed_timers_cloud(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_CLOUD;

    ctx.pending_timers[0] = tim;
    ctx.num_pending = 1;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(ctx.cloud_created == 1);

    printf("  PASS: proceed_timers_cloud\n");
}

static void test_proceed_timers_think(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_THINK_21;
    tim.xA = 3;
    tim.yA = 5;

    ctx.pending_timers[0] = tim;
    ctx.num_pending = 1;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(ctx.creature_thought == 1);

    printf("  PASS: proceed_timers_think\n");
}

static void test_proceed_timers_poison(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_POISON;
    tim.actor = 2;
    tim.valueA = 15;

    ctx.pending_timers[0] = tim;
    ctx.num_pending = 1;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(ctx.poison_processed == 1);

    printf("  PASS: proceed_timers_poison\n");
}

static void test_proceed_timers_light(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord tim;
    memset(&tim, 0, sizeof(tim));
    tim.type = DM2_TIMER_TYPE_LIGHT;
    tim.valueA = 2;

    ctx.pending_timers[0] = tim;
    ctx.num_pending = 1;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(ctx.light_recalced == 1);

    printf("  PASS: proceed_timers_light\n");
}

static void test_proceed_timers_multiple(void)
{
    DM2_V1_TimProcCallbacks cb = make_callbacks();
    TestCtx ctx;
    reset_ctx(&ctx);

    DM2_V1_TimerRecord t1, t2, t3;
    memset(&t1, 0, sizeof(t1));
    memset(&t2, 0, sizeof(t2));
    memset(&t3, 0, sizeof(t3));

    t1.type = DM2_TIMER_TYPE_SOUND;
    t1.valueA = 1;
    t2.type = DM2_TIMER_TYPE_WEATHER;
    t3.type = DM2_TIMER_TYPE_CLOUD;

    ctx.pending_timers[0] = t1;
    ctx.pending_timers[1] = t2;
    ctx.pending_timers[2] = t3;
    ctx.num_pending = 3;

    DM2_V1_ProceedTimersReceipt r;
    dm2_v1_proceed_timers(&cb, &ctx, &r);
    assert(r.valid == 1);
    assert(r.timers_processed == 3);
    assert(ctx.sound_processed == 1);
    assert(ctx.weather_updated == 1);
    assert(ctx.cloud_created == 1);

    printf("  PASS: proceed_timers_multiple\n");
}

static void test_direction_tables(void)
{
    assert(dm2_v1_tim_proc_dir_dx[0] == 0);
    assert(dm2_v1_tim_proc_dir_dy[0] == -1);
    assert(dm2_v1_tim_proc_dir_dx[1] == 1);
    assert(dm2_v1_tim_proc_dir_dy[1] == 0);
    assert(dm2_v1_tim_proc_dir_dx[2] == 0);
    assert(dm2_v1_tim_proc_dir_dy[2] == 1);
    assert(dm2_v1_tim_proc_dir_dx[3] == -1);
    assert(dm2_v1_tim_proc_dir_dy[3] == 0);

    printf("  PASS: direction_tables\n");
}

static void test_timer_type_enum(void)
{
    assert(DM2_TIMER_TYPE_STEP_DOOR == 0x01);
    assert(DM2_TIMER_TYPE_DESTROY_DOOR == 0x02);
    assert(DM2_TIMER_TYPE_ACTUATE == 0x04);
    assert(DM2_TIMER_TYPE_0C == 0x0C);
    assert(DM2_TIMER_TYPE_RESURRECTION == 0x0D);
    assert(DM2_TIMER_TYPE_LIGHT == 0x46);
    assert(DM2_TIMER_TYPE_WEATHER == 0x54);
    assert(DM2_TIMER_TYPE_POISON == 0x4B);

    printf("  PASS: timer_type_enum\n");
}

int main(void)
{
    printf("dm2_v1_tim_proc_pc34_compat tests:\n");

    test_null_safety();
    test_compute_flag();
    test_process_timer_0c_alive();
    test_process_timer_0c_dead();
    test_resurrection_phase0();
    test_resurrection_phase2_cloud();
    test_destroy_door();
    test_process_timer_3d();
    test_process_timer_light();
    test_process_timer_light_zero();
    test_invoke_message();
    test_invoke_actuator();
    test_proceed_timers_empty();
    test_proceed_timers_sound();
    test_proceed_timers_weather();
    test_proceed_timers_cloud();
    test_proceed_timers_think();
    test_proceed_timers_poison();
    test_proceed_timers_light();
    test_proceed_timers_multiple();
    test_direction_tables();
    test_timer_type_enum();

    printf("All 22 tests passed.\n");
    return 0;
}
