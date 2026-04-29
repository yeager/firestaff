# Pass 166 pass 3 — source portrait route rerun

Date: 2026-04-29  
Host: N2 (`Firestaff-Worker-VM`)  
Tool: `tools/pass166_source_portrait_click_route_probe.py` under `xvfb-run -a`

## Command

```sh
xvfb-run -a python3 tools/pass166_source_portrait_click_route_probe.py
```

## Evidence

- Run base: `<N2_RUNS>/20260429-074347-pass166-source-portrait-click-route-probe`
- Evidence root: `parity-evidence/verification/pass166_source_portrait_click_route_probe/`
- Manifest: `parity-evidence/verification/pass166_source_portrait_click_route_probe/manifest.json`
- README: `parity-evidence/verification/pass166_source_portrait_click_route_probe/README.md`

## Result

Pass 3 completed both source-locked scenarios with no runtime errors:

- `enter_portrait11182_then_resurrect`: `blocked/static-no-party`; known static no-party hash `48ed3743ab6a` present.
- `enter_portrait11182_then_reincarnate`: `blocked/static-no-party`; known static no-party hash `48ed3743ab6a` present.

## Interpretation

This confirms the pass 2 source audit conclusion in runtime form. The ReDMCSB portrait coordinate (`x=111,y=82`) and candidate button centers (`C160` at `130,115`, `C161` at `186,115`) are valid source geometry, but the tested state is still not an actual front-wall champion portrait sensor (`C127`) state. The route returns to/static-collapses on no-party dungeon hash `48ed3743ab6a`, so pass 3 does not unblock original-faithful party/control capture.

Next useful pass should not repeat `111,82` immediately after entrance. It must first locate/route to a proven visible front-wall champion portrait tile, then click `111,82` only after the viewport contains that portrait.
