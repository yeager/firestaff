# DM1 V1 Touch Status Hand Zones Source Lock

Scope: champion status-box ready/action hand click zones only. This locks the
C211..C218 source coordinates and the two-step routing boundary used by touch
translation. It does not claim full touchscreen support.

## Source Evidence

- `Toolchains/Common/Source/COMMAND.C:375-395` defines the primary active game
  interface table. Left clicks in champion status boxes route first through
  C012..C015 on C151..C154.
- `Toolchains/Common/Source/COMMAND.C:484-497` defines `G0455`, the champion
  name/hand subroute table. C020..C027 map to C211..C218.
- `Toolchains/Common/Source/COMMAND.C:1097-1139` initializes mouse boxes from
  source zones; I34E stores right/bottom as `left + width - 1` and
  `top + height - 1`.
- `Toolchains/Common/Source/COMMAND.C:1437-1449` scans mouse input with
  inclusive X/Y bounds and source table order.
- `Toolchains/Common/Source/COMMAND.C:2158-2162` dispatches C012..C015 through
  `F0367_COMMAND_ProcessTypes12To27_ClickInChampionStatusBox`.
- `Toolchains/Common/Source/CLIKCHAM.C:24-35` either sets the leader when that
  champion inventory is already open, or scans `G0455` and sends C020..C027 to
  `F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox`.
- `Toolchains/Common/Source/CHAMPION.C:662-713` implements the slot-box swap
  semantics for the routed hand slot.
- `Toolchains/Common/Source/DATA.C:978-985` lists the status hand slot zones
  C211..C218 for layout data, and `DEFS.H:3800-3807` names those zone IDs.

## Firestaff Gate

- `src/shared/champion_status_slotbox_pc34_compat.c` is the focused compat
  helper for the top-row champion ready/action hand slots.
- `tests/test_champion_status_slotbox_pc34_compat_integration.c` proves all
  eight C211..C218 boxes use inclusive 16x16 bounds, the C020..C027 command
  sequence, the slot-box-to-champion/hand mapping, and source evidence strings
  that cite the ReDMCSB route lines above.
- CTest target: `champion_status_slotbox_pc34_compat`.
