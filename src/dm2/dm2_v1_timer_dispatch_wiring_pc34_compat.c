#include "dm2_v1_timer_dispatch_wiring_pc34_compat.h"
#include "dm2_v1_timer_ops_pc34_compat.h"
#include "dm2_v1_runtime_parity_pc34_compat.h"
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

/* Build a DM2_V1_TimerActuatorCallbacks view from the wiring context. */
static DM2_V1_TimerActuatorCallbacks make_actuator_cb(DM2_V1_TimerDispatchWiringContext *w)
{
    DM2_V1_TimerActuatorCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_tile_byte = NULL;
    cb.get_record_address = w->get_record_address;
    cb.queue_noise = w->queue_noise;
    cb.requeue_timer = NULL;
    cb.invoke_actuator = NULL;
    cb.current_map = w->current_map;
    cb.party_map = w->party_map;
    cb.redraw_flags = NULL;
    return cb;
}

/* Adapter: STEP_DOOR (0x01) */
static int handle_step_door(void *context, const DM2_V1_SourceTimer *timer,
                            uint16_t source_index __attribute__((unused)),
                            DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->step_door_fn)
        return 0;
    DM2_V1_TimerActuatorCallbacks cb = make_actuator_cb(w);
    int16_t x = (int16_t)(timer->value_a & 0xff);
    int16_t y = (int16_t)((timer->value_a >> 8) & 0xff);
    w->step_door_fn(x, y, timer->value_b, (int16_t)timer->actor, &cb, context);
    return 1;
}

/* Actuator-tile subdispatch adapters (0x04 classes 2,4,5,6) */
static int handle_actuate_pitfall(void *context, const DM2_V1_SourceTimer *timer,
                                  uint16_t source_index __attribute__((unused)),
                                  DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->actuate_pitfall)
        return 0;
    DM2_V1_TimerActuatorCallbacks cb = make_actuator_cb(w);
    int16_t x = (int16_t)(timer->value_a & 0xff);
    int16_t y = (int16_t)((timer->value_a >> 8) & 0xff);
    w->actuate_pitfall(x, y, timer->value_b, &cb, context);
    return 1;
}

static int handle_actuate_door(void *context, const DM2_V1_SourceTimer *timer,
                               uint16_t source_index __attribute__((unused)),
                               DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->actuate_door_fn)
        return 0;
    DM2_V1_TimerActuatorCallbacks cb = make_actuator_cb(w);
    int16_t x = (int16_t)(timer->value_a & 0xff);
    int16_t y = (int16_t)((timer->value_a >> 8) & 0xff);
    w->actuate_door_fn(x, y, timer->value_b, &cb, context);
    return 1;
}

static int handle_actuate_teleporter(void *context, const DM2_V1_SourceTimer *timer,
                                     uint16_t source_index __attribute__((unused)),
                                     DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->actuate_teleporter_fn)
        return 0;
    DM2_V1_TimerActuatorCallbacks cb = make_actuator_cb(w);
    int16_t x = (int16_t)(timer->value_a & 0xff);
    int16_t y = (int16_t)((timer->value_a >> 8) & 0xff);
    w->actuate_teleporter_fn(x, y, timer->value_b, &cb, context);
    return 1;
}

static int handle_actuate_trickwall(void *context, const DM2_V1_SourceTimer *timer,
                                    uint16_t source_index __attribute__((unused)),
                                    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->actuate_trickwall_fn)
        return 0;
    DM2_V1_TimerActuatorCallbacks cb = make_actuator_cb(w);
    int16_t x = (int16_t)(timer->value_a & 0xff);
    int16_t y = (int16_t)((timer->value_a >> 8) & 0xff);
    w->actuate_trickwall_fn(x, y, timer->value_b, &cb, context);
    return 1;
}

