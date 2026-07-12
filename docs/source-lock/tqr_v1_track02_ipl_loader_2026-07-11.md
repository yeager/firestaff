# Theron's Quest Track 02 IPL Loader Receipt

## Evidence

The operator-staged, MD5-verified raw MODE1/2352 Track 02 media was inspected
directly:

| Variant | Track 02 MD5 | CUE `TRACK 02 INDEX 01` | IPL record | sectors | load/entry |
|---|---|---:|---:|---:|---|
| JP | `b7afb338ad31be1025b53f9aff12d73a` | `00:02:74` (raw sector 224) | `0x0003a3` | 3 | `$4000` / `$4000` |
| US | `f23601102138f87c33025877767ebf76` | `00:03:00` (raw sector 225) | `0x0003a3` | 4 | `$4000` / `$4000` |

Track 01 is declared `AUDIO` in both original CUE sheets. It is narration, not
an executable. Track 02 is declared `MODE1/2352`; it is the first code/data
track and contains the boot path.

The second logical Track 02 sector (raw `INDEX 01 + 1`) contains the PC Engine
IPL information block. Its first eight user-data bytes are
`00 03 a3 <03|04> 00 40 00 40`, and it contains the literal
`PC Engine CD-ROM SYSTEM` at user-data offset `0x20`. The IPL record is
relative to Track 02 `INDEX 01`, therefore the executable begins at raw sector
`1155` (JP) or `1156` (US).

The externally documented System Card API defines `CD_READ` as function 3 at
the `$e000` jump table, hence `$e009`. Its record address is relative to the
first code track and `DH=$01` requests a local destination; `$fe/$ff` are the
VRAM modes. Sources: [PC Engine System Card documentation](https://www.zeograd.com/download/pce_bios.html)
and [PC Engine CD format research](https://retrocomputing.stackexchange.com/questions/27518/did-the-pc-engine-turbografx-super-cd-rom-have-a-standardized-file-system).

## Verified Loader Link

Within both IPL executables, CPU `$40cd` (user-data offset `0xcd`) is
`JSR $e009`. The immediately preceding setup is
`A9 00 85 FA A9 30 85 FB A9 01 85 FF`, setting `BX=$3000` and `DH=$01`.
This verifies an original loader read from Track 02 to local RAM `$3000`.

It does **not** identify the selected record value, byte count, decompressor,
or any subsequent destination. Most importantly, it is not a VDC VRAM transfer
(`DH` is not `$fe` or `$ff`). No graphics candidate, palette, tile bank,
compression format, or rendering route is promoted by this receipt. The
existing verified-media rendering block remains required.

## Second-Stage Loader Record

The same bootstrap contains a separate, fully literal System Card `CD_EXEC`
setup at CPU `$40a4`. It reads four bytes from its local table at `$40d5` as
`CL,DL,CH,AL`: `00 e7 03 11`. Under the documented API this is record
`0x0003e7`, 17 sectors, local destination `$4000`; `JSR $e00f` loads and jumps
to that destination. This is the canonical stage-two bootstrap handoff: local
RAM `$4000` is both the load address and entry address. The record begins at
raw sector 1223 (JP) or 1224 (US),
again relative to the relevant Track 02 INDEX 01.

Both 17-sector second-stage bodies contain the identical literal `CD_READ` at
CPU `$4090`: `AL=1`, `DH=1`, `BX=$3800`, then `JSR $e009`. This proves a
one-sector local-RAM read to `$3800`. It does not prove a record number: the
code does not set `CL/CH/DL` in that fixed setup, so those values remain live
state and Firestaff records them as unbound. The only other direct `$e009`
call in the body is likewise not a static VRAM request. Neither validated
stage uses `DH=$fe` or `$ff`; this is not evidence that later dynamic code
cannot transfer graphics to VRAM.

The receipt now exposes this as a bounded dynamic-read boundary: its record
register mask is exactly `CL|DL|CH`, while the authenticated setup fixes only
the one-sector count, local-RAM destination, and local destination mode. It
does not derive a record value, successor loader context, payload boundary,
or runtime route from those live registers.
