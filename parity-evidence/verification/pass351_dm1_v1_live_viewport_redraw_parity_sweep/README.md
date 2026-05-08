# Pass351 DM1 V1 live viewport redraw parity sweep

Scope: source-locked sweep after pass345/pass346/pass347 integration for the live route `input -> compat movement -> viewport redraw` path.

Result: verifier checks 11 anchors across ReDMCSB `Toolchains/Common/Source` and Firestaff live/pipeline files. It proves deterministic wiring and redraw request propagation; it deliberately does **not** claim original runtime pixel parity.

Run:

```sh
python3 tools/verify_pass351_dm1_v1_live_viewport_redraw_parity_sweep.py
```
