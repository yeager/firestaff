# DM2 file-open dynamic trace boundary (2026-04-30)

Scope: follow-up gate for the DM2 startup file-open/source boundary. This is still evidence-only: no runtime, rendering, asset, or gameplay parity is claimed.

## ReDMCSB source audit used first

ReDMCSB remains a DM/CSB comparison source only, not DM2 C source. Exact audited ranges:

- `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/FILENAME.C:4-10` — global filename definitions for `DATA\DUNGEON.DAT` and `DATA\GRAPHICS.DAT`.
- `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/MEMORY.C:1212-1285` — `F0477_MEMORY_OpenGraphicsDat_CPSDF`, including `F0770_FILE_Open(G2130_GraphicsDatFileName)` and the graphics-open error boundary.
- `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/LOADSAVE.C:2333-2377` — dungeon-load open block in `F0436_LOADSAVE_LoadGameFile`, including `F0770_FILE_Open(G1059_pc_DungeonFileName)` and platform-specific dungeon fallbacks.

## DM2 canonical open anchors

The verifier normalizes file offsets versus loaded-image offsets for canonical `SKULL.EXE` from `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/Dungeon-Master-II-Skullkeep_DOS_EN.zip`:

- Header size: 6 paragraphs / 96 bytes; entry `0000:04f6`.
- Direct wrapper calls: image `0x1439` / file `0x1499`, and image `0x1464` / file `0x14c4`.
- Open wrapper entry: image `0x198f` / file `0x19ef`.
- Generic open inner entry: image `0x19af` / file `0x1a0f`.
- `INT 21h/AH=3Dh` open anchor: image `0x19da` / file `0x1a3a` bytes `B4 3D CD 21`.
- Member strings: `DUNGEON.DAT` image `0x70c05` / file `0x70c65`; `GRAPHICS.DAT` image `0x70c38` / file `0x70c98`.

Relevant `SKULL.ASM` ranges:

- `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm2-dos-asm/SKULL.ASM:6139-6153` — wrapper prepares args and calls the generic open inner routine.
- `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm2-dos-asm/SKULL.ASM:6154-6200` — generic DOS open routine reaches `AH=3Dh / int 21h`.
- `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm2-dos-asm/SKULL.ASM:4761-4783` and `4813-4826` — the only direct wrapper callers found so far pass a `BP+0Ah` path buffer.
- `/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm2-dos-asm/SKULL.ASM:461403-461472` — db-only member strings; no xrefs in this dump.

## Finding / blocker

`dosbox-x` and `dosbox` exist on N2, but this pass intentionally stops before an unattended runtime trace: the safe gate is now exact. A caller-path claim requires a trace at the `open_int21_ah3d` anchor capturing `AH` and `DS:DX` for every `INT 21h/AH=3Dh` open attempt. The pass condition is a frame where `DS:DX` resolves to `DUNGEON.DAT` or `GRAPHICS.DAT`.

Current status: `blocked_pre_trace`. Static evidence proves the open routine and member strings, but still does not bridge the startup member-string table to the `BP+0Ah` path buffer.

## Validation

```sh
python3 -m py_compile tools/verify_dm2_fileopen_trace_boundary.py
python3 tools/verify_dm2_fileopen_trace_boundary.py
python3 tools/verify_dm2_fileopen_trace_boundary.py > parity-evidence/verification/dm2_fileopen_trace_boundary_20260430/evidence.json
git diff --check
```
