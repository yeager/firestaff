#ifndef FIRESTAFF_DM1_V1_DOOR_BASH_FEEDBACK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DOOR_BASH_FEEDBACK_PC34_COMPAT_H

/*
 * DM1 V1 door-bash feedback contract.
 *
 * ReDMCSB source anchors (Atari ST DM1 V1 / I34E PC):
 *   - MENU.C lines 1311-1319: bash/kick/hack/berzerk/swing/chop on a closed
 *     door emits M563_SOUND_COMBAT_ATTACK_SKELETON_ANIMATED_ARMOUR_DETH_KNIGHT
 *     with C01_MODE_PLAY_IF_PRIORITIZED, then sets the per-action
 *     ActionDisabledTicks to 6, then calls
 *     F0232_GROUP_IsDoorDestroyedByAttack(targetX, targetY, action-hand
 *     strength, C0_FALSE, 2), then emits
 *     C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM with
 *     C02_MODE_PLAY_ONE_TICK_LATER so the attack swing always plays before
 *     the wooden-thud destruction follow-up.
 *   - PROJEXPL.C lines 1554-1600: F0232_GROUP_IsDoorDestroyedByAttack looks
 *     up the door via F0157_DUNGEON_GetSquareFirstThingData, refuses both
 *     magic-only-allowed doors against melee and melee-only-allowed doors
 *     against magic, and when attack >= Defense and the square is
 *     C4_DOOR_STATE_CLOSED schedules a C02_EVENT_DOOR_DESTRUCTION on the
 *     timeline at GameTime + Ticks (2 ticks from MENU.C) or sets
 *     C5_DOOR_STATE_DESTROYED directly when Ticks == 0.
 *   - DEFS.H lines 560-565: G0254_as_Graphic559_DoorInfo[4] (Portcullis 110,
 *     Wooden door 42, Iron door 230, Ra door 255) and the
 *     "Melee attacks can only destroy wooden doors because melee attacks are
 *     limited to 100" comment.
 *   - DEFS.H lines 934, 1573-1580, 5632, 5627: C02_EVENT_DOOR_DESTRUCTION,
 *     DOOR_INFO layout, Attributes field, MASK0x0001_CREATURES_CAN_SEE_THROUGH
 *     and MASK0x0002_PROJECTILES_CAN_PASS_THROUGH.
 *   - DEFS.H lines 136-138: C00_MODE_PLAY_IMMEDIATELY,
 *     C01_MODE_PLAY_IF_PRIORITIZED, C02_MODE_PLAY_ONE_TICK_LATER.
 *   - DEFS.H lines 7998-7999: F0312_CHAMPION_GetStrength(champion,
 *     C01_SLOT_ACTION_HAND).
 *   - DEFS.H lines 7712-7719: F0238_TIMELINE_AddEvent_GetEventIndex_CPSE.
 *   - DATA.C lines 483 and 1172: SOUND_DATA[C04_SOUND_WOODEN_THUD] =
 *     { 536, 0, 112, 10, 0, 3, 6 } (variant-neutral ReDMCSB table row).
 *
 * The gate is a contract-only source-lock snapshot: it does not load
 * GRAPHICS.DAT / DUNGEON.DAT, does not schedule real timeline events, and
 * makes no claim of pixel parity with an original DOS run. The M11 driver
 * still owns the actual M563/C04 F0064_SOUND_RequestPlay_CPSD emission and
 * the real F0238_TIMELINE_AddEvent_GetEventIndex_CPSE call.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DEFS.H:1555-1569 DOOR_INFO ordinals (Atari ST DM1 V1) */
#define DM1_V1_DOOR_INFO_PORTCULLIS_PC34 0
#define DM1_V1_DOOR_INFO_WOODEN_PC34     1
#define DM1_V1_DOOR_INFO_IRON_PC34       2
#define DM1_V1_DOOR_INFO_RA_PC34         3

/* ReDMCSB DEFS.H:1555-1569 DOOR_INFO attributes */
#define DM1_V1_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH_PC34    0x01
#define DM1_V1_DOOR_ATTR_PROJECTILES_CAN_PASS_THROUGH_PC34 0x02

/* ReDMCSB DEFS.H:1555-1569 / DUNGEON.C:560-564 door Defense values */
#define DM1_V1_DOOR_DEFENSE_PORTCULLIS_PC34 110
#define DM1_V1_DOOR_DEFENSE_WOODEN_PC34     42
#define DM1_V1_DOOR_DEFENSE_IRON_PC34       230
#define DM1_V1_DOOR_DEFENSE_RA_PC34         255

/* ReDMCSB DEFS.H:560-564 melee cap that confines melee to wooden doors */
#define DM1_V1_DOOR_MELEE_ATTACK_CAP_PC34 100

/* ReDMCSB MENU.C:1313-1316 closed-door bash contract */
#define DM1_V1_DOOR_BASH_ACTION_DISABLED_TICKS_PC34 6
#define DM1_V1_DOOR_BASH_DESTRUCTION_DELAY_TICKS_PC34 2

/* ReDMCSB DEFS.H:136-138 / 934 sound play modes & door destruction event */
#define DM1_V1_SOUND_MODE_PLAY_IF_PRIORITIZED_PC34 1
#define DM1_V1_SOUND_MODE_PLAY_ONE_TICK_LATER_PC34 2
#define DM1_V1_EVENT_DOOR_DESTRUCTION_PC34         2

