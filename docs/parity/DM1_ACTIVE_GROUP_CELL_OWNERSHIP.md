# DM1 active-group cell ownership audit

Date: 2026-09-06. Status: incomplete parity, source/static evidence only.

## Original contract

ReDMCSB `DUNGEON.C` F0145 (1264-1280) interprets C04.Cells as an
ACTIVE_GROUP index on the party map and returns that active record's Cells.
F0146 writes the active record on that map, or the C04 byte off-map.
F0147 (1296-1314) returns packed active Directions on the party map and
the repeated-direction lookup for C04.Direction off-map. F0176 in
`GROUP.C:88-103` consumes these effective values for occupied-cell selection.

## Current native representation

`memory_tick_orchestrator_pc34_compat.c` does not consistently retain this
raw union representation. The creature tick initializes activeGroup.cells
from group->cells, applies behavior.updatedGroupCells back into group->cells,
then mirrors it into ai->groupCells. `orch_write_raw_group_compat` writes
group->cells directly to raw C04 byte 5. Packed directions are separately
held in pc34ActiveGroupDirections; the direction setter mirrors the primary
direction into C04 and retains the packed value for active ticks.

Therefore a blind F0145-style dereference of decoded group->cells would
interpret packed positions as an active-record index. Conversely, directly
using this byte from an original live memory capture can mistake an index
for positions. Current projectile footprint tests exercise normalized groups,
not authentic live ACTIVE_GROUP imports. Their success does not settle this
representation boundary.

## Required next evidence and implementation

Additional admission/removal evidence: original GROUP.C F0183:434-435
copies C04.Cells into ACTIVE_GROUP.Cells and replaces the union byte with
ActiveGroupIndex. F0184:473 restores Cells. Native generated admission
(`orch_add_generated_group_active_state_compat`) instead records the group
index in ai.reserved0 and copies normalized cells into ai.groupCells.
The map-rebuild removal path constructs temporary active records from
ai.groupCells before F0817c removes them. Creature ticks also write
group->cells back to ai.groupCells. These separate writers must be audited
for stale mirrors on damage/removal; replacing only the projectile reader
would not establish the original state-transition contract.

1. Trace fresh admission, party-map transitions, group removal and capture
   import to distinguish normalized decoded state from original C04 bytes.
2. Establish an explicit group-to-active-record mapping and effective
   cell/direction accessors. Do not infer active status merely from a byte
   value or a matching AI position.
3. Test an active record whose index differs from its packed cells, alongside
   an off-map group with identical raw byte 5. Verify F0176 targeting,
   F0190 survivor compaction, F0208 movement and raw serialization together.
4. Pair those transitions with original runtime captures. Savegame work
   remains deferred; neither synthetic RAM fixtures nor helper metadata
   substitute for original capture parity.

No runtime change is claimed by this audit. Per-creature packed directions,
raw byte ownership and cross-platform activation semantics remain open.
