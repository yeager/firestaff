# Theron's Quest Track 19 Startup Envelope Receipt - 2026-08-06

## Scope

This note records one byte-authenticated Track 19 record window. It does not
claim a Track 19 dungeon parser, object-table format, tile/material mapping,
palette ownership, or later-level runtime handoff.

## Verified Corpus

| Variant | Source | Size | MD5 | Result |
|---------|--------|------|-----|--------|
| US Track 19 | `TQUS19.iso` | 5,984,256 | `51b40a17b92a30339957ba564aa0015c` | Envelope present |
| JP Track 19 | `TQJP19.iso` | 6,291,456 | `f9f069a5e489b91207f3156059b756f1` | Envelope present |
| US retail Track 02 concatenation | `TQUS19.iso + TQUS02End.iso` | canonical image | `ceb02343868f80cec899e9b239aff2da` | Same envelope bytes |

The Track 19 US and JP images both contain the exact 12-byte header at byte
offset `0x5a9114`:

```text
00 20 00 1b 01 08 e9 38 00 26 01 03
```

The following `0x360` bytes form the bounded 32×27 grid span, for a total
envelope size of `0x36c` (876) bytes. FNV-1a over the full envelope is
`0x54fce0a0` for both variants. The first `0x36c` bytes at the same offset in
the canonical retail Track 02 concatenation compare byte-for-byte equal.

## Runtime Boundary

The production Track 19 inventory records this window as
`startup_level_envelope_verified`, including offset, length, and FNV-1a. The
existing `startup_usable`, `level_usable`, and `bitmap_usable` flags remain
zero. A matching record does not identify the original consumer of the grid,
does not decode its tile/material or collision values, and does not bind any
object or palette records. Later-level publication remains blocked until the
source loader/disassembly supplies that consumer relation.

The Track 19 inventory probe mutates one envelope byte for each real variant
and requires validation to fail, preserving the fail-closed intake contract.
