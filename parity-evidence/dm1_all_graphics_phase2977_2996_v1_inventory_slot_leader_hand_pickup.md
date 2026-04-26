# DM1 all-graphics parity — phase 2977–2996: V1 inventory slot leader-hand pickup

## Source evidence

- `COMMAND.C` routes normal inventory left-clicks through `G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory`: commands `C028..C057` target viewport-relative slot-box zones `C507..C536`.
- `COMMAND.C` dispatches commands `C028..C065` to `F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(...)`.
- `F0302` reads `G4055_s_LeaderHandObject.Thing`, removes a clicked slot object with `F0300_CHAMPION_GetObjectRemovedFromSlot(...)`, then calls `F0297_CHAMPION_PutObjectInLeaderHand(...)` when the leader hand was empty.

## Firestaff change

- Added the inverse source slot-box bridge from `C507..C527` / source slot-box indices `8..28` back to Firestaff champion slots.
- Routed normal V1 inventory pointer clicks through the existing source mouse table. The safe empty-hand pickup half now removes the clicked inventory object and populates the dedicated transient leader-hand object state.
- Deliberately did not implement held-object placement/swap yet: source `F0302` requires `AllowedSlots` validation before placing the held object, and that slot-mask bridge is not exposed in this bounded path.

## Probe coverage

- `INV_GV_362` now checks both forward and inverse slot-box mapping, including non-aliasing of original slots Firestaff does not model yet.
- `INV_GV_362A` clicks source `C507` in normal V1 inventory and verifies the champion slot is cleared while `M11_GameView_GetV1LeaderHandThing/Name` reports the picked-up dagger from the dedicated leader-hand runtime state.
