# pass453_dm1_v1_hall_state_machine_audit

Scope: DM1 V1 Hall of Champions only.

Status: audit/coordination document. This does not claim new pixel parity and does not copy original screenshots into the repo.

## Current state input

Controller state file reviewed: `/Users/bosse/.openclaw/workspace-main/memory/firestaff-dm1v1-state.json`.

Current state: `LAND_EVIDENCE`.

Last transition: `INGEST_N2_ORIGINAL_CAPTURE -> LAND_EVIDENCE`.

External artifact root: `/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509`.

N2 source artifact root: `/home/trv2/openclaw-artifacts/dm1-hall-dosbox-20260509/final_candidate_frames/`.

Artifact status: `NARROWED_ORIGINAL_HALL_PANEL_VISIBLE_CANDIDATE_CLICK_NO_TRANSITION`.

## State machine

| State | Entry evidence | Exit guard | Writes allowed |
| --- | --- | --- | --- |
| `SOURCE_LOCKED` | ReDMCSB anchors for portrait click, C127 candidate append, panel draw, cancel/confirm, sensor disable, HUD/modal blockers. | Source verifiers pass with PC34 DM1 hashes locked. | Source-lock reports only. |
| `FIRESTAFF_RUNTIME_LOCKED` | Unit/runtime probes show Firestaff candidate append, panel-active, cancel, resurrect, reincarnate, and sensor-disable semantics. | Runtime gates pass without replacing original evidence. | Runtime probe reports only. |
| `FRAMEBUFFER_CONTRACT_LOCKED` | pass449 contract lists required original scenes/regions and comparator schema. | Comparator still refuses parity while original true-stop frames are missing. | Comparator/schema manifests only. |
| `ORIGINAL_CAPTURE_READY` | Capture tooling is available on a host that can run PC34 original without storing captures in the repo. | Route is reproducible and original data hashes match. | External artifacts under `/Volumes/Extern-disk` or N2 artifact storage. |
| `INGEST_N2_ORIGINAL_CAPTURE` | External manifest, README, and checksums exist for the N2 Hall run. | At least one captured frame can be classified and every non-promotable frame remains labelled as blocked/review-only. | Repo report may reference hashes/paths, not embed screenshots. |
| `LAND_EVIDENCE` | Current state. Land concise evidence updates into pass449/pass450/pass451 docs/gates. | Repo documents the N2 finding: front mirror visible is promotable, candidate transition frames are not. | Small docs/gates with no broad implementation. |
| `NEXT_BLOCKER` | LAND_EVIDENCE complete. | A next lane owns exactly one unresolved blocker and produces a new external manifest or no-change source/runtime audit. | One blocker-specific report/gate. |

## Current artifact classification

The N2 DOSBox-X run is useful but not sufficient for candidate-panel parity.

Promotable:

- `03_panel_visible_north_front_mirror` / `run12_02_turn_left_north_front_mirror`: original Hall front-mirror visible frame/crop.
  - PC320 sha256: `766c73a66f4d253f0b9e6e1df7bef2e945191a5f635eff87d9d381ce7d031ec0`
  - Viewport224x136 sha256: `66a1f82c9a7a039918811efddee03dd07430e53f5dabb72d35adaabbd3d9189f`

Blocked / review-only:

- `04_candidate_select_click_111_82`: source-safe portrait click did not visibly transition to the candidate panel.
- `05_resurrect_confirm_130_115` and `run12_05_confirm_resurrect`: confirm click cannot be promoted because candidate mode was not visibly reached first.
- `run12_03_click_left_center` and `run12_04_click_right_center`: visual mirror-center clicks changed/repainted view evidence but did not produce a promotable candidate-panel transition.

Therefore pass449 must remain blocked for `candidate_select`, `cancel`, `resurrect_confirm`, `reincarnate_confirm`, and `hud_status_after` original true-stop scenes.

## Transition decision

`INGEST_N2_ORIGINAL_CAPTURE -> LAND_EVIDENCE` is valid only as a narrowed blocker transition, not a parity-completion transition.

LAND_EVIDENCE should land references to the external manifest/checksums and should explicitly preserve these claims:

1. Original PC34 data are hash-locked to DM PC 3.4 English / I34E.
2. The front Hall mirror visible frame is now available as original visual evidence.
3. Candidate-panel and confirmation frames are still missing as promotable original evidence because the captured clicks did not visibly enter candidate mode.
4. No original screenshot or bulky generated artifact belongs in the repository.

## Recommended next non-overlapping states

1. `LAND_N2_FRONT_MIRROR_EVIDENCE`
   - Owner: evidence/documentation lane.
   - Goal: update pass449/pass450 reports or gates to recognize the N2 external manifest and promote only the Hall front-mirror-visible frame/crop.
   - Done when: pass449/pass450 still block candidate scenes, but no longer say the latest Hall front-mirror original evidence is absent.

2. `DIAGNOSE_ORIGINAL_CLICK_ROUTE_TRUE_STOP`
   - Owner: N2 DOSBox-debug/source-runtime lane.
   - Goal: determine why PC x=111,y=82 and visual mirror-center clicks did not trigger candidate mode in stock PC34.
   - Boundaries: no framebuffer parity work; inspect route prerequisites only: scaling, input mode, left/right click command, facing/cell/sensor availability, empty-hand condition, and wait/true-stop timing.
   - Done when: a no-change audit or new external manifest identifies the failing precondition.

3. `CAPTURE_CANDIDATE_TRUE_STOP_SCENES`
   - Owner: capture lane after the click-route blocker is solved.
   - Goal: capture labelled original fullframes/crops for `candidate_select`, `cancel`, `resurrect_confirm`, `reincarnate_confirm`, and `hud_status_after`.
   - Done when: external manifest/checksums exist and pass449 comparator has promotable original inputs for every required scene.

These states are intentionally non-overlapping: one lands current evidence, one diagnoses route failure, one captures candidate scenes only after route diagnosis succeeds.
