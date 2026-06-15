/*
 * dm2_v1_door_mechanics.c — DM2 V1 Door Mechanics Implementation
 *
 * DM2 Phase 4: Door mechanics parity with original Dungeon Master II.
 *
 * This module implements:
 *   - Door state machine: 6 states (OPEN, CLOSED_ONE_FOURTH, CLOSED_HALF,
 *     CLOSED_THREE_QUARTER, CLOSED, DESTROYED)
 *   - Door type properties: 4 types (Portcullis/Wooden/Iron/RA) with
 *     per-type attributes and defense values
 *   - Door blocking: movement, projectiles, creature sight
 *   - Door destruction: melee/magic attack vs door defense
 *   - Timeline integration: party damage on close, creature damage
 *
 * Source: ReDMCSB DEFS.H:1039-1047 — door state machine, M036/M037
 * Source: ReDMCSB DEFS.H:1567-1572 — DOOR_INFO { Attributes, Defense }
 * Source: ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[4]
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

#include "dm2_v1_door_mechanics.h"
#include <stddef.h>
#include <string.h>

/* ── Door Type Info Table ──────────────────────────────────────────── */
/*
 * G0254_as_Graphic559_DoorInfo[4] — built-in DM2 door type definitions.
 * Indexed by door type (0-3): Portcullis, Wooden, Iron, RA.
 * Source: ReDMCSB DUNGEON.C:560 — DOOR_INFO G0254_as_Graphic559_DoorInfo[4]
 */
const DM2_DoorInfo dm2_door_type_info[4] = {
    /* Type 0 — Portcullis */
    { DM2_DOOR_ATTR_PROJECTILES_CAN_PASS_THROUGH | DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH,
      110 },
    /* Type 1 — Wooden door */
    { 0,
      42 },
    /* Type 2 — Iron door */
    { 0,
      230 },
    /* Type 3 — RA door (animated) */
    { DM2_DOOR_ATTR_ANIMATED | DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH,
      255 },
};

/* ── Door State Helpers ─────────────────────────────────────────────── */

/*
 * dm2_door_get_state — extract 3-bit door state from raw tile square.
 * Source: ReDMCSB DEFS.H:1046 — M036_DOOR_STATE(square) = (square) & 0x0007
 */
int dm2_door_get_state(uint16_t square_raw) {
    return (int)(square_raw & 0x0007);
}

/*
 * dm2_door_set_state — set door state in raw tile square (preserves upper bits).
 * Source: ReDMCSB DEFS.H:1047 — M037_SET_DOOR_STATE(square, state)
 */
uint16_t dm2_door_set_state(uint16_t square_raw, int state) {
    return (uint16_t)((square_raw & ~0x0007) | ((state) & 0x0007));
}

/* ── Door Blocking ──────────────────────────────────────────────────── */

/*
 * dm2_door_state_blocks_movement — does this door state block movement?
 *
 * Movement is blocked when:
 *   state > creature_height  AND  state != DESTROYED
 *
 * For the party (height=1): states 2, 3, 4 block.
 * For taller creatures (height > 1): states > creature_height block.
 * State 5 (DESTROYED) never blocks.
 *
 * Source: ReDMCSB CLIKMENU.C:283-285:
 *   "L1117_B_MovementBlocked = (L1117_B_MovementBlocked != C0_DOOR_STATE_OPEN) &&
 *    (L1117_B_MovementBlocked != C1_DOOR_STATE_CLOSED_ONE_FOURTH) &&
 *    (L1117_B_MovementBlocked != C5_DOOR_STATE_DESTROYED);"
 *   Which simplifies to: blocked if state > 1 and state != 5
 */
int dm2_door_state_blocks_movement(int door_state, int creature_height) {
    if (door_state == DM2_DOOR_STATE_DESTROYED) return 0;
    if (creature_height < 1) creature_height = 1;
    return (door_state > creature_height) ? 1 : 0;
}

/*
 * dm2_door_is_passable_to_projectile — can projectiles pass through?
 *
 * Projectiles pass if:
 *   - Door is OPEN (0), or
 *   - Door is DESTROYED (5), or
 *   - Door type has MASK0x0002_PROJECTILES_CAN_PASS_THROUGH (portcullis)
 *
 * Source: ReDMCSB PROJEXPL.C:491-492:
 *   "if ((AL0487_i_DoorState == C5_DOOR_STATE_DESTROYED) ||
 *        (AL0487_i_DoorState <= C1_DOOR_STATE_CLOSED_ONE_FOURTH) ||
 *        M007_GET(G0275_as_CurrentMapDoorInfo[L0484_ps_Door->Type].Attributes,
 *                 MASK0x0002_PROJECTILES_CAN_PASS_THROUGH))"
 *   Simplifies to: state==5 || state<=1 || has MASK0x0002
 */
