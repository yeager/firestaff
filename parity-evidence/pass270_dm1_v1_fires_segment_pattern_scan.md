# pass270 — DM1 V1 FIRES.EXENEW segment/pattern scan

Date: 2026-05-06
Worktree: `firestaff-oauth-n2-dm1v1-pass270-fires-segment-pattern-scan-20260506-2007`
Status: `BLOCKED_ZERO_DATA_PATTERNS_NON_UNIQUE_CODE_PATTERNS_ONLY`

## ReDMCSB source audit first

Primary source root: `<redmcsb-source>/ReDMCSB_WIP20210206/Toolchains/Common/Source`.

- `COMMAND.C:2045-2156` ok=True missing=[]
- `COMMAND.C:1734-1812` ok=True missing=[]
- `MOVESENS.C:316-556` ok=True missing=[]
- `GAMELOOP.C:55-91` ok=True missing=[]
- `DUNVIEW.C:8318-8611` ok=True missing=[]

## FIRES.EXENEW input

- Read-only local evidence path: `<firestaff-repo>-dm1v1-viewport-walls-source-lock-20260506-0237/parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW`
- SHA256: `fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94`
- MZ header bytes: `10240`
- Body bytes scanned: `0x29030`
- Load segment used from pass246/pass263 lineage: `0733`
- Binary policy: no original/decompressed binary copied or committed.

## Bounded code segment scan

| ID | static CS:IP | runtime with +0733 | static linear | result | 12-byte prefix occurrences |
| --- | --- | --- | --- | --- | ---: |
| A | `22AF:06E9` | `29E2:06E9` | `0x231d9` | inside_fires_exenew_body | 1 |
| B | `1EA4:010D` | `25D7:010D` | `0x1eb4d` | inside_fires_exenew_body | 2 |
| C | `1EA4:01AA` | `25D7:01AA` | `0x1ebea` | inside_fires_exenew_body | 1 |
| D | `1859:0516` | `1F8C:0516` | `0x18aa6` | inside_fires_exenew_body | 1 |
| E | `2AFF:110E` | `3232:110E` | `0x2c0fe` | outside_fires_exenew_body | n/a |

Candidate E remains exactly blocked: its static linear offset is beyond the `FIRES.EXENEW` body. A-D are valid static-image anchors only; this scan does not claim a gameplay/runtime hit.

## IDC-derived data target scan

| Global | IDC linear | static CS:IP | runtime with +0733 | bytes at target |
| --- | ---: | --- | --- | --- |
| `G0306_i_PartyMapX` | `0x20D0C` | `20D0:000C` | `2803:000C` | `ff 76` |
| `G0307_i_PartyMapY` | `0x20D0E` | `20D0:000E` | `2803:000E` | `fe 56` |
| `G0308_i_PartyDirection` | `0x20D10` | `20D1:0000` | `2804:0000` | `0e e8` |
| `G0432_as_CommandQueue` | `0x25880` | `2588:0000` | `2CBB:0000` | `0e 80 04 25 a6 0f 20 14 6e 50 0f 14 00 32 31 a9 33 42 0e 84 03 41 fc 03 00 02 06 10 45 98` |
| `G0433_i_CommandQueueFirstIndex` | `0x2589E` | `2589:000E` | `2CBC:000E` | `61 03` |
| `G0434_i_CommandQueueLastIndex` | `0x258A0` | `258A:0000` | `2CBD:0000` | `05 32` |

## Pattern uniqueness result

- 8 zero bytes (party tuple shape): `6320` matches.
- 36 zero bytes (queue-sized zero block): `3944` matches.
- 30 zero bytes + first=0 + last=4: `0` matches.
- 34 zero bytes (queue + zero indices shape): `3954` matches.

## Exact blocker / next step

The tiny bounded scan is possible and now done, but it proves the remaining global-address problem cannot be solved by the simple IDC-linear-to-FIRES-body model or by naive zero-pattern scanning: the IDC target offsets land on non-zero/code-like bytes, and zero-shaped tuple/queue patterns are massively non-unique elsewhere. The viable next step is to preserve/build an I34E `DM.MAP`/public-symbol table, or to dump live FIRES data segments after a controlled queue write and scan for a changed non-zero queue/index pattern before re-running `BPM`.

Manifest: `parity-evidence/verification/pass270_dm1_v1_fires_segment_pattern_scan/manifest.json`
