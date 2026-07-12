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
CPU `$4090`: `AL=1`, `DH=1`, `BX=$3800`, then `JSR $e009`. Static inspection
proves a one-sector local-RAM read and that `CL/CH/DL` remain live.

On 2026-07-12, source-built Mednafen with the PCE/HuC6280 debugger and a
minimal CD READ trace ran the authenticated original CUEs. The first read
immediately after the 17-sector stage-two transfer was:

| Variant | Stage 2 raw sectors | PCE CD LBA | Track 02 record | sectors | destination |
|---|---|---:|---:|---:|---|
| JP | `1223..1239` | `$1205` | `$04df` | 1 | local RAM `$3800` |
| US | `1224..1240` | `$10a1` | `$04e0` | 1 | local RAM `$3800` |

The CUE TOC maps those LBAs to the shown Track 02-relative records. This binds
the live `CL|DL|CH` state at `$4090`; it does not classify the loaded payload,
derive a graphics format, or authorize a VRAM transfer. Neither validated
stage uses `DH=$fe` or `$ff`.

## Dynamic Payload Shape

Both traced one-sector payloads have the same bounded structural envelope:
the first two big-endian words are `$00ff` and `$0308`, bytes `$520..$7ff`
are zero-filled, and the nonzero `$000..$51f` prefix fits exactly 218
six-byte units after the four-byte lead. Firestaff exposes this as a
hash-gated manifest receipt only. The entries have no assigned object, level,
text, palette, or graphics semantics yet.
