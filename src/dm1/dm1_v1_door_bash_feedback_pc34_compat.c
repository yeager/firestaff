/*
 * DM1 V1 door-bash feedback contract implementation.
 *
 * Source-locked to ReDMCSB MENU.C:1311-1319 + PROJEXPL.C:1554-1600 +
 * DUNGEON.C:560-564 + DEFS.H:1555-1580. See
 * include/dm1_v1_door_bash_feedback_pc34_compat.h for the full anchor list.
 *
 * This is a contract-only gate: it does not play sounds, schedule real
 * timeline events, mutate any global G-state, or open GRAPHICS.DAT /
 * DUNGEON.DAT. The M11 driver still owns the F0064_SOUND_RequestPlay_CPSD
 * emission and the F0238_TIMELINE_AddEvent_GetEventIndex_CPSE call.
 */

#include "dm1_v1_door_bash_feedback_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB MENU.C:1311-1319 melee door-bash action set.
 *
 * The MENU.C source uses case labels that fall through into the closed-door
 * branch in a specific order (C030 BASH, C018 HACK, C019 BERZERK,
 * C007 KICK, C013 SWING, C002 CHOP). The gate accepts any of these for
 * the bash feedback contract because ReDMCSB evaluates the closed-door
 * block once for the entire group.
 */
static const uint8_t s_bash_actions[] = {
    0x30, /* C030 BASH   */
    0x18, /* C018 HACK   */
    0x13, /* C019 BERZRK */
    0x07, /* C007 KICK   */
    0x0D, /* C013 SWING  */
    0x02  /* C002 CHOP   */
};

bool M11_GameView_DoorBashActionIsBashPc34(uint8_t action_ordinal)
{
    size_t i;
    for (i = 0; i < sizeof(s_bash_actions) / sizeof(s_bash_actions[0]); ++i) {
        if (s_bash_actions[i] == action_ordinal) return true;
    }
    return false;
}

