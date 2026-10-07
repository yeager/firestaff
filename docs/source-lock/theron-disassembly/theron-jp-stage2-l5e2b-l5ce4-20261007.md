# Theron JP Stage-2 `$5e2b` and `$5ce4..$5e15` callee windows

This source lock extends the authenticated JP `$4943..$49f9` caller window to
its direct callees at `$5e2b` and `$5ce4`, plus the exact `$5ce7` BVC target
at `$5d1c` and that window's three direct call targets `$5d93`, `$5ddb`, and
`$5df5`. It records bounded HuC6280 decode
windows only; no runtime entry, selector range, hardware effect, or gameplay
behavior is inferred.

## Authentic source and extraction

The inputs were the original Track 02 BINs at
`~/.firestaff/data/theron/TQJP02.bin` and `TQUS02.bin`, checked against full
image SHA-256 values JP `d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb`
and US `f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565`.
The Stage-2 first raw sectors are 1223 (JP) and 1224 (US), derived from
INDEX 01 sectors 224/225 plus the authenticated Stage-2 record offset
`0x3e7`. For a user-data offset `u`, raw offset is
`(first_stage2_sector + floor(u / 2048)) * 2352 + 16 + (u mod 2048)`.

| User window | Bytes | JP raw offset | US raw offset | SHA-256, both editions | FNV-1a-64, both editions |
| --- | ---: | ---: | ---: | --- | --- |
| `$5e2b..$5e80` | 86 | 2,903,963 | 2,906,315 | `02f993418bd7a54dc2679854de4f98bc61f43b4fbc676978305c37bd9f3e4a9e` | `eda767645ebeb320` |
| `$5ce4..$5d1b` | 56 | 2,903,636 | 2,905,988 | `f9732c087fd6f61fcc5449282154b83b4f7ac75595375f1a0c5df1a6a886a8cf` | `e411d8214ac332a3` |
| `$5d1c..$5d92` | 119 | 2,903,692 | 2,906,044 | `08a58b9b934100232bfd381845cead9b11bd88904c76bf3128f9f8467bdfaee0` | `6dd386d0d03bb574` |
| `$5d93..$5dda` | 72 | 2,903,811 | 2,906,163 | `e6bd85ceb98a37737b5d8e0002aeaff2f5029978ff998ded34678e1240413339` | `d3134abbb0e0dfe8` |
| `$5ddb..$5df4` | 26 | 2,903,883 | 2,906,235 | `c1becb66780c655428f31f2b1bbd8bf92f62ced603949614cd35b17597b05453` | `7b7c74e766d09c4d` |
| `$5df5..$5e15` | 33 | 2,903,909 | 2,906,261 | `aa37228d4a9401afabc88c37ee57a7b18dc29a7c0cba3b35a9f44d635ef098dd` | `f5af46bc925fc89f` |

The two JP windows are byte-identical to the same user-offset windows in US.
MAME `unidasm -arch h6280 -basepc 0x5e2b -skip <raw-offset> -count 0x56`
and the corresponding `$5ce4`/`0x38` command produced identical JP/US
listings in three loops. The `$5d1c` listing used
`unidasm -arch h6280 -basepc 0x5d1c -skip <raw-offset> -count 0x77`; its
JP/US bytes and listing matched in three loops. Listing hashes are recorded
in the `.asm` files. The `$5d93`, `$5ddb`, and `$5df5` windows were also
independently extracted from both original images; each had matching bytes and
MAME listings across editions in three loops.

## Decode boundary

At `$5e2b`, the decoded path reads `$3b78`, returns when it is zero, otherwise
loads `$3b79`, doubles it, and performs `JMP ($5e81,X)`. The first three words
at `$5e81` are `$5e39`, `$5e61`, and `$5e80`; the last target is an `RTS` at the
end of the bounded code window. This locks those table bytes and destinations,
but does not prove the runtime selector range or which entry is executed.
Both code and table are the same in authentic JP and US images.

The `$5ce4` listing has a `BVC $5d1c` branch whose destination is exactly the
end of the 56-byte window. The fall-through contains `TII $58e0,$58e1,$03ff`,
an indexed loop, and stores to the listed addresses. The note preserves those
decoded operands without assigning them broader subsystem semantics. At
`$5d1c`, the bounded continuation ends with `RTS` at `$5d92` (exclusive end
`$5d93`). Direct branch edges remain within the span; its JSR at `$5d62`
targets `$5d93`, exactly the exclusive boundary, so this lock does not extend
that callee. `$5d93` loops back to `$5d96` and returns at `$5dda`. `$5ddb`'s
branches target `$5dee` and `$5ddd` within that bounded routine, which returns
at `$5df4`. `$5df5` stores the `$5c/$5d` pointer and emits
`TIA $0000,$0404,$0020` before returning at `$5e15`. These opcode-level
observations do not establish wider routine semantics.

The registered `test_theron_v1_stage2_disassembly_chain` now locks each JP and
US span hash, compares every byte across editions, checks the branch and
indirect-jump encodings, and binds the three table words. The Python media
test independently verifies both full-image hashes before reading spans.
This is static
authentic-source evidence, not execution evidence or Theron gameplay parity.