/* Adapter: PROCESS_0C (0x0C) */
static int handle_process_0c(void *context, const DM2_V1_SourceTimer *timer,
                             uint16_t source_index __attribute__((unused)),
                             DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->hero_timeridx || !w->hero_curHP || !w->hero_heroflag)
        return 0;
    int idx = timer->actor;
    if (idx < 0 || idx >= w->heros_in_party)
        return 0;
    DM2_V1_Timer0CCallbacks cb = {
        .hero_timeridx = &w->hero_timeridx[idx],
        .hero_curHP = &w->hero_curHP[idx],
        .hero_heroflag = &w->hero_heroflag[idx]
    };
    dm2_v1_process_timer_0c_cb(timer->actor, &cb);
    return 1;
}

/* Adapter: PROCESS_SOUND (0x15) */
static int handle_process_sound(void *context, const DM2_V1_SourceTimer *timer,
                                uint16_t source_index __attribute__((unused)),
                                DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->play_sound)
        return 0;
    DM2_V1_SoundTimerCallbacks cb = { .play_sound = w->play_sound };
    dm2_v1_process_timer_sound(timer->value_a, &cb, context);
    return 1;
}

/* Adapter: RESURRECTION (0x0D) */
static int handle_resurrection(void *context, const DM2_V1_SourceTimer *timer,
                               uint16_t source_index __attribute__((unused)),
                               DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->bring_champion_to_life && !w->get_tile_record_link && !w->create_cloud)
        return 0;
    DM2_V1_ResurrectionCallbacks cb = {
        .bring_champion_to_life = w->bring_champion_to_life,
        .get_tile_record_link = w->get_tile_record_link,
        .get_next_record_link = w->get_next_record_link,
        .query_cls1_from_record = w->query_cls1_from_record,
        .query_cls2_from_record = w->query_cls2_from_record,
        .add_item_charge = w->add_item_charge,
        .cut_record_from = w->cut_record_from,
        .dealloc_record = w->dealloc_record,
        .create_cloud = w->create_cloud,
        .queue_timer = w->queue_resurrection_timer
    };
    uint8_t xA = (uint8_t)(timer->value_a & 0xff);
    uint8_t yA = (uint8_t)((timer->value_a >> 8) & 0xff);
    uint8_t xB = (uint8_t)(timer->value_b & 0xff);
    uint8_t yB = (uint8_t)((timer->value_b >> 8) & 0xff);
    return dm2_v1_process_timer_resurrection(xA, yA, xB, yB, (uint8_t)timer->actor, &cb, context);
}

/* Adapter: PROCESS_CLOUD (0x19) */
static int handle_process_cloud(void *context, const DM2_V1_SourceTimer *timer,
                                uint16_t source_index __attribute__((unused)),
                                DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    DM2_V1_ProcessCloudCallbacks cb = {
        .get_record_address = w->get_record_address,
        .get_tile_value = w->get_tile_value,
        .calc_cloud_damage = w->calc_cloud_damage,
        .attack_door = w->attack_door,
        .attack_party = w->attack_party,
        .get_creature_at = w->get_creature_at,
        .is_creature_immune = w->is_creature_immune,
        .attack_creature = w->attack_creature,
        .cut_record_from = w->cut_record_from,
        .dealloc_record = w->dealloc_record,
        .queue_timer = w->queue_cloud_timer,
        .queue_noise_gen2 = w->queue_noise_gen2,
        .current_map = w->current_map,
        .party_map = w->party_map,
        .party_x = w->party_x_cloud,
        .party_y = w->party_y_cloud
    };
    uint8_t xA = (uint8_t)(timer->value_a & 0xff);
    uint8_t yA = (uint8_t)((timer->value_a >> 8) & 0xff);
    return dm2_v1_process_cloud((uint16_t)timer->value_a, xA, yA, &cb, context);
}

/* Adapter: STEP_MISSILE (0x1E) */
static int handle_step_missile(void *context, const DM2_V1_SourceTimer *timer,
                               uint16_t source_index __attribute__((unused)),
                               DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    DM2_V1_StepMissileCallbacks cb = {
        .get_record_address = w->get_record_address,
        .get_tile_value = w->get_tile_value,
        .get_creature_at = w->get_creature_at,
        .query_creature_ai_spec_flags = w->query_creature_ai_spec_flags,
        .move_record = w->move_record,
        .cut_record_from = w->cut_record_from,
        .append_record_to = w->append_record_to,
        .delete_missile_record = w->delete_missile_record,
        .attack_creature = w->attack_creature,
        .queue_timer = w->queue_missile_timer,
        .current_map = w->current_map,
        .party_x = w->party_x_cloud,
        .party_y = w->party_y_cloud,
        .dx_table = w->missile_dx_table,
        .dy_table = w->missile_dy_table
    };
    uint8_t x = (uint8_t)(timer->value_a & 0xff);
    uint8_t y = (uint8_t)((timer->value_a >> 8) & 0xff);
    return dm2_v1_step_missile((uint16_t)timer->value_b, x, y, &cb, context);
}

