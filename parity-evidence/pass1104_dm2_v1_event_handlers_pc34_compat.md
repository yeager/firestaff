# Pass 1104 — DM2 V1 Event Handlers

## What

Port 8 UI event handler functions from skproject c_events.cpp into
Firestaff using the callback-based architecture pattern. These are the
click/interaction handlers dispatched by the touch-click zone matrix.

## Implemented handlers

| Handler | skproject source | Description |
|---------|-----------------|-------------|
| CLICK_ITEM_SLOT | c_events.cpp:45 | Inventory slot click — swap held/slot items |
| CLICK_VWPT | c_events.cpp:457 | Viewport click — routes to wall test or push/pull |
| CLICK_MAGICAL_MAP_RUNE | c_events.cpp:395 | Toggle rune on spell panel |
| CLICK_INVENTORY_EYE | c_events.cpp:1846 | Toggle inventory detail view |
| ACTIVATE_ACTION_HAND | c_events.cpp:2784 | Select champion for action |
| PROCEED_COMMAND_SLOT | c_events.cpp:2818 | Execute command from action menu |
| PLAYER_TESTING_WALL | c_events.cpp:625 | Test wall for alcove/lever/keyhole |
| PUSH_PULL_RIGID_BODY | c_events.cpp:467 | Push/pull rigid body in viewport |

## Design

Each handler uses a callback struct (`DM2_V1_*Callbacks`) containing
function pointers for all game state access. This keeps the module
testable without linking the runtime. Receipt structs report what
actions were taken.

CLICK_ITEM_SLOT implements the full slot-swap logic: validates hero
alive, checks slot accessibility, verifies item fitness, then performs
the remove-from-hand / remove-from-slot / take-object / equip sequence
with mouse hide/show bracketing.

PUSH_PULL_RIGID_BODY maps the 6 viewport sub-zones (0-5) to push
direction and mode, matching the source's switch at c_events.cpp:489.

## Test

`test_dm2_v1_event_handlers_pc34_compat` — 8 tests covering null
safety, dead-hero rejection, item swap lifecycle, rune mask toggle,
no-active-hero command rejection.
