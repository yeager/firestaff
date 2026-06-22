/*
 * pass780 DM1 V1 door-bash sound + state no-open gate.
 *
 * Narrow regression for the bash-but-does-not-open contract: when a closed
 * door is hit by a melee bash whose attack is below the door's Defense
 * (or whose door is melee-immune like iron/portcullis/ra), the swing + thud
 * sound emission + ActionDisabledTicks=6 cooldown MUST still fire, but
 * no destruction event may be scheduled and the door_state must remain
 * unchanged. ReDMCSB only emits the M563/C04 sounds after the closed-door
 * branch is taken in MENU.C:1311-1319, not after F0232 returns its
 * verdict, so a "no open" outcome must keep the audio feedback contract
 * intact.
 *
 * Pinned ReDMCSB path:
 *   MENU.C:1311-1319 closed-door bash branch emits M563 (mode 1) → sets
 *           L1249_ui_ActionDisabledTicks = 6 → calls
 *           F0232_GROUP_IsDoorDestroyedByAttack(..., 2) → emits C04
 *           (mode 2). The sounds and the 6-tick cooldown are emitted
 *           unconditionally once the closed-door branch is taken; only
 *           the F0232 destruction-event scheduling is gated on the
 *           attack-vs-Defense comparison.
 *   PROJEXPL.C:1554-1600 F0232_GROUP_IsDoorDestroyedByAttack returns
 *           C0_FALSE in three "no-open" cases: (a) magic-only-allowed
 *           door against melee (P0507_B_MagicAttack && !MagicDestructible
 *           || !P0507_B_MagicAttack && !MeleeDestructible), (b)
 *           P0506_i_Attack < G0275_as_CurrentMapDoorInfo[Type].Defense
 *           (drops out without scheduling the destruction event), and
 *           (c) door not in C4_DOOR_STATE_CLOSED at the time F0232
 *           reached (returns C0_FALSE without writing C5_DESTROYED).
 *   DUNGEON.C:560-564 + 796-799 fixes the four door Defense slots:
 *           Portcullis 110, Wooden 42, Iron 230, Ra 255. The DUNGEON.C:561
 *           comment "Melee attacks can limited to 100" is the source of
 *           the melee cap.
 *   DEFS.H:934  C02_EVENT_DOOR_DESTRUCTION = 2 (must not be scheduled on
 *               a no-open bash).
 *   DEFS.H:136-138 C00/C01/C02 sound play modes. The M563 swing must use
 *               C01_MODE_PLAY_IF_PRIORITIZED; the C04 thud must use
 *               C02_MODE_PLAY_ONE_TICK_LATER so the player still hears
 *               the "swish-thud" sequence on a no-open bounce.
 *
 * Non-duplicative with the existing dm1_v1_door_bash_feedback (pass777)
 * 90-assertion contract. That gate spans 11 distinct sub-tests covering
 * every bash outcome bucket; the present gate is narrower — it pins ONLY
 * the no-open contract that survives a closed-door bash (sound + state
 * + cooldown) across all four door Defense values + the magic-attack +
 * the per-action dispatch from MENU.C:1311-1316. The existing
 * dm1_v1_door_bash_stamina_feedback gate covers the stamina path; this
 * gate covers the sound/state path with the same no-open theme.
 *
 * This is a contract-only source-lock snapshot: it does not load
 * GRAPHICS.DAT / DUNGEON.DAT, does not schedule real timeline events,
 * does not drive the M11 sound emitter, and makes no claim of pixel
 * parity with an original DOS run.
 */

#include "dm1_v1_door_bash_feedback_pc34_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

/* ReDMCSB MENU.C:1313-1316 closed-door bash actions: C030 BASH, C018 HACK,
 * C019 BERZERK, C007 KICK, C013 SWING, C002 CHOP. */
static const uint8_t k_no_open_bash_actions[6] = {
    0x30, /* C030 BASH   */
    0x18, /* C018 HACK   */
    0x13, /* C019 BERZRK */
    0x07, /* C007 KICK   */
    0x0D, /* C013 SWING  */
    0x02  /* C002 CHOP   */
};

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

