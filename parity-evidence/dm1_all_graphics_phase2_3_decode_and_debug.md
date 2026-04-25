# DM1 all-graphics phase 2/3 — debug containment + padded IMG3 decode

Date: 2026-04-25 10:28 Europe/Stockholm
Scope: Firestaff V1 / DM1 PC 3.4 in-game runtime graphics.

## Changes covered

### Normal V1 debug/message containment

Normal DM1/V1 rendering no longer draws Firestaff's rolling synthetic `messageLog` into the party panel. That log is mostly runtime/debug telemetry (movement, projectile travel, tick status, action scaffolding), not an original DM1 message surface.

Debug/control-strip rendering remains available behind `state->showDebugHUD && !m11_v1_chrome_mode_enabled()`.

Known banned normal-V1 strings include:

- `MISSILE FADES`
- ` FADES`
- ` OUT OF BOUNDS`
- ` COLLIDES IN FLIGHT`
- `CAST SPELL #`
- `G GRAB`
- `P DROP`
- `TICK`

The normal probe log in this phase contains none of those banned strings.

### Odd-width GRAPHICS.DAT / PASS action-area decode

The action/PASS area is `GRAPHICS.DAT` graphic `0010` (`87x45`). It exposed an odd-width packed-nibble row bug.

Fix direction implemented:

- packed row stride is derived from even-padded pixel width: `(((w + 1) & ~1) / 2)` bytes per row
- unpacking uses `iy * packedStride + ix / 2`, not a continuous `w*h` nibble stream
- decode buffer capacity is at least both packed and visible unpacked size
- `INV_GV_90B` asserts graphic 10 loads as `87x45` with visible palette data

### Padded IMG3 expander heap corruption

GuardMalloc showed a real overrun in `IMG3_Compat_ExpandOneCommandPadded` / `IMG3_Compat_ExpandFromSource` while decoding odd-width/padded assets. Root cause: the padded-command loop tracked visible pixels consumed by comparing remaining-in-line before/after a command. If one run crossed line boundaries, that calculation undercounted, causing the command loop to continue past the visible pixel total and write into allocator metadata.

Fix direction implemented:

- `IMG3_Compat_ExpandOneCommandPadded(...)` now returns the decoded run count.
- `IMG3_Compat_ExpandCommandsPadded(...)` accumulates that count directly.

This keeps decode termination based on actual visible pixels consumed, independent of row wrap.

### SDL texture resize instability

Launcher smoke intermittently crashed when switching presentation sizes (`480x270` launcher/menu to `320x200` game view), often inside SDL texture destruction.

Mitigation implemented:

- render texture minimum is now `480x270`, large enough for both launcher and game presentation paths
- texture reuse accepts larger-than-requested textures and renders via explicit source rectangles
- old textures are retired rather than destroyed mid-present if a future larger resize is needed

## Evidence artifacts

Postfix screenshot/probe set:

- `verification-m11/dm1-all-graphics/phase2-3-postfix-20260425-1028/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase2-3-postfix-20260425-1028/normal/*.png`
- `verification-m11/dm1-all-graphics/phase2-3-postfix-20260425-1028/debug/*.pgm`
- `verification-m11/dm1-all-graphics/phase2-3-postfix-20260425-1028/debug/*.png`

`GRAPHICS.DAT` SHA-256:

```text
2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e  /Users/bosse/.firestaff/data/GRAPHICS.DAT
```

## Verification

Commands run successfully after the fixes:

```sh
cmake --build build -j4
ctest --test-dir build --output-on-failure
DYLD_INSERT_LIBRARIES=/usr/lib/libgmalloc.dylib ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
for i in $(seq 1 25); do MallocScribble=1 MallocPreScribble=1 MallocGuardEdges=1 ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"; done
for i in $(seq 1 20); do ./run_firestaff_m11_launcher_smoke.sh verification-m11/dm1-all-graphics/launcher-smoke-loop-20260425-1025/run-$i; done
./run_firestaff_m12_startup_runtime_fuzz.sh /tmp/firestaff-phase012e-fuzz
MallocScribble=1 MallocPreScribble=1 MallocGuardEdges=1 ./firestaff --data-dir "$HOME/.firestaff/data" --script "key:enter,key:down,key:down,key:down,key:down,key:down,key:down,key:down,key:enter" --duration 15000
```

Results:

- CTest: `4/4 PASS`
- game-view probe: `362/362 invariants passed`
- GuardMalloc game-view probe: PASS
- 25x malloc-guarded game-view probe loop: PASS
- 20x launcher smoke loop: PASS
- M12 startup runtime fuzz: PASS
- malloc-guarded scripted DM1 launch: PASS
- no new `firestaff*.ips` crash report after 10:09 during these post-fix runs

## Caveat

This is a crash/decode/debug-containment phase, not a completed 1:1 graphics claim. Continue with source-bound visual parity work for viewport, right panel/action/spell area, champion HUD, inventory/items, creatures/effects, and palette/brightness.
