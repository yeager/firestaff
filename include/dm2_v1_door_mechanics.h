#ifndef FIRESTAFF_DM2_V1_DOOR_MECHANICS_H
#define FIRESTAFF_DM2_V1_DOOR_MECHANICS_H
/*
 * dm2_v1_door_mechanics.h — DM2 V1 Door Mechanics
 *
 * DM2 Phase 4: Door open/close/locked behavior matching original DM2.
 *
 * Source: ReDMCSB DEFS.H:1039-1047 — door state machine (C0-C5), M036/M037 macros
 * Source: ReDMCSB DEFS.H:1567-1572 — DOOR_INFO { Attributes, Defense }
 * Source: ReDMCSB DEFS.H:1570-1572 — MASK0x0001/02/04 door attribute flags
 * Source: ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[4] definition
 * Source: ReDMCSB DUNGEON.C:2737-2738 — G0275_as_CurrentMapDoorInfo setup
 * Source: ReDMCSB PROJEXPL.C:1554-1600 — F0232_GROUP_IsDoorDestroyedByAttack
 * Source: ReDMCSB TIMELINE.C:750-810 — F0241_TIMELINE_ProcessEvent6_Square_Door
 * Source: ReDMCSB TIMELINE.C:882-908 — door destruction event (C02_EVENT_DOOR_DESTRUCTION)
 * Source: ReDMCSB GROUP.C:1190-1210 — creature blocked by door check
 * Source: ReDMCSB GROUP.C:1540-1548 — party movement blocked by door
 * Source: ReDMCSB CLIKMENU.C:283-285 — click movement blocked by door
 * Source: ReDMCSB PANEL.C:626 — door damage to party on close
 * Source: ReDMCSB MENU.C:1312-1315 — melee attack against door
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Door Attribute Flags ──────────────────────────────────────────── */
/*
 * Door attribute flags from DOOR_INFO.Attributes.
 * Source: ReDMCSB DEFS.H:1570-1572
 */
#define DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH   0x0001
#define DM2_DOOR_ATTR_PROJECTILES_CAN_PASS_THROUGH 0x0002
#define DM2_DOOR_ATTR_ANIMATED                    0x0004

/* ── Door Info Structure ────────────────────────────────────────────── */
/*
 * Per-map door type properties.
 * G0275_as_CurrentMapDoorInfo[0/1] = G0254_as_Graphic559_DoorInfo[Map.DoorSet0/1]
 * Source: ReDMCSB DEFS.H:1567, DUNGEON.C:2737-2738
 */
typedef struct {
    uint16_t attributes;  /* DM2_DOOR_ATTR_* ORed together */
    uint16_t defense;     /* attack strength needed to destroy */
} DM2_DoorInfo;

/* Built-in door type table (ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[4]) */
extern const DM2_DoorInfo dm2_door_type_info[4];

/* ── Door State Machine ─────────────────────────────────────────────── */
/*
 * DM2 door state machine: 6 discrete states encoded in lower 3 bits
 * of a tile square (M036_DOOR_STATE = square & 0x0007).
 * Source: ReDMCSB DEFS.H:1039-1046
 */
typedef enum {
    DM2_DOOR_STATE_OPEN              = 0,  /* C0_DOOR_STATE_OPEN */
    DM2_DOOR_STATE_CLOSED_ONE_FOURTH = 1,  /* C1_DOOR_STATE_CLOSED_ONE_FOURTH */
    DM2_DOOR_STATE_CLOSED_HALF       = 2,  /* C2_DOOR_STATE_CLOSED_HALF */
    DM2_DOOR_STATE_CLOSED_THREE_QUARTER = 3, /* C3_DOOR_STATE_CLOSED_THREE_FOURTH */
    DM2_DOOR_STATE_CLOSED            = 4,  /* C4_DOOR_STATE_CLOSED */
    DM2_DOOR_STATE_DESTROYED         = 5,  /* C5_DOOR_STATE_DESTROYED */
} DM2_DoorState;