/* Adapter: THINK_CREATURE_A/B (0x21/0x22) — delegates to the host-bound
 * think_creature_handler, normally dm2_v1_think_creature_timer_handler
 * (dm2_v1_think_creature_pc34_compat.h) with think_creature_context set to
 * a DM2_V1_ThinkCreatureBinding*.  Indirected through a plain
 * DM2_V1_TimerTypeHandler so this wiring module does not need to link the
 * record-pool/dungeon-loader dependency chain that binding pulls in. */
static int handle_think_creature(void *context, const DM2_V1_SourceTimer *timer,
                                 uint16_t source_index,
                                 DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->think_creature_handler)
        return 0;
    return w->think_creature_handler(w->think_creature_context, timer, source_index, receipt);
}

/* Adapter: ORNATE_NOISE (0x5A) */
static int handle_ornate_noise(void *context, const DM2_V1_SourceTimer *timer,
                               uint16_t source_index __attribute__((unused)),
                               DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    DM2_V1_OrnateNoiseCallbacks cb = {
        .get_record_address = w->get_record_address,
        .get_tile_value = w->get_tile_value,
        .get_wall_decoration = w->get_wall_decoration,
        .get_floor_decoration = w->get_floor_decoration,
        .get_ornate_anim_len = w->get_ornate_anim_len,
        .queue_timer_with_data = w->queue_ornate_noise_timer,
        .queue_noise_gen2 = w->queue_noise_gen2,
        .party_map = w->party_map
    };
    uint8_t xA = (uint8_t)(timer->value_a & 0xff);
    uint8_t yA = (uint8_t)((timer->value_a >> 8) & 0xff);
    return dm2_v1_continue_ornate_noise((uint16_t)timer->value_a, xA, yA, w->current_map, &cb, context);
}

/* Adapter: HERO_ENCH_FLAG (0x47) */
static int handle_hero_ench_flag(void *context, const DM2_V1_SourceTimer *timer __attribute__((unused)),
                                 uint16_t source_index __attribute__((unused)),
                                 DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->ench_flag_counter)
        return 0;
    DM2_V1_HeroEnchFlagCallbacks cb = {
        .counter = w->ench_flag_counter,
        .hero_slot = w->ench_flag_hero_slot,
        .hero_heroflag = w->hero_heroflag,
        .hero_curHP = w->hero_curHP
    };
    dm2_v1_process_timer_hero_ench_flag(&cb);
    return 1;
}

/* Adapter: ENCH_POWER (0x48) */
static int handle_ench_power(void *context, const DM2_V1_SourceTimer *timer,
                             uint16_t source_index __attribute__((unused)),
                             DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->hero_ench_power)
        return 0;
    DM2_V1_EnchPowerCallbacks cb = {
        .heros_in_party = w->heros_in_party,
        .hero_ench_power = w->hero_ench_power,
        .hero_curHP = w->hero_curHP
    };
    dm2_v1_process_timer_ench_power((uint8_t)timer->actor, timer->value_a, &cb);
    return 1;
}

/* Adapter: POISON (0x4B) */
static int handle_poison(void *context, const DM2_V1_SourceTimer *timer,
                         uint16_t source_index __attribute__((unused)),
                         DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->hero_poisoned || !w->hero_poison)
        return 0;
    DM2_V1_PoisonTimerCallbacks cb = {
        .hero_poisoned = w->hero_poisoned,
        .hero_poison = w->hero_poison,
        .process_poison = w->process_poison
    };
    dm2_v1_process_timer_poison((uint8_t)timer->actor, timer->value_a, &cb, context);
    return 1;
}

