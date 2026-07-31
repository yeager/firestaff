/*
 * dm2_v1_trigger.c - DM2 V1 trigger admission boundary
 *
 * Original trigger descriptors belong to loaded DM2 record chains and their
 * actuators (SKULLWIN/c_trigger.cpp).  Until Firestaff imports that owner,
 * every legacy trigger entry point rejects work.  This prevents invented
 * timer, combat, square and item-trigger targets from entering gameplay.
 */

#include "dm2_v1_trigger.h"

void dm2_v1_trigger_reset_state(void) {}

void dm2_v1_trigger_set_now_ms(int now_ms)
{
    (void)now_ms;
}

int dm2_v1_trigger_get_now_ms(void) { return 0; }
int dm2_v1_trigger_get_builtin_count(void) { return 0; }

const DM2_V1_Trigger *dm2_v1_trigger_get_builtin(int trigger_id)
{
    (void)trigger_id;
    return NULL;
}

int dm2_v1_trigger_lookup_index(int trigger_id)
{
    (void)trigger_id;
    return -1;
}

int dm2_v1_trigger_fire(int trigger_id)
{
    (void)trigger_id;
    return (int)DM2_TRIGGER_RESULT_NOT_FOUND;
}

int dm2_v1_trigger_tick(int now_ms)
{
    (void)now_ms;
    return 0;
}

int dm2_v1_trigger_signal_square_entered(int x, int y, int level)
{
    (void)x;
    (void)y;
    (void)level;
    return 0;
}

int dm2_v1_trigger_signal_item_used(int item_id)
{
    (void)item_id;
    return 0;
}

int dm2_v1_trigger_signal_combat_ended(int victory)
{
    (void)victory;
    return 0;
}

int dm2_v1_trigger_get_fire_count(int trigger_id)
{
    (void)trigger_id;
    return -1;
}

int dm2_v1_trigger_is_active(int trigger_id)
{
    (void)trigger_id;
    return 0;
}

const DM2_V1_TriggerState *dm2_v1_trigger_get_state(int trigger_id)
{
    (void)trigger_id;
    return NULL;
}

const DM2_V1_TriggerEvent *dm2_v1_trigger_last_event(void) { return NULL; }

int dm2_v1_trigger_copy_last_event(DM2_V1_TriggerEvent *out)
{
    (void)out;
    return 0;
}

int dm2_v1_trigger_total_fires(void) { return 0; }
int dm2_v1_trigger_total_signals(void) { return 0; }

const char *dm2_v1_trigger_source_evidence(void)
{
    return
        "DM2 V1 trigger admission boundary\n"
        "Source: skproject/SKULLWIN/c_trigger.cpp (trigger record logic)\n"
        "Source: skproject/SKWIN/DME.h (trigger_descriptor_t)\n"
        "Admission: no decoded source record-chain/actuator owner is bound;\n"
        "           no fixture trigger, message, coordinate or target is retained.\n";
}