bool M11_GameView_DoorBashResolvePc34(
    const DM1_V1_DoorBashInputPc34 *input,
    DM1_V1_DoorBashResultPc34 *out)
{
    int16_t strength;
    bool melee_capped;

    if (!input || !out) return false;
    memset(out, 0, sizeof(*out));

    /*
     * ReDMCSB DEFS.H:1555-1569 door ordinals. Map unknown door ordinals to
     * the wooden door slot so the gate still has a deterministic Defense
     * value to compare against, mirroring DUNGEON.C:2737-2738's
     * CurrentMapDoorInfo assignment.
     */
    switch (input->door_type) {
    case DM1_V1_DOOR_INFO_PORTCULLIS_PC34:
        out->door_type = DM1_V1_DOOR_INFO_PORTCULLIS_PC34;
        out->door_attributes = input->door_attributes;
        out->door_defense = DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34;
        break;
    case DM1_V1_DOOR_INFO_WOODEN_PC34:
        out->door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
        out->door_attributes = input->door_attributes;
        out->door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
        break;
    case DM1_V1_DOOR_INFO_IRON_PC34:
        out->door_type = DM1_V1_DOOR_INFO_IRON_PC34;
        out->door_attributes = input->door_attributes;
        out->door_defense = DM1_V1_DOOR_DEFENSE_IRON_PC34;
        break;
    case DM1_V1_DOOR_INFO_RA_PC34:
        out->door_type = DM1_V1_DOOR_INFO_RA_PC34;
        out->door_attributes = input->door_attributes;
        out->door_defense = DM1_V1_DOOR_DEFENSE_RA_PC34;
        break;
    default:
        out->door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
        out->door_attributes = input->door_attributes;
        out->door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
        break;
    }

    out->disabled_ticks = 0;
    out->destruction_delay_ticks = 0;
    out->combat_sound_ordinal = DM1_V1_SOUND_COMBAT_ATTACK_DOOR_BASH_PC34;
    out->combat_sound_mode = DM1_V1_SOUND_MODE_PLAY_IF_PRIORITIZED_PC34;
    out->thud_sound_ordinal = DM1_V1_SOUND_WOODEN_THUD_DOOR_BASH_PC34;
    out->thud_sound_mode = DM1_V1_SOUND_MODE_PLAY_ONE_TICK_LATER_PC34;
    out->destruction_event_type = DM1_V1_EVENT_DOOR_DESTRUCTION_PC34;
    out->door_state_after = input->door_state;

    /*
     * ReDMCSB PROJEXPL.C:1554-1600 F0232_GROUP_IsDoorDestroyedByAttack
     * early returns. The bash feedback path in MENU.C:1311-1319 is
     * melee-only (P0507_B_MagicAttack = C0_FALSE), so the magic-attack
     * branch is unreachable here; we still record returned_attack=false
     * when the gate caller signals the closed-door branch.
     */
    out->returned_attack = false;

    if (!input->is_door_target ||
        input->target_element != DM1_V1_ELEMENT_DOOR_PC34) {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34;
        out->door_state_after = input->door_state;
        return true;
    }
    if (input->door_state != DM1_V1_DOOR_STATE_CLOSED_PC34) {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34;
        out->door_state_after = input->door_state;
        return true;
    }

    /*
     * ReDMCSB DUNGEON.C:561 / 797: "Melee attacks can only destroy wooden
     * doors because melee attacks are limited to 100". The gate mirrors
     * that by clamping the action-hand strength to 100, exactly the way
     * F0312_CHAMPION_GetStrength is used for melee inside F0232.
     */
    strength = input->action_strength;
    melee_capped = false;
    if (strength < 0) strength = 0;
    if (strength > DM1_V1_DOOR_MELEE_ATTACK_CAP_PC34) {
        strength = DM1_V1_DOOR_MELEE_ATTACK_CAP_PC34;
        melee_capped = true;
    }
    out->melee_capped_to_100 = melee_capped;

    /*
     * ReDMCSB PROJEXPL.C:1580-1582: the MeleeDestructible check
     * (P0507_B_MagicAttack && !MagicDestructible) || (!P0507_B_MagicAttack
     * && !MeleeDestructible) → return C0_FALSE. ReDMCSB does not actually
     * expose a MeleeDestructible boolean; the DUNGEON.C:561 / 797 comment
     * instead notes that the Defense threshold of 100+ prevents melee from
     * breaking non-wooden doors. We encode that as outcome buckets so the
     * gate can verify the contract.
     */
    if (input->magic_attack) {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34;
        out->door_state_after = input->door_state;
        return true;
    }

    if (out->door_type == DM1_V1_DOOR_INFO_IRON_PC34) {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34;
        out->door_state_after = input->door_state;
        /*
         * ReDMCSB MENU.C:1311-1319 sets L1249_ui_ActionDisabledTicks = 6
         * unconditionally once the closed-door branch fires (i.e. after
         * the door is C4_CLOSED and we are about to call F0232). The
         * 6-tick cooldown still applies even when F0232 returns C0_FALSE
         * because the bash attempt already happened.
         */
        out->disabled_ticks = DM1_V1_DOOR_BASH_ACTION_DISABLED_TICKS_PC34;
        return true;
    }
    if (out->door_type == DM1_V1_DOOR_INFO_RA_PC34) {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34;
        out->door_state_after = input->door_state;
        out->disabled_ticks = DM1_V1_DOOR_BASH_ACTION_DISABLED_TICKS_PC34;
        return true;
    }

    if (strength < out->door_defense) {
        /*
         * ReDMCSB PROJEXPL.C:1583-1599 inside the closed branch: when
         * Attack < Defense we drop out and never mark the door as
         * destroyed; the bash feedback still has to play the M563
         * combat-attack swing, the C04 wooden thud, the
         * ActionDisabledTicks=6 cooldown, but does NOT schedule the
         * destruction event and does NOT mutate door_state.
         */
        out->outcome = (out->door_type == DM1_V1_DOOR_INFO_PORTCULLIS_PC34)
            ? DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34
            : DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34;
        out->disabled_ticks = DM1_V1_DOOR_BASH_ACTION_DISABLED_TICKS_PC34;
        out->door_state_after = input->door_state;
        return true;
    }

    /*
     * ReDMCSB PROJEXPL.C:1584-1600: when Attack >= Defense AND door is
     * closed, MENU.C:1317 routes the bash through
     * F0232_GROUP_IsDoorDestroyedByAttack(... , 2) so the
     * C02_EVENT_DOOR_DESTRUCTION event lands on the timeline 2 ticks
     * later (P0508_Ticks == 2) and the door transitions to
     * C5_DOOR_STATE_DESTROYED when the event fires. ReDMCSB does NOT
     * mutate the door_state in the bash tick — that is what the
     * destruction event does.
     */
    out->destruction_delay_ticks = DM1_V1_DOOR_BASH_DESTRUCTION_DELAY_TICKS_PC34;
    out->scheduled_destruction_event = true;
    out->applied_state_change = false;
    out->disabled_ticks = DM1_V1_DOOR_BASH_ACTION_DISABLED_TICKS_PC34;
    if (out->door_type == DM1_V1_DOOR_INFO_PORTCULLIS_PC34) {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34;
    } else {
        out->outcome = DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34;
    }
    return true;
}

