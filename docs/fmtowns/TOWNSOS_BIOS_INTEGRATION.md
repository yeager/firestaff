# TownsOS BIOS integration research

Session 2026-08-07 — research findings for eventually running the
real DM1/CSB/DM2 FM Towns executables through a hosted TownsOS
BIOS. This document consolidates what has been established from
public disassembly and Tsugaru emulator source; no synthesis.

## The `call fs:[0x20]` pattern

DM1 FM Towns (EDM.EXP, TMENU.EXP) use the Phar Lap 386|DOS-Extender
runtime. Real-mode BIOS access from protected-mode code goes through:

```
mov ax, 0x250f
int 0x21           ; Phar Lap: switch to real mode
call fs:[0x20]     ; call TBIOS via real-mode selector fs=0x110
mov ax, 0x250e
int 0x21           ; Phar Lap: switch back to protected mode
```

Verified in Firestaff's `dm1_v1_fmtowns_tmenu_input.h` — the pattern
appears at TBIOS_POLL (TMENU.EXP:0xa130) and in EVENT_HELPER
(0x30aa, 0x30d0) with subfunction codes AH=7 (poll) and AH=8 (read).

Selector 0x110 is the Phar Lap real-mode selector; `fs:[0x20]` is
the TownsOS TBIOS entry point vector installed by Phar Lap init.

## Physical BIOS ROM

**FMT_F20.ROM** (Fujitsu FM Towns II F20) is a TownsOS system ROM. Copy it
from a real FM Towns machine or an authorised archive when running the full
emulator. **FMT_FNT.ROM** is the separate 256 KiB Kanji/font device that
TBIOS programs read for 16×16 JIS glyphs. Firestaff's narrow glyph shim must
consume `FMT_FNT.ROM`, never pretend that F20 contains the font table.

## TBIOS version fingerprints (from Tsugaru source)

Tsugaru identifies TBIOS by 4x 8-byte string fields at fixed offsets
in the BIOS blob. Observed versions and their identifier strings:

| TBIOS version | Fingerprint | Date | Location |
|---|---|---|---|
| V31L22A | `V31L22A\0\0` + `89/03/08towns   tbios` | 1989-03-08 | 0x1F750 |
| V31L23A | `V31L23A\0\0` + `90/09/21towns   tbios` | 1990-09-21 | 0x1F3D0 |
| V31L31_90 | `V31L31\0\0` + `90/11/21towns   tbios` | 1990-11-21 | 0x1F3D0 |
| V31L31_91 | `V31L31\0\0` + `91/10/05towns   tbios` | 1991-10-05 | 0x100000 |
| V31L31_92 | `V31L31\0\0` + `92/10/16towns   tbios` | 1992-10-16 | 0x100000 |
| V31L31_93 | `V31L31\0\0` + `93/01/07towns   tbios` | 1993-01-07 | 0x100000 |
| V31L35 | `V31L35\0\0` + `93/10/15towns   tbios` | 1993-10-15 | 0x100000 |
| V31L35 (FSC11) | `V31L35\0\0` + `94/12/03towns   tbios` | 1994-12-03 | 0x100000 |

DM1 FM Towns (HMA-240, 1992) targets V31L31_92 or V31L31_93 based on
release timing.

## Hardware I/O ports (from Tsugaru `keyboard.cpp`)

TBIOS ultimately reads keyboard state via:

- `TOWNSIO_KEYBOARD_DATA`      = **0x600** — read to poll queued scancode
- `TOWNSIO_KEYBOARD_STATUS_CMD` = **0x602** — command / status register
- `TOWNSIO_KEYBOARD_IRQ`       = **0x604** — IRQ enable

