# Pass 97 — original overlay/capture unblock verification on N2

Date: 2026-04-28
Host: N2 (`Firestaff-Worker-VM`)
Branch: `sync/n2-dm1-v1-20260428`
Base commit before this note: `056bde43badd0489350914cacb5bd21e12024d69`
Run directory: `<N2_RUNS>/20260428-0825-overlay-capture-verify/`
Original PC 3.4 data: `<N2_ORIGINAL_GAMES>/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT`

## Wrapper gates requested by the worker lane

| Command | rc | Evidence |
| --- | ---: | --- |
| `./run_firestaff_m11_ingame_capture_smoke.sh` | 0 | `run_firestaff_m11_ingame_capture_smoke.log` says `In-game capture smoke PASS: 6 screenshots`. |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh` | 0 | Build wrapper completed: `built: .../firestaff_memory_graphics_dat_original_visible_dispatch_probe`. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh` | 0 | Build wrapper completed: `built: .../firestaff_memory_graphics_dat_original_present_probe`. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh` | 0 | Build wrapper completed: `built: .../firestaff_memory_graphics_dat_original_screen_probe`. |

## Original-data probe smoke checks

Positive controls against visible source viewport assets pass:

| Command | rc | Result |
| --- | ---: | --- |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index78.pgm" 78` | 0 | `ok`, `kind=BITMAP_SAFE`, `width=224`, `height=97`. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 78` | 0 | `ok`, `width=224`, `height=97`, `presentedBytes=10864`. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 78` | 0 | `ok`, `width=224`, `height=97`, `screenCopiedBytes=10864`. |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index79.pgm" 79` | 0 | `ok`, `kind=BITMAP_SAFE`, `width=224`, `height=39`. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 79` | 0 | `ok`, `width=224`, `height=39`, `presentedBytes=4368`. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 79` | 0 | `ok`, `width=224`, `height=39`, `screenCopiedBytes=4368`. |

## Remaining blocker found during verification

The wrappers pass when invoked exactly as lane gates, and the original-data path is healthy for bitmap-safe viewport assets 78/79. However, invoking the same probes against default graphic index `0` is still not landable as an actual original overlay/capture proof:

| Command | rc | Result |
| --- | ---: | --- |
| `./run_firestaff_memory_graphics_dat_original_visible_dispatch_probe.sh "$GRAPHICS_DAT" "$RUN_DIR/original_visible_dispatch_index0.pgm" 0` | 134 | aborts with `free(): invalid next size (normal)` after producing a PGM. |
| `./run_firestaff_memory_graphics_dat_original_present_probe.sh "$GRAPHICS_DAT" 0` | 134 | `failed: present surface copy mismatch for graphic 0`, then heap abort. |
| `./run_firestaff_memory_graphics_dat_original_screen_probe.sh "$GRAPHICS_DAT" 0` | 134 | `failed: screen surface mismatch for graphic 0`, then heap abort. |

Conclusion: the requested wrapper gates are green on N2, and the runner fixes are sufficient for build/smoke execution plus real bitmap-safe original-data probes. Full default-index/original overlay verification remains blocked by graphic-0 presentation/screen mismatch and heap corruption.
