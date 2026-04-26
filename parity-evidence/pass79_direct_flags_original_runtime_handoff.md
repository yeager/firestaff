# Pass 79 — original runtime handoff via DM command-line flags

Date: 2026-04-26

## Goal

Continue from pass 78's selector blocker and find a deterministic way to reach original DM1 PC 3.4 `320×200` graphics frames without relying on fragile host selector input.

## Source finding

ReDMCSB `Toolchains/Common/Source/DM.C` documents the original launcher flags:

- `-vv` selects VGA graphics;
- `-sn` selects no sound;
- `-pk` selects keyboard simulation input;
- `-r` prints the command line only;
- passing flags disables menu-choice prompting unless `-o` is used.

That means the selector can be bypassed honestly with:

```text
DM -vv -sn -pk
```

## Harness support

`DM1_ORIGINAL_PROGRAM` was added in pass 78 and now has a verified use:

```sh
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:10000 shot wait:500 shot wait:500 shot wait:500 shot wait:500 shot wait:500 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Result: six raw DOSBox screenshots, all accepted by the audit as `320×200` graphics frames.

Evidence:

- `parity-evidence/overlays/pass79/pass79_direct_flags_title_audit.json`
- `verification-screens/pass79-original-direct-flags-title/raw_manifest.tsv`
- `verification-screens/pass79-original-direct-flags-title/original_viewport_224x136_manifest.tsv`

## Gameplay entry probe

A second run pressed `Enter` from the title/menu before capturing:

```sh
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
DM1_ORIGINAL_ROUTE_EVENTS='wait:8000 enter wait:10000 shot wait:1000 shot right wait:1000 shot up wait:1000 shot one wait:1000 shot i wait:1000 shot' \
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
```

Result: six raw DOSBox screenshots, all accepted by the audit as `320×200` graphics frames. Visual inspection shows the run reaches the graphical dungeon/corridor view rather than selector text mode.

Evidence:

- `parity-evidence/overlays/pass79/pass79_direct_flags_enter_audit.json`
- `parity-evidence/overlays/pass79/pass79_direct_flags_enter_fullscreen_compare_stats.json`
- `verification-screens/pass79-original-direct-flags-enter/raw_manifest.tsv`
- `verification-screens/pass79-original-direct-flags-enter/original_viewport_224x136_manifest.tsv`

## Measurement-only overlay

`pass74_fullscreen_panel_pair_compare.py` was run against the direct-flags Enter capture set. The comparison is still measurement-only: the original route is not yet claimed semantically identical to Firestaff pass 77.

Viewport/full-frame deltas from the Enter capture set:

| pair | scene | viewport delta | full-frame delta |
| --- | --- | ---: | ---: |
| 01 | ingame_start | 74.3829% | 53.3687% |
| 02 | ingame_turn_right | 76.0012% | 54.1391% |
| 03 | ingame_move_forward | 78.6896% | 55.4188% |
| 04 | ingame_spell_panel | 78.6896% | 56.0828% |
| 05 | ingame_after_cast | 78.6896% | 55.4188% |
| 06 | ingame_inventory_panel | 93.4874% | 62.4625% |

## Status

Pass 79 resolves the pass 78 launcher/selector blocker: original runtime capture can now produce audited `320×200` graphics frames without selector automation.

Remaining blocker: exact original semantic route parity. We still need to prove the original input route transitions through the same states as Firestaff pass 77 (`start → turn right → move west → spell panel → actual Ful Ir cast → inventory`) before treating pixel deltas as parity evidence.
