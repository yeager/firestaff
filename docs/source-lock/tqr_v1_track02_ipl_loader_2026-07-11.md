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

An authenticated US-CUE/System Card 3.0 Mednafen trace separately records the
two preceding loader calls at their executed PCs: `$40cd -> $e009` reads the
live table bytes `00 e3 03 02`, and `$40a4 -> $e00f` reads `00 e7 03 11`.
Their paired returns are `$40d0` and `$40a7`. The trace also retains CPU and
bank state but does not assign any game-data role to those calls. The
fail-closed `verify_theron_stage2_system_card_call_trace.sh` verifier locks
these exact observed rows.

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

The first two bytes are also a HuC6280 `BRK $ff` instruction. PCEDev's
HuC6280 software manual specifies that `BRK` pushes `PC+2` and dispatches
through the IRQ2 vector at `$fff6/$fff7`; the immediate `$ff` therefore
belongs to the loader's interrupt protocol. Execution continues through the
installed handler rather than treating the following manifest units as CPU
code. The System Card API accesses Track 02 through record numbers relative
to a configured CD base. The original Hu7 toolchain emits those record
constants into the executable; the finished disc has no runtime directory to
scan. Firestaff must therefore follow proven runtime record handoffs, never
infer file names or ISO-9660 paths. This explains the control-transfer shape
but still does not name the 218 manifest units. Sources: [PCEDev HuC6280
Software Manual](https://archive.org/details/PCEDev) and [PC Engine CD record
format analysis](https://retrocomputing.stackexchange.com/questions/27518/did-the-pc-engine-turbografx-super-cd-rom-have-a-standardized-file-system).

## Post-Return Continuation

On 2026-07-13, the Mednafen 1.32.1 debugger patch was verified with a clean
source-tree dry-run, then built and exercised with the authenticated US CUE
and System Card 3.0. The trace proves the observed return chain
`$cbef -> $cb2f -> $e109`. `$cb2f` is an `RTS`; its snapshot has
`$1800..$1804 = 00 00 00 02 00` and `$f5 = 00`. This is an execution and
register-state receipt only. It observes no System Card `CD_READ` request,
record number, destination, data type, bitmap, palette, object, or level.
`scripts/verify_theron_cb20_post_return_trace.sh` validates the required
ordered trace rows and deliberately makes no semantic promotion.

The next clean-source Mednafen capture was run against the authenticated US
CUE and System Card 3.0. It proves the immediate continuation
`$cb2f -> $e109 -> $c860`: `$e109` executes `JSR $C860`. At that exact
instruction, the observed CD registers are `$1800..$1804 = 00 00 00 02 00`.
This is a control-flow and register-state receipt only. It does not identify
`$c860`, issue or complete a `CD_READ`, or bind a Track 02 record, payload,
bitmap, palette, object, or level. `scripts/verify_theron_e109_post_return_trace.sh`
checks the ordered rows and rejects traces that omit either transfer.

The next authenticated US-CUE capture extends that same receipt through
`$c860`. The observed instructions are `LDA #$7B` at `$c860`, `JSR $c950` at
`$c868`, and `JSR $fe92` at `$c86b`; the transfer recorder proves the last
call enters `$fe92`. The captured CD register snapshot at the new window
starts as `$1800..$1804 = 00 00 00 02 00`. This proves only execution order
and observed registers. It does not classify `$c950` or `$fe92`, identify a
CD request, or bind a Track 02 record, payload, bitmap, palette, object, or
level. `scripts/verify_theron_c860_post_return_trace.sh` requires the ordered
`$e109 -> $c860 -> $fe92` rows and rejects the preceding trace format.

The following clean-source US-CUE capture enters the actual `$fe92` System
Card routine and returns through `$febe` to `$c86e`. It reads `$18c5`, then
the observed `$18c1 = aa`, `$18c2 = 55`, and `$18c3 = 03` handshake state;
the CD data registers remain `$1800..$1804 = 00 00 00 02 00` throughout the
routine. The trace consequently proves a status/handshake path, not a CD data
request. It binds no Track 02 record, destination, size, payload, bitmap,
palette, object, or level. `scripts/verify_theron_fe92_status_trace.sh`
requires the exact entry, observed status reads, RTS, and return to `$c86e`.

The same authenticated capture now extends through the first observed
post-status caller route. At `$c86e`, the carry branch to `$c88c` is not taken
and execution continues through `$c870`, `$c873`, `$c875`, and the two
observed `$18c0` writes at `$c877` and `$c87c`. The command latch is observed
as `$d0` at `$c87a`; execution then calls `$c950`, branches to `$c897`, and
reads `$2241`. This is deliberately recorded only as live control flow and
register behavior. It does not identify a `CD_READ`, Track 02 record,
destination, size, payload, bitmap, palette, object, or level.
`scripts/verify_theron_c86e_command_trace.sh` requires that exact ordered
receipt and rejects traces that skip any transition.

After authenticated Run input, the next real controller sequence is now
captured from `$c897` through the System Card `$e900` path. It executes
`LDA #$81`/`STA $1801`, `LDA #$60`/`STA $1800`, then
`LDA #$ff`/`STA $1801`; the trace observes `$1801` become `$81` and then
`$ff`. The following `$ea27/$ea35` helper sets `$1802` bit 7, with the
observed state `$1800/$1802 = $90/$80` at `$ea3a`. This is an authenticated
instruction/register receipt, not a semantic API classification: it does not
prove a `CD_READ`, record number, RAM destination, transfer size, payload, or
any game asset. `scripts/verify_theron_post_latch_cd_controller_trace.sh`
requires this ordered receipt and rejects pre-Run or incomplete traces.

The next observed System Card route is the first controller dispatch after
that completed exchange. The source-built Mednafen trace reaches `$e97a`,
where it reads the live `$224c,X` byte, and then reaches `$e981` after the
corresponding controller-register transition. This binds only the order of
the original CPU locations and their observed register/RAM bytes. In
particular, `$224c` has not been classified as a command, `CD_READ` record,
size, destination, payload, bitmap, palette, object, or level. The new
`scripts/verify_theron_post_latch_cd_dispatch_trace.sh` requires the completed
first exchange before the ordered `$e97a -> $e981` receipt, so later trace
work cannot silently promote an unrelated polling iteration.