/* Adapter: UPDATE_WEATHER (0x54) */
static int handle_update_weather(void *context, const DM2_V1_SourceTimer *timer,
                                 uint16_t source_index __attribute__((unused)),
                                 DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->update_weather)
        return 0;
    DM2_V1_WeatherTimerCallbacks cb = { .update_weather = w->update_weather };
    dm2_v1_process_timer_update_weather(timer->value_a, &cb, context);
    return 1;
}

/* Adapter: PROCESS_59 (0x59) */
static int handle_process_59(void *context, const DM2_V1_SourceTimer *timer,
                             uint16_t source_index __attribute__((unused)),
                             DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    DM2_V1_Timer59Callbacks cb = {
        .get_record_address = w->get_record_address,
        .current_map = w->current_map,
        .party_map = w->party_map,
        .redraw_byte = w->redraw_byte_59
    };
    dm2_v1_process_timer_59((uint16_t)timer->value_a, w->current_map, &cb, context);
    return 1;
}

/* Adapter: 5B_RECORD_CLEAR (0x5B) — record byte@4 &= ~1 */
static int handle_5b_record_clear(void *context, const DM2_V1_SourceTimer *timer,
                                  uint16_t source_index __attribute__((unused)),
                                  DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    uint8_t *rec = w->get_record_address(context, (uint16_t)timer->value_a);
    if (!rec)
        return 0;
    rec[4] &= ~0x01;
    return 1;
}

/* Adapter: 5C_RECORD_SET (0x5C) — record byte@2 |= 1 */
static int handle_5c_record_set(void *context, const DM2_V1_SourceTimer *timer,
                                uint16_t source_index __attribute__((unused)),
                                DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->get_record_address)
        return 0;
    uint8_t *rec = w->get_record_address(context, (uint16_t)timer->value_a);
    if (!rec)
        return 0;
    rec[2] |= 0x01;
    return 1;
}

/* Adapter: MOVE_RECORD_ROTATE (0x5D) */
static int handle_move_record_rotate(void *context, const DM2_V1_SourceTimer *timer,
                                     uint16_t source_index __attribute__((unused)),
                                     DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->move_record_to)
        return 0;
    DM2_V1_MoveRecordRotateCallbacks cb = {
        .move_record_to = w->move_record_to,
        .party_rotate = w->party_rotate,
        .party_map = w->party_map
    };
    dm2_v1_process_timer_move_record_rotate(
        (uint16_t)timer->value_a, w->current_map, w->party_x, w->party_y, &cb, context);
    return 1;
}

/* Adapter: ALLOC_NEW_CREATURE (0x5E) */
static int handle_alloc_new_creature(void *context, const DM2_V1_SourceTimer *timer,
                                     uint16_t source_index __attribute__((unused)),
                                     DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused)))
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->alloc_new_creature || !w->rand_dir)
        return 0;
    uint8_t x = (uint8_t)(timer->value_a & 0xff);
    uint8_t y = (uint8_t)((timer->value_a >> 8) & 0xff);
    DM2_V1_AllocNewCreatureCallbacks cb = {
        .alloc_new_creature = w->alloc_new_creature,
        .rand_dir = w->rand_dir,
        .calc_vector_dir = w->calc_vector_dir,
        .party_x = w->party_x,
        .party_y = w->party_y
    };
    dm2_v1_process_timer_alloc_new_creature(x, y, (uint8_t)timer->actor, &cb, context);
    return 1;
}

/* Adapter: WALL_MECHA (actuator_tile class 0) */
static int handle_actuate_wall_mecha(void *context, const DM2_V1_SourceTimer *timer,
                                     uint16_t source_index,
                                     DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->wall_mecha_handler)
        return 0;
    return w->wall_mecha_handler(
        w->wall_mecha_context ? w->wall_mecha_context : context,
        timer, source_index, receipt);
}

