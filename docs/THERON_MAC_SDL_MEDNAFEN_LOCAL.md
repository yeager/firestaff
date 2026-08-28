# Local macOS runbook: Theron, SDL2 and Mednafen

This document describes the local Firestaff environment on Bosse's Mac. It is
a working note for reuse, not game data and not a claim that a capture is
semantically admitted.

## Fixed paths on the external disk

```text
Firestaff-repo:
  /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp

Firestaff-build:
  /Volumes/Extern-disk/firestaff-theron-active2-build

Real SDL2 prefix (Cocoa, not sdl2-compat):
  /Volumes/Extern-disk/theron-sdl2-real-20260809b/install

Clean Mednafen source tree:
  /Volumes/Extern-disk/theron-mednafen-clean-source-20260809/mednafen

Instrumented Mednafen build:
  /Volumes/Extern-disk/mednafen-firestaff-real-sdl2-20260811

Instrumented binary:
  /Volumes/Extern-disk/mednafen-firestaff-real-sdl2-20260811/install/bin/mednafen

Clean PCE video binary without Firestaff capture hooks:
  /Volumes/Extern-disk/mednafen-1.32.1-clean-pce-20260811/src/mednafen
```

Real game media and the System Card are stored at:

```text
/Users/bosse/.firestaff/data/theron/TQUS02.bin
/Users/bosse/.firestaff/data/theron/TQUS19.iso
/Users/bosse/.mednafen/firmware/syscard3.pce
```

Always use hash-verified files from this local data directory. Do not put game
data in the repository or create replacement media.

## Build real SDL2 once

On this machine, Homebrew's `sdl2` installation is `sdl2-compat`. It must not
be used as evidence of authentic Cocoa/Quartz input. Instead, build SDL2 from
an official SDL2 source archive onto the external disk:

```bash
SDL_ROOT=/Volumes/Extern-disk/theron-sdl2-real-20260809b
SDL_ARCHIVE=/Volumes/Extern-disk/SDL2-2.30.9.tar.gz

mkdir -p "$SDL_ROOT/source" "$SDL_ROOT/install"
tar -xzf "$SDL_ARCHIVE" -C "$SDL_ROOT/source" --strip-components=1
cd "$SDL_ROOT/source"
./configure --prefix="$SDL_ROOT/install" \
  --disable-video-wayland --disable-video-x11 \
  --disable-video-kmsdrm --disable-video-vulkan \
  --disable-audio-alsa --disable-audio-pulseaudio \
  --disable-audio-jack --disable-audio-pipewire
make -j"$(sysctl -n hw.ncpu)"
make install
```

Verify the link before capture:

```bash
otool -L \
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
scripts/verify_theron_mednafen_sdl2_runtime.sh \
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
```

The verification must show a direct link to the SDL2 prefix's
`libSDL2-2.0.0.dylib` and must not show `sdl2-compat`.

## Build instrumented Mednafen

The source tree must be clean because Firestaff scripts apply several
instrumentation patches. Reuse the same clean source tree and always choose a
new, explicit build root when the patch series changes:

```bash
cd /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp

FIRESTAFF_MEDNAFEN_SDL2_PREFIX=/Volumes/Extern-disk/theron-sdl2-real-20260809b/install \
FIRESTAFF_MEDNAFEN_BUILD_ROOT=/Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809 \
scripts/build_mednafen_theron_irq2_trace.sh \
  /Volumes/Extern-disk/theron-mednafen-clean-source-20260809/mednafen
```

The build script copies the source, applies Firestaff capture patches, builds,
and runs the SDL2 link check. The Mednafen build is a local working artifact;
it must not be committed.

## Capture modes

### Pixel-exact video

First check the window title. `Dungeon Master - Theron's Quest (USA)` is
Mednafen; a visible `DOSBox-X` window is a different emulator and must not be
used as a Theron reference. If the image is colour noise, has the wrong HUD, or
shows entirely different game graphics, close that instance before evaluating
any graphics.

