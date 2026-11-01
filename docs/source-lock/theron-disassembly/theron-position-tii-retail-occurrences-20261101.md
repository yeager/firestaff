# Theron position-transfer TII source candidates (2026-11-01)

## Finding

An identity-gated scan of authentic US and Japanese Rev. 1 Track 02 BIN files
found 21 occurrences per edition of the seven-byte candidate encoding for the
observed two-byte transfer from `$20B4` to `$2040`. Each edition has seven
128-raw-sector-spaced groups, with three candidate sites at the same byte
positions inside each group's sector. This is a shortlist for a future
source-read/runtime-fetch join.

The authentic JP PCE Fast state-only trace reaches logical PC `$C1FC` and
records a two-byte transfer to `$2040/$2041`; its active map, caller and source
occurrence are unresolved. Finding matching bytes in Track 02 does not bind
the executing code to any listed offset. It also does not distinguish a party
position update from another copy operation or establish collision or
rendering behavior. See the [state-only instruction context](theron-pce-fast-tii-control-flow-20261031.md)
and the [active-map capture plan](../../../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).

## Authenticated media

| Edition | Size | MD5 |
|---|---:|---|
| US Track 02 | 8,104,992 | `f23601102138f87c33025877767ebf76` |
| JP Rev. 1 Track 02 | 8,102,640 | `b7afb338ad31be1025b53f9aff12d73a` |

## Candidate locations

Offsets are raw byte offsets in the regional Track 02 BIN. The sector-byte
positions are identical across groups; “group” is only an observed spacing
pattern and is not a dungeon or runtime-bank label.

| Group | US raw sector | US raw offsets | JP raw sector | JP raw offsets |
|---:|---:|---|---:|---|
| 0 | 284 | `$0A334C`, `$0A33FF`, `$0A34B0` | 283 | `$0A2A1C`, `$0A2ACF`, `$0A2B80` |
| 1 | 412 | `$0ECB4C`, `$0ECBFF`, `$0ECCB0` | 411 | `$0EC21C`, `$0EC2CF`, `$0EC380` |
| 2 | 540 | `$13634C`, `$1363FF`, `$1364B0` | 539 | `$135A1C`, `$135ACF`, `$135B80` |
| 3 | 668 | `$17FB4C`, `$17FBFF`, `$17FCB0` | 667 | `$17F21C`, `$17F2CF`, `$17F380` |
| 4 | 796 | `$1C934C`, `$1C93FF`, `$1C94B0` | 795 | `$1C8A1C`, `$1C8ACF`, `$1C8B80` |
| 5 | 924 | `$212B4C`, `$212BFF`, `$212CB0` | 923 | `$21221C`, `$2122CF`, `$212380` |
| 6 | 1052 | `$25C34C`, `$25C3FF`, `$25C4B0` | 1051 | `$25BA1C`, `$25BACF`, `$25BB80` |

Each candidate is within its raw sector's data area. The source scanner also
hashes a 39-byte context around each occurrence without printing or storing
game-code bytes. US contexts form three signatures with seven occurrences
each; JP contexts form four signatures with counts 7, 7, 6 and 1. This records
regional byte variation, not function boundaries or code ownership.

## Reproduction

Run the identity-gated scanner against the installed authentic BINs:

```sh
python3 tools/analyze_theron_position_tii_source.py \
  "$HOME/.firestaff/data/theron/TQUS02.bin" \
  "$HOME/.firestaff/data/theron/TQJP02.bin"
```

The scanner rejects any size, MD5, occurrence-count or offset-list mismatch.
CTest registers the same scanner as
`theron_v1_position_tii_source_candidates` and skips only when either
authentic BIN is unavailable. Three consecutive local runs produced identical
candidate lists and context hashes.

## Evidence boundary

This report establishes only source-byte candidate locations in the two
authenticated raw BINs. It does not prove that any candidate is instruction
code in its source context, is loaded, maps to `$C1FC`, or executes in the
original game. A confirming capture must join the fetched instruction bytes
and MPR mapping to the same-session CD read and RAM destination, then bind the
active map, party pose and tile consumer before native movement or rendering
semantics are changed.
