# ReDMCSB Callable Symbol Full Audit

## Scope

This ledger inventories numbered callable symbols in the local ReDMCSB
`ReDMCSB_WIP20210206/Toolchains/Common/Source` archive and records candidate
Firestaff mappings against `src/` and `include/`. ReDMCSB is the primary
functional reference for DM1 and CSB. The TSV is a symbol-discovery and
triage ledger; a symbol match is not proof of behavior, game-route
integration, or platform parity.

The inventory includes four-digit named callable symbols, documented unnamed
F runtime/startup entries, and ReDMCSB Atari ST/PRIM three-digit ABI aliases.
It excludes raw numeric constants and `S0092`, which the source identifies as
compiler/linker BSS initialization rather than a callable symbol.

## Inventory

| Kind | Count |
|---|---:|
| F | 2,100 |
| E | 6 |
| R | 8 |
| S | 19 |
| **Total** | **2,133** |

## Mapping rules

- `UNCERTAIN_NUMBERED_EVIDENCE`: the same numeric token appears in Firestaff
  source/header; behavioral equivalence is unverified.
- `MISSING`: no exact numbered token was found in Firestaff source/header; a
  differently named implementation may still exist.
- `IMPLEMENTED_NARROW`: a source-locked scoped adapter has a focused test; this
  does not establish complete host-platform equivalence.
- `VERIFIED_SOURCE_MAPPING`: the evidence maps a source behavior but does not
  by itself prove all caller or platform behavior.
- `PARTIAL`: source behavior has a bounded Firestaff counterpart, with material
  behavior or supported-platform coverage still open.
- `SOURCE_NONAPPLICABLE`: source review found no applicable PC 3.4 route.
- `PC34_SOURCE_IMPLEMENTED`: the source implements the behavior on PC 3.4;
  confirm the Firestaff runtime route separately.

## Current status

| Status | Count |
|---|---:|
| `MISSING` | 794 |
| `UNCERTAIN_NUMBERED_EVIDENCE` | 567 |
| `IMPLEMENTED_NARROW` | 534 |
| `VERIFIED_SOURCE_MAPPING` | 95 |
| `SOURCE_NONAPPLICABLE` | 65 |
| `PC34_SOURCE_IMPLEMENTED` | 5 |
| `PARTIAL` | 73 |
| **Total** | **2,133** |

The open rows include platform service routines, ABI aliases, unreferenced
symbols, and gameplay/UI functions. They must be triaged by reachability and
supported game/platform before interpreting the totals as missing game
features. These status counts describe the current audit rows, not complete
functional parity: every applicable ReDMCSB function still requires a Firestaff
counterpart and behavior review for each supported DM1/CSB platform and edition.
The current platform coverage register is in
`docs/parity/REDMCSB_FIRESTAFF_COMPLETE_AUDIT.md`.

## Machine check

```sh
awk -F '\t' 'NR > 1 { count[$2]++; rows++ }
  END { print rows, count["F"], count["E"], count["R"], count["S"] }' \
  docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv
```
