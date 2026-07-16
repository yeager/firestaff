# ReDMCSB Callable Symbol Full Audit

## Scope

This is an exhaustive ledger of ReDMCSB callable numbered symbols from `/Volumes/Extern-disk/firestaff-reference-cache/redmcsb/source/Toolchains/Common/Source`, audited against Firestaff `src/` and `include/` only. The companion TSV contains one row per audited symbol and is the source of the totals below.

## Inventory

| Kind | Count |
| --- | ---: |
| F | 2104 |
| E | 6 |
| R | 8 |
| S | 19 |
| **Total** | **2137** |

The inventory includes four-digit named callable symbols, documented unnamed F runtime/startup entries, and the ReDMCSB Atari ST/PRIM three-digit ABI aliases. It excludes raw numeric constants and `S0092`, which the source identifies as compiler/linker BSS initialization rather than a callable symbol.

## Mapping Rules

- `UNCERTAIN_NUMBERED_EVIDENCE` means the same numeric token appears in a Firestaff source/header location. This is only a candidate trace; it does **not** claim a behaviorally equivalent or implemented ReDMCSB symbol.
- `MISSING` means no exact numbered token was found in Firestaff `src/` or `include/`. It makes no claim about an unnamed or differently numbered implementation.
- `IMPLEMENTED_NARROW` means a source-locked, scoped compatibility adapter has a focused regression test. It does not establish complete host-platform equivalence.
- `SOURCE_NONAPPLICABLE` means source review found no PC 3.4 route to implement. Source locations are first textual anchors in the supplied ReDMCSB reference corpus.

## Result

| Status | Count |
| --- | ---: |
| IMPLEMENTED_NARROW | 342 |
| MISSING | 824 |
| PC34_SOURCE_IMPLEMENTED | 5 |
| SOURCE_NONAPPLICABLE | 64 |
| UNCERTAIN_NUMBERED_EVIDENCE | 892 |
| VERIFIED_SOURCE_MAPPING | 10 |
| **Total** | **2137** |

Machine-check the ledger shape with:

```sh
awk -F '\t' 'NR > 1 { count[$2]++; rows++ } END { print rows, count["F"], count["E"], count["R"], count["S"] }' docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv
```
