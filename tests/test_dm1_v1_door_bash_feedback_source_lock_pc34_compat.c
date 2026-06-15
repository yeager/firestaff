/*
 * pass777 DM1 V1 door-bash feedback source-lock contract gate.
 *
 * Pinned ReDMCSB path:
 *   MENU.C:1311-1319 closed-door bash: M563 combat swing (mode 1) then
 *   ActionDisabledTicks=6, then F0232(P0508_Ticks=2) destruction event,
 *   then C04 wooden thud (mode 2). PROJEXPL.C:1554-1600 F0232_GROUP_IsDoor
 *   DestroyedByAttack rejects magic-only doors against melee and
 *   melee-only doors against magic, refuses to break when Attack<Defense,
 *   and otherwise schedules C02_EVENT_DOOR_DESTRUCTION at GameTime+2.
 *   DUNGEON.C:560-564 + 796-799 fixes the four door-info slots:
 *   Portcullis 110, Wooden 42, Iron 230, Ra 255. The ReDMCSB comment
 *   "Melee attacks can only destroy wooden doors because melee attacks are
 *   limited to 100" is the source of the strength clamp.
 *
 * Non-duplicative with the existing D0L/D0R F0107 wall-ornament,
 * F0108 floor+ceiling+ornament, F0111 door transparency, F0115 thing-pass,
 * chest scroll-wheel / pickup / resurrect / cross-rotation, mirror
 * candidate, C028 resurrect-confirm, C045 food/water accept, C040
 * browse-pickup, champion-panel redraw, and F0217/F0219 wall-impact
 * projectile sound gates. This slice pins the bash feedback path on
 * MENU.C/PROJEXPL.C alone; it does not assert original DOS pixel parity
 * and does not load GRAPHICS.DAT / DUNGEON.DAT.
 */

#include "dm1_v1_door_bash_feedback_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static const char *k_source_lock_summary =
    "MENU.C:1311-1319 closed-door bash, PROJEXPL.C:1554-1600 "
    "F0232_GROUP_IsDoorDestroyedByAttack, DUNGEON.C:560-564 door info, "
    "DEFS.H:1555-1580 DOOR_INFO, DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION, "
    "DEFS.H:136-138 sound play modes, DEFS.H:7998-7999 "
    "F0312_CHAMPION_GetStrength, DEFS.H:7712-7719 "
    "F0238_TIMELINE_AddEvent_GetEventIndex_CPSE, DATA.C:483 + DATA.C:1172 "
    "SOUND_DATA[C04_SOUND_WOODEN_THUD].";

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, want, anchor);
    }
}

static void expect_u8(const char *id, uint8_t got, uint8_t want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", id, (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %u anchor=%s\n", id, (unsigned)want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, (int)got, (int)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, (int)want, anchor);
    }
}

