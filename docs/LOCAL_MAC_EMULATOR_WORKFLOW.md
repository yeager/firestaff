# Local Mac workflow for Firestaff, Mednafen, and Tsugaru

This is the workflow for the external development disk. It is a local
reference, not a requirement to check in any emulator or game media.

## Firestaff and SDL3

SDL3 is available through Homebrew on this Mac. Run the following commands
from the root of the external worktree:

```sh
brew install sdl3
repo="$PWD"
build_dir="/Volumes/Extern-disk/firestaff-theron-debug-build"
cmake -S "$repo" -B "$build_dir" -DCMAKE_BUILD_TYPE=Debug
cmake --build "$build_dir" --parallel 1
```

Launch the native Theron Debug runtime with authentic JP data:

```sh
build_dir="/Volumes/Extern-disk/firestaff-theron-debug-build"
"$build_dir/firestaff" \
  --game theron --theron-native jp --debug \
  --data-dir "$HOME/.firestaff/data/theron"
```

Theron runs in Firestaff's native C/SDL runtime. During dungeon play, Up/W
moves forward, Down/S moves backward, Left/A and Right/D step sideways, and
Q/E turn; Home/End and keypad 4/6 are alternate turn keys. The visible native
4x3 mouse panel dispatches the same movement and action inputs. Left-click
activates a panel control, right-click goes back, and middle-click is inert.
Clicks outside the panel are ignored until source-backed world hit regions are
verified. Mouse motion highlights a panel control but does not select objects.
On touch devices, a short tap is Button I and a long press is Button II. These
are native Firestaff host bindings; they do not turn the emulator into the
runtime or claim original PC Engine mouse support.

## Mednafen on macOS

Mednafen is a terminal application. The local installation is normally at
`/opt/homebrew/bin/mednafen`. Start a verified PC Engine CD image by passing
the CUE file as its argument. Replace the filename with the authenticated
regional CUE listed in the real-data inventory:

```sh
/opt/homebrew/bin/mednafen \
  "$HOME/.firestaff/data/theron/<verified-region-cue>.cue"
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
