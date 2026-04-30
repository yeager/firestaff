# DM2 INT 21h file-open trace salvage (2026-04-30)

Scope: narrow N2-only follow-up for the DM2 startup file-open dynamic trace blocker. This artifact does **not** claim runtime, rendering, asset, or gameplay parity.

## Runnable artifact

`tools/trace_dm2_int21_file_open_next.py` extracts the canonical DOS DM2 archive on N2, starts DOSBox-X with `-log-int21` and `-log-fileio`, and accepts only a frame at `INT 21h/AH=3Dh` that includes `DS:DX` and resolves to `DUNGEON.DAT` or `GRAPHICS.DAT`.

Run:

```sh
python3 tools/trace_dm2_int21_file_open_next.py --timeout 75
```

Exit code `0` means a DS:DX-resolved target frame was captured. Exit code `2` means the run produced a precise blocker rather than proof.

## Source policy / anchors

- ReDMCSB source is limited to DM/CSB comparison only: `/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`.
- DM2 anchors are N2-local only: `/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/`.
- Prior gate `473d17c` supplied the canonical `open_int21_ah3d` anchor: image `0x19da`, file `0x1a3a`, bytes `B4 3D CD 21`.
- `SKULL.ASM:6154-6200` is the generic open routine: `mov si,[bp+0Eh]`, leading-space trim, `mov dx,si`, `mov ah,3Dh`, `int 21h`.

## Result on N2

The 75-second run observed DOSBox-X file-I/O opening `C:DATA\\GRAPHICS.DAT`, but no debugger/register frame containing `AH=3Dh` plus `DS:DX`. Therefore the dynamic caller-path proof remains blocked until a debugger path can break/watch the `open_int21_ah3d` anchor and dump DS:DX.

See `evidence.json` for command, tool paths, target-open excerpt, source policy, anchors, and blocker text. The raw DOSBox-X trace was intentionally left untracked because `parity-evidence/verification/*` is ignored and the JSON carries the needed excerpt.
