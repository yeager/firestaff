# DM1 all-graphics parity — V1 leader-hand transient state

## Source anchors

- `PC.H` / `DEFS.H`: `G4055_s_LeaderHandObject` is the transient leader/mouse-hand object, separate from champion inventory slots.
- `CHAMPION.C F0297_CHAMPION_PutObjectInLeaderHand`: stores `G4055_s_LeaderHandObject.Thing`, derives `IconIndex` with `F0033_OBJECT_GetIconIndex`, extracts the mouse-pointer icon, and draws the object name.
- `CHAMPION.C F0298_CHAMPION_GetObjectRemovedFromLeaderHand`: clears `Thing` to `C0xFFFF_THING_NONE`, clears `IconIndex`, and clears the name.
- `OBJECT.C F0034_OBJECT_DrawLeaderHandObjectName`: prints the object name into `C017_ZONE_LEADER_HAND_OBJECT_NAME` in cyan on black, bounded to `C014_OBJECT_NAME_MAXIMUM_LENGTH`.
- `OBJECT.C F0035_OBJECT_ClearLeaderHandObjectName`: clears `C017_ZONE_LEADER_HAND_OBJECT_NAME` to black.

## Change

M11 V1 now carries an explicit `leaderHandObjectPresent` / `leaderHandThing` / `leaderHandIconIndex` state on `M11_GameViewState`. This is a dedicated `G4055_s_LeaderHandObject` equivalent and is not synthesized from the active champion ready-hand slot.

New helpers:

- `M11_GameView_SetV1LeaderHandObject(...)`
- `M11_GameView_ClearV1LeaderHandObject(...)`
- existing `M11_GameView_GetV1LeaderHandThing(...)`, icon, and name resolvers now read the dedicated state.

Normal V1 rendering continues to clear/draw source zone `C017` (`233,33,87,6`). When the dedicated transient state holds a dagger thing, the C017 readout renders `DAGGER` in DM cyan. When only champion ready-hand inventory holds the same thing, C017 stays blank.

## Probe coverage

- `INV_GV_15OA`: ready-hand inventory does not synthesize `G4055`.
- `INV_GV_15OB`: C017 remains blank without dedicated transient state.
- `INV_GV_15OC`: dedicated transient state resolves source thing/icon/name (`DAGGER`).
- `INV_GV_15OD`: normal V1 draws the transient object name into C017.
- `INV_GV_15OE`: remove flow clears the dedicated state and does not fall back to champion equipment.

## Remaining gaps

- Real gameplay pointer put/remove commands are not yet routed through this state.
- Mouse-pointer icon bitmap/surface parity is still not rendered to the host cursor.
- Inventory-panel interactions and source draw-order work remain out of scope for this slice.
