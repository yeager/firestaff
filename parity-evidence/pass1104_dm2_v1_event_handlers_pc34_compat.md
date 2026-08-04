# Pass 1104 — DM2 V1 Event Handlers

## What

Port all 30 UI event handler functions from skproject c_events.cpp into
Firestaff using the callback-based architecture pattern. These are the
click/interaction handlers dispatched by the touch-click zone matrix.

## Implemented handlers

| Handler | skproject source | Description |
|---------|-----------------|-------------|
| CLICK_ITEM_SLOT | c_events.cpp:45 | Inventory slot click — swap held/slot items |
| events_5BFB | c_events.cpp:170 | Sound/UI helper for save/load |
| events_AB26 | c_events.cpp:201 | Game load dialogue UI |
| events_38c8_0002 | c_events.cpp:327 | Viewport enter state |
| events_38c8_0060 | c_events.cpp:355 | Viewport exit state |
| CLICK_MAGICAL_MAP_RUNE | c_events.cpp:395 | Toggle rune on spell panel |
| events_13262 | c_events.cpp:427 | Zone-based click dispatch |
| PUSH_PULL_RIGID_BODY | c_events.cpp:467 | Push/pull rigid body in viewport |
| PLAYER_TESTING_WALL | c_events.cpp:625 | Test wall for alcove/lever/keyhole |
| events_32cb_0287 | c_events.cpp:670 | Stone room visibility check |
| events_32cb_03a6 | c_events.cpp:736 | Item draw and hit-test on tile |
| events_121e_013a | c_events.cpp:974 | Pick up item from tile |
| eventa_121e_0222 | c_events.cpp:1051 | Drop held item onto tile |
| events_121e_03ae | c_events.cpp:1171 | Try place item via 32cb_03a6 |
| events_121e_0003 | c_events.cpp:1222 | Compute target tile from click zone |
| events_37BBB | c_events.cpp:1290 | Hero wall test helper |
| events_121e_0351 | c_events.cpp:1298 | Arrow panel click test |
| guivp_32cb_01b6 | c_events.cpp:1345 | Creature hit-test for click |
| CLICK_VWPT | c_events.cpp:1404 | Viewport click dispatcher |
| CLICK_INVENTORY_EYE | c_events.cpp:1846 | Toggle inventory detail view |
| ADD_RUNE_TO_TAIL | c_events.cpp:1871 | Add rune to spell sequence |
| REMOVE_RUNE_FROM_TAIL | c_events.cpp:1931 | Remove last rune |
| CLICK_MONEYBOX | c_events.cpp:1941 | Deposit/withdraw coins |
| events_2f3f_04ea | c_events.cpp:1996 | Cryocell/champion release |
| events_2e62_0cfa | c_events.cpp:2153 | Stat refresh loop |
| ACTIVATE_ACTION_HAND | c_events.cpp:2784 | Select champion for action |
| PROCEED_COMMAND_SLOT | c_events.cpp:2818 | Execute command from action menu |
| events_30DEA | c_events.cpp:2903 | Item charge management |
| events_3C1E5 | c_events.cpp:2949 | Wall interaction (alcove/lever/sensor) |
| events_443c_0434 | c_events.cpp:3958 | Cursor refresh from held item |

Spell functions (FIND_SPELL_BY_RUNES, CAST_SPELL_PLAYER, PROCEED_SPELL_FAILURE,
TRY_CAST_SPELL) were already ported in dm2_v1_spell_cast_player.c.

DM2_443c_0004 is an internal helper of events_443c_0434 (bitmap palette
compositing) — its logic is absorbed into the generate_cursor_from_item
callback.

## Design

Each handler uses a callback struct (`DM2_V1_*Callbacks`) containing
function pointers for all game state access. This keeps the module
testable without linking the runtime. Receipt structs report what
actions were taken.

The viewport click internals (32cb_*, 121e_*, guivp_*) form a call
chain: CLICK_VWPT dispatches to 121e_013a/0222/03ae which call
32cb_03a6/0287 for item/creature hit-testing on viewport tiles.

## Test

`test_dm2_v1_event_handlers_pc34_compat` — 22 tests covering null
safety, dead-hero rejection, item swap lifecycle, rune mask toggle,
no-active-hero command rejection, sound dispatch, and viewport sub-handler
null safety.
