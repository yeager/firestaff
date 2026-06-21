# Pass1053 DM1 V1 original champion candidate-panel capture

Status: `PASS1053_ORIGINAL_CHAMPION_CANDIDATE_PANEL_EVIDENCE_TRACKED`

This pass promotes the existing pass455 original-PC34 champion portrait click
artifact into tracked repository evidence.  It does not claim full four-champion
HUD parity; it narrows the champion-panel original-capture gap from "no original
panel screenshot exists" to "candidate/resurrect panel exists, while full party
HUD/status-panel pairing remains open".

## Source

- Source artifact: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509`
- Source run: `probe-initial-south-corrected`
- Source gate: `parity-evidence/pass455_dm1_v1_hall_corrected_click_primitive_capture.md`
- PC34 provenance:
  - `DUNGEON.DAT` SHA256 `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
  - `GRAPHICS.DAT` SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
  - `TITLE` SHA256 `adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745`

## Route

The source run logged the corrected client-relative click mapping separately from
absolute/root coordinates.  The promoted sequence is:

```text
wait:9000
enter
enter
wait:1800
shot:start
click:111,82
wait:2500
shot:after_portrait
wait:1500
shot:panel_visible
click:130,115
wait:2000
shot:confirm
wait:1000
shot:hud_status_after
wait:500
shot:extra
```

Source locks from pass455:

- `CLIKVIEW.C` C080 viewport click -> `MOVESENS.C` C127 champion portrait -> `REVIVE.C` F0280 candidate append.
- `COMMAND.C` C160 panel command -> `REVIVE.C` F0282 confirm/cleanup path.

## Tracked artifacts

Artifacts live under
`verification-screens/pass1053-dm1-original-champion-candidate-panel/`.

| Label | File | SHA256 | Bytes | Note |
|---|---|---:|---:|---|
| `start_before_portrait_click` | `start_before_portrait_click.png` | `50bead319e59bd42c9b5af6e4a39275e6cfc7a02fee96e6f6b766e575858fabc` | 17995 | Original state before portrait click |
| `candidate_select_after_click_111_82` | `candidate_select_after_click_111_82.png` | `e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e` | 25550 | Candidate panel after source portrait click |
| `resurrect_terminal_hud_after_click_130_115` | `resurrect_terminal_hud_after_click_130_115.png` | `7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f` | 13095 | Terminal/HUD state after C160 resurrect click |

## Measured panel deltas

The tracked manifest records region statistics for each frame.  The most useful
sanity check is the candidate-button crop:

- before portrait click: non-black ratio `0.344213`
- after portrait click: non-black ratio `0.882827`
- after C160 click: non-black ratio `0.876376`

This matches the pass455 conclusion that the corrected click primitive produces
a visible candidate transition, then a visible post-C160 transition.

## Non-claims

- This is original DM1 PC 3.4 candidate/resurrect-panel evidence, not a full
  four-champion party HUD capture.
- This does not include a Firestaff-vs-original pixel diff.
- This does not close the creature-chain or collision-transcript gaps.