Do not use an old global Mednafen profile for troubleshooting. The clean
profile must contain only the user-owned System Card file as a symbolic link
at `$MEDNAFEN_HOME/firmware/syscard3.pce`; game data and BIOS must never be
copied into the repository. Then launch with the flags below. This locks both
the emulator and video route to the verified Theron run.

For Theron, Mednafen must use the original pixel steps without interpolation:

```ini
pce.videoip 0
pce.shader none
pce.special none
pce.stretch aspect
pce.xscale 2.000000
pce.yscale 2.000000
```

`pce.videoip 1` and `aspect_mult2` produce a filtered/interpolated image that
can look wrong compared with the original PCE graphics. The local profile and
the capture profile on the external disk therefore use the settings above.

Use the clean PCE video binary when reviewing the image or playing Theron. The
capture binary is primarily for tracing. The current build was rebuilt against
the same clean `huc6280.cpp` as the reference binary and does not read extra
CPU operands before each instruction. A visual check on 2026-08-11 with the
same authentic `TQUS.cue`, System Card and pixel profile gave a clean startup
image with the clean binary, while the then-current capture binary produced
corrupt magenta/olive streaks. This was a fault in the old local capture build,
not in the original Track 02 media or pixel-profile scaling. The rebuilt binary
is the one named above.

The older capture binary must not be used. It produced corrupt magenta/olive
streaks. The current capture binary retains CD/FIFO/RAM observations through
its hooks, but must still undergo the same visual smoke test before a new
runtime capture; a capture that distorts the image is invalid even if its
receipts appear formally complete.

Verified capture binary after the correction (2026-08-11):

```text
MD5 1ec797bb7d1aea4d756521686d7b0c36
```

The previous corrupt image was caused by a capture hook in HuC6280 `WrMem()`
that performed two mapped writes for the same CPU byte. The second write
affected VDC/VCE and produced magenta/olive image faults. The hook now only
observes and performs the original single `WriteMap` write. The clean video
binary and capture binary were subsequently compared using the same authentic
Track 02, System Card, video flags and temporary Mednafen home at both 3 and
8 seconds.

Start the video check as follows:

```bash
MEDNAFEN_HOME=/Users/bosse/.mednafen \
/Volumes/Extern-disk/mednafen-1.32.1-clean-pce-20260811/src/mednafen \
  -pce.stretch aspect -pce.videoip 0 -pce.special none \
  -pce.xscale 2.000000 -pce.yscale 2.000000 \
  /Volumes/Extern-disk/theron-full-media-20260810/TQUS.cue
```

For capture, use the same graphics flags with the current capture binary:

```bash
MEDNAFEN_HOME=/Users/bosse/.mednafen \
/Volumes/Extern-disk/mednafen-firestaff-real-sdl2-20260811/install/bin/mednafen \
  -pce.stretch aspect -pce.videoip 0 -pce.special none \
  -pce.xscale 2.000000 -pce.yscale 2.000000 \
  /Volumes/Extern-disk/theron-full-media-20260810/TQUS.cue
```

Stop the process with `Ctrl-C` when the check is complete. Do not run an
instrumented capture concurrently with the same Mednafen home.

### Headless smoke-test

Dummy video is useful for checking that patches and receipt format work, but it
is not a Quartz/app capture:

```bash
THERON_STATE="/Users/bosse/.mednafen/mcs/Dungeon Master - Theron's Quest (USA).bee0988239a817f20a64cd38fc8caeac.mc0"

THERON_MEDNAFEN_HOME=/Users/bosse/.mednafen \
MEDNAFEN_BIN=/Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen \
THERON_US_CUE=/Volumes/Extern-disk/theron-capture-input/TQUS-minimal.cue \
THERON_SYSTEM_CARD=/Users/bosse/.mednafen/firmware/syscard3.pce \
THERON_LIVE_TRACE_OUTPUT=/Volumes/Extern-disk/theron-auth-capture.trace \
THERON_CAPTURE_AUTOLOAD_STATE="$THERON_STATE" \
THERON_CAPTURE_SECONDS=20 \
THERON_CAPTURE_STARTUP_GRACE=5 \
THERON_CAPTURE_SDL_VIDEODRIVER=dummy \
THERON_CAPTURE_SOUND=0 \
scripts/capture_theron_mednafen_live_trace.sh
```

