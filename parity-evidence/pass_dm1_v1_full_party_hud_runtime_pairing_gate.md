# pass_dm1_v1_full_party_hud_runtime_pairing_gate

Status: `PASS_RUNTIME_PAIRING_EVIDENCE`

Detail: Probe exercised full4 HUD, single1 status panel, two-/three-champion intermediate cases, and all transitions against real Firestaff V1 assets.

## Probe

- Source: `probes/m11/firestaff_dm1_v1_full_party_hud_runtime_pairing_probe.c`
- Binary: `/Volumes/Extern-disk/openclaw-work/firestaff/coding-worktrees-builds/20260626101715562127000_dm1_v1_full_party_hud_runtime_pairing_gate_build/firestaff_dm1_v1_full_party_hud_runtime_pairing_probe`
- Data dir: `/Users/bosse/.firestaff/data/dm1`
- Return code: `0`
- Pass count: `276`
- Fail count: `0`

## Companion Pass

- pass1071 companion report: `parity-evidence/pass1071_dm1_v1_champion_panel_pairing_readiness.md` (exists: `True`)

## Panel-Region Fingerprints

These 64-bit FNV-1a hashes cover the 274x29 party panel region (C151..C154 status boxes side by side). They are forensic evidence only; they let us detect cross-run / cross-asset drift without claiming original DOS parity.

| Case | Panel-region FNV-1a64 |
|---|---:|
| `full4_panel_fnv1a64` | `0x46204c70fa361433` |
| `single1_panel_fnv1a64` | `0xcae5465af0f96ba4` |
| `two2_panel_fnv1a64` | `0x275fb9537963913d` |
| `three3_panel_fnv1a64` | `0x3f0011f93f1a4be2` |
| `single1_to_full4_panel_fnv1a64` | `0x46204c70fa361433` |
| `full4_to_single1_panel_fnv1a64` | `0xcae5465af0f96ba4` |
| `two2_to_full4_panel_fnv1a64` | `0x46204c70fa361433` |

## Honesty Boundary

- No Firestaff-vs-original DM1 PC 3.4 pixel diff is performed.
- No full four-champion HUD or single-champion status-panel
  original pairing exists; pass1071 still records that blocker.
- This verifier only records that the Firestaff M11 V1 draw   stack populates the full 4-champion HUD, populates the   single-champion status panel, populates the 2-/3-champion   intermediate cases, and actively clears / repopulates the   inactive slots during the single<->full and full<->two<->full
  transitions.

Manifest: `parity-evidence/verification/pass_dm1_v1_full_party_hud_runtime_pairing_gate/manifest.json`