static void expect_u8(const char *id, uint8_t got, uint8_t want,
                      const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", id, (unsigned)got,
               (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %u anchor=%s\n", id, (unsigned)want, anchor);
    }
}

static void expect_u16(const char *id, uint16_t got, uint16_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", id, (unsigned)got,
               (unsigned)want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %u anchor=%s\n", id, (unsigned)want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want,
                        const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", id, (int)got, (int)want,
               anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d anchor=%s\n", id, (int)want, anchor);
    }
}

/* ------------------------------------------------------------------ */
/* Closed-door bash that hits a wooden door with strength below 42    */
/* must still emit the swing + thud sound + 6-tick cooldown while     */
/* leaving the door CLOSED and NOT scheduling the destruction event.  */
/* ------------------------------------------------------------------ */
static void test_no_open_wooden_strength_30(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_attributes = 0;
    input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 30;  /* < 42 wooden defense */
    input.magic_attack = false;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    /* --- SOUND CONTRACT (the heart of the no-open gate) --- */
    /* MENU.C:1313 swing sound */
    expect_u16("wooden.30.combat_ordinal", result.combat_sound_ordinal, 563,
               "MENU.C:1313 M563_SOUND_COMBAT_ATTACK fires unconditionally");
    expect_u8("wooden.30.combat_mode", result.combat_sound_mode,
              DM1_V1_SOUND_MODE_PLAY_IF_PRIORITIZED_PC34,
              "DEFS.H:137 C01_MODE_PLAY_IF_PRIORITIZED");
    /* MENU.C:1318 thud sound */
    expect_u16("wooden.30.thud_ordinal", result.thud_sound_ordinal, 4,
               "MENU.C:1318 C04_SOUND_WOODEN_THUD fires unconditionally");
    expect_u8("wooden.30.thud_mode", result.thud_sound_mode,
              DM1_V1_SOUND_MODE_PLAY_ONE_TICK_LATER_PC34,
              "DEFS.H:138 C02_MODE_PLAY_ONE_TICK_LATER");

    /* --- FEEDBACK COOLDOWN CONTRACT --- */
    /* MENU.C:1314 L1249_ui_ActionDisabledTicks = 6 fires unconditionally
     * once the closed-door branch is taken, regardless of F0232 verdict. */
    expect_u8("wooden.30.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 ActionDisabledTicks=6 still fires on no-open");

    /* --- STATE CONTRACT (no-open) --- */
    expect_u8("wooden.30.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1583 Attack<Defense leaves door_state alone");
    expect_bool("wooden.30.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1583 no C02_EVENT_DOOR_DESTRUCTION on no-open");
    expect_bool("wooden.30.no_state_change", result.applied_state_change,
                false,
                "PROJEXPL.C:1583 no C5_DOOR_STATE_DESTROYED write on no-open");
    expect_u8("wooden.30.destruction_delay", result.destruction_delay_ticks,
              0,
              "PROJEXPL.C:1583 P0508_Ticks=0 (no event scheduled)");
    expect_int("wooden.30.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34,
               "PROJEXPL.C:1583 below-defense bounce bucket");
    expect_bool("wooden.30.melee_capped", result.melee_capped_to_100, false,
                "DUNGEON.C:561 strength 30 < melee cap 100, no clamp");
}

