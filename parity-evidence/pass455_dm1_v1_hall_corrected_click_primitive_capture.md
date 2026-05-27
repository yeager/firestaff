# pass455_dm1_v1_hall_corrected_click_primitive_capture

- status: `PASS_PASS455_CORRECTED_CLICK_PRIMITIVE_AND_CANDIDATE_TRANSITION_PROVEN`
- artifact root: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509`
- external manifest: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509/pass455_dm1_v1_hall_corrected_click_primitive_capture.json`
- parity claim: corrected click primitive and candidate transition are proven; full pixel parity is still handled by pass449/pass450 comparator gates.

## Evidence summary

- corrected-coordinate runs: `probe-initial-south-corrected, probe-turn-click-primitive-stable, probe-turn-click-grid, probe-turn-click-grid-pm, probe-turn-click-grid-globalmouse, probe-turn-click-grid-cliclick`
- click primitive proven: `True`
- candidate transition promoted: `True`
- promotable labels from this capture: `candidate_select, resurrect_confirm_or_terminal_hud_after_c160`
- PC34 data provenance remained hash-locked; no filename-only comparison was used.

## Proven transitions

- `candidate_select` via `click:111,82`: `probe-initial-south-corrected/image0002-raw.png` sha256 `e4b373078be6aa0c27e793ccd476b6e886b34ef0c4b063c6d2274815351af53e`
- `resurrect_confirm_or_terminal_hud_after_c160` via `click:130,115`: `probe-initial-south-corrected/image0003-raw.png` sha256 `7523b67fa765ffb02a088bf8dbb0c2ba3630fcf5bcc2fb11f956b4e442b52b8f`

## Remaining scope

Pass455 only resolves the corrected-click/candidate-transition blocker. Full Hall framebuffer parity still belongs to pass449/pass450 and separate reincarnate/cancel captures.