/* ── Door Type Enumeration ──────────────────────────────────────────── */
/*
 * The 4 DM2 door types indexed 0-3 in G0254_as_Graphic559_DoorInfo.
 * Source: ReDMCSB DUNGEON.C:560
 */
typedef enum {
    DM2_DOOR_TYPE_PORTCULLIS = 0,  /* Portcullis — projectiles pass, creatures see through */
    DM2_DOOR_TYPE_WOODEN     = 1,  /* Wooden — melee-destroyable (defense=42) */
    DM2_DOOR_TYPE_IRON       = 2,  /* Iron — high defense (230), magic-destroyable */
    DM2_DOOR_TYPE_RA         = 3,  /* RA door — animated, creatures see through */
} DM2_DoorType;

/* ── Door Destruction Result ────────────────────────────────────────── */
typedef enum {
    DM2_DOOR_DESTROYED_NO        = 0,  /* Door not destroyed */
    DM2_DOOR_DESTROYED_YES       = 1,  /* Door was/will be destroyed */
    DM2_DOOR_DESTROYED_IMMUNE    = 2,  /* Door is immune to this attack type */
} DM2_DoorDestroyResult;

/* ── Door Mechanics API ─────────────────────────────────────────────── */

/*
 * dm2_door_get_state — extract 3-bit door state from raw tile square.
 * Source: ReDMCSB DEFS.H:1046 — M036_DOOR_STATE(square)
 */
int dm2_door_get_state(uint16_t square_raw);

/*
 * dm2_door_set_state — set door state in raw tile square.
 * Source: ReDMCSB DEFS.H:1047 — M037_SET_DOOR_STATE(square, state)
 */
uint16_t dm2_door_set_state(uint16_t square_raw, int state);

/*
 * dm2_door_state_blocks_movement — does this door state block movement?
 *
 * Party/creature movement is blocked when:
 *   state > creature_height  AND  state != DESTROYED
 *
 * For party (height=1): blocked by states 2,3,4
 * For creatures: blocked if state > creature_height (from door's Vertical field)
 *
 * Destroyed doors never block (C5_DOOR_STATE_DESTROYED).
 * Source: ReDMCSB CLIKMENU.C:283-285, GROUP.C:1540-1548
 */
int dm2_door_state_blocks_movement(int door_state, int creature_height);

/*
 * dm2_door_is_passable — can projectiles pass through this door?
 *
 * Projectiles pass if:
 *   - Door is OPEN or DESTROYED, OR
 *   - Door type has MASK0x0002_PROJECTILES_CAN_PASS_THROUGH
 *
 * Source: ReDMCSB DEFS.H:1571, PROJEXPL.C:491-492
 */
int dm2_door_is_passable_to_projectile(int door_state, uint16_t door_attributes);

/*
 * dm2_door_creatures_can_see_through — can creatures see through this door?
 *
 * Returns 1 if door type has MASK0x0001_CREATURES_CAN_SEE_THROUGH.
 * Portcullis (type 0) and RA door (type 3) are see-through.
 * Wooden (type 1) and Iron (type 2) are not.
 *
 * Source: ReDMCSB DEFS.H:1570, GROUP.C:1196-1202
 */
int dm2_door_creatures_can_see_through(uint16_t door_attributes);

/*
 * dm2_door_creature_is_blocked_by_closed_door —
 *   can a creature move onto/through this square?
 *
 * Creature is blocked if:
 *   - Square is a door tile
 *   - Door state is CLOSED_THREE_QUARTER or CLOSED (3 or 4)
 *   - Door type does NOT have CREATURES_CAN_SEE_THROUGH
 *   - Creature is not NON_MATERIAL
 *
 * Source: ReDMCSB GROUP.C:1190-1202
 */
int dm2_door_creature_is_blocked_by_closed_door(
    int door_state,
    uint16_t door_attributes,
    int creature_is_non_material
);

/*
 * dm2_door_get_defense — get defense value for a door type.
 * Source: ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[type].Defense
 */
uint16_t dm2_door_get_defense(int door_type);

/*
 * dm2_door_get_attributes — get attribute flags for a door type.
 * Source: ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[type].Attributes
 */
