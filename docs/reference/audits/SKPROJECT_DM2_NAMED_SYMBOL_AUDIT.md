# Skproject DM2 Named Symbol Audit

## Scope

This is a line-addressable inventory of callable named definitions from the
local skproject source authority at
`/Users/bosse/Documents/skproject-codex-ref`. It covers the readable project
owned `.cpp` sources under `SKWIN/` and `SKULLWIN/`, excluding bundled
third-party SDK trees. The companion TSV has one row per extracted definition
and is the authoritative machine-readable ledger.

The inventory is generated with the local `ctags -x` parser, then checked for
literal identifier occurrences in Firestaff `src/dm2/` and `include/dm2*.h`.
That is a discovery pass, not a behavioral equivalence pass.

## Source Coverage

| Source set | Files | Result |
| --- | ---: | --- |
| `SKWIN/` and `SKULLWIN/` project-owned `.cpp` | 81 | Parsed into callable-definition rows |
| `SKULLWIN/c_music_wav.cpp` | 1 | Locally unreadable within a 3-second read timeout; recorded as `UNCERTAIN` sentinel |
| `SKULLWIN/c_rect.cpp` | 1 | Locally unreadable within a 3-second read timeout; recorded as `UNCERTAIN` sentinel |

The TSV deliberately excludes parser artefacts shorter than three characters
and the C++ keyword `if`; neither is a named project callable. Header-only
declarations are not duplicated because the ledger is anchored to source
definitions and their executable behavior.

## Mapping Rules

- `IMPLEMENTED` requires a separately verified, source-locked Firestaff
  behavior and test. This discovery audit assigns no such status.
- `UNCERTAIN` means the exact identifier occurs in DM2 Firestaff source or
  headers. It is only a candidate mapping and does not claim equivalent
  behavior, data layout, timing, or side effects.
- `MISSING` means no exact identifier occurred in the scanned Firestaff DM2
  source/header surface. It does not rule out a differently named port.
- `NONAPPLICABLE` is limited to identified SKWIN desktop UI/debug/SDL/MIDI
  variant files outside the Firestaff DM2 core; it makes no runtime-port claim.

## Result

| Status | Count |
| --- | ---: |
| VERIFIED_SOURCE_MAPPING | 94 |
| UNCERTAIN | 131 |
| MISSING | 1457 |
| NONAPPLICABLE | 69 |
| **Total** | **1751** |

The two unreadable-file sentinels are included in `UNCERTAIN`. Every mapping
must be promoted only after a specific skproject call path, owned GDAT/save
data, and focused Firestaff test demonstrate the same behavior. In particular,
an `UNCERTAIN` title/menu token does not authorize a synthetic session or
dungeon fallback.

Machine-check the ledger:

```sh
awk -F '\t' 'NR > 1 { count[$6]++; rows++ }
  END { print rows, count["VERIFIED_SOURCE_MAPPING"], count["UNCERTAIN"],
              count["MISSING"], count["NONAPPLICABLE"] }' \
  docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv
```
