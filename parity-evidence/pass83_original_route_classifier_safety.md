# Pass 83 — original route classifier safety guard evidence

This evidence note records the original-route visual-safety guard now used before accepting DM1 V1 screenshots as parity evidence.

## Locked guard

- `tools/pass80_original_frame_classifier.py` classifies flat close-wall frames as `wall_closeup` before inventory or generic dungeon gameplay.
- The unsafe shape is source-layout based: a dense, low-color DM1 viewport with a mostly dark right column whose wall pixels also fill the historical inventory panel extent (`x=80 y=53 w=144 h=73`).
- This prevents a close wall from being promoted as inventory/spell proof when raw DOSBox route captures miss their intended semantic state.

## Route-manifest support

- `scripts/dosbox_dm1_original_viewport_reference_capture.sh` accepts `f1`-`f4` route tokens so original champion inventory toggles can be represented explicitly in capture manifests.
- The generated Swift helper logs each posted route token, making focus/input delivery failures visible in `original-viewpoint-route-keys.log`.

## Verification

- `python3 -m py_compile tools/pass80_original_frame_classifier.py`
- `python3 tools/pass80_original_frame_classifier.py --self-test` → `pass: true`, `cases: 3`
- `bash -n scripts/dosbox_dm1_original_viewport_reference_capture.sh`
- `DM1_ORIGINAL_ROUTE_EVENTS=wait:5000 shot f1 wait:100 shot f2 wait:100 shot f3 wait:100 shot f4 wait:100 shot shot scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run` reports `route shape OK: 15 tokens, 6 shots`
- `git diff --check`
- Targeted diff secret scan

## Honesty note

This is evidence-hardening only. It prevents one false positive class from being accepted as inventory/spell parity; it does not claim new Firestaff/original pixel parity by itself.