A headless result must not alone open RNG, spawn, AI, T700, T900 or graphics
semantics. Sidecars must have the right media, System Card and disassembly
provenance and pass their respective validators.

The capture script first sends the requested soft timeout signal, but performs
a controlled forced termination five seconds later if Mednafen only handles
the signal internally and continues its loop. This leaves no emulator process
or locked isolated Mednafen home after a failed capture. An interrupted or
negative receipt is still not semantic evidence.

### Authentic Cocoa/Quartz input

Leave `THERON_CAPTURE_SDL_VIDEODRIVER` empty or use `cocoa`. Mednafen must be
the foreground application, and Terminal/the running helper process must have
Accessibility/Input Monitoring permissions in macOS. The capture script uses
the checked-in Swift/Quartz helper and requires PID-bound focus.

Example PCE input through the host:

```bash
THERON_CAPTURE_SDL_VIDEODRIVER=cocoa \
THERON_CAPTURE_HOST_KEY_SEQUENCE='run@8,i@480,i@900' \
THERON_CAPTURE_HOST_KEY_HOLD=1 \
THERON_CAPTURE_INPUT_ROUTE=pid \
scripts/capture_theron_mednafen_live_trace.sh
```

For a reproducible test without Quartz, the corresponding PCE buttons may be
sent through the instrumented scripted-input route, but that is an emulator
run and not evidence of physical macOS input.

## Quick check before reuse

```bash
git -C /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp status --short --branch
otool -L /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen \
  | grep 'libSDL2-2.0.0.dylib'
scripts/verify_theron_mednafen_sdl2_runtime.sh \
  /Volumes/Extern-disk/theron-mednafen-real-sdl2-capture-20260809/install/bin/mednafen
```

If verification shows `sdl2-compat`, do not rebuild game semantics or call the
result authentic. Instead point `FIRESTAFF_MEDNAFEN_SDL2_PREFIX` to the real
SDL2 prefix above and rebuild Mednafen.

## Tsugaru on Mac: FM Towns

Tsugaru applies to Firestaff's FM Towns route, not to Theron's Quest on PC
Engine. Theron Track 02 and its HuC6280/System Card capture use Mednafen
above. Tsugaru is used for real FM Towns CD images, the original TownsOS/TBIOS
and the separate FM Towns execution route for DM1, CSB and DM2.

Tsugaru takes a directory of FM Towns ROM files as its first argument. This is
not the same as Firestaff's single `FMT_FNT.ROM` for the narrow Shift-JIS glyph
shim; `FMT_F20.ROM` is the separate system ROM for a full TownsOS run. Keep the
emulator, BIOS and game media on the external disk and never put them in Git:

```bash
TOWNS_ROOT=/Volumes/Extern-disk/TOWNSEMU
TSUGARU_GUI="$TOWNS_ROOT/gui/build/main_gui/Tsugaru_GUI.app"
TSUGARU_CUI="$TOWNS_ROOT/gui/build/main_cui/Tsugaru_CUI.app/Contents/MacOS/Tsugaru_CUI"
TOWNS_ROM_DIR=/Volumes/Extern-disk/FirestaffUserData/firmware/fm-towns-rom
TOWNS_DISC=/Volumes/Extern-disk/FirestaffUserData/data/dm1/fmtowns_extract/Dungeon-Master_FM-Towns_JA-EN/track01.iso
```