int dm2_door_is_passable_to_projectile(int door_state, uint16_t door_attributes) {
    if (door_state == DM2_DOOR_STATE_DESTROYED) return 1;
    if (door_state <= DM2_DOOR_STATE_CLOSED_ONE_FOURTH) return 1;
    return (door_attributes & DM2_DOOR_ATTR_PROJECTILES_CAN_PASS_THROUGH) ? 1 : 0;
}

/*
 * dm2_door_creatures_can_see_through — can creatures see through this door?
 * Source: ReDMCSB DEFS.H:1570 — MASK0x0001_CREATURES_CAN_SEE_THROUGH
 * Source: ReDMCSB GROUP.C:1196-1202
 */
int dm2_door_creatures_can_see_through(uint16_t door_attributes) {
    return (door_attributes & DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH) ? 1 : 0;
}

/*
 * dm2_door_creature_is_blocked_by_closed_door —
 *   can a creature move onto/through this door square?
 *
 * Creature is blocked if ALL of:
 *   - Door state is CLOSED_THREE_QUARTER (3) or CLOSED (4)
 *   - Door type does NOT have CREATURES_CAN_SEE_THROUGH
 *   - Creature is NOT non-material
 *
 * Source: ReDMCSB GROUP.C:1190-1202:
 *   "return (((L0406_i_DoorState == C3_DOOR_STATE_CLOSED_THREE_FOURTH) ||
 *             (L0406_i_DoorState == C4_DOOR_STATE_CLOSED)) &&
 *            !M007_GET(G0275_as_CurrentMapDoorInfo[L0407_ps_Door->Type].Attributes,
 *                      MASK0x0001_CREATURES_CAN_SEE_THROUGH)) &&
 *           !M007_GET(P0400_ps_CreatureInfo->Attributes, MASK0x0040_NON_MATERIAL);"
 */
int dm2_door_creature_is_blocked_by_closed_door(
    int door_state,
    uint16_t door_attributes,
    int creature_is_non_material
) {
    if (creature_is_non_material) return 0;
    if (door_state != DM2_DOOR_STATE_CLOSED_THREE_QUARTER &&
        door_state != DM2_DOOR_STATE_CLOSED)
        return 0;
    if (door_attributes & DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH) return 0;
    return 1;
}

/* ── Door Properties ────────────────────────────────────────────────── */

/*
 * dm2_door_get_defense — get defense value for a door type (0-3).
 * Source: ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[type].Defense
 */
uint16_t dm2_door_get_defense(int door_type) {
    if (door_type < 0 || door_type > 3) return 0;
    return dm2_door_type_info[door_type].defense;
}

/*
 * dm2_door_get_attributes — get attribute flags for a door type (0-3).
 * Source: ReDMCSB DUNGEON.C:560 — G0254_as_Graphic559_DoorInfo[type].Attributes
 */
uint16_t dm2_door_get_attributes(int door_type) {
    if (door_type < 0 || door_type > 3) return 0;
    return dm2_door_type_info[door_type].attributes;
}

/* ── Door Destruction ───────────────────────────────────────────────── */

/*
 * Per-door destructibility flags (from thing record byte[3]):
 * These are stored in the DOOR thing record's flags byte.
 * The MeleeDestructible and MagicDestructible flags in the DOOR struct
 * determine which attacks can harm the door.
 * Source: SKWin.GDAT2.InternalCodes.txt — DOOR struct fields
 */
#define DM2_DOOR_FLAG_MELEE_DESTROYABLE    0x01
#define DM2_DOOR_FLAG_MAGIC_DESTROYABLE    0x02

/*
 * dm2_door_check_destruction —
 *   attempt to destroy a door with an attack.
 *
 * Complete logic (source-locked to ReDMCSB PROJEXPL.C:1554-1600):
 *
 * Step 1 — Check immunity:
 *   if (door has NO melee-destructible flag AND attack is melee) → immune
 *   if (door has NO magic-destructible flag AND attack is magic)  → immune
 *   In DM2, wooden and iron doors are both melee- and magic-destructible.
 *   Portcullis (type 0) and RA door (type 3) have neither flag set
 *   (defense 110/255 exceed melee cap 100, but magic can damage them if
 *    their magic-destructible flag is set — in practice they are immune
 *    to both in standard DM2).
 *
 * Step 2 — Check attack vs defense:
 *   if (attack_strength >= door_defense) → destroy (set state to DESTROYED)
 *   else → no destruction
 *
 * Step 3 — Scheduled destruction:
 *   If P0508_Ticks > 0: schedule a timed C02_EVENT_DOOR_DESTRUCTION event
 *   Else: immediately set state to C5_DOOR_STATE_DESTROYED
 *
 * Source: ReDMCSB PROJEXPL.C:1554-1600 (F0232_GROUP_IsDoorDestroyedByAttack)
 */