uint16_t dm2_door_get_attributes(int door_type);

/*
 * dm2_door_check_destruction —
 *   attempt to destroy a door with an attack.
 *
 * Logic (source-locked to ReDMCSB PROJEXPL.C:1554-1600):
 *   1. If door has no MeleeDestructible flag (wooden portcullis / RA) and
 *      attack is non-magic → immune.
 *   2. If attack strength >= door Defense → door destroyed (sets state to C5).
 *   3. Otherwise → door not destroyed.
 *
 * magic_attack: C0_FALSE = melee, C1_TRUE = magic/explosion
 * door_destructible_flags: per-door thing record flags (MeleeDestructible etc.)
 *
 * Returns: DM2_DoorDestroyResult.
 * Source: ReDMCSB PROJEXPL.C:1554-1600 (F0232_GROUP_IsDoorDestroyedByAttack)
 */
DM2_DoorDestroyResult dm2_door_check_destruction(
    int          door_type,
    int          door_state,
    int          attack_strength,
    int          magic_attack,          /* 0 = melee, 1 = magic/explosion */
    uint8_t      door_destructible_flags /* per-door flags from thing record */
);

/*
 * dm2_door_advance_open — step door one tick toward OPEN during timeline event.
 *
 * Called during Event 6 (C01_EFFECT_CLEAR = close/retract).
 * Decrements state toward OPEN. Triggers door-rattle sound.
 * Returns new door state.
 * Source: ReDMCSB TIMELINE.C:791-792
 */
int dm2_door_advance_open(int current_state);

/*
 * dm2_door_advance_close — step door one tick toward CLOSED during timeline event.
 *
 * Called during Event 6 (C00_EFFECT_SET = close/slam).
 * Increments state toward CLOSED. Triggers door-rattle sound.
 * Returns new door state.
 * Source: ReDMCSB TIMELINE.C:800
 */
int dm2_door_advance_close(int current_state);

/*
 * dm2_door_get_type_from_thing_record — get door type (0-3) from thing record.
 *
 * The thing record for a door (4 bytes, type 0) is:
 *   byte[0]: door type (DM2_DOOR_TYPE_* value)
 *   byte[1]: state (initial)
 *   byte[2]: door_set (0 or 1)
 *   byte[3]: flags
 *
 * Source: ReDMCSB DUNGEON.C:35 — Door record layout
 */
int dm2_door_get_type_from_thing_record(const uint8_t thing_record[4]);

/*
 * dm2_door_is_immune_to_melee — can this door type be destroyed by melee?
 *
 * Wooden doors (type 1) have defense=42, melee can destroy.
 * Portcullis (type 0, defense=110) and Iron (type 2, defense=230)
 * exceed the melee damage cap of 100, so are immune to melee.
 * RA door (type 3, defense=255) is also immune.
 *
 * Source: ReDMCSB DUNGEON.C:560 comment: "Melee attacks can only destroy
 * wooden doors because melee attacks are limited to 100"
 */
int dm2_door_is_immune_to_melee(int door_type);

/*
 * dm2_door_wounds_champion_on_close —
 *   does a closing door wound the party champion?
 *
 * A closing door wounds champions if:
 *   - Door is at the party's current square
 *   - Door is closing (not already closed or open)
 *   - The party has at least one champion
 *
 * Door orientation determines wound location:
 *   - Vertical doors wound head
 *   - Horizontal doors wound ready-hand and action-hand
 *
 * Source: ReDMCSB TIMELINE.C:756-764 (BUG0_78 — missing parens)
 */
int dm2_door_wounds_champion_on_close(
    int party_count,
    int door_state,
    int door_vertical /* 0=horizontal, 1=vertical */
);

/*
 * dm2_door_state_label — human-readable door state name.
 */
const char *dm2_door_state_label(int state);

/*
 * dm2_door_type_label — human-readable door type name.
 */
const char *dm2_door_type_label(int type);

/*
 * dm2_door_mechanics_source_evidence — return source-lock citation string.
 */
const char *dm2_door_mechanics_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DOOR_MECHANICS_H */
