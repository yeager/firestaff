# Pass 173 / pass 4 — gated source portrait route probe

- run base: `/home/trv2/.openclaw/data/firestaff-n2-runs/20260429-075246-pass173-source-portrait-route-gate-probe`
- evidence root: `parity-evidence/verification/pass173_source_portrait_route_gate_probe`
- completed: 2
- errors: 0
- buckets: blocked/static-no-party-after-gate=2

## Route precondition

- DM1 V1 initial party: map0 x=1 y=3 dir=South.
- Front wall square: map0 x=1 y=4 contains sensor 16 type C127 wall champion portrait.
- Therefore no movement is required; only entrance gate must be passed before clicking x=111,y=82.

## Results

- `gate_click_portrait_then_resurrect`: **blocked/static-no-party-after-gate** — known no-party hash present after gate: 48ed3743ab6a — `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_resurrect`
- `gate_click_portrait_then_reincarnate`: **blocked/static-no-party-after-gate** — known no-party hash present after gate: 48ed3743ab6a — `parity-evidence/verification/pass173_source_portrait_route_gate_probe/gate_click_portrait_then_reincarnate`