Mouse state (per Tsugaru's Wing Commander 2 integration) is
communicated via a memory-mapped event queue:

```
WC2_EVENTQUEUE_BASE_ADDR   = 0x3CC       ; queue base
WC2_EVENTQUEUE_LAST_OFFSET = 0x6E4 - 0x3CC
WC2_EVENTQUEUE_READ_PTR    = 0x6EC - 0x3CC
WC2_EVENTQUEUE_FILLED      = 0x6F4 - 0x3CC
```

Queue length = 100 8-byte records (matching Firestaff's own TMENU
disassembly at `dm1_v1_fmtowns_tmenu_input`: 8-byte event records
at 0x575c stride 8, queue head at 0x5890).

## Recommended integration path

Two options:

### Option A — Firestaff hosts Tsugaru as a subprocess

Firestaff launches Tsugaru with the FM Towns ROM directory as the first
argument and `-CD track01.iso` (or the original `.cue`/`.mds`). Tsugaru runs
the real TMENU.EXP + EDM.EXP. The `FMT_F20.ROM` path used by Firestaff's
fail-closed TBIOS shim is a separate BIOS input and must not be confused with
Tsugaru's ROM-directory argument.
Firestaff's role reduces to input/screen bridging (SDL3 to
Tsugaru's I/O bus).

**Pro**: Zero synthesis; whole game runs on real bytes.
**Con**: External dependency; Tsugaru's licence (BSD-3 with
attribution) must be honoured; no Firestaff parity control over
rendering pipeline.

### Option B — Minimal TBIOS shim inside Firestaff

Firestaff embeds a small TBIOS-emulation layer that only
implements the specific `fs:[0x20]` subfunctions DM1/CSB/DM2 use
(mouse poll, keyboard poll, timer, video mode). Every call
byte-checked against real TBIOS ROM behaviour.

**Pro**: Firestaff stays self-contained; parity-verifiable per
subfunction.
**Con**: Big investment; every game may use different subfunctions.

## What is decodable today without a host

Even without a running TownsOS, these Firestaff modules already
consume real disc data and would be direct beneficiaries of any
BIOS integration:

- `dm1_v1_fmtowns_font_rasteriser` — needs no BIOS.
- `dm1_v1_fmtowns_menu_render` — panel geometry from region table;
  glyphs from font raster. Needs no BIOS.
- `dm1_v1_fmtowns_tmenu_input` — schema only; BIOS runs the poll.
- `dm1_v1_fmtowns_jdm_font` — ASCII rendering complete via shared
  raster (byte-verified 2026-08-07 as identical to English disc);
  Shift-JIS pairs are the primary BIOS-integration beneficiary,
  since TownsOS TBIOS `fs:[0x20]` glyph-fetch is the ONLY authorised
  kanji glyph source. The render API already exposes a
  `sjis_glyph_draw` callback slot that a Tsugaru-backed shim can
  fill without any change to callers.
- Menu-BSS symbols — layout only; BIOS + Phar Lap runtime own the
  actual reads/writes.

The menu paint pipeline in Firestaff today is display-only. Input
requires option A or B above.

## Host contract (implemented 2026-08-07)

The C host ABI a Tsugaru bridge or in-process shim implements is
now defined in `include/fmtowns_bios_host.h`:

- Four canonical slots: `FMTOWNS_BIOS_SLOT_TBIOS`, `..._SECONDARY`,
  `..._TIMING`, `..._HARDWARE_INIT`.
- Register bundle `fmtowns_bios_regs_t` carries eax/ebx/ecx/edx +
  esi/edi/ebp + ds/es/fs/flags across the far-call boundary.
- I/O port r/w for 0x04E9 SOUND_INT_REASON + 4 CMOS/RS232C ports.
- Direct `tbios_fetch_sjis_glyph` entry — the specific subfunction
  the JDM font module consumes. The JDM font render API's
  `sjis_glyph_draw` callback can trampoline straight into this
  entry with no game-code changes.

Default binding is fail-closed: every dispatcher returns
`FMTOWNS_BIOS_HOST_UNBOUND` until a caller registers a real host
via `fmtowns_bios_host_bind_pc34()`. Tests cover fail-closed,
partial-host (unsupported slot), bad-slot, bad-args, and bound-
dispatch paths.

## Option-B minimal TBIOS shim (implemented 2026-08-07)

`include/fmtowns_tbios_shim.h` + `src/shared/fmtowns_tbios_shim.c`
ship the Option-B path. The shim:

- Reads a real Fujitsu `FMT_FNT.ROM` via
  `fmtowns_tbios_shim_load_rom_pc34(bytes, size)`. It accepts exactly the
  native 256 KiB `8192 × 32`-byte device extent and rejects incomplete or
  oversized input. The external verification corpus used here has SHA-256
  `aa9e9565d3047c51ee418712be8daa391b0d8f21f92fc02e4616e3523ca50b9d`.
