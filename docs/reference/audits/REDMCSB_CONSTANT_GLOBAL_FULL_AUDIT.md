# ReDMCSB Constant/Global Full Audit

## Scope

This is the exhaustive, one-row-per-symbol audit of the named ReDMCSB `C` and `G` families in the primary source corpus:

`~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/`

It contains all `#define Cddd...`/`#define Cdddd...` constants and all named `Gdddd_...` globals found in that corpus. The un-suffixed `G3459` is intentionally excluded: it is outside the named `Gdddd_...` family.

| Family | Count |
| --- | ---: |
| `C` constants | 866 |
| Named `G` globals | 2,074 |
| **Total** | **2,940** |

The complete ledger is [REDMCSB_CONSTANT_GLOBAL_FULL_AUDIT.tsv](REDMCSB_CONSTANT_GLOBAL_FULL_AUDIT.tsv).

## Ledger Fields

- `source_file` and `source_line` identify the ReDMCSB definition owner for a constant, or the best non-`extern` declaration owner for a global; the historical source index falls back to the first available declaration/reference where necessary.
- `semantic_role` is a conservative per-symbol classification: it records whether the source declaration is a constant or global state and the behavior-bearing domain inferred from its identifier, declaration, and source owner.
- `firestaff_mapping` is explicit. A named Firestaff module is only a semantic candidate, not an assertion of symbol-for-symbol parity. `none` records either no direct counterpart or platform/toolchain-only source state.
- `firestaff_status` retains that distinction: `semantic candidate (unverified)`, `unmapped / no direct counterpart`, or `out-of-scope: platform/toolchain`.
- `evidence` preserves the source declaration/value or declaration evidence, source-reference count, and the candidate-module rationale.

## Results

| Firestaff status | Symbols |
| --- | ---: |
| semantic candidate (unverified) | 1,830 |
| unmapped / no direct counterpart | 587 |
| out-of-scope: platform/toolchain | 523 |

| Semantic domain | Symbols |
| --- | ---: |
| audio | 61 |
| champion-inventory | 269 |
| creature-combat | 66 |
| dungeon-world | 476 |
| input-ui | 289 |
| memory-runtime | 78 |
| save-load | 222 |
| spell-timeline | 57 |
| unclassified | 800 |
| viewport-rendering | 622 |

## Audit Boundary

ReDMCSB is the primary reference. Firestaff is evaluated by semantic ownership, never by numeric ordinal or name similarity alone. A candidate requires symbol-specific behavioral evidence before it can be promoted to a parity claim. This audit documents the current mapping evidence and explicitly keeps missing or uncertain symbols visible.