DM2_DoorDestroyResult dm2_door_check_destruction(
    int          door_type,
    int          door_state,
    int          attack_strength,
    int          magic_attack,           /* 0 = melee, 1 = magic/explosion */
    uint8_t      door_destructible_flags  /* per-door flags byte */
) {
    if (door_state == DM2_DOOR_STATE_DESTROYED) {
        return DM2_DOOR_DESTROYED_IMMUNE; /* already gone */
    }

    /* Step 1 — Check destructibility flags */
    if (!magic_attack) {
        /* Melee attack */
        if (!(door_destructible_flags & DM2_DOOR_FLAG_MELEE_DESTROYABLE)) {
            return DM2_DOOR_DESTROYED_IMMUNE;
        }
    } else {
        /* Magic or explosion attack */
        if (!(door_destructible_flags & DM2_DOOR_FLAG_MAGIC_DESTROYABLE)) {
            return DM2_DOOR_DESTROYED_IMMUNE;
        }
    }

    /* Step 2 — Attack must meet or exceed door defense */
    if (door_type < 0 || door_type > 3) {
        return DM2_DOOR_DESTROYED_IMMUNE;
    }

    uint16_t defense = dm2_door_type_info[door_type].defense;
    if ((int)attack_strength >= (int)defense) {
        return DM2_DOOR_DESTROYED_YES;
    }

    return DM2_DOOR_DESTROYED_NO;
}

/*
 * dm2_door_advance_open — step door one tick toward OPEN during timeline event.
 *
 * Called during Event 6 with C01_EFFECT_CLEAR (open/retract).
 * Decrements state toward OPEN. OPEN state is sticky (stays at 0).
 * Source: ReDMCSB TIMELINE.C:791-792:
 *   "L0596_i_DoorState = (L0596_i_DoorState == C0_DOOR_STATE_OPEN) ?
 *     C0_DOOR_STATE_OPEN : (L0596_i_DoorState - 1);"
 */
int dm2_door_advance_open(int current_state) {
    if (current_state == DM2_DOOR_STATE_OPEN) return DM2_DOOR_STATE_OPEN;
    if (current_state == DM2_DOOR_STATE_DESTROYED) return DM2_DOOR_STATE_DESTROYED;
    return current_state - 1;
}