The local data directory also contains real FM Towns media for CSB and DM2;
only change `TOWNS_DISC` to the relevant `track01.iso` or `.cue`. An ISO is
suitable for data-only testing. For a disc with CD audio, use the original
complete `.cue`/`.bin` or `.mds`/`.mdf`. Tsugaru supports ISO, CUE and MDS, but
upstream recommends MDS when audio tracks are present because CUE can be
ambiguous around PREGAP/INDEX 00.

### Build Tsugaru on the external disk

This is the verified macOS layout from Tsugaru's upstream repository. `public`
must be below `gui/src`; do not build directly from the repository root:

```bash
git clone https://github.com/captainys/TOWNSEMU.git "$TOWNS_ROOT"
git -C "$TOWNS_ROOT/gui/src" clone https://github.com/captainys/public.git public
cmake -S "$TOWNS_ROOT/gui/src" -B "$TOWNS_ROOT/gui/build"
cmake --build "$TOWNS_ROOT/gui/build" --config Release --parallel

# Tsugaru's macOS GUI needs the CUI program in the same app environment.
cp "$TOWNS_ROOT/gui/build/main_cui/Tsugaru_CUI.app/Contents/MacOS/Tsugaru_CUI" \
   "$TOWNS_ROOT/gui/build/main_gui/Tsugaru_GUI.app/Contents/MacOS/"
```

If the source tree or build already exists, do not run `git clone` again.
Instead, check that these files exist:

```bash
test -x "$TSUGARU_CUI"
test -d "$TSUGARU_GUI"
test -d "$TOWNS_ROM_DIR"
```

### Start the GUI

On macOS, launch the `Tsugaru_GUI` app bundle. The first time, choose the real
Towns ROM directory in Tsugaru settings, then open the disc image through
File/Open. Launch from Terminal if the app bundle is not visible in Finder:

```bash
open "$TSUGARU_GUI"
```

If macOS blocks a locally built app, open it once with Ctrl-click → Open and
approve the local development build. Do not use a downloaded BIOS or game
replacement to work around the problem.

### Start the CUI reproducibly

This is the useful command line for Firestaff's local FM Towns media:

```bash
"$TSUGARU_CUI" \
  "$TOWNS_ROM_DIR" \
  -CD "$TOWNS_DISC" \
  -GAMEPORT0 KEY \
  -SCALE 160
```

Show all local flags with:

```bash
"$TSUGARU_CUI" -HELP
```

`-TOWNSTYPE MARTY` is used only with a real Marty ROM and must not be set for a
full FM Towns ROM. `-CMOS /path/CMOS.BIN` may be added to save a separate CMOS
profile. Quit the emulator using its normal Quit command so CMOS can be
written; forced termination can leave it unsaved.

### Controls

Firestaff's Theron runtime uses a normal SDL mouse and keyboard, not a joystick
pointer that jumps between visible objects. The mouse continuously reports its
current position in the source-mapped 320x200 view; a mouse-move event selects
no object. Mouse button 1 (left) is PC Engine Button I and mouse button 2
(right) is Button II. The physical middle button is deliberately unbound.

In the dungeon, the keyboard is:

```text
Up / W:         forward
Down / S:       backward
Left / A:       turn left
Right / D:      turn right
```

Held keys are sampled at Theron's input boundary rather than relying on
macOS/SDL autorepeat. A short touch corresponds to Button I and a long touch
to Button II. Startup's own buttons use the same Button I/II route, so the
control scheme does not need to change between menu, handoff and dungeon.

With `-GAMEPORT0 KEY`, Tsugaru uses keyboard emulation for gamepad 0:

```text
Directions:  arrow keys
Action A:   A
Action B:   S
Action C:   Z
Action D:   X
```

These are Tsugaru's FM Towns controls and must not be confused with Firestaff's
Theron binding, `WASD` + mouse buttons 1/2. For a physical controller, use
`PHYS0`–`PHYS3`; if directions are reported as an analogue stick, use
`ANA0`–`ANA3` instead of `KEY`.