- Exposes a `fmtowns_bios_host_t*` via
  `fmtowns_tbios_shim_host_pc34()` that consumers can bind through
  `fmtowns_bios_host_bind_pc34()`. Only `tbios_fetch_sjis_glyph` is
  populated — every other slot returns `UNSUPPORTED`, which is the
  correct behaviour when only the JDM text-render surface is being
  served (games' Phar Lap thunks would need a full CPU emulator
  such as Tsugaru).
- Converts Shift-JIS to JIS X 0208, then applies Tsugaru's exact
  `TownsPhysicalMemory::KanjiROMAccess::FontROMCode()` block mapping to
  `FMT_FNT.ROM`'s 8192 16×16 slots (32 bytes/glyph). No host font or linear
  F20 offset is involved.
- Fail-closed contract: `UNBOUND` until a ROM is loaded; `FAILED`
  for pairs that fall outside a recognised region; `BAD_ARGS` for
  undersized destination buffers.

Tests cover fail-closed, malformed extent rejection, Tsugaru-compatible
slot mapping with planted glyph bytes, out-of-range trail, buffer-too-small,
unload and end-to-end dispatch through `fmtowns_bios_host_fetch_sjis_glyph_pc34()`.
When `FIRESTAFF_FMTOWNS_FONT_ROM` names the authorised external FNT ROM, the
test also checks F31J chooser glyph `ど` against its captured native glyph
receipt (`af14d837`).

M11 uses the same opt-in path for CSB's Japanese C06 selector. It reads the
exact 256 KiB `FMT_FNT.ROM` into process memory, binds only the glyph callback
and presents the source-owned 640×400 chooser. A missing, short or oversized
ROM leaves that C06 route unavailable; Firestaff never falls back to a macOS,
SDL or bundled font.

## Tsugaru bridge contract (implemented 2026-08-07)

`include/fmtowns_tsugaru_bridge.h` + `src/shared/fmtowns_tsugaru_bridge.c`
define the optional-adapter path for the remaining slots. The bridge
is a pure-C ABI that a separately built Tsugaru wrapper populates:

- `fmtowns_tsugaru_bridge_vtable_v1_t`: init + shutdown + one entry
  per canonical slot + I/O port r/w. Required entries: `init`,
  `shutdown`, `call_tbios`. Optional (dispatcher returns UNSUPPORTED
  if NULL): `call_secondary`, `call_timing`, `call_hardware_init`,
  `io_port_read_u8`, `io_port_write_u8`.
- `fmtowns_tsugaru_bridge_register_pc34(vt, bios_rom_path)` copies the
  vtable, runs `init(bios_rom_path)`, and — on success — exposes a
  `fmtowns_bios_host_t*` via `fmtowns_tsugaru_bridge_host_pc34()`
  that the caller binds through `fmtowns_bios_host_bind_pc34()`.
  Init failure rolls back cleanly; shutdown is NOT called for a
  failed init.
- The bridge deliberately leaves `tbios_fetch_sjis_glyph = NULL`.
  That surface is served by the Option-B shim (font-ROM table
  lookup); a full-emulator Tsugaru call is overkill for a single
  glyph fetch, and calling both means the shim wins.
- Version discipline: the vtable symbol is version-suffixed
  (`fmtowns_tsugaru_bridge_vtable_v1`). Any ABI break bumps the
  suffix so old bridges refuse to bind rather than mismatch silently.

Tests cover NULL vtable, missing required entry, init-failure
rollback, successful registration with a partial vtable (UNSUPPORTED
for NULL entries), bound dispatch through the global host, re-register
(triggers shutdown of the prior binding), and double-unregister.

The bridge itself is expected to live in a separate
`libfirestaff_tsugaru_bridge.{dylib,so,dll}` built from Tsugaru with
an `extern "C"` shim. Users who never build the bridge get:

  * JDM text render: works via Option-B shim
  * Every other BIOS entry: UNBOUND (fail-closed, no fabrication)

## References

- Tsugaru source: https://github.com/captainys/TOWNSEMU
  - `src/towns/tbiosid.cpp` — TBIOS version fingerprints
  - `src/towns/keyboard/keyboard.cpp` — keyboard I/O ports
- FMT_F20.ROM: https://github.com/Abdess/retrobios/blob/main/bios/Fujitsu/FM%20Towns/FMT_F20.ROM
- Emulator background: https://gekk.info/articles/fmtowns.html
- DM1 TMENU disassembly: `docs/dm1/fmtowns_real_data_hashes.json`
  + `include/dm1_v1_fmtowns_tmenu_input.h`
