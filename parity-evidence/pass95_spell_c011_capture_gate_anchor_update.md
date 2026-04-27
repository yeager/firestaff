# Pass 95 follow-up — spell C011 capture gate anchor update

Date: 2026-04-27

## Problem

The full `ctest` gate exposed that `m11_ingame_capture_smoke` was still probing an obsolete selected-rune rectangle:

- old probe: `x=239..252`, `y=91..103`
- observed result after the current V1 right-column spell-panel work: `brown=0`, `red=0`

A direct preserved capture inspection showed the native C011 brown/red rune cell is present, but in the top-right spell-panel strip:

- updated probe: `x=248..261`, `y=43..55`
- observed coverage in that box: brown `79`, red `40`

This is a smoke-gate anchor fix only. It does not claim exact original overlay parity.

## Gates

```sh
bash -n run_firestaff_m11_ingame_capture_smoke.sh
FIRESTAFF_DATA="$HOME/.firestaff/data" ./run_firestaff_m11_ingame_capture_smoke.sh
ctest --test-dir build --output-on-failure -j1
```

Result: all pass, including `m11_ingame_capture_smoke`.
