# Theron's Quest Stage-Two Bytecode Dispatch Table

## Evidence and scope

The authenticated raw Track 02 BIN files contain the same 170-byte dispatch
table at HuC6280 address `$410d`:

| Edition | Track 02 MD5 | Stage-two user-data start | Table file offset | Table FNV-1a |
|---|---|---:|---:|---:|
| US | `f23601102138f87c33025877767ebf76` | `0x2bed90` | `0x2bee9d` | `0x7f6a7f04` |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `0x2be460` | `0x2be56d` | `0x7f6a7f04` |

The stage-two user-data starts are raw sectors 1224 (US) and 1223 (JP), plus
the 16-byte MODE1/2352 sector header. The loader enters the stage-two payload
at `$4000`; therefore CPU `$410d` is file offset `$10d` from those starts.
These offsets and the 17-sector load/entry are recorded in
`docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md`.

At `$40dc`, the interpreter reads one byte through `($1c)`, doubles it, and
uses it as the X offset of `JMP ($410d,x)`. `$410d..$41b6` is consequently an
85-entry little-endian pointer table, ending immediately before the helper at
`$41b7`. The listing in
`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm` now emits
this range as 85 `.addr` entries, labels each table target, and marks the
dispatch explicitly instead of treating the table as linear instructions.

## Index-to-target map

No gameplay or command names are assigned. Each right-hand value is only the
static HuC6280 jump target reached by the corresponding table index.

```text
00:41c5 01:41cb 02:41d8 03:41de 04:41e6 05:41ec 06:41f0 07:41f4
08:4214 09:4253 0a:4254 0b:4259 0c:4263 0d:4271 0e:4280 0f:4288
10:4291 11:42d3 12:4319 13:45f0 14:45f8 15:45fe 16:4615 17:461d
18:4623 19:4635 1a:4629 1b:462f 1c:4345 1d:4497 1e:4433 1f:445f
20:4647 21:464f 22:4234 23:42fb 24:4334 25:4916 26:4910 27:45ca
28:4375 29:43dd 2a:4409 2b:4653 2c:4674 2d:468f 2e:46ca 2f:4794
30:47a6 31:47c5 32:47d3 33:469d 34:44bd 35:46b8 36:4361 37:480a
38:47f3 39:4842 3a:485f 3b:447f 3c:4862 3d:489f 3e:48ac 3f:4901
40:491b 41:42be 42:4973 43:4995 44:45eb 45:49ab 46:49b4 47:49bb
48:4a5e 49:44eb 4a:4a81 4b:4aca 4c:49d3 4d:49e8 4e:4a3b 4f:4a14
50:4a1b 51:4a42 52:4a50 53:49fb 54:484e
```

## Target-rooted disassembly and regional comparison

`theron-stage2-da65.info` marks `$410d..$41b6` as an address table and gives
each of its 85 target addresses a one-byte `CODE` root. This makes da65
decode from every indirect-jump destination, even where a linear sweep had
rendered the bytes as data. Two corrected examples are index `$02` at `$41d8`
(`BSR $41f8`) and index `$41` at `$42be` (`LDA $1c; PHA; LDA $1d; PHA`).
All 85 roots now have a decoded HuC6280 instruction at their entry under da65
V2.18; this confirms opcode decoding only, not that every index occurs in a
valid bytecode stream. Subsequent bytes on some paths can still be ambiguous
or remain `.byte` data because the listing is a static linear output; the root
markers do not prove complete handler control flow.
The info file also marks the 64-byte source of the US `TIA` at `$5e57` as data
(`$5e5f..$5e9e`). The resulting US source-lock listing is
`theron-us-stage2-huc6280.asm`.

To reproduce the payload, deinterleave the 17 raw MODE1/2352 sectors: keep
bytes 16 through 2063 from each sector, for US raw sectors 1224–1240 or JP
raw sectors 1223–1239, then concatenate those 2048-byte user-data portions.
A contiguous 34,816-byte copy from the first user-data offset is wrong because
it includes the next sector's 16-byte header at each boundary. The first
sector offsets and table bytes above remain within that first user-data
portion.

The 170 table bytes are identical in both editions. In the deinterleaved
payload interval corresponding to `$4000..$4acf`, the editions differ at only
seven bytes, in five spans: `$42f6`, `$4314..$4315`, `$43be`, `$466f..$4670`,
and `$46c5`. All 85 target first bytes match. Their first 16 bytes also match
at 84 targets; the sole differing prefix is index `$35` at `$46b8`:

The other regional deltas also fall on three additional call sites and one
immediate operand in separately rooted listings. Index `$11` at `$42d3` calls `$5e27`
(US) / `$5e57` (JP) at `$42f5` (the changed low operand byte is `$42f6`);
index `$23` at `$42fb` and index `$2b` at `$4653` call `$56af` (US) / `$5729`
(JP) at `$4313` and `$466e`, respectively (changed operands `$4314..$4315`
and `$466f..$4670`). Index `$28` at `$4375` calls `$43b5`, which saves the
pointer pair `$37d6/$37d7`, loads `$5e9f` (US) or `$5ecf` (JP) into it, calls
`$37d8`, restores the pair, and calls `$3848`. The immediate at `$43bd` differs
(`#$9f` US / `#$cf` JP); each resulting pointer is one byte past that edition's
64-byte TIA source span above. This alignment is established statically; the
role of the pointer passed to `$37d8` remains unknown. The snippets establish
the differing operands and their surrounding static instructions, not what
any dispatch index means.

```text
US $46c4: JSR $5e4d   JP $46c4: JSR $5e7d
```

The surrounding control flow is byte-identical: `INY; LDA ($1c),Y; BNE $46c4;
LDA #$13; JSR $3ab7; BRA $46c7; JSR [regional target]; JMP $40f5`. The two
regional callees have the same instruction sequence, shifted by `$30`: set
`$0402` to `$e0`, set `$0403` to zero, transfer 64 bytes with `TIA` to `$0404`,
then return. Their source spans (`$5e5f..$5e9e` US and `$5e8f..$5ece` JP) are
byte-identical with FNV-1a `591d332b`. These observations establish static
control flow and data identity only; they do not assign a gameplay command
meaning or prove the index occurs in a valid stream.

## Verification boundary

`theron_v1_huc6280_disassembly_read_file()` now verifies the complete table
against both raw BIN identities, checks all 85 decoded little-endian targets
against the listing, and confirms each target lies in the `$4000..$7fff`
stage-two address span. It does not independently establish a runtime memory
mapping for that span. The receipt is static-source evidence only. The table
alone does not prove which indices occur in valid stream data, the meaning of
any handler, operand/stream advancement for every entry, or which real
level/resource uses the interpreter. It does not authorize a host bytecode
interpreter or gameplay semantics.