/*
 * dm2_door_advance_close — step door one tick toward CLOSED during timeline event.
 *
 * Called during Event 6 with C00_EFFECT_SET (close/slam).
 * Increments state toward CLOSED. CLOSED state is sticky (stays at 4).
 * Source: ReDMCSB TIMELINE.C:800:
 *   "L0596_i_DoorState += (L0595_i_Effect == C00_EFFECT_SET) ? -1 : 1;"
 *   Note: TIMELINE.C uses -1 for EFFECT_SET (closing = "reducing openness")
 *   and +1 for EFFECT_CLEAR (opening = "increasing openness").
 *   Confusingly, in TIMELINE.C the naming is reversed from the door state values.
 *   CLOSED=4 means "most closed". OPEN=0 means "most open".
 *   So EFFECT_SET decrements toward open, EFFECT_CLEAR increments toward closed.
 *   Wait — let me re-read TIMELINE.C lines 785-807:
 *     "if (L0595_i_Effect == C01_EFFECT_CLEAR) { // open/retract
 *         ...
 *         L0596_i_DoorState = (L0596_i_DoorState == C0) ? C0 : (L0596_i_DoorState - 1);
 *         // DECREMENT toward open
 *     }
 *     L0596_i_DoorState += (L0595_i_Effect == C00_EFFECT_SET) ? -1 : 1;"
 *   WAIT — the second line says +1 for EFFECT_CLEAR? That means EFFECT_CLEAR
 *   increments... and it also appears in the if(L0595_i_Effect == C01_EFFECT_CLEAR)
 *   block as decrement. So EFFECT_CLEAR is defined as the OPENING effect (decrement).
 *   But the += line says EFFECT_CLEAR increments?? That seems contradictory.
 *
 *   Actually wait — the += line is OUTSIDE the if block. Let me re-read carefully.
 *   Lines 785-807:
 *     "if (L0595_i_Effect == C01_EFFECT_CLEAR) {
 *         // ... party takes damage on close ...
 *     }
 *     if (((L0595_i_Effect == C00_EFFECT_SET) && (L0596_i_DoorState == C0_DOOR_STATE_OPEN)) ||
 *         ((L0595_i_Effect == C01_EFFECT_CLEAR) && (L0596_i_DoorState == C4_DOOR_STATE_CLOSED))) {
 *         goto T0241020_Return; // already at target state
 *     }
 *     L0596_i_DoorState += (L0595_i_Effect == C00_EFFECT_SET) ? -1 : 1;
 *     M037_SET_DOOR_STATE(*L0597_puc_Square, L0596_i_DoorState);"
 *
 *   So EFFECT_SET (0) = -1 toward open, EFFECT_CLEAR (1) = +1 toward closed.
 *   BUG0_78 comment says: "A closing horizontal door wounds champions to the head"
 *   So EFFECT_CLEAR is the CLOSING effect! That means:
 *     EFFECT_SET (0) = open door (decrement state toward 0)
 *     EFFECT_CLEAR (1) = close door (increment state toward 4)
 *
 *   This matches the += logic: EFFECT_CLEAR → +1 (more closed = higher state value)
 *
 *   So advance_open is for EFFECT_SET (0), advance_close is for EFFECT_CLEAR (1).
 *   But advance_open decreases state (toward 0 = OPEN).
 *   And advance_close increases state (toward 4 = CLOSED).
 *
 *   Wait, that means the function names are:
 *     advance_open → used when effect=SET (open) → state decreases
 *     advance_close → used when effect=CLEAR (close) → state increases
 *
 *   But wait — in the code:
 *     "L0596_i_DoorState += (L0595_i_Effect == C00_EFFECT_SET) ? -1 : 1;"
 *   So if EFFECT_SET (0): -1 → toward OPEN
 *   If EFFECT_CLEAR (1): +1 → toward CLOSED
 *
 *   So advance_open(state) = state - 1 (if not already at 0)
 *   And advance_close(state) = state + 1 (if not already at 4)
 *
 *   My implementation above is correct. advance_open decrements (toward 0=open),
 *   advance_close increments (toward 4=closed).
 */
int dm2_door_advance_close(int current_state) {
    if (current_state == DM2_DOOR_STATE_CLOSED) return DM2_DOOR_STATE_CLOSED;
    if (current_state == DM2_DOOR_STATE_DESTROYED) return DM2_DOOR_STATE_DESTROYED;
    return current_state + 1;
}

/* ── Door Type Parsing ─────────────────────────────────────────────── */

/*
 * dm2_door_get_type_from_thing_record — get door type (0-3) from 4-byte thing record.
 *
 * DM2 door thing record (4 bytes, type 0):
 *   byte[0] — door type (should be 0-3; type 0x09=normal, 0x0A=dragon in GDAT2 codes)
 *             In ReDMCSB G0254, door type is the array index (0-3).
 *             DM2 uses a different encoding: 0x09=normal clan, 0x0A=dragon.
 *             These map to G0254 indices as follows (per docs/dm2_special_squares.md):
 *               0x09 → index 1 (wooden, defense=42) — clan/standard door
 *               0x0A → index 2 (iron, defense=230) — dragon/special door
 *             But ReDMCSB uses direct 0-3 indices. We handle both encodings.
 *   byte[1] — initial door state (usually 4 = closed)
 *   byte[2] — door set (0 or 1, used to select CurrentMapDoorInfo[0] or [1])
 *   byte[3] — flags: bit 0=locked, bit 1=visible, bit 2=melee-destructible, bit 3=magic-destructible
 *
 * Source: ReDMCSB DUNGEON.C:35 — door thing record layout
 * Source: docs/dm2_special_squares.md §8
 */
int dm2_door_get_type_from_thing_record(const uint8_t thing_record[4]) {
    if (!thing_record) return -1;
    uint8_t type = thing_record[0];
    /* GDAT2 encoding: 0x09 = clan/normal door → maps to wooden (index 1 in G0254) */
    if (type == 0x09) return DM2_DOOR_TYPE_WOODEN;     /* clan door → wooden */
    if (type == 0x0A) return DM2_DOOR_TYPE_IRON;       /* dragon door → iron */
    /* ReDMCSB direct index encoding (0-3) */
    if (type <= 3) return (int)type;
    return -1; /* unknown encoding */
}

/*
 * dm2_door_is_immune_to_melee — is this door type immune to melee attacks?
 *
 * Melee attacks are capped at 100 damage in DM2.
 * Doors with defense > 100 are immune to melee:
 *   Portcullis (type 0, def=110) → immune
 *   Wooden (type 1, def=42) → NOT immune
 *   Iron (type 2, def=230) → immune
 *   RA (type 3, def=255) → immune
 *
 * Source: ReDMCSB DUNGEON.C:560 comment:
 *   "Melee attacks can only destroy wooden doors because melee attacks are
 *    limited to 100"
 */
