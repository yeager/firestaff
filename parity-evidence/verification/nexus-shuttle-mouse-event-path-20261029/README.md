# Nexus ShuttleMouse report and event path

Read-only pcrecomp/Ghidra analysis of authentic Japanese retail `DM.BIN`, using saved Ymir captures only for corroboration checks. No repository edits or emulator runs were made.

## Source and method

- `DM.BIN` SHA-256: `3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`.
- Imported at `0x06010000` as `SuperH:BE:32:SH-2` in Ghidra 12.2. Existing pcrecomp scripts and a read-only immediate-value scan were used. Ghidra console logs and a raw table dump are stored here. The repository copies of the console logs have trailing whitespace trimmed; no log content was otherwise changed.
- Event values in this report are decimal names with hexadecimal values shown explicitly: 46=`0x2e`; 44=`0x2c`.

## Static packet and movement path

1. `FUN_06024932` calls the peripheral parser through `PTR_FUN_06024b18` (`0x06034f1c`). For device class 2, it accepts packet header bytes `0x20, 0x05`; otherwise it rejects that report. This is the Saturn game's internal report class/header test.
2. In `FUN_06034f1c`, class 2 requires packet byte 0 `0x20`. Signed relative deltas at packet offsets `+3` and `+4` are accumulated in 16.16 form into cursor coordinates at `0x0607fb64`; `FUN_06034e74` clamps them to stored bounds. The exact scale factor and screen-coordinate interpretation are carried by runtime configuration. I call this the mouse path based on relative X/Y input and cursor hit-testing; the binary evidence here does not name the physical peripheral model.
3. For mouse descriptor selection, `FUN_06024b44(7)` chooses one of two authentic descriptor tables at `0x0604b31c` or `0x0604b37c`, then calls `FUN_06034eba` to install the selected pointer at `0x0607fb78`. Each table's first row is mask `0x0401`, event `0x2e` (decimal 46), args zero. `FUN_06034f1c` invokes `FUN_06024518` on the installed table. The descriptor matcher tests the pressed-edge state, so a matched row queues its event and arguments through `FUN_06024288`.
4. The same parser checks the event queue head with `FUN_06024372`. When the head is `0x2e`, it removes the event through `FUN_060242d6` and, if the hit-test flag is enabled, calls `FUN_06034ed6` with the current cursor coordinates and the live hitbox-record pointer at `0x0607fb84`.
5. `FUN_06034ed6` scans six-halfword records: four coordinate bounds, event ID, and event argument. A matching record is enqueued through `FUN_06024288(record.event, record.arg, 0)`. This establishes that event 46 causes a cursor-position hit test; relative deltas also update coordinates independently. The event-row predicate uses the `0x0400` rising-edge bit, whose physical button/report meaning is not identified by these static references.

The exact descriptor row bytes are preserved in [`descriptor-table-dump.txt`](descriptor-table-dump.txt). The selected table branch depends on the runtime value at `0x0607985c`; the binary contains both alternatives.

## Event 44 consumer

`FUN_060100fa` consumes the same generic event queue. Its `case 0x2c` (decimal 44) calls `FUN_06036e58` through the function pointer stored at `0x06010948`, passing the context at `PTR_DAT_060102b0` and the queued argument. `FUN_06036e58` then validates the selected target/state and invokes retail action routines including `FUN_0603705c`, `FUN_060371ac`, and `FUN_06036af0`; it updates a per-record flag. This proves the retail consumer path for event 44, but not that a particular live hitbox row emits 44 or that event 44 is exclusively a mouse click.

## Ymir evidence and limits

The retained Ymir evidence archive contains no scheduled ShuttleMouse report. The saved menu/controller captures do not provide an observed mouse packet or active runtime hitbox table, so they cannot corroborate the physical report bytes, trigger condition, or a specific click target. The inspected title-menu capture's relevant hitbox buffer is zeroed (capture SHA-256 `74a0c5b1b99fc85cebfc37c32cf33c123680f09e9f706b0e8e6ae47b20e1cb56`). No synthetic input or inferred hitbox rows were used.

Static evidence proves packet-class routing, relative-delta cursor updates, the two authentic descriptor-table alternatives that can queue event 46, the event-46 cursor hit-test path, and the event-44 retail consumer. It does not prove a physical ShuttleMouse wiring/protocol mapping, the runtime-selected descriptor table in a particular play session, or which live hitbox emits event 44. A Ymir capture with a scheduled ShuttleMouse report and populated hitbox state would be required to close those dynamic gaps.
