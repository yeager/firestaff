# Pass 126 — M12 V2 scaffolding/assets probe rerun on N2

Scope: verification-only rerun after N2 smoke probe runner fixes. No source changes and no push.

Host/context:

- Worker: N2 (`Firestaff-Worker-VM`), repo `/home/trv2/work/firestaff`.
- Branch: `sync/n2-dm1-v1-20260428`.
- Starting HEAD: `056bde4 Fix N2 smoke probe runners` (ahead of `origin/main` by local-only fixes).
- Rerun directory: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-0625-v2-assets-verify/`.

Commands and results:

| Command | Shell rc | Probe result |
| --- | ---: | --- |
| `./run_firestaff_m12_startup_menu_probe.sh` | 0 | **BLOCKED/FAIL**: probe binary segfaulted; log ends with `Segmentation fault (core dumped)` and no PASS summary. |
| `./run_firestaff_m12_modern_menu_probe.sh` | 0 | **PASS**: `# summary: 14/14 invariants passed`. |
| `./run_firestaff_m12_menu_mouse_probe.sh` | 0 | **FAIL**: `# summary: 5/10 invariants passed`; failed `INV_MOUSE_01`, `INV_MOUSE_02`, `INV_MOUSE_03`, `INV_MOUSE_05`, `INV_MOUSE_08`. |
| `./run_firestaff_m12_settings_smoke.sh` | 0 | **PASS**: `# summary: 6/6 invariants passed`. |

Generated/updated runtime evidence:

- `verification-m12/startup-menu/startup_menu_probe.log`
- `verification-m12/modern-menu/modern_menu_probe.log`
- `verification-m12/modern-menu/01_main.ppm`
- `verification-m12/modern-menu/02_settings.ppm`
- `verification-m12/modern-menu/03_game_options_v2.ppm`
- `verification-m12/modern-menu/04_message.ppm`
- `verification-m12/modern-menu/05_mode_v3.ppm`
- `verification-m12/modern-menu/06_checksum_ok.ppm`
- `verification-m12/menu-mouse/menu_mouse_probe.log`
- `verification-m12/menu-mouse/main_with_hover.ppm`
- `verification-m12/menu-mouse/settings_sv.ppm`
- `verification-m12/menu-mouse/gameopts_launch.ppm`
- `verification-m12/settings-smoke/settings_smoke.log`

Conclusion:

The V2/M12 scaffolding/assets lane is **not landable as green** from this N2 rerun despite all wrapper scripts returning rc=0. Modern menu rendering and settings smoke are green, but startup-menu segfault and menu-mouse invariant failures remain blockers. The wrapper rc behavior also needs follow-up because failing/segfaulting probes did not propagate a non-zero shell status.