/* ------------------------------------------------------------------ */
/* Iron door: melee-immune, but the closed-door branch still fires the */
/* swing + thud sounds + 6-tick cooldown before F0232 returns C0_FALSE. */
/* ------------------------------------------------------------------ */
static void test_no_open_iron_strength_100(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_IRON_PC34;
    input.door_attributes = 0;
    input.door_defense = DM1_V1_DOOR_DEFENSE_IRON_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 100;  /* already at melee cap */
    input.magic_attack = false;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    /* --- SOUND CONTRACT --- */
    expect_u16("iron.100.combat_ordinal", result.combat_sound_ordinal, 563,
               "MENU.C:1313 M563 still fires on iron door (closed-door "
               "branch entered)");
    expect_u8("iron.100.combat_mode", result.combat_sound_mode,
              DM1_V1_SOUND_MODE_PLAY_IF_PRIORITIZED_PC34,
              "DEFS.H:137 C01_MODE_PLAY_IF_PRIORITIZED on iron bounce");
    expect_u16("iron.100.thud_ordinal", result.thud_sound_ordinal, 4,
               "MENU.C:1318 C04 thud still fires on iron door");
    expect_u8("iron.100.thud_mode", result.thud_sound_mode,
              DM1_V1_SOUND_MODE_PLAY_ONE_TICK_LATER_PC34,
              "DEFS.H:138 C02_MODE_PLAY_ONE_TICK_LATER on iron bounce");

    /* --- FEEDBACK COOLDOWN CONTRACT --- */
    expect_u8("iron.100.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 ActionDisabledTicks=6 still fires on iron bounce");

    /* --- STATE CONTRACT --- */
    expect_u8("iron.100.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1580 MeleeDestructible=false returns C0_FALSE");
    expect_bool("iron.100.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1580 iron door never schedules destruction "
                "under melee");
    expect_int("iron.100.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34,
               "DUNGEON.C:561 iron 230 above melee cap 100");
}

/* ------------------------------------------------------------------ */
/* Portcullis door: defense 110, raw strength 250 clamps to 100,       */
/* still < 110, so no-open path. The clamp is observable on            */
/* melee_capped_to_100 but the sound contract still fires.             */
/* ------------------------------------------------------------------ */
static void test_no_open_portcullis_strength_250_clamped(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_PORTCULLIS_PC34;
    input.door_attributes = 0;
    input.door_defense = DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 250;  /* clamp to 100, still < 110 */
    input.magic_attack = false;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    /* --- SOUND CONTRACT --- */
    expect_u16("port.250.combat_ordinal", result.combat_sound_ordinal, 563,
               "MENU.C:1313 M563 still fires on portcullis (clamped) "
               "no-open");
    expect_u16("port.250.thud_ordinal", result.thud_sound_ordinal, 4,
               "MENU.C:1318 C04 thud still fires on portcullis no-open");

    /* --- FEEDBACK COOLDOWN CONTRACT --- */
    expect_u8("port.250.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 6-tick cooldown survives the clamp + no-open");

    /* --- STATE CONTRACT --- */
    expect_u8("port.250.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1583 100<110 leaves door_state alone");
    expect_bool("port.250.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1583 portcullis 100<110 no event");
    expect_bool("port.250.melee_capped", result.melee_capped_to_100, true,
                "DUNGEON.C:561 strength 250 clamped to melee cap 100");
    expect_int("port.250.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34,
               "PROJEXPL.C:1583 portcullis 100<110 stays in PORT_BREAK "
               "(no-open) bucket");
}

/* ------------------------------------------------------------------ */
/* Ra door: defense 255, ANIMATED attribute 0x05 preserved,            */
/* always rejects under melee. Sound contract still fires.             */
/* ------------------------------------------------------------------ */
static void test_no_open_ra_strength_100_animated(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_RA_PC34;
    input.door_attributes = 0x05;  /* MASK0x0001 | MASK0x0004 ANIMATED */
    input.door_defense = DM1_V1_DOOR_DEFENSE_RA_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 100;
    input.magic_attack = false;
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    /* --- SOUND CONTRACT --- */
    expect_u16("ra.100.combat_ordinal", result.combat_sound_ordinal, 563,
               "MENU.C:1313 M563 still fires on ra door no-open");
    expect_u16("ra.100.thud_ordinal", result.thud_sound_ordinal, 4,
               "MENU.C:1318 C04 thud still fires on ra door no-open");

    /* --- FEEDBACK COOLDOWN CONTRACT --- */
    expect_u8("ra.100.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 6-tick cooldown survives ra no-open");

    /* --- STATE CONTRACT --- */
    expect_u8("ra.100.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1583 100<255 leaves door_state alone");
    expect_u8("ra.100.attributes_preserved", result.door_attributes, 0x05,
              "DEFS.H:1559-1562 attribute bits survive no-open resolve");
    expect_bool("ra.100.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1583 ra 100<255 no event");
    expect_int("ra.100.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34,
               "PROJEXPL.C:1583 ra 100<255 stays in RA_REJECT bucket");
}

