# Theron JP story candidate pointer search

This note records a bounded search around the seven authentic JP Track 02
story candidates. It does not establish which dungeon ordinal selects any
candidate, how the strings are framed at runtime, or whether the original game
consumes them through a pointer table.

## Raw-sector correspondence

The source image is the authentic `TQJP02.bin` with MD5
`b7afb338ad31be1025b53f9aff12d73a` and SHA-256
`d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`. Its
2352-byte sectors contain 2048-byte user-data payloads beginning 16 bytes into
each sector. For user-data offset `u`, the corresponding raw BIN byte offset
is `(u / 2048) * 2352 + 16 + (u % 2048)`. Rechecking each candidate against
the raw image gives:

| Candidate | User-data span | Raw BIN start |
| --- | --- | --- |
| Akutuba | `[0x27596D,0x275A97)` | `0x2D308D` |
| Drator | `[0x275A97,0x275BBF)` | `0x2D31B7` |
| Formic | `[0x275BBF,0x275D2F)` | `0x2D32DF` |
| Sarmon | `[0x275D2F,0x275E59)` | `0x2D344F` |
| Shado | `[0x275E59,0x275FCF)` | `0x2D3579` |
| Thief | `[0x275FCF,0x276187)` | `0x2D36EF` |
| Demon | `[0x276187,0x2762A3)` | `0x2D39D7` |

The local `TQJP19.iso` has MD5
`f9f069a5e489b91207f3156059b756f1`. Its full 6,291,456 bytes equal the
prefix of the projected `TQJP02.bin` user-data stream after dropping the
first 224 sectors (`0x70000` user-data bytes). Thus its copies of these story
spans occur at the listed user-data offsets minus `0x70000`; Track 19 is not
an independent source for a selector or pointer table.

The real-media regression
[`test_theron_v1_jp_dungeon_story_candidate_source_lock.py`](
../../tests/test_theron_v1_jp_dungeon_story_candidate_source_lock.py) checks
the authenticated image identity and candidate hashes in the projected
user-data stream, and now checks each candidate's framing bytes at its mapped
raw BIN endpoints as well. Every candidate begins `81 50` and ends `81 97`;
these remain observed bytes, not established record delimiters.

## Bounded pointer-table search

The projected JP Track 02 user-data stream was scanned at every two-byte-
aligned offset for a tightly packed table of eight 16-bit pointers: seven
starts plus the exclusive end sentinel. The expected values were modeled as
an unknown base plus each observed candidate's relative start, including
wraparound; both little- and big-endian encodings were checked. No sequence
matched. Additional bounded searches across the local JP/US Track 02 and
Track 19 images checked direct raw/user-data offsets, low 16-bit offsets,
ordered pointer arrays, relative-start arrays, length arrays, and 16-/24-/32-
bit direct-address encodings. No coherent seven-entry pointer sequence was
identified.

These negative searches rule out only the tested representations in the
examined images. They do not rule out split or differently aligned tables,
other transformed pointers, code-generated addresses, other media, or runtime
selection logic. No ordinal mapping or production JP story accessor is
justified by this search.
