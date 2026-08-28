# Local Mac workflow for Firestaff, Mednafen, and Tsugaru

This is the workflow for the external development disk. It is a local
reference, not a requirement to check in any emulator or game media.

## Firestaff and SDL3

SDL3 is available through Homebrew on this Mac. Build from the external worktree:

```sh
brew install sdl3
cmake -S /Volumes/Extern-disk/firestaff-theron-active2.mEOJKp \
  -B /Volumes/Extern-disk/firestaff-theron-active2-build \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build /Volumes/Extern-disk/firestaff-theron-active2-build --parallel
```

Use SDL's dummy video driver for headless verification:

```sh
SDL_VIDEODRIVER=dummy \
  /Volumes/Extern-disk/firestaff-theron-active2-build/firestaff_m11_phase_a_probe
```

Theron in Firestaff uses `WASD` for movement. Mouse button 1 is Button I and
mouse button 2 is Button II. On touch devices, a short tap is Button I and a
long press (500 ms) is Button II. Touch must use the same Firestaff input route
as the mouse; it must not create separate game mechanics.

## Mednafen on macOS

Mednafen is a terminal application. The local installation is normally at
`/opt/homebrew/bin/mednafen`. Start a verified PC Engine CD image by passing
the CUE file as its argument:

```sh
/opt/homebrew/bin/mednafen \
  "/Volumes/Extern-disk/theron-mednafen-us/Dungeon Master - Theron's Quest (USA).cue"
```

Place the System Card in the Mednafen profile outside the repository, for
example `~/.mednafen/firmware/syscard3.pce`. It must never be copied into the
Firestaff worktree, placed in the repository's `.firestaff/data`, or pushed to GitHub.

Mednafen has no standalone launcher GUI on macOS. During execution, open Player
1's input mapping with `Alt+Shift+1` and Player 2's with `Alt+Shift+2`. Follow
all prompts in order, including turbo and extra-layout fields. Firestaff's own
WASD/mouse/touch bindings belong to the SDL input path and must not be confused
with Mednafen mappings.

Use the real CUE/BIN source and System Card from their external locations for a
local Theron capture. Trace, screenshot, and debug output must likewise remain
on the external disk or in a non-repository scratch directory, never in the Git index.

## Tsugaru on macOS

Tsugaru is the FM TOWNS/Marty emulator. The official macOS route is the GUI
application `Tsugaru_GUI.app`; the GUI uses the CUI binary, which must remain
in the same distribution according to Tsugaru's documentation.

Local installation and execution:

```sh
open "/path/to/Tsugaru_GUI.app"
```

If building it locally from the official source:

```sh
git clone https://github.com/captainys/TOWNSEMU.git /Volumes/Extern-disk/TOWNSEMU
cd /Volumes/Extern-disk/TOWNSEMU/gui/src
git clone https://github.com/captainys/public.git
cd ..
cmake -S src -B build
cmake --build build --config Release
cp build/main_cui/Tsugaru_CUI.app/Contents/MacOS/Tsugaru_CUI \
   build/main_gui/Tsugaru_GUI.app/Contents/MacOS/
open build/main_gui/Tsugaru_GUI.app
```

Tsugaru can read ISO, CUE, and MDS, but its documentation recommends MDS/MDF
for CD images with audio and warns about ambiguous PREGAP interpretation in
CUE. For Firestaff source fidelity, document the format, track layout, and hash
before using data. Tsugaru ROMs and firmware must remain in its own local
profile, never in the Firestaff repository.

## Repository gate

Check before committing and pushing:

```sh
git status --short
bash scripts/verify_no_original_media_tracked.sh
```

Original BIN/CUE/ISO/BIOS/System Card files, dumped ROMs, and large emulator
data files must not be in the Git index. Only source code, metadata, hashes,
receipts, tests, and verified screenshots without media payloads may be
published.

Sources: [Tsugaru README](https://github.com/captainys/TOWNSEMU) and
[Mednafen documentation](https://mednafen.github.io/documentation/).