### What Tsugaru proves — and does not prove

Tsugaru is the right tool for original FM Towns TownsOS/TBIOS, BIOS ROM,
keyboard I/O and native `TMENU.EXP`/`EDM.EXP` execution. It does not
automatically prove Firestaff's own FM Towns renderer or Theron semantics.
Firestaff's separate C bridge remains fail-closed until a real Tsugaru wrapper
binds TBIOS, timing and I/O calls. For source reference and this boundary, see
`docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md`.

The source for the command line, ROM directory, CD flag, control mapping and
CUE limitations is Tsugaru's official README:
<https://github.com/captainys/TOWNSEMU#starting-the-command-line-program>.

### Firestaff's Tsugaru boundary

The source-bound FM Towns documentation is in
`docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md`. It describes two separate routes:

1. Tsugaru as a subprocess, where the real `TMENU.EXP`/`EDM.EXP` programs run
   through Tsugaru's TownsOS/TBIOS.
2. A minimal, fail-closed TBIOS shim in Firestaff for verified BIOS glyph and
   TBIOS calls.

The optional C bridge is declared in `include/fmtowns_tsugaru_bridge.h` and
implemented in `src/shared/fmtowns_tsugaru_bridge.c`. This is not the same as
Tsugaru already being linked in production: without a separate Tsugaru wrapper
the bridge is `UNBOUND`, and Firestaff must not invent BIOS, disc or image
results.

For Shift-JIS glyphs, the real BIOS-ROM-bound local shim is sufficient under
the existing documentation; a full Tsugaru wrapper is needed only for TBIOS
calls, I/O and actual FM Towns program execution. This does not affect the
Mednafen capture chain for Theron.

## Most recently verified Mednafen run

On 2026-08-09, the instrumented binary was run with the real SDL2 prefix, the
complete US CUE, verified US Track 02 (`TQUS02.bin`) and System Card 3.0. The
bounded scripted replay produced the following source-bound transport receipt:

```text
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
raw_sector_spans=161
scsi_read_commands=51
scsi_read_sector_bindings=161
authenticated_cd_ram_receipts=2
main_ram_consumer_reads=4096
vce_palette_snapshot_bytes=1024
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
transition=observed
```

This proves CD-sector/FIFO → RAM transport and an observed main-RAM consumer,
but not which level, object, tile or creature owner reads the bytes. The run
therefore must not open RNG, spawn, AI, attack, damage, loot, generators, T700
or T900. Scripted replay is also an emulator-internal input route, not physical
macOS input.

The trace base is outside the repository on the external disk:

```text
/Volumes/Extern-disk/theron-auth-capture-full-scripted-20260809.trace
```

The older run without authenticated CD→RAM receipts must not be merged with
this run; capture sessions remain separate.

### Latest operator-driven transition receipt

The external, instrumented Mednafen process subsequently reached an observed
startup→dungeon transition in one session with the same authenticated US Track
02 and System Card. The receipt file is local on the external disk and contains:

```text
track02_md5=f23601102138f87c33025877767ebf76
system_card_md5=ff1a674273fe3540ccef576376407d1d
input_transactions=131072
host_key_events=23
cd_irq_callbacks=25
raw_sector_spans=134
scsi_read_commands=38
scsi_read_sector_bindings=134
byte_exact_fifo_ram_destinations=0
byte_exact_origin_ram_receipts=256
authenticated_cd_ram_receipts=256
game_main_ram_e009_dispatches=31
main_ram_consumer_reads=65536
main_ram_target_reads=0
main_ram_target_writes=0
spawn_consumer_reads=0
spawn_register_samples=136
spawn_preconsumer_4644_samples=33
spawn_helper_4667_samples=96
spawn_entry_b0e5_samples=0
rng_consumer_samples=0
rng_code_windows=0
vdc_vram_snapshot_bytes=65536
vce_palette_snapshot_bytes=1024
transition=observed
```

