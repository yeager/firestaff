# skproject DM2 Named Symbol Audit

## Scope

This line-addressable inventory covers callable definitions in the
project-owned `.cpp` files under skproject `SKWIN/` and `SKULLWIN/`, excluding
bundled third-party SDK trees. Its source authority is the upstream revision
locked by `docs/parity/DM2_V1_PHASE0_PROVENANCE_GATE.md`:

- Repository: <https://github.com/gbsphenx/skproject>
- Locked commit: `2c59a2c5a7978923ff5c822c15c3479e4d448493`

The companion `SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` is the machine-readable
ledger, with one row per named definition. Each row records a Firestaff
candidate, a disposition, and notes. An implementation disposition may cover
a narrow function slice and does not by itself prove integration into a
playable game route or parity on every supported platform.

## Inventory

| Source set | Definitions | Result |
|---|---:|---|
| Project-owned `.cpp` in `SKWIN/` and `SKULLWIN/` | 1,751 | Callable-definition rows in the ledger |

## Dispositions

| Status | Count |
|---|---:|
| `IMPLEMENTED_PARITY` | 1,113 |
| `NOT_APPLICABLE_ARCH` | 560 |
| `NONAPPLICABLE` | 73 |
| `UNOWNED_SOURCE_MUTATION` | 1 |
| `PARTIAL` | 2 |
| `STATIC_INPUT_ONLY` | 0 |
| `PARTIAL_SOURCE_OWNER` | 1 |
| **Total** | **1,751** |

Open source-owned behavior includes `DM2_ALLOC_NEW_RECORD` and
`DM2_DEALLOC_RECORD`, whose complete runtime allocation/reclamation path still
needs map-chain, recycler, record-pool, CAII, and timer ownership. The bounded
`DM2_DEALLOC_RECORD` primitive now marks a record's first word as NULL but does
not unlink it or connect reclamation to callers. `DM2_ARRANGE_DUNGEON`
has a bounded source-owned mutation path integrated into new-game loading, but
still lacks source-by-source mutation and all-edition/platform parity proof.
`DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` and
`DM2_RECYCLE_A_RECORD_FROM_THE_WORLD` remain partial. See each TSV row's notes
before treating a mapping as a parity result.

`NOT_APPLICABLE_ARCH` and `NONAPPLICABLE` are separate dispositions: neither
claims that a supported DM2 game feature is implemented. The inventory also
does not establish that every relevant callable was covered on DOS, Amiga,
FM Towns, and Macintosh. That requires platform-specific source-function
rows and runtime evidence.

## Machine check

```sh
awk -F '\t' 'NR > 1 { count[$6]++; rows++ }
  END { print rows, count["IMPLEMENTED_PARITY"], count["NOT_APPLICABLE_ARCH"],
              count["NONAPPLICABLE"], count["UNOWNED_SOURCE_MUTATION"] }' \
  docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv
```