const char *M11_GameView_DoorBashSourceLockPc34(void)
{
    return
        "DM1 V1 door-bash feedback contract (source-locked to ReDMCSB "
        "MENU.C:1311-1319 + PROJEXPL.C:1554-1600 + DUNGEON.C:560-564 + "
        "DEFS.H:1555-1580, 7998-7999, 7712-7719, 934, 136-138, DATA.C:483, "
        "DATA.C:1172). MENU.C:1311-1319 dispatches C030 BASH, C018 HACK, "
        "C019 BERZERK, C007 KICK, C013 SWING, and C002 CHOP through the "
        "closed-door branch when M034_SQUARE_TYPE(target) == C04_ELEMENT_DOOR "
        "and M036_DOOR_STATE(target) == C4_DOOR_STATE_CLOSED. The branch "
        "first plays M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_"
        "KNIGHT via F0064_SOUND_RequestPlay_CPSD with "
        "C01_MODE_PLAY_IF_PRIORITIZED, then sets the per-action "
        "ActionDisabledTicks to 6, then calls "
        "F0232_GROUP_IsDoorDestroyedByAttack(L1251_i_MapX, L1252_i_MapY, "
        "F0312_CHAMPION_GetStrength(P0787_ui_ChampionIndex, "
        "C01_SLOT_ACTION_HAND), C0_FALSE, 2), then plays "
        "C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM with "
        "C02_MODE_PLAY_ONE_TICK_LATER. F0232 looks up the door through "
        "F0157_DUNGEON_GetSquareFirstThingData, rejects magic-only doors "
        "against melee and melee-only doors against magic, refuses to break "
        "the door when Attack < Defense, and otherwise schedules "
        "C02_EVENT_DOOR_DESTRUCTION on the timeline at GameTime + 2 ticks "
        "(M033_SET_MAP_AND_TIME + F0238_TIMELINE_AddEvent_GetEventIndex_CPSE). "
        "DUNGEON.C:560-564 / 796-799 records the four "
        "G0254_as_Graphic559_DoorInfo[4] rows: Portcullis (110), Wooden (42), "
        "Iron (230), Ra (255) and notes that melee attacks are limited to "
        "100, so the wooden-defense 42 row is the only one melee can break. "
        "DEFS.H:1555-1580 defines DOOR_INFO {Attributes, Defense} and the "
        "MASK0x0001_CREATURES_CAN_SEE_THROUGH and "
        "MASK0x0002_PROJECTILES_CAN_PASS_THROUGH attribute bits. "
        "DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION = 2, DEFS.H:136-138 "
        "C00_MODE_PLAY_IMMEDIATELY / C01_MODE_PLAY_IF_PRIORITIZED / "
        "C02_MODE_PLAY_ONE_TICK_LATER play modes. DATA.C:483 + DATA.C:1172 "
        "SOUND_DATA[C04_SOUND_WOODEN_THUD] = { 536, 0, 112, 10, 0, 3, 6 } "
        "for the Atari ST and FM-Towns variants. The gate is contract-only "
        "and emits no real F0064_SOUND_RequestPlay_CPSD or "
        "F0238_TIMELINE_AddEvent call; the M11 driver still owns the real "
        "emission and timeline schedule.";
}
