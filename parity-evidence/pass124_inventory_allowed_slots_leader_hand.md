# pass124 DM1 V1 inventory allowed-slot leader-hand placement

## Source anchors

- `firestaff_extracted_frontends_probe.c:5338..5383` mirrors `F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(...)` for commands `C028..C065` / inventory slot boxes. The decisive parity gate is at `firestaff_extracted_frontends_probe.c:5367`: if `G4055_s_LeaderHandObject.Thing` is not `C0xFFFF_THING_NONE`, source checks `G0237_as_Graphic559_ObjectInfo[F0141_DUNGEON_GetObjectInfoIndex(...)] .AllowedSlots & G0038_ai_Graphic562_SlotMasks[L0904_ui_SlotIndex]` before mutating any slot.
- `firestaff_extracted_frontends_probe.c:181..260` provides the 180-entry `G0237_as_Graphic559_ObjectInfo` table. Key proof rows used by the new probe: Torch has `AllowedSlots = 0x0400` (chest/backpack only) at object info index 25; Dagger has `AllowedSlots = 0x05C0` (quiver/pouch/chest) at object info index 31.
- Existing M11 slot-box mapping remains source-box based: `m11_game_view.c:15390..15396` keeps quiver slot `C514` and maps compact backpack slots through `C521..C527` (including `C527`); previous evidence and probe coverage still keep `C528..C536` unaliased until the full chest/backpack model exists. This pass does not reinterpret `/quiver/backpack`; it only adds the source allowed-slot gate for the slots M11 already exposes.
- Quality-feedback cross-check: `m11_game_view.c:2098` is unrelated bar-graph color provenance (`G0046_auc_Graphic562_ChampionColor[...]`) and was inspected; no inventory mutation change was needed there.

## Implemented parity improvement

- Added `m11_allowed_slots_for_thing(...)`, keyed by the existing source-backed `m11_object_info_index_for_thing(...)`, so M11 can read the original 180 `AllowedSlots` masks from `G0237_as_Graphic559_ObjectInfo`.
- Added `m11_v1_inventory_source_slot_box_mask(...)` for the exposed V1 slot-box namespace: hands `0x0200`, head/neck/torso/legs/feet bits, pouch `0x0100`, quiver line2 `0x0080`, quiver line1 `0x0040`, and compact exposed backpack/chest boxes `0x0400`.
- Replaced the former held-object hard reject in `m11_process_v1_inventory_slot_box_click(...)` with source-style placement/swap behavior:
  - reject before mutation if `AllowedSlots & slotMask == 0`;
  - place the leader-hand object into a valid slot;
  - if the clicked slot held an object, move that object into the dedicated leader-hand runtime state.

## Probe coverage

- Added `INV_GV_362C` in `probes/m11/firestaff_m11_game_view_probe.c:7186..7229`:
  - validates Dagger (`0x05C0`) can be placed into pouch slot-box `19` / `CHAMPION_SLOT_POUCH_1`;
  - validates Torch (`0x0400`) is rejected from that pouch slot without mutating the existing Dagger and without clearing the leader-hand object.

## Verification

- `cmake -S . -B build` completed.
- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` passed after adding `INV_GV_362C`.
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` passed: `# summary: 581/581 invariants passed`.
- Initial `ctest --test-dir build --output-on-failure` failed only because several CTest executables had not been built in the fresh worktree.
- After `cmake --build build -- -j2`, full `ctest --test-dir build --output-on-failure` passed: `100% tests passed, 0 tests failed out of 7`.