static void expect_contains(
    const char *id,
    const char *haystack,
    const char *needle,
    const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void test_action_ordinal_set(void)
{
    /* MENU.C:1311-1316 bash group is { C030 BASH, C018 HACK, C019 BERZERK,
     * C007 KICK, C013 SWING, C002 CHOP }. */
    expect_bool("action.c030_bash",
                M11_GameView_DoorBashActionIsBashPc34(0x30), true,
                "MENU.C:1311 C030 BASH");
    expect_bool("action.c018_hack",
                M11_GameView_DoorBashActionIsBashPc34(0x18), true,
                "MENU.C:1312 C018 HACK");
    expect_bool("action.c019_berzerk",
                M11_GameView_DoorBashActionIsBashPc34(0x13), true,
                "MENU.C:1313 C019 BERZERK");
    expect_bool("action.c007_kick",
                M11_GameView_DoorBashActionIsBashPc34(0x07), true,
                "MENU.C:1314 C007 KICK");
    expect_bool("action.c013_swing",
                M11_GameView_DoorBashActionIsBashPc34(0x0D), true,
                "MENU.C:1315 C013 SWING");
    expect_bool("action.c002_chop",
                M11_GameView_DoorBashActionIsBashPc34(0x02), true,
                "MENU.C:1316 C002 CHOP");
    /* Negative cases: ranged / spell / pickup actions are not bash. */
    expect_bool("action.negative_c006_punch",
                M11_GameView_DoorBashActionIsBashPc34(0x06), false,
                "MENU.C:1320+ melee falls through to F0402");
    expect_bool("action.negative_c015_thrust",
                M11_GameView_DoorBashActionIsBashPc34(0x0F), false,
                "MENU.C:1320+ melee falls through to F0402");
    expect_bool("action.negative_c025_melee_other",
                M11_GameView_DoorBashActionIsBashPc34(0x19), false,
                "MENU.C:1320+ melee falls through to F0402");
    expect_bool("action.negative_c040_spit",
                M11_GameView_DoorBashActionIsBashPc34(0x28), false,
                "MENU.C:1299 C040_ACTION_SPIT is ranged spell");
}

static void test_wooden_door_break_with_full_action_hand_strength(void)
{
    /* Strongest happy path: action-hand strength 50, wooden door (defense
     * 42), closed state → C04 wooden thud schedules destruction 2 ticks
     * later and 6-tick action disabled cooldown. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;
    bool ok;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_attributes = 0;
    input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 50;
    input.magic_attack = false;
    input.is_door_target = true;

    ok = M11_GameView_DoorBashResolvePc34(&input, &result);
    expect_bool("wooden.break.resolve", ok, true, "PROJEXPL.C:1554-1600 F0232 entry");
    expect_u8("wooden.break.door_type", result.door_type,
              DM1_V1_DOOR_INFO_WOODEN_PC34, "DEFS.H:1555-1569 DOOR_INFO type");
    expect_u8("wooden.break.door_defense", result.door_defense, 42,
              "DUNGEON.C:560-564 wooden defense 42");
    expect_u8("wooden.break.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 L1249_ui_ActionDisabledTicks = 6");
    expect_u8("wooden.break.destruction_delay_ticks",
              result.destruction_delay_ticks, 2,
              "MENU.C:1317 F0232 P0508_Ticks = 2");
    expect_u8("wooden.break.combat_sound_lo", (uint8_t)(result.combat_sound_ordinal & 0xFF), 563 & 0xFF,
              "MENU.C:1313 M563_SOUND_COMBAT_ATTACK lo byte");
    expect_u8("wooden.break.combat_sound_hi", (uint8_t)((result.combat_sound_ordinal >> 8) & 0xFF), (uint8_t)((563 >> 8) & 0xFF),
              "MENU.C:1313 M563_SOUND_COMBAT_ATTACK hi byte");
    expect_u8("wooden.break.combat_sound_mode", result.combat_sound_mode, 1,
              "DEFS.H:137 C01_MODE_PLAY_IF_PRIORITIZED");
    expect_u8("wooden.break.thud_sound_lo", (uint8_t)(result.thud_sound_ordinal & 0xFF), 4,
              "MENU.C:1318 C04_SOUND_WOODEN_THUD");
    expect_u8("wooden.break.thud_sound_hi", (uint8_t)((result.thud_sound_ordinal >> 8) & 0xFF), 0,
              "MENU.C:1318 C04_SOUND_WOODEN_THUD hi byte");
    expect_u8("wooden.break.thud_sound_mode", result.thud_sound_mode, 2,
              "DEFS.H:138 C02_MODE_PLAY_ONE_TICK_LATER");
    expect_u8("wooden.break.destruction_event_type",
              result.destruction_event_type, 2,
              "DEFS.H:934 C02_EVENT_DOOR_DESTRUCTION");
    expect_bool("wooden.break.scheduled_event",
                result.scheduled_destruction_event, true,
                "PROJEXPL.C:1585-1593 M033_SET_MAP_AND_TIME + F0238");
    expect_bool("wooden.break.applied_state",
                result.applied_state_change, false,
                "PROJEXPL.C:1593-1595 P0508_Ticks != 0 leaves door_state");
    expect_bool("wooden.break.returned_attack",
                result.returned_attack, false,
                "PROJEXPL.C:1580 magic-only reject branch");
    expect_bool("wooden.break.melee_capped", result.melee_capped_to_100, false,
                "PROJEXPL.C P0506_i_Attack <= 100 melee cap");
    expect_int("wooden.break.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34,
               "PROJEXPL.C:1583-1599 wooden door break path");
    /* Bash MUST NOT mutate the door_state in the bash tick — the
     * C02_EVENT_DOOR_DESTRUCTION event applies the C5 transition 2 ticks
     * later. */
    expect_u8("wooden.break.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1593 P0508_Ticks=2 defers door_state write");
}

static void test_wooden_door_bounce_below_defense(void)
{
    /* Strength 30 against a wooden door (defense 42) → bounce: no
     * destruction event, no door_state change, but the M563/C04 swing +
     * thud + ActionDisabledTicks=6 still fire (ReDMCSB does not gate the
     * swing sounds on a successful break). */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 30;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_u8("wooden.bounce.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 L1249_ui_ActionDisabledTicks = 6 unconditional");
    expect_u8("wooden.bounce.destruction_delay", result.destruction_delay_ticks, 0,
              "PROJEXPL.C:1583 Attack<Defense drops out without scheduling");
    expect_bool("wooden.bounce.no_scheduled_event",
                result.scheduled_destruction_event, false,
              "PROJEXPL.C:1583 Attack<Defense returns C0_FALSE");
    expect_bool("wooden.bounce.no_state_change",
                result.applied_state_change, false,
              "PROJEXPL.C:1583 no state write below defense");
    expect_u8("wooden.bounce.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1583 below-defense returns C0_FALSE");
    expect_int("wooden.bounce.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34,
               "PROJEXPL.C:1583 Attack<Defense bounce path");
    expect_bool("wooden.bounce.melee_capped", result.melee_capped_to_100, false,
                "DUNGEON.C:561 melee cap is 100, 30 < 100");
}

static void test_iron_door_always_rejects_melee(void)
{
    /* Iron door (defense 230) is unreachable by melee per
     * DUNGEON.C:561-797. The ReDMCSB comment "Melee attacks are limited to
     * 100" means the action-hand strength gets capped to 100, which is
     * still < 230. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_IRON_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 100; /* already at the melee cap */
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_u8("iron.door_type", result.door_type, DM1_V1_DOOR_INFO_IRON_PC34,
              "DEFS.H:1555-1569 DOOR_INFO type 2");
    expect_u8("iron.defense", result.door_defense, 230,
              "DUNGEON.C:563 iron defense 230");
    expect_bool("iron.melee_capped", result.melee_capped_to_100, false,
                "DUNGEON.C:561 strength 100 == melee cap, no clamp");
    expect_bool("iron.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1583 100<230 returns C0_FALSE");
    expect_bool("iron.no_state_change",
                result.applied_state_change, false,
                "PROJEXPL.C:1583 below-defense returns C0_FALSE");
    expect_int("iron.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34,
               "DUNGEON.C:561 melee-can-only-destroy-wooden-doors comment");
    /* The 6-tick cooldown + 0-tick destruction delay still apply because
     * the bash attempt still happened. */
    expect_u8("iron.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 disabled-ticks is unconditional once the closed-"
              "door branch fired");
    expect_u8("iron.destruction_delay", result.destruction_delay_ticks, 0,
              "PROJEXPL.C:1583 below-defense returns C0_FALSE so the 2-tick "
              "destruction delay never schedules");
    expect_u8("iron.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1583 below-defense leaves door_state alone");
}

static void test_portcullis_defense_110_clamped_to_100(void)
{
    /* Portcullis (defense 110) is also above the 100 melee cap. A raw
     * strength of 250 must be clamped to 100 (DUNGEON.C:561) and then
     * rejected because 100 < 110. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_PORTCULLIS_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 250;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_u8("port.door_type", result.door_type,
              DM1_V1_DOOR_INFO_PORTCULLIS_PC34,
              "DEFS.H:1555-1569 DOOR_INFO type 0");
    expect_u8("port.defense", result.door_defense, 110,
              "DUNGEON.C:560 portcullis defense 110");
    expect_bool("port.melee_capped", result.melee_capped_to_100, true,
                "DUNGEON.C:561 melee cap = 100, 250 > 100 → clamp");
    expect_bool("port.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1583 100<110 returns C0_FALSE");
    expect_int("port.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34,
               "PROJEXPL.C:1583 portcullis never breaks under melee");
}

static void test_ra_door_defense_255_always_rejects(void)
{
    /* Ra door (defense 255, also MASK0x0004_ANIMATED) is the highest
     * defense. Even the 100-melee-cap is < 255, so the bash always
     * bounces. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_RA_PC34;
    input.door_attributes = 0x05; /* MASK0x0001 | MASK0x0004 ANIMATED */
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 100;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_u8("ra.door_type", result.door_type, DM1_V1_DOOR_INFO_RA_PC34,
              "DEFS.H:1555-1569 DOOR_INFO type 3");
    expect_u8("ra.defense", result.door_defense, 255,
              "DUNGEON.C:564 ra defense 255");
    expect_u8("ra.attributes", result.door_attributes, 0x05,
              "DEFS.H:1559-1562 attributes preserved through resolve");
    expect_int("ra.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34,
               "DUNGEON.C:564 ra defense 255 is melee-immune");
    expect_bool("ra.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1583 100<255 returns C0_FALSE");
}

static void test_magic_attack_bash_path_rejected(void)
{
    /* MENU.C:1311-1319 routes the bash with P0507_B_MagicAttack=C0_FALSE
     * explicitly. A magic-attack caller would never reach MENU.C's
     * closed-door branch, but if it did the bash must still go through
     * PROJEXPL.C:1580-1582's "magic-only against melee" reject. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 50;
    input.magic_attack = true;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_int("magic.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34,
               "PROJEXPL.C:1580-1582 magic-vs-melee reject");
    expect_bool("magic.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1580-1582 reject returns C0_FALSE");
}

static void test_not_closed_door_does_not_break(void)
{
    /* Door exists but state is not C4_CLOSED (e.g. open, partly-open,
     * destroyed). The bash feedback must short-circuit before the swing
     * sounds: MENU.C only enters the closed-door branch when the door is
     * actually closed. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;
    int state;

    for (state = 0; state <= 5; ++state) {
        if (state == DM1_V1_DOOR_STATE_CLOSED_PC34) continue;
        input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
        input.door_state = (uint8_t)state;
        input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
        input.action_strength = 50;
        input.is_door_target = true;
        (void)M11_GameView_DoorBashResolvePc34(&input, &result);
        expect_int("not_closed.state", (int)result.outcome,
                   (int)DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34,
                   "MENU.C:1312 M036_DOOR_STATE != C4_DOOR_STATE_CLOSED");
        expect_bool("not_closed.no_scheduled_event",
                    result.scheduled_destruction_event, false,
                    "MENU.C:1312 non-closed door skips F0232");
    }
}

static void test_no_door_target_short_circuits(void)
{
    /* When the target is a wall/corridor/pit/etc the bash feedback
     * contract never fires. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = 0; /* C00_ELEMENT_CORRIDOR */
    input.action_strength = 50;
    input.is_door_target = false;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_int("no_door.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34,
               "MENU.C:1312 M034_SQUARE_TYPE != C04_ELEMENT_DOOR");
    expect_bool("no_door.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "MENU.C:1312 non-door target skips F0232");
    expect_u8("no_door.disabled_ticks", result.disabled_ticks, 0,
              "MENU.C:1312 non-door target skips ActionDisabledTicks=6");
}

static void test_sound_ordinals_locked_to_known_indices(void)
{
    /* ReDMCSB SOUND.C + DATA.C keep the C04 wooden thud and the M563
     * combat attack at fixed ordinals; the gate's result surface must
     * pin those ordinals so a future drift in MENU.C is caught. */
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 50;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_int("sounds.combat_ordinal_not_zero",
               (int)result.combat_sound_ordinal, 563,
               "MENU.C:1313 M563_SOUND_COMBAT_ATTACK");
    expect_int("sounds.thud_ordinal_not_zero",
               (int)result.thud_sound_ordinal, 4,
               "MENU.C:1318 C04_SOUND_WOODEN_THUD");
    expect_int("sounds.combat_ordinal_fits_u16",
               result.combat_sound_ordinal > 255 ? 1 : 0, 1,
               "MENU.C:1313 M563 needs 16-bit SoundIndex");
    expect_int("sounds.combat_mode_prioritized",
               (int)result.combat_sound_mode, 1,
               "DEFS.H:137 C01_MODE_PLAY_IF_PRIORITIZED");
    expect_int("sounds.thud_mode_one_tick_later",
               (int)result.thud_sound_mode, 2,
               "DEFS.H:138 C02_MODE_PLAY_ONE_TICK_LATER");
    /* The combat swing MUST come before the wooden thud so the player
     * hears a "swish-thud" rather than a "thud-swish". */
    expect_int("sounds.combat_before_thud_mode",
               (int)result.combat_sound_mode <
                   (int)result.thud_sound_mode ? 1 : 0, 1,
               "MENU.C:1313-1318 sequence: combat sound first, thud one tick later");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *evidence = M11_GameView_DoorBashSourceLockPc34();
    expect_contains("evidence.menu", k_source_lock_summary, "MENU.C:1311-1319",
                    "MENU.C closed-door bash branch anchor");
    expect_contains("evidence.f0232", k_source_lock_summary,
                    "PROJEXPL.C:1554-1600",
                    "F0232_GROUP_IsDoorDestroyedByAttack anchor");
    expect_contains("evidence.dungeon_defense", k_source_lock_summary,
                    "DUNGEON.C:560-564",
                    "DUNGEON.C door-info slot defenses anchor");
    expect_contains("evidence.door_info", k_source_lock_summary,
                    "DEFS.H:1555-1580",
                    "DEFS.H DOOR_INFO struct + attribute bits anchor");
    expect_contains("evidence.event_type", k_source_lock_summary,
                    "DEFS.H:934",
                    "DEFS.H C02_EVENT_DOOR_DESTRUCTION anchor");
    expect_contains("evidence.sound_modes", k_source_lock_summary,
                    "DEFS.H:136-138",
                    "DEFS.H C00/C01/C02 play-mode anchor");
    expect_contains("evidence.data_table", k_source_lock_summary,
                    "DATA.C:483",
                    "DATA.C SOUND_DATA[C04_SOUND_WOODEN_THUD] anchor");
    expect_contains("evidence.timeline", k_source_lock_summary,
                    "F0238_TIMELINE_AddEvent_GetEventIndex_CPSE",
                    "DEFS.H:7712-7719 timeline dispatch anchor");
    expect_contains("evidence.get_strength", k_source_lock_summary,
                    "F0312_CHAMPION_GetStrength",
                    "DEFS.H:7998-7999 strength anchor");
    expect_contains("evidence.m563", evidence, "M563_SOUND_COMBAT_ATTACK",
                    "MENU.C:1313 combat-attack sound anchor");
    expect_contains("evidence.c04", evidence, "C04_SOUND_WOODEN_THUD",
                    "MENU.C:1318 wooden-thud sound anchor");
    expect_contains("evidence.6_ticks", evidence,
                    "ActionDisabledTicks to 6",
                    "MENU.C:1314 6-tick disabled cooldown anchor");
    expect_contains("evidence.2_ticks", evidence,
                    "F0232_GROUP_IsDoorDestroyedByAttack(L1251_i_MapX",
                    "MENU.C:1317 2-tick destruction delay anchor");
    expect_contains("evidence.melee_cap", evidence,
                    "limited to 100",
                    "DUNGEON.C:561 melee cap 100 anchor");
    expect_contains("evidence.no_real_emit", evidence,
                    "contract-only",
                    "Gate does not actually emit F0064 / F0238");
}

int main(void)
{
    printf("probe=dm1_v1_door_bash_feedback_source_lock_pc34_compat\n");

    test_action_ordinal_set();
    test_wooden_door_break_with_full_action_hand_strength();
    test_wooden_door_bounce_below_defense();
    test_iron_door_always_rejects_melee();
    test_portcullis_defense_110_clamped_to_100();
    test_ra_door_defense_255_always_rejects();
    test_magic_attack_bash_path_rejected();
    test_not_closed_door_does_not_break();
    test_no_door_target_short_circuits();
    test_sound_ordinals_locked_to_known_indices();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_door_bash_feedback_source_lock_pc34_compat "
               "failures=%d assertions=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_door_bash_feedback_source_lock_pc34_compat "
           "%d/%d assertions\n", g_assertions, g_assertions);
    return 0;
}
