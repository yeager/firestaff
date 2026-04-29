# Pass 105 — N2 original entrance route unblock probe

Date: 2026-04-28
Lane: original DOSBox route/capture unblock
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
Run-dir: `<N2_RUNS>/20260428-0930-original-route-unblock/`

## Scope

This pass stays limited to the Linux/N2 original DOSBox route/capture path. It does not change HUD/inventory implementation and does not promote original artifacts into the default pass70 location.

## Fix: xdotool click coordinates

The prior pass94 click diagnostic logged clicks such as:

```text
click-mapped 260,50 -> 841,413 window=640x400
```

That exposed a Linux-only injector bug: `xdotool mousemove --window` expects window-relative coordinates, but the helper added the absolute X/Y window origin first. Under Xvfb the DOSBox window origin was non-zero (`origin=320,312`), so click probes were shifted outside the 640x400 client.

`scripts/dosbox_dm1_original_viewport_reference_capture.sh` now maps original 320x200 coordinates to DOSBox client-relative X/Y for the generated `original_viewport_route_keys_xdotool.sh` helper. The same probe now logs:

```text
click-mapped 260,50 -> window-relative 521,101 window=640x400 origin=320,312
```

## Classifier hardening

`tools/pass80_original_frame_classifier.py` now distinguishes a low-color playable corridor from a flat close-wall frame by requiring low luma variance (`viewport.luma_stddev < 45.0`) for the `wall_closeup` guard. N2 evidence showed playable corridor frames with low color and six unique gray levels, but high luma variance (`viewport.luma_stddev=57.48`, `inventory_extent.luma_stddev=68.96`). Those should classify as `dungeon_gameplay`; the actual flat wall remains `wall_closeup` (`viewport.luma_stddev=27.99`).

## Commands and results

| Command | rc | Result |
| --- | ---: | --- |
| `bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh` | 0 | shell syntax OK |
| `python3 -m py_compile tools/pass80_original_frame_classifier.py` | 0 | classifier syntax OK |
| `python3 tools/pass80_original_frame_classifier.py --self-test` | 0 | `pass=true`, 5 cases |
| patched pass94 route under `DOSBOX=/usr/bin/dosbox xvfb-run -a ... --run` into `$RUN_DIR/pass94-xvfb-click-relative` | 0 | six 320x200 raw PNGs captured and normalized; click log now window-relative |
| `python3 tools/pass80_original_frame_classifier.py $RUN_DIR/pass94-xvfb-click-relative --expected pass94-diagnostic --fail-on-duplicates` | 1 | still negative: entrance action sequence partially found, but route includes menu/wall/duplicate frames |
| entrance + spin probe into `$RUN_DIR/probe-kp6-spin` | 0 | six raw frames captured |
| `python3 tools/pass80_original_frame_classifier.py $RUN_DIR/probe-kp6-spin --fail-on-duplicates` | 1 | `dungeon_gameplay` frames are now classified, but duplicate/unsafe wall frames remain |
| entrance + navigation mix into `$RUN_DIR/probe-route-navigation-mix` | 0 | six raw frames captured |
| `python3 tools/pass80_original_frame_classifier.py $RUN_DIR/probe-route-navigation-mix --fail-on-duplicates` | 1 | 3 `dungeon_gameplay`, 3 `wall_closeup`, duplicate wall frame remains |
| entrance + spell/inventory key probe into `$RUN_DIR/probe-spell-inventory-after-entry` | 0 | six raw frames captured |
| `python3 tools/pass80_original_frame_classifier.py $RUN_DIR/probe-spell-inventory-after-entry --fail-on-duplicates` | 1 | 3 `dungeon_gameplay`, 3 `graphics_320x200_unclassified`; visual audit shows frames 4-6 are `GAME FROZEN`, not spell/inventory |

## Narrowed entrance/menu sequence

The route that transitions from the original PC 3.4 entrance/menu into visible dungeon gameplay on N2 is not a single `enter` or `click:260,50`. The tightest currently proven sequence is:

```text
wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800
```

Evidence:

- `$RUN_DIR/probe-route-unique-gameplay` first shot after that sequence classified `dungeon_gameplay` with sha256 `355a191cd07b...`.
- `$RUN_DIR/probe-route-navigation-mix` first shot after that sequence classified `dungeon_gameplay` with sha256 `71043f12e2ac...`.
- `$RUN_DIR/probe-spell-inventory-after-entry` first shot after that sequence classified `dungeon_gameplay` with sha256 `1ee706538fb3...`.

Interpretation: `enter`/`click:250,53` opens/selects the entrance option, `click:247,135` advances the entrance/menu state, and the first `kp6` input drops the remaining menu overlay and yields a playable dungeon view. Subsequent `kp6` turns change orientation, but the route quickly alternates between corridor gameplay and a flat wall, producing duplicate wall hashes if used naively.

## Remaining blocker

This pass narrowed the exact original entrance/menu action candidate and fixed a real Linux click-injection bug, but it did **not** produce a six-shot pass70/pass84-ready route. Remaining failures are semantic, not mechanical:

- duplicate raw frame hashes still appear in the explored six-shot routes;
- several valid gameplay states are unsafe `wall_closeup` frames;
- `one/four/four/enter/esc/i/f1` did not open spell/inventory; it produced a repeated `GAME FROZEN` state after `esc`.

Do not promote these artifacts as parity references. Next exact candidate should start from the proven entrance sequence above, then avoid the wall-facing loop and use source/manual evidence for actual original PC 3.4 spell/inventory controls before attempting a pass84-ready six-shot route.
