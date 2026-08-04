#include "dm2_v1_timer_dispatch_wiring_pc34_compat.h"
#include "dm2_v1_timer_ops_pc34_compat.h"
#include <string.h>

/* Adapter: LIGHT (0x46) */
static int handle_light(void *context, const DM2_V1_SourceTimer *timer,
                        uint16_t source_index __attribute__((unused)),
                        DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->global_light || !w->light_table)
        return 0;
    DM2_V1_LightTimerCallbacks cb = {
        .global_light = w->global_light,
        .queue_light_timer = w->queue_light_timer,
        .light_table = w->light_table,
        .light_table_size = w->light_table_size
    };
    dm2_v1_process_timer_light(timer->value_a, &cb, context);
    return 1;
}

/* Adapter: DESTROY_DOOR (0x02) */
static int handle_destroy_door(void *context, const DM2_V1_SourceTimer *timer,
                               uint16_t source_index __attribute__((unused)),
                               DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_tile_byte)
        return 0;
    DM2_V1_DestroyDoorCallbacks cb = {
        .get_tile_byte = w->get_tile_byte,
        .current_map = w->current_map,
        .party_map = w->party_map,
        .redraw_flags = w->redraw_flags
    };
    dm2_v1_process_timer_destroy_door(
        (uint8_t)timer->value_a, (uint8_t)timer->value_b, &cb, context);
    return 1;
}

/* Adapter: RELEASE_DOOR_BUTTON (0x58) */
static int handle_release_door_button(void *context, const DM2_V1_SourceTimer *timer,
                                       uint16_t source_index __attribute__((unused)),
                                       DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    DM2_V1_RecordAddressCallbacks cb = {
        .get_record_address = w->get_record_address
    };
    dm2_v1_process_timer_release_door_button((uint16_t)timer->value_a, &cb, context);
    return 1;
}

/* Adapter: PROCESS_3D (0x3D) */
static int handle_process_3d(void *context, const DM2_V1_SourceTimer *timer,
                              uint16_t source_index __attribute__((unused)),
                              DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->move_record_to)
        return 0;
    DM2_V1_Timer3DCallbacks cb = {
        .move_record_to = w->move_record_to,
        .queue_noise = w->queue_noise
    };
    dm2_v1_process_timer_3d(
        (uint16_t)timer->value_a,
        (uint8_t)(timer->value_b & 0xFF),
        (uint8_t)((timer->value_b >> 8) & 0xFF),
        timer->type, &cb, context);
    return 1;
}

/* Adapter: PROCESS_0E (0x0E) */
static int handle_process_0e(void *context, const DM2_V1_SourceTimer *timer,
                              uint16_t source_index __attribute__((unused)),
                              DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address || !w->alloc_memory)
        return 0;
    DM2_V1_Timer0ECallbacks cb = {
        .get_record_address = w->get_record_address,
        .alloc_memory = w->alloc_memory,
        .dealloc_memory = w->dealloc_memory,
        .set_itemtype = w->set_itemtype,
        .process_item_bonus = w->process_item_bonus,
        .copy_memory = w->copy_memory,
        .get_item_size = w->get_item_size
    };
    dm2_v1_process_timer_0e(
        (uint16_t)timer->value_a, (uint16_t)timer->value_b,
        timer->actor, (uint16_t)timer->reserved, &cb, context);
    return 1;
}

/* Adapter: ORNATE_ANIMATOR (0x55) */
static int handle_ornate_animator(void *context, const DM2_V1_SourceTimer *timer,
                                   uint16_t source_index __attribute__((unused)),
                                   DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address || !w->get_ornate_anim_len)
        return 0;
    DM2_V1_OrnateAnimCallbacks cb = {
        .get_record_address = w->get_record_address,
        .get_ornate_anim_len = w->get_ornate_anim_len,
        .queue_timer = w->queue_ornate_timer
    };
    dm2_v1_continue_ornate_animator(
        (uint16_t)timer->value_a, timer->actor, &cb, context);
    return 1;
}

/* Adapter: TICK_GENERATOR (0x56) */
static int handle_tick_generator(void *context, const DM2_V1_SourceTimer *timer,
                                  uint16_t source_index __attribute__((unused)),
                                  DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address || !w->invoke_actuator)
        return 0;
    DM2_V1_TickGenCallbacks cb = {
        .get_record_address = w->get_record_address,
        .invoke_actuator = w->invoke_actuator,
        .requeue_timer = w->requeue_tick_timer
    };
    DM2_V1_TickGenTimerState ts = {
        .timer_yb = (uint8_t)(timer->value_b & 0xFF),
        .timer_b_bit8 = (uint8_t)((timer->value_b >> 8) & 0x01)
    };
    dm2_v1_continue_tick_generator(
        (uint16_t)timer->value_a, &ts, &cb, context);
    return 1;
}

#define WIRED_COUNT 7

void dm2_v1_timer_dispatch_wiring_init(
    DM2_V1_TimerDispatcher *dispatcher,
    DM2_V1_TimerDispatchWiringContext *wiring_ctx)
{
    if (!dispatcher || !wiring_ctx)
        return;
    dispatcher->context = wiring_ctx;
    memset(dispatcher->handlers, 0, sizeof(dispatcher->handlers));
    memset(dispatcher->actuator_tile, 0, sizeof(dispatcher->actuator_tile));
    dispatcher->tile_class_at = NULL;

    dispatcher->handlers[DM2_V1_TIMER_DESTROY_DOOR] = handle_destroy_door;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_0E] = handle_process_0e;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_3D] = handle_process_3d;
    dispatcher->handlers[DM2_V1_TIMER_LIGHT] = handle_light;
    dispatcher->handlers[DM2_V1_TIMER_ORNATE_ANIMATOR] = handle_ornate_animator;
    dispatcher->handlers[DM2_V1_TIMER_TICK_GENERATOR] = handle_tick_generator;
    dispatcher->handlers[DM2_V1_TIMER_RELEASE_DOOR_BUTTON] = handle_release_door_button;
}

int dm2_v1_timer_dispatch_wiring_count(void)
{
    return WIRED_COUNT;
}
