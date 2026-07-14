# RedMCSB Constant and Global Audit (DM1/CSB, Partition 2)

## Scope

This partition audits the named `C` and `G` symbol families in the real RedMCSB source corpus at `/Volumes/Extern-disk/firestaff-reference-cache/redmcsb/source/Toolchains/Common/Source`. It covers **866 unique constants** (`#define Cddd...` and `#define Cdddd...`) and **2,074 unique named globals** (`Gdddd_name`): **2,940 symbols** total. The un-suffixed `G3459` is intentionally excluded because it is not part of the named `Gxxxx_...` family specified by this partition.

The complete, one-row-per-symbol ledger is [REDMCSB_CONSTANT_GLOBAL_AUDIT.tsv](REDMCSB_CONSTANT_GLOBAL_AUDIT.tsv).

## Method

- **Source owner** is the RedMCSB defining location for a constant, or the best available non-`extern` declaration location for a global, falling back to its first declaration/reference when the corpus exposes only declarations.
- **Semantic domain** is derived from the source declaration's behavior-bearing label and surrounding ownership: save/load, champion/inventory, dungeon/world, viewport/rendering, creature/combat, spell/timeline, audio, input/UI, memory/runtime, or unclassified. This is deliberately not a spelling-only C/G identifier match.
- **Firestaff status** is deliberately conservative. `semantic candidate (unverified)` identifies a behaviorally relevant named Firestaff subsystem, but is not a parity claim until a symbol-specific contract and behavioral review exist. `unmapped / no direct counterpart` marks source state that has no direct Firestaff symbol. `out-of-scope: platform/toolchain` marks RedMCSB editor, utility, or hardware/platform state with no DM1/CSB runtime parity claim.
- **Evidence** records a real source location, source-reference count, and the Firestaff module used as a semantic candidate.

## Results

| Family | Symbols |
| --- | ---: |
| `C` constants | 866 |
| Named `G` globals | 2,074 |
| **Total** | **2,940** |

| Firestaff status | Symbols |
| --- | ---: |
| semantic candidate (unverified) | 1830 |
| out-of-scope: platform/toolchain | 523 |
| unmapped / no direct counterpart | 587 |

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

## Interpretation

The ledger does not treat a matching `C`/`G` ordinal, a source-name resemblance, or a broad subsystem match as evidence of parity. RedMCSB retains broad cross-platform and utility/editor state, while Firestaff models DM1/CSB behavior through named runtime modules. A later promotion from `semantic candidate (unverified)` requires a symbol-specific behavioral contract and focused evidence.
