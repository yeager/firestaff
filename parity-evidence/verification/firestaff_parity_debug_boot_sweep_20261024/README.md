# Real-data debug boot sweep — 2026-10-24

Firestaff was built on TRV2 from the current shared workspace snapshot with
`CMAKE_BUILD_TYPE=Debug`, then the supported CLI boot routes were run in a
headless loop using `--debug --boot-probe` and the authentic
`/home/trv2/.firestaff/data/{dm1,dm2,csb,theron,nexus}` directories. The JSON
manifest retains each complete diagnostic output, return code, startup
receipt, and selected asset MD5.

The sweep reached 12 boot receipts, found one missing-media route, and found
one explicit Nexus startup blocker:

| Route | Outcome | Evidence |
| --- | --- | --- |
| DM1: PC, Amiga, Atari ST, FM Towns | Boot receipt | Retail asset MD5 recorded per route |
| CSB: Amiga, Atari ST, FM Towns | Boot receipt | Retail asset MD5 recorded per route |
| DM2: PC, Amiga, FM Towns | Boot receipt | Retail asset MD5 recorded per route |
| DM2: Macintosh | Missing media | Macintosh English retail was not selectable from the current data directory |
| Theron: JP and US PC Engine | Boot receipt | Track 02 MD5 recorded per edition |
| Nexus: Saturn | Blocked | `title-vdp-owner-compositor-capture-required` |

The Nexus run selected the hash-identified retail `DM.BIN` (`e88d60859f65f08fa622e1992b02280f`) and loaded the real title/UI resources. Its startup receipt remained blocked at `nexus-title`; no level, menu, or party was loaded. A zero process exit code is not treated as a parity pass when the receipt reports this blocker.

These are startup diagnostics only. A boot receipt does not establish gameplay,
visual, audio, or full platform parity. The debug build used the shared
workspace snapshot based on revision `dd79c4f18`; the snapshot contained
uncommitted shared changes. No source edits, emulator runs, pushes, or releases
were performed on TRV2 during the sweep.
