# DM1 all-graphics parity — V1 leader-hand runtime ready-source bridge

## Scope

Move the V1 leader-hand object-name HUD from a geometry-only anchor toward runtime data.

## Source anchors

- `CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand`: stores `G4055_s_LeaderHandObject.Thing`, derives `IconIndex` via `F0033_OBJECT_GetIconIndex`, then draws the name.
- `OBJECT.C F0034_OBJECT_DrawLeaderHandObjectName`: clears and prints the object name into `C017_ZONE_LEADER_HAND_OBJECT_NAME` in cyan on black, bounded by `C014_OBJECT_NAME_MAXIMUM_LENGTH`.
- Current M11 compat state has `PartyState_Compat.activeChampionIndex` and champion inventory slots including `CHAMPION_SLOT_HAND_LEFT` / ready hand. It does **not** yet expose a distinct transient `G4055_s_LeaderHandObject` mouse-hand field.

## Change

- Added `M11_GameView_GetV1LeaderHandThing()` resolving the active leader's ready-hand thing from compat state.
- Added `M11_GameView_GetV1LeaderHandObjectIconIndex()` using the existing source object-info/icon mapping.
- Added `M11_GameView_GetV1LeaderHandObjectName()` using the existing object-name table path.
- Normal V1 HUD now clears/draws C017 at `(233,33,87,6)` with cyan object-name text when the active leader has a ready-hand object.

## Invariant

`INV_GV_15OA`/`15OB`/`15OC` lock that leader-hand runtime follows the active leader, resolves the actual ready-hand thing to source icon/name, draws into C017, and returns empty when the current leader has no ready-hand object.

## Remaining blocker

For exact DM1 mouse-hand parity, compat state still needs a separate serialized/runtime field equivalent to:

```c
unsigned short leaderHandThing; /* G4055_s_LeaderHandObject.Thing, THING_NONE when empty */
int leaderHandIconIndex;        /* cached/source icon index, optional derivable */
```

This patch intentionally keeps the resolver isolated so that future field can replace the ready-hand bridge without touching the HUD drawing path.
