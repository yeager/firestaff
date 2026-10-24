# Cross-game CTest inventory — 2026-10-24

`manifest.json` records the CTest names discovered by the per-game sweep tool
in the Debug build at `/home/trv2/work/firestaff-parity-debug-20261024/.build-debug`.
This run listed test inventory and platform identifiers; it did not execute the
tests.

| Game | Discovered tests | Platform identifiers found in names or labels |
| --- | ---: | --- |
| DM1 | 1,276 | Amiga 18; Atari ST 30; DOS PC 931; FM Towns 19 |
| DM2 | 482 | Amiga 10; DOS PC 147; FM Towns 10; Macintosh 25 |
| CSB | 557 | Amiga 15; Atari ST 24; FM Towns 24 |
| Theron | 277 | PC Engine 108 |
| Nexus | 362 | Saturn 100 |

The counts describe CTest entries, not verified runtime support or parity. An
identifier absent from the manifest means the current test names and labels do
not identify that platform; it does not establish that Firestaff lacks support.
The build came from shared source snapshot revision `dd79c4f18` and included
concurrent uncommitted changes. The exact build path and discovery output are
preserved in the manifest.