This proves an authenticated CD→RAM and main-RAM loader chain, but not a
game-owned level, object, tile, creature, RNG, T700 or T900 consumer.
`spawn_consumer_reads=0`, `main_ram_target_reads=0` and the absence of `$B0E5`
are negative evidence; they must not be replaced by synthetic records or
host-side formulas. The receipt file is not game data and remains outside the
repository:
`/Users/bosse/.firestaff/cache/theron/manual-capture/out/theron.transition`.

The bounded main-RAM sidecar is about 8.7 MiB. Firestaff's parser now permits
up to 16 MiB for this explicitly bounded capture, so the real 65,536-sample
file can be verified byte- and PC-wise. This does not change that
`target_2600_bytes_present=0` and `semantic_publication_allowed=0`.

### Extended spawn-register window

By default, the capture script sends
`FIRESTAFF_THERON_SPAWN_REGISTER_SAMPLE_LIMIT=65536` to the external
instrumented Mednafen binary. The limit is explicit and validated to
`1..1048576`; it changes neither emulator data nor semantics. It can be
lowered for diagnostics with `THERON_CAPTURE_SPAWN_REGISTER_SAMPLE_LIMIT`,
while the patch's raw default limit remains 2048.

There is also a separate reserve of at most 256 `$B0E5` register samples. It
preserves the exact regular-spawn entry even when dense `$C3A0`/`$CAxx` windows
have filled the normal capture window. The reserve is diagnostic only and must
never alone activate spawn, RNG or AI semantics.

The latest clean US boot run used Track 02 MD5
`f23601102138f87c33025877767ebf76` and System Card MD5
`ff1a674273fe3540ccef576376407d1d`. It verified 161 raw-sector spans, two
authenticated CD→RAM origin receipts, 32 game-main-RAM dispatches and 89
register samples, 18 at `$4644` and 64 at `$4667`. It still lacked `$B0E5` and
a dynamic RNG return; it therefore does not open spawn, AI, T700 or T900
semantics.

### Raw-code sidecar at `$5D64/$5D6A`

When an authenticated run reaches the RNG entry, the capture script also writes
`${THERON_LIVE_TRACE_OUTPUT}.rng-code`. Mednafen marks the file with
`source=mednafen-pce-instrumented-rng-code-v1` and writes 256 bytes from the
actual HuC6280 address together with logical and physical PC. The sidecar must
not be interpreted as a finished RNG implementation; it is raw disassembly
evidence.

The verified `.mc0` run from 2026-08-09 produced 50 `$B0E5` entries, a
512-step RNG window and a code window at `$5D64` (`physical_pc=$000D1D64`).
Track 02, System Card and Mednafen had MD5 values
`f23601102138f87c33025877767ebf76`, `ff1a674273fe3540ccef576376407d1d`
and `3ee7c8df8aad7b87ef0ecc05aaa29d8d`, respectively. The run lacked CD→RAM
receipts, so it does not open RNG, spawn, AI, T700 or T900 semantics and must
not be merged with another session.

The parser can also perform the source-byte join locally without putting game
data in the repository:

```sh
THERON_REAL_RNG_CODE_TRACE=/Volumes/Extern-disk/theron-auth-capture-mc0-rngcode-long-20260809.trace.rng-code \
THERON_REAL_US_TRACK02=/Users/bosse/.firestaff/data/theron/TQUS02.bin \
ctest --test-dir /Volumes/Extern-disk/firestaff-theron-active2-build \
  -R theron_v1_mednafen_spawn_consumer_trace --output-on-failure
```

An admitted receipt requires the real 8,104,992-byte US file and an exact
match at one of `0x975c4 + n*0x49800`, `n = 0..6`. A synthetic trace or
synthetic RNG byte is not sufficient.