/* ReDMCSB DEFS.H:1573-1576 door state nibble */
#define DM1_V1_DOOR_STATE_CLOSED_PC34    4
#define DM1_V1_DOOR_STATE_DESTROYED_PC34 5

/* ReDMCSB DEFS.H:1573-1576 element nibble for the target square */
#define DM1_V1_ELEMENT_DOOR_PC34 4

/* ReDMCSB DEFS.H:5300-5340 melee action ordinals covered by MENU.C:1311-1319 */
#define DM1_V1_ACTION_BASH_PC34     0x30 /* C030 */
#define DM1_V1_ACTION_HACK_PC34     0x18 /* C024 */ /* see MENU.C:1311 list */
#define DM1_V1_ACTION_KICK_PC34     0x07 /* C007 */
#define DM1_V1_ACTION_SWING_PC34    0x0D /* C013 */
#define DM1_V1_ACTION_CHOP_PC34     0x02 /* C002 */
#define DM1_V1_ACTION_BERZERK_PC34  0x13 /* C019 */
/*
 * Note: the C-prefix values above are the ReDMCSB labels used in MENU.C;
 * the gate accepts any of them as a door-bash action via
 * DM1_V1_DoorBash_ActionIsDoorBashPc34().
 */

/* ReDMCSB SOUND.C / DATA.C sound ordinals wired by MENU.C:1311-1319 */
#define DM1_V1_SOUND_COMBAT_ATTACK_DOOR_BASH_PC34 563 /* M563 */
#define DM1_V1_SOUND_WOODEN_THUD_DOOR_BASH_PC34     4  /* C04  */

typedef enum {
    DM1_V1_DOOR_BASH_OUTCOME_NO_DOOR_PC34 = 0,   /* target square is not a door */
    DM1_V1_DOOR_BASH_OUTCOME_NOT_CLOSED_PC34,    /* door is open/partly-open */
    DM1_V1_DOOR_BASH_OUTCOME_MELEE_REJECTED_PC34,/* iron/portcullis/ra with str<=100 */
    DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BOUNCE_PC34, /* wood door, str < 42, no destroy */
    DM1_V1_DOOR_BASH_OUTCOME_PORT_BREAK_PC34,    /* portcullis, str >= 110 (capped melee) */
    DM1_V1_DOOR_BASH_OUTCOME_WOODEN_BREAK_PC34,  /* wood door, str >= 42, destroyed */
    DM1_V1_DOOR_BASH_OUTCOME_IRON_REJECT_PC34,   /* iron/ra 230/255 with capped melee */
    DM1_V1_DOOR_BASH_OUTCOME_RA_REJECT_PC34      /* ra door, str < 255 */
} DM1_V1_DoorBashOutcomePc34;

typedef struct {
    uint8_t door_type;                /* DM1_V1_DOOR_INFO_*_PC34 */
    uint8_t door_attributes;          /* DOOR_INFO.Attributes */
    uint8_t door_defense;             /* DOOR_INFO.Defense */
    uint8_t door_state;               /* M036_DOOR_STATE(*square) */
    uint8_t target_element;           /* M034_SQUARE_TYPE(target) */
    int16_t action_strength;          /* F0312_CHAMPION_GetStrength value */
    bool magic_attack;                /* F0232 P0507_B_MagicAttack */
    bool is_door_target;              /* gate caller confirms C04_ELEMENT_DOOR */
} DM1_V1_DoorBashInputPc34;

typedef struct {
    uint8_t door_type;
    uint8_t door_attributes;
    uint8_t door_defense;
    uint8_t door_state_after;
    uint8_t disabled_ticks;           /* ActionDisabledTicks */
    uint8_t destruction_delay_ticks;  /* F0232 P0508_Ticks */
    uint16_t combat_sound_ordinal;    /* M563 (ReDMCSB SoundIndex is int16_t) */
    uint8_t combat_sound_mode;        /* C01_MODE_PLAY_IF_PRIORITIZED */
    uint16_t thud_sound_ordinal;      /* C04 (ReDMCSB SoundIndex is int16_t) */
    uint8_t thud_sound_mode;          /* C02_MODE_PLAY_ONE_TICK_LATER */
    uint8_t destruction_event_type;   /* C02_EVENT_DOOR_DESTRUCTION */
    bool scheduled_destruction_event; /* did we route through the timeline? */
    bool applied_state_change;        /* did we set C5_DOOR_STATE_DESTROYED? */
    bool returned_attack;             /* C0_FALSE: bash is melee-only, never magic */
    bool melee_capped_to_100;         /* strength was clamped to 100 */
    DM1_V1_DoorBashOutcomePc34 outcome;
} DM1_V1_DoorBashResultPc34;

bool M11_GameView_DoorBashResolvePc34(
    const DM1_V1_DoorBashInputPc34 *input,
    DM1_V1_DoorBashResultPc34 *out);

bool M11_GameView_DoorBashActionIsBashPc34(uint8_t action_ordinal);

const char *M11_GameView_DoorBashSourceLockPc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DOOR_BASH_FEEDBACK_PC34_COMPAT_H */
