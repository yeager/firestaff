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

## Regional Item-name Bank — 2026-08-20

The same complete-file gate now carries all 69 item-name records into the live
native world. The US name span is `0x0e9271..0x0e951e` with FNV-1a
`0x5be5602d`; the JP span is `0x0e92b1..0x0e955e` with FNV-1a `0x1020ac88`.
US bytes are retained as source ASCII. JP bytes are retained losslessly as
Shift-JIS and are not passed to a host text renderer.

Production parses those real spans directly. The readable US 69-name catalog
is excluded from the runtime archive and remains available only to historical
fixtures. The same applies to the 15 readable US level labels: production
parses the exact `0x203a3b..0x203ac2` span after verifying FNV-1a `7f7d9f67`.
The inventory probe mutates both source spans and requires rejection.

Immediately before each 69-name span is a six-byte separator preceded by a
69-byte source type-code table. The US table is at `0x0e9226` with FNV-1a
`21533bb5`; the JP table is at `0x0e9266` with FNV-1a `f9c3eabb`. Runtime now
authenticates and retains all 69 bytes in the regional name bank. A changed
type-code byte is rejected independently of name and property mutations.
Both tables are byte-identical to their edition's dungeon-4 Track 02 type-code
table. The world binder also compares the complete 396-byte property tables.
When both comparisons pass in the same authenticated regional session, it
opens the positional Track 02 dungeon-4→Track 19 index mapping. Other dungeons
remain closed. The object accessor checks source origin, mapped dungeon, index
bounds and the individual type code before returning raw name bytes.

Production chooses `TQUS19.iso` or `TQJP19.iso` only from the authenticated
Track 02 region and then requires the exact Track 19 MD5 above. The bank is
available by explicit Track 19 index and, for dungeon 4 only, by a source-owned
Track 02 object. `item_mapping_proven` is set only in a world containing both
matching banks. `host_text_rendering_proven` remains zero.

The six other dungeon banks were checked rather than assumed to share this
layout. Exact source-name matching finds many unique labels, but their mapped
type codes and property rows do not preserve the Track 19 positions. No other
dungeon therefore receives a positional mapping. The selected-inventory
receipt uses the proven Track 19 accessor for dungeon 4 and retains the
authenticated dungeon-local Track 02 name path elsewhere.
