# pass_dm1_v1_full_party_hud_runtime_pairing_gate

Status: `FAIL_PROBE_BINARY_MISSING`

Detail: Probe binary was not built; check the CMake build.

## Probe

- Source: `probes/m11/firestaff_dm1_v1_full_party_hud_runtime_pairing_probe.c`
- Binary: `/home/yeager/Documents/Codex/2026-08-24/jobba-med-github-com-yeager-firestaff/work/firestaff-incomplete-20260824/build/firestaff_dm1_v1_full_party_hud_runtime_pairing_probe`
- Data dir: `/home/yeager/.firestaff/data/dm1`
- Return code: `None`
- Pass count: `0`
- Fail count: `0`

## Companion Pass

- pass1071 companion report: `parity-evidence/pass1071_dm1_v1_champion_panel_pairing_readiness.md` (exists: `True`)

## Panel-Region Fingerprints

These 64-bit FNV-1a hashes cover the 274x29 party panel region (C151..C154 status boxes side by side). They are forensic evidence only; they let us detect cross-run / cross-asset drift without claiming original DOS parity.

| Case | Panel-region FNV-1a64 |
|---|---:|
| `full4_panel_fnv1a64` | `0x<missing>` |
| `single1_panel_fnv1a64` | `0x<missing>` |
| `two2_panel_fnv1a64` | `0x<missing>` |
| `three3_panel_fnv1a64` | `0x<missing>` |
| `single1_to_full4_panel_fnv1a64` | `0x<missing>` |
| `full4_to_single1_panel_fnv1a64` | `0x<missing>` |
| `two2_to_full4_panel_fnv1a64` | `0x<missing>` |

## Honesty Boundary

- No Firestaff-vs-original DM1 PC 3.4 pixel diff is performed.
- No full four-champion HUD or single-champion status-panel
  original pairing exists; pass1071 still records that blocker.
- This verifier only records that the Firestaff M11 V1 draw   stack populates the full 4-champion HUD, populates the   single-champion status panel, populates the 2-/3-champion   intermediate cases, and actively clears / repopulates the   inactive slots during the single<->full and full<->two<->full
  transitions.

Manifest: `parity-evidence/verification/pass_dm1_v1_full_party_hud_runtime_pairing_gate/manifest.json`