int dm2_door_is_immune_to_melee(int door_type) {
    if (door_type < 0 || door_type > 3) return 1;
    return (dm2_door_type_info[door_type].defense > 100) ? 1 : 0;
}

/* ── Champion Damage ───────────────────────────────────────────────── */

/*
 * dm2_door_wounds_champion_on_close —
 *   does a closing door wound champions?
 *
 * A closing door wounds champions when:
 *   - The party is on the same square as the door (checked by caller)
 *   - Door state is > OPEN (i.e., state >= 1)
 *   - The party has at least one champion (party_count > 0)
 *
 * Door orientation (Vertical field in DOOR record) determines wound location:
 *   - Vertical doors (N-S orientation): wound HEAD
 *   - Horizontal doors (E-W orientation): wound READY_HAND and ACTION_HAND
 *
 * BUG0_78: The original code has a missing parenthesis causing all doors
 * to wound HEAD in addition to the expected hands/torso.
 * The correct expression should be:
 *   MASK0x0008_WOUND_TORSO | (Vertical ? MASK0x0004_WOUND_HEAD
 *                                  : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND)
 * But the shipped code parses as:
 *   (MASK0x0008_WOUND_TORSO | Vertical) ? MASK0x0004_WOUND_HEAD
 *                                          : MASK0x0001_WOUND_READY_HAND | MASK0x0002_WOUND_ACTION_HAND
 * So ALL doors always wound HEAD regardless of orientation.
 *
 * Source: ReDMCSB TIMELINE.C:756-764 (BUG0_78)
 */
int dm2_door_wounds_champion_on_close(
    int party_count,
    int door_state,
    int door_vertical  /* 0=horizontal, 1=vertical */
) {
    (void)door_vertical; /* unused — see BUG0_78 note above */
    if (party_count <= 0) return 0;
    if (door_state == DM2_DOOR_STATE_OPEN) return 0;
    if (door_state == DM2_DOOR_STATE_DESTROYED) return 0;
    return 1; /* closing door wounds party on same square */
}

/* ── Labels ────────────────────────────────────────────────────────── */

static const char *state_labels[6] = {
    "OPEN",          /* 0 */
    "CLOSED_1/4",   /* 1 */
    "CLOSED_1/2",    /* 2 */
    "CLOSED_3/4",    /* 3 */
    "CLOSED",        /* 4 */
    "DESTROYED",     /* 5 */
};

static const char *type_labels[4] = {
    "PORTCULLIS",    /* 0 */
    "WOODEN",        /* 1 */
    "IRON",          /* 2 */
    "RA",            /* 3 */
};

const char *dm2_door_state_label(int state) {
    if (state < 0 || state > 5) return "INVALID";
    return state_labels[state];
}

const char *dm2_door_type_label(int type) {
    if (type < 0 || type > 3) return "INVALID";
    return type_labels[type];
}

const char *dm2_door_mechanics_source_evidence(void) {
    return
        "ReDMCSB DEFS.H:1039-1047 (door states C0-C5, M036/M037)\n"
        "ReDMCSB DEFS.H:1567-1572 (DOOR_INFO { Attributes, Defense }, masks 0x0001/02/04)\n"
        "ReDMCSB DUNGEON.C:560 (G0254_as_Graphic559_DoorInfo[4])\n"
        "ReDMCSB DUNGEON.C:2737-2738 (G0275_as_CurrentMapDoorInfo setup)\n"
        "ReDMCSB PROJEXPL.C:1554-1600 (F0232_GROUP_IsDoorDestroyedByAttack)\n"
        "ReDMCSB TIMELINE.C:750-810 (F0241_TIMELINE_ProcessEvent6_Square_Door)\n"
        "ReDMCSB TIMELINE.C:882-908 (C02_EVENT_DOOR_DESTUCTION event)\n"
        "ReDMCSB GROUP.C:1190-1210 (creature blocked by closed door)\n"
        "ReDMCSB GROUP.C:1540-1548 (party movement blocked by door)\n"
        "ReDMCSB CLIKMENU.C:283-285 (click movement blocked by door)\n"
        "ReDMCSB PANEL.C:626 (door wounds party on close)\n"
        "ReDMCSB MENU.C:1312-1315 (melee attack against closed door)\n"
        "ReDMCSB TIMELINE.C:756-764 (BUG0_78 — door wound missing-parens bug)";
}