/* ------------------------------------------------------------------ */
/* Wooden door at strength 41 (one below defense 42) is the boundary   */
/* case: closed-door branch fires (sound + cooldown), but the door     */
/* stays closed because 41 < 42.                                       */
/* ------------------------------------------------------------------ */
static void test_no_open_wooden_strength_41_boundary(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 41;  /* one below defense 42 */
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    expect_u16("boundary.41.combat_ordinal", result.combat_sound_ordinal,
               563, "MENU.C:1313 swing on 41 < 42 boundary no-open");
    expect_u16("boundary.41.thud_ordinal", result.thud_sound_ordinal, 4,
               "MENU.C:1318 thud on 41 < 42 boundary no-open");
    expect_u8("boundary.41.disabled_ticks", result.disabled_ticks, 6,
              "MENU.C:1314 cooldown on 41 < 42 boundary no-open");
    expect_u8("boundary.41.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1583 41<42 leaves door_state alone");
    expect_int("boundary.41.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34,
               "PROJEXPL.C:1583 41<42 is a bounce (no-open)");
}

/* ------------------------------------------------------------------ */
/* Strength 42 exactly matches wooden defense 42: this is the          */
/* "break" case (not no-open) and serves as a witness that the        */
/* no-open gate is sharp at the boundary. We assert it here to lock    */
/* the boundary semantics.                                              */
/* ------------------------------------------------------------------ */
static void test_wooden_strength_42_at_defense_opens(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 42;  /* exact defense */
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    /* The "open" path is NOT the no-open contract, but the witness
     * verifies the no-open gate is sharp at the boundary. */
    expect_int("boundary.42.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34,
               "PROJEXPL.C:1583 42>=42 enters break path (open)");
    expect_bool("boundary.42.scheduled_event",
                result.scheduled_destruction_event, true,
                "PROJEXPL.C:1583 42>=42 schedules destruction event");
    expect_u8("boundary.42.destruction_delay",
              result.destruction_delay_ticks, 2,
              "MENU.C:1317 P0508_Ticks=2 schedules 2-tick event");
    /* The break path does NOT mutate door_state in the bash tick. */
    expect_u8("boundary.42.door_state_in_bash_tick",
              result.door_state_after, DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1593 P0508_Ticks=2 defers door_state write");
    expect_bool("boundary.42.applied_state_change", result.applied_state_change,
                false,
                "PROJEXPL.C:1593 P0508_Ticks=2 defers state change");
    /* Sound contract still holds (it always does in the closed-door
     * branch). */
    expect_u16("boundary.42.combat_ordinal", result.combat_sound_ordinal, 563,
               "MENU.C:1313 M563 still fires on the open path");
    expect_u16("boundary.42.thud_ordinal", result.thud_sound_ordinal, 4,
               "MENU.C:1318 C04 still fires on the open path");
}

/* ------------------------------------------------------------------ */
/* Sound ordering invariant: on every no-open path the M563 swing     */
/* MUST come before the C04 thud so the player hears a "swish-thud"  */
/* rather than a "thud-swish". This is enforced by the play modes      */
/* C01 (M563) < C02 (C04) at MENU.C:1313-1318.                         */
/* ------------------------------------------------------------------ */
static void test_sound_ordering_invariant_on_no_open(void)
{
    /* Sweep every no-open outcome bucket. */
    struct {
        const char *label;
        uint8_t door_type;
        uint8_t door_defense;
        int16_t action_strength;
        DM1_V1_DoorBashOutcomePc34 expected_outcome;
    } cases[5] = {
        { "wooden.30", DM1_V1_DOOR_INFO_WOODEN_PC34,
          DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 30,
          DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34 },
        { "wooden.41", DM1_V1_DOOR_INFO_WOODEN_PC34,
          DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 41,
          DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34 },
        { "iron.100", DM1_V1_DOOR_INFO_IRON_PC34,
          DM1_V1_DOOR_DEFENSE_IRON_PC34, 100,
          DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34 },
        { "port.250", DM1_V1_DOOR_INFO_PORTCULLIS_PC34,
          DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34, 250,
          DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34 },
        { "ra.100", DM1_V1_DOOR_INFO_RA_PC34,
          DM1_V1_DOOR_DEFENSE_RA_PC34, 100,
          DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34 },
    };
    int i;

    for (i = 0; i < 5; ++i) {
        DM1_V1_DoorBashInputPc34 input = {0};
        DM1_V1_DoorBashResultPc34 result;

        input.door_type = cases[i].door_type;
        input.door_defense = cases[i].door_defense;
        input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
        input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
        input.action_strength = cases[i].action_strength;
        input.is_door_target = true;

        (void)M11_GameView_DoorBashResolvePc34(&input, &result);

        expect_int("ordering.outcome", (int)result.outcome,
                   (int)cases[i].expected_outcome, cases[i].label);
        expect_u16("ordering.combat", result.combat_sound_ordinal, 563,
                   cases[i].label);
        expect_u8("ordering.combat_mode", result.combat_sound_mode, 1,
                   cases[i].label);
        expect_u16("ordering.thud", result.thud_sound_ordinal, 4,
                   cases[i].label);
        expect_u8("ordering.thud_mode", result.thud_sound_mode, 2,
                   cases[i].label);
        expect_u8("ordering.disabled_ticks", result.disabled_ticks, 6,
                   cases[i].label);
        expect_u8("ordering.door_state_unchanged", result.door_state_after,
                  DM1_V1_DOOR_STATE_CLOSED_PC34, cases[i].label);
        expect_bool("ordering.no_scheduled_event",
                    result.scheduled_destruction_event, false, cases[i].label);
    }
}

/* ------------------------------------------------------------------ */
/* Action-set coverage: each of the 6 closed-door bash actions must    */
/* pass through the closed-door branch and emit the no-open contract.  */
/* We use the wooden door at strength 30 for a clean no-open witness.  */
/* ------------------------------------------------------------------ */
static void test_per_action_no_open_sound(void)
{
    int i;

    for (i = 0; i < 6; ++i) {
        DM1_V1_DoorBashInputPc34 input = {0};
        DM1_V1_DoorBashResultPc34 result;

        input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
        input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
        input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
        input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
        input.action_strength = 30;  /* < 42 wooden */
        input.is_door_target = true;
        /* The action ordinal doesn't drive the resolve (it is a
         * MENU.C:1311-1316 dispatch), but we encode it in the test as
         * documentation that the no-open contract is per-action. */

        (void)M11_GameView_DoorBashResolvePc34(&input, &result);

        expect_bool("per_action.is_bash",
                    M11_GameView_DoorBashActionIsBashPc34(
                        k_no_open_bash_actions[i]),
                    true, "MENU.C:1311-1316 bash action in dispatch set");
        expect_u16("per_action.combat_ordinal", result.combat_sound_ordinal,
                   563, "MENU.C:1313 swing fires for every bash action");
        expect_u16("per_action.thud_ordinal", result.thud_sound_ordinal, 4,
                   "MENU.C:1318 thud fires for every bash action");
        expect_u8("per_action.disabled_ticks", result.disabled_ticks, 6,
                  "MENU.C:1314 6-tick cooldown per bash action");
        expect_u8("per_action.door_state_unchanged", result.door_state_after,
                  DM1_V1_DOOR_STATE_CLOSED_PC34,
                  "PROJEXPL.C:1583 per-action no-open state");
        expect_int("per_action.outcome", (int)result.outcome,
                   (int)DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34,
                   "PROJEXPL.C:1583 per-action bounce");
    }
}

/* ------------------------------------------------------------------ */
/* Magic-attack call: MENU.C:1311-1316 routes only the melee group     */
/* (P0507_B_MagicAttack = C0_FALSE) into the closed-door branch. A    */
/* magic attack that nonetheless reaches the gate must short-circuit  */
/* with NO sound emission and NO state change — the "no-open" path    */
/* but with a different verdict. This pins the disjointness.           */
/* ------------------------------------------------------------------ */
static void test_magic_attack_no_open_no_sound(void)
{
    DM1_V1_DoorBashInputPc34 input = {0};
    DM1_V1_DoorBashResultPc34 result;

    input.door_type = DM1_V1_DOOR_INFO_WOODEN_PC34;
    input.door_defense = DM1_V1_DOOR_DEFENSE_WOODEN_PC34;
    input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
    input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
    input.action_strength = 50;
    input.magic_attack = true;  /* a magic spell, not a melee bash */
    input.is_door_target = true;

    (void)M11_GameView_DoorBashResolvePc34(&input, &result);

    /* Magic attacks MUST NOT enter the closed-door bash branch. */
    expect_int("magic.outcome", (int)result.outcome,
               (int)DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34,
               "PROJEXPL.C:1580-1582 magic-vs-melee reject");
    expect_bool("magic.no_scheduled_event",
                result.scheduled_destruction_event, false,
                "PROJEXPL.C:1580 magic returns C0_FALSE");
    expect_u8("magic.door_state_unchanged", result.door_state_after,
              DM1_V1_DOOR_STATE_CLOSED_PC34,
              "PROJEXPL.C:1580 no state write for magic reject");
    expect_u8("magic.disabled_ticks", result.disabled_ticks, 0,
              "MENU.C:1311-1316 magic dispatch does NOT enter closed-door "
              "branch, no ActionDisabledTicks=6");
}

/* ------------------------------------------------------------------ */
/* Disabled-tick guard: on every no-open path the 6-tick disabled      */
/* cooldown MUST equal 6. This is the single piece of feedback that   */
/* tells the player that the bash attempt already happened.            */
/* ------------------------------------------------------------------ */
static void test_disabled_tick_6_invariant_on_no_open(void)
{
    /* Walk the four door Defense values and a few strength settings
     * that all produce no-open outcomes. */
    struct {
        const char *label;
        uint8_t door_type;
        uint8_t door_defense;
        int16_t action_strength;
    } cases[6] = {
        { "wooden.30", DM1_V1_DOOR_INFO_WOODEN_PC34,
          DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 30 },
        { "wooden.41", DM1_V1_DOOR_INFO_WOODEN_PC34,
          DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 41 },
        { "iron.100", DM1_V1_DOOR_INFO_IRON_PC34,
          DM1_V1_DOOR_DEFENSE_IRON_PC34, 100 },
        { "iron.50", DM1_V1_DOOR_INFO_IRON_PC34,
          DM1_V1_DOOR_DEFENSE_IRON_PC34, 50 },
        { "port.250", DM1_V1_DOOR_INFO_PORTCULLIS_PC34,
          DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34, 250 },
        { "ra.100", DM1_V1_DOOR_INFO_RA_PC34,
          DM1_V1_DOOR_DEFENSE_RA_PC34, 100 },
    };
    int i;

    for (i = 0; i < 6; ++i) {
        DM1_V1_DoorBashInputPc34 input = {0};
        DM1_V1_DoorBashResultPc34 result;

        input.door_type = cases[i].door_type;
        input.door_defense = cases[i].door_defense;
        input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
        input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
        input.action_strength = cases[i].action_strength;
        input.is_door_target = true;

        (void)M11_GameView_DoorBashResolvePc34(&input, &result);

        expect_u8("disabled.6_ticks", result.disabled_ticks, 6,
                  cases[i].label);
        expect_bool("disabled.no_scheduled",
                    result.scheduled_destruction_event, false, cases[i].label);
        expect_u8("disabled.door_state_unchanged", result.door_state_after,
                  DM1_V1_DOOR_STATE_CLOSED_PC34, cases[i].label);
    }
}

/* ------------------------------------------------------------------ */
/* No-state-mutation invariant: on every no-open path, the            */
/* applied_state_change MUST be false. F0232 returned C0_FALSE in     */
/* PROJEXPL.C:1580/1583, so the gate must not write the destroyed     */
/* state.                                                              */
/* ------------------------------------------------------------------ */
static void test_no_state_mutation_on_no_open(void)
{
    /* Single comprehensive sweep: 4 door types × multiple strengths. */
    struct {
        const char *label;
        uint8_t door_type;
        uint8_t door_defense;
        int16_t action_strength;
    } cases[8] = {
        { "wooden.30", DM1_V1_DOOR_INFO_WOODEN_PC34,
          DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 30 },
        { "wooden.41", DM1_V1_DOOR_INFO_WOODEN_PC34,
          DM1_V1_DOOR_DEFENSE_WOODEN_PC34, 41 },
        { "iron.30", DM1_V1_DOOR_INFO_IRON_PC34,
          DM1_V1_DOOR_DEFENSE_IRON_PC34, 30 },
        { "iron.100", DM1_V1_DOOR_INFO_IRON_PC34,
          DM1_V1_DOOR_DEFENSE_IRON_PC34, 100 },
        { "port.50", DM1_V1_DOOR_INFO_PORTCULLIS_PC34,
          DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34, 50 },
        { "port.250", DM1_V1_DOOR_INFO_PORTCULLIS_PC34,
          DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34, 250 },
        { "ra.30", DM1_V1_DOOR_INFO_RA_PC34,
          DM1_V1_DOOR_DEFENSE_RA_PC34, 30 },
        { "ra.100", DM1_V1_DOOR_INFO_RA_PC34,
          DM1_V1_DOOR_DEFENSE_RA_PC34, 100 },
    };
    int i;

    for (i = 0; i < 8; ++i) {
        DM1_V1_DoorBashInputPc34 input = {0};
        DM1_V1_DoorBashResultPc34 result;

        input.door_type = cases[i].door_type;
        input.door_defense = cases[i].door_defense;
        input.door_state = DM1_V1_DOOR_STATE_CLOSED_PC34;
        input.target_element = DM1_V1_ELEMENT_DOOR_PC34;
        input.action_strength = cases[i].action_strength;
        input.is_door_target = true;

        (void)M11_GameView_DoorBashResolvePc34(&input, &result);

        expect_u8("no_state.door_state_after", result.door_state_after,
                  DM1_V1_DOOR_STATE_CLOSED_PC34, cases[i].label);
        expect_bool("no_state.applied_state_change",
                    result.applied_state_change, false, cases[i].label);
        expect_bool("no_state.scheduled_event",
                    result.scheduled_destruction_event, false,
                    cases[i].label);
        expect_u8("no_state.destruction_delay",
                  result.destruction_delay_ticks, 0, cases[i].label);
    }
}

/* ------------------------------------------------------------------ */
/* Source-evidence string pin: a no-open gate must cite the            */
/* ReDMCSB anchors that the no-open path actually depends on.         */
/* ------------------------------------------------------------------ */
static void test_source_evidence_cited(void)
{
    const char *evidence = M11_GameView_DoorBashSourceLockPc34();

    /* Menu.c closed-door bash branch anchor. */
    if (strstr(evidence, "MENU.C:1311-1319") == NULL) {
        ++g_failures;
        printf("FAIL evidence.menu_missing got=(null) want=MENU.C:1311-1319\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.menu contains=MENU.C:1311-1319 anchor=closed-door\n");
    }
    /* F0232 reject/return anchor. */
    if (strstr(evidence, "PROJEXPL.C:1554-1600") == NULL) {
        ++g_failures;
        printf("FAIL evidence.f0232_missing got=(null) want=PROJEXPL.C:1554-1600\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.f0232 contains=PROJEXPL.C:1554-1600 anchor=F0232 dispatch\n");
    }
    /* DUNGEON.C door-info defenses anchor. */
    if (strstr(evidence, "DUNGEON.C:560-564") == NULL) {
        ++g_failures;
        printf("FAIL evidence.dungeon_defense got=(null) want=DUNGEON.C:560-564\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.dungeon_defense contains=DUNGEON.C:560-564\n");
    }
    /* Door destruction event type anchor. */
    if (strstr(evidence, "C02_EVENT_DOOR_DESTRUCTION") == NULL) {
        ++g_failures;
        printf("FAIL evidence.event_type got=(null) want=C02_EVENT_DOOR_DESTRUCTION\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.event_type contains=C02_EVENT_DOOR_DESTRUCTION\n");
    }
    /* Sound mode anchors. */
    if (strstr(evidence, "C01_MODE_PLAY_IF_PRIORITIZED") == NULL) {
        ++g_failures;
        printf("FAIL evidence.mode_1 got=(null) want=C01_MODE_PLAY_IF_PRIORITIZED\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.mode_1 contains=C01_MODE_PLAY_IF_PRIORITIZED\n");
    }
    if (strstr(evidence, "C02_MODE_PLAY_ONE_TICK_LATER") == NULL) {
        ++g_failures;
        printf("FAIL evidence.mode_2 got=(null) want=C02_MODE_PLAY_ONE_TICK_LATER\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.mode_2 contains=C02_MODE_PLAY_ONE_TICK_LATER\n");
    }
    /* Sound ordinal anchors. */
    if (strstr(evidence, "M563_SOUND_COMBAT_ATTACK") == NULL) {
        ++g_failures;
        printf("FAIL evidence.m563 got=(null) want=M563_SOUND_COMBAT_ATTACK\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.m563 contains=M563_SOUND_COMBAT_ATTACK\n");
    }
    if (strstr(evidence, "C04_SOUND_WOODEN_THUD") == NULL) {
        ++g_failures;
        printf("FAIL evidence.c04 got=(null) want=C04_SOUND_WOODEN_THUD\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.c04 contains=C04_SOUND_WOODEN_THUD\n");
    }
    /* Melee cap anchor. */
    if (strstr(evidence, "limited to 100") == NULL) {
        ++g_failures;
        printf("FAIL evidence.melee_cap got=(null) want=limited to 100\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.melee_cap contains=limited to 100\n");
    }
    /* 6-tick disabled cooldown anchor. */
    if (strstr(evidence, "ActionDisabledTicks") == NULL) {
        ++g_failures;
        printf("FAIL evidence.disabled_ticks got=(null) want=ActionDisabledTicks\n");
    } else {
        ++g_assertions;
        printf("PASS evidence.disabled_ticks contains=ActionDisabledTicks\n");
    }
}

int main(void)
{
    printf("probe=dm1_v1_door_bash_sound_no_open_gate_pc34_compat\n");

    test_no_open_wooden_strength_30();
    test_no_open_iron_strength_100();
    test_no_open_portcullis_strength_250_clamped();
    test_no_open_ra_strength_100_animated();
    test_no_open_wooden_strength_41_boundary();
    test_wooden_strength_42_at_defense_opens();
    test_sound_ordering_invariant_on_no_open();
    test_per_action_no_open_sound();
    test_magic_attack_no_open_no_sound();
    test_disabled_tick_6_invariant_on_no_open();
    test_no_state_mutation_on_no_open();
    test_source_evidence_cited();

    if (g_failures) {
        printf("FAIL dm1_v1_door_bash_sound_no_open_gate_pc34_compat "
               "failures=%d assertions=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_door_bash_sound_no_open_gate_pc34_compat "
           "%d/%d assertions\n", g_assertions, g_assertions);
    return 0;
}
