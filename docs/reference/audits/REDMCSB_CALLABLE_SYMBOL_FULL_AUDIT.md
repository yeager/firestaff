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
- No row is marked implemented or verified by this audit. Source locations are first textual anchors in the supplied ReDMCSB reference corpus.

## Result

| Status | Count |
| --- | ---: |
| UNCERTAIN_NUMBERED_EVIDENCE | 1156 |
| MISSING | 957 |
| **Total** | **2137** |

Machine-check the ledger shape with:

```sh
awk -F '\t' 'NR > 1 { count[$2]++; rows++ } END { print rows, count["F"], count["E"], count["R"], count["S"] }' docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv
```