/* Adapter: FLOOR_MECHA (actuator_tile class 1) */
static int handle_actuate_floor_mecha(void *context, const DM2_V1_SourceTimer *timer,
                                      uint16_t source_index,
                                      DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_TimerDispatchWiringContext *w = (DM2_V1_TimerDispatchWiringContext *)context;
    if (!w->floor_mecha_handler)
        return 0;
    return w->floor_mecha_handler(
        w->floor_mecha_context ? w->floor_mecha_context : context,
        timer, source_index, receipt);
}

#define WIRED_COUNT 26

void dm2_v1_timer_dispatch_wiring_init(
    DM2_V1_TimerDispatcher *dispatcher,
    DM2_V1_TimerDispatchWiringContext *wiring_ctx)
{
    if (!dispatcher || !wiring_ctx)
        return;
    dispatcher->context = wiring_ctx;
    memset(dispatcher->handlers, 0, sizeof(dispatcher->handlers));
    memset(dispatcher->actuator_tile, 0, sizeof(dispatcher->actuator_tile));
    dispatcher->tile_class_at = wiring_ctx->tile_class_at;

    dispatcher->handlers[DM2_V1_TIMER_STEP_DOOR] = handle_step_door;
    dispatcher->handlers[DM2_V1_TIMER_DESTROY_DOOR] = handle_destroy_door;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_0C] = handle_process_0c;
    dispatcher->handlers[DM2_V1_TIMER_RESURRECTION] = handle_resurrection;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_0E] = handle_process_0e;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_SOUND] = handle_process_sound;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_CLOUD] = handle_process_cloud;
    dispatcher->handlers[DM2_V1_TIMER_STEP_MISSILE] = handle_step_missile;
    dispatcher->handlers[DM2_V1_TIMER_THINK_CREATURE_A] = handle_think_creature;
    dispatcher->handlers[DM2_V1_TIMER_THINK_CREATURE_B] = handle_think_creature;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_3D] = handle_process_3d;
    dispatcher->handlers[DM2_V1_TIMER_LIGHT] = handle_light;
    dispatcher->handlers[DM2_V1_TIMER_HERO_ENCH_FLAG] = handle_hero_ench_flag;
    dispatcher->handlers[DM2_V1_TIMER_ENCH_POWER] = handle_ench_power;
    dispatcher->handlers[DM2_V1_TIMER_POISON] = handle_poison;
    dispatcher->handlers[DM2_V1_TIMER_UPDATE_WEATHER] = handle_update_weather;
    dispatcher->handlers[DM2_V1_TIMER_ORNATE_ANIMATOR] = handle_ornate_animator;
    dispatcher->handlers[DM2_V1_TIMER_TICK_GENERATOR] = handle_tick_generator;
    dispatcher->handlers[DM2_V1_TIMER_RELEASE_DOOR_BUTTON] = handle_release_door_button;
    dispatcher->handlers[DM2_V1_TIMER_PROCESS_59] = handle_process_59;
    dispatcher->handlers[DM2_V1_TIMER_ORNATE_NOISE] = handle_ornate_noise;
    dispatcher->handlers[DM2_V1_TIMER_5B_RECORD_CLEAR] = handle_5b_record_clear;
    dispatcher->handlers[DM2_V1_TIMER_5C_RECORD_SET] = handle_5c_record_set;
    dispatcher->handlers[DM2_V1_TIMER_MOVE_RECORD_ROTATE] = handle_move_record_rotate;
    dispatcher->handlers[DM2_V1_TIMER_ALLOC_NEW_CREATURE] = handle_alloc_new_creature;

    /* 0x04 ACTUATE_TILE subdispatch: all 7 classes wired.
     * Class 3 is a source-level fall-through no-op (never dispatched). */
    dispatcher->actuator_tile[0] = handle_actuate_wall_mecha;
    dispatcher->actuator_tile[1] = handle_actuate_floor_mecha;
    dispatcher->actuator_tile[2] = handle_actuate_pitfall;
    dispatcher->actuator_tile[4] = handle_actuate_door;
    dispatcher->actuator_tile[5] = handle_actuate_teleporter;
    dispatcher->actuator_tile[6] = handle_actuate_trickwall;
}

int dm2_v1_timer_dispatch_wiring_count(void)
{
    return WIRED_COUNT;
}
