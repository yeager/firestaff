# Pass 113 — DM1 PC 3.4 original party-state route follow-up

Date: 2026-04-28
Host: N2 (`Firestaff-Worker-VM`, `N2`)
Branch: `sync/n2-dm1-v1-20260428`
Run dir: `<N2_RUNS>/20260428-1344-party-state-followup/`

## Scope

Focused follow-up to pass112/pass105: stop treating direct-start/no-party captures as eligible HUD/spell/inventory references. This pass checks the original PC 3.4 data/source references, runs a sequential N2 DOSBox route probe, and adds a narrow semantic party-state verifier.

## References checked

- Original PC 3.4 extracted set: `~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/`
- Original archive integrity: `cd ~/.openclaw/data/firestaff-original-games/DM && sha256sum -c SHA256SUMS` completed for the local N2 reference set.
- Greatstone atlas XML: `~/.openclaw/data/firestaff-greatstone-atlas/raw/greatstone.free.fr__dm__db_data__c_3d2d1d__dungeon.dat__dungeon_xml.zip.zip`
- ReDMCSB local index: `~/.openclaw/data/firestaff-redmcsb-source/README_FIRESTAFF.md` and related local index files were present on N2.

Source-backed Hall of Champions facts from Greatstone's extracted `0000.DUNGEON [Dungeon].xml`:

- Dungeon start: map 0, `start_x="3"`, `start_y="2"`, `start_facing="East"`.
- Map 0 overview row y=2 has open corridor from x=3 through x=9: `WWW.......WW...`.
- First champion portrait actuator found at wall `(10;2)`, position west, data `0` / champion identifier `Elija`.

Interpretation: a source-plausible route to the first champion is to start at `(3,2)` facing east and advance to `(9,2)` facing east, then interact with the west-facing champion portrait on wall `(10,2)`. This pass did **not** prove that route in the original runtime, because direct-start inputs did not produce usable movement/control state.

## Commands run

### Forward-click probe

```sh
cd ~/work/firestaff
RUN_BASE="$HOME/.openclaw/data/firestaff-n2-runs/20260428-1344-party-state-followup"
OUT_DIR="$RUN_BASE/probe-forward-to-elija" \
DM1_ORIGINAL_STAGE_DIR="$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34" \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800 shot:start click:276,140 wait:500 shot:fwd1 click:276,140 wait:500 shot:fwd2 click:276,140 wait:500 shot:fwd3 click:276,140 wait:500 shot:fwd4 click:276,140 wait:500 shot:fwd5" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
python3 tools/pass80_original_frame_classifier.py "$RUN_BASE/probe-forward-to-elija" --fail-on-duplicates
```

Result:

- Capture succeeded: six raw 320x200 screenshots normalized.
- Classifier failed: `entrance_menu` once, then five `dungeon_gameplay` frames.
- Duplicate blocker: frames 2-6 all hash to `48ed3743ab6a...`.
- The direct-start route did not advance through distinct movement states or reach spell/inventory/party controls.

### Keypad-forward probe

```sh
cd ~/work/firestaff
RUN_BASE="$HOME/.openclaw/data/firestaff-n2-runs/20260428-1344-party-state-followup"
OUT_DIR="$RUN_BASE/probe-kp8-to-elija" \
DM1_ORIGINAL_STAGE_DIR="$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34" \
DM1_ORIGINAL_PROGRAM="DM -vv -sn -pk" \
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \
WAIT_BEFORE_INPUT_MS=5000 \
NEW_FILE_TIMEOUT_MS=6000 \
DM1_ORIGINAL_ROUTE_EVENTS="wait:7000 enter wait:1200 click:250,53 wait:1200 click:247,135 wait:1200 kp6 wait:800 shot:start kp8 wait:500 shot:kp8_1 kp8 wait:500 shot:kp8_2 kp8 wait:500 shot:kp8_3 kp8 wait:500 shot:kp8_4 kp8 wait:500 shot:kp8_5" \
DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
python3 tools/pass80_original_frame_classifier.py "$RUN_BASE/probe-kp8-to-elija" --fail-on-duplicates
python3 tools/pass113_original_party_state_probe.py "$RUN_BASE/probe-kp8-to-elija" \
  --out-json parity-evidence/pass113_original_party_state_probe.json \
  --out-md parity-evidence/pass113_original_party_state_probe.md
```

Result:

- Capture succeeded: six raw 320x200 screenshots normalized.
- `pass80` classified all six frames as `dungeon_gameplay`, but still failed due duplicate raw frames.
- `pass113` marked `party_control_ready=false` and `direct_start_no_party_signature=true`.
- Rows 2-6 all hash to `48ed3743ab6a...`; right-column nonblack stays `0.0626`, spell-area nonblack stays `0.0000`.

## New verifier

Added `tools/pass113_original_party_state_probe.py`.

It joins the pass80 classifier JSON with the route-label manifest and blocks captures that only prove direct-start dungeon imagery rather than a usable party-control state. The direct-start/no-party signature requires:

- all classified frames are `dungeon_gameplay`,
- no `spell_panel` or `inventory` class appears,
- right-column/spell control regions stay blank on most frames,
- duplicate raw frame hashes remain.

Self-test command:

```sh
python3 tools/pass113_original_party_state_probe.py --self-test
```

Result:

```json
{
  "pass": true,
  "ready_party_control": true,
  "blank_no_party_signature": true
}
```

## Blocker

No party/champion state was captured in this pass.

The current source-plausible target is clear — start `(3,2)` east, first champion portrait at wall `(10,2)` west side — but the original PC 3.4 direct-start state reached by the pass105 sequence is still not a usable party/movement/control state for parity references. Both mouse-forward and keypad-forward attempts repeat the same dungeon frame instead of reaching the champion portrait or opening spell/inventory controls.

Do not promote these captures as HUD/spell/inventory original references. The next unblocker is to determine the original runtime's correct Hall-of-Champions movement/recruitment input path from the entrance state, or capture a manual/interactive N2 route that reaches `(9,2)` facing east and opens/recruits Elija before running the six-shot HUD/spell/inventory overlay fixture.
