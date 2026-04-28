# Pass 157 — 082b dungeon control probe

- program: `DM -vv -sn`
- seed route: `pre_dungeon_click_grid` through `wait_13` (pass155 hash `082b4d249740`)
- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260428-224314-pass157-082b-dungeon-control-probe`
- evidence root: `parity-evidence/verification/pass157_082b_dungeon_control_probe`
- probes: 15 (includes settle_baseline)
- completed: 15
- errors: 0
- buckets: ignored/no-op=15

## Action matrix

- `settle_baseline` `wait 0.95`: **ignored/no-op** `355a191cd07b`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/settle_baseline`
- `key_up` `key Up`: **ignored/no-op** `355a191cd07b`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_up`
- `key_down` `key Down`: **ignored/no-op** `48ed3743ab6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_down`
- `key_left` `key Left`: **ignored/no-op** `355a191cd07b`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_left`
- `key_right` `key Right`: **ignored/no-op** `4f0687504d6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_right`
- `key_w` `key w`: **ignored/no-op** `4f0687504d6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_w`
- `key_a` `key a`: **ignored/no-op** `4e5d1adfaef9`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_a`
- `key_s` `key s`: **ignored/no-op** `b5bace4ae767`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_s`
- `key_d` `key d`: **ignored/no-op** `5f1bf5b12f9f`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_d`
- `key_space_use` `key Space`: **ignored/no-op** `4f0687504d6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_space_use`
- `key_return_action` `key Return`: **ignored/no-op** `4f0687504d6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/key_return_action`
- `click_forward_panel` `click 272 124`: **ignored/no-op** `355a191cd07b`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/click_forward_panel`
- `click_left_panel` `click 236 148`: **ignored/no-op** `48ed3743ab6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/click_left_panel`
- `click_right_panel` `click 304 148`: **ignored/no-op** `48ed3743ab6a`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/click_right_panel`
- `click_back_panel` `click 272 164`: **ignored/no-op** `355a191cd07b`/dungeon_gameplay -> `48ed3743ab6a`/dungeon_gameplay changed_pixels=0 bbox=None — `parity-evidence/verification/pass157_082b_dungeon_control_probe/click_back_panel`

## Evidence contents

Each action directory contains raw route/control frames, `summary.json`, `pass157_rows.json`, `before_*.png`/`result_*.png` crops, annotated bboxes, and `diff.png`/`after_diff_bbox.png` when pixels changed.
