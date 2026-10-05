# Theron's Quest Stage-Two Bytecode Dispatch Table

## Evidence and scope

The authenticated raw Track 02 BIN files contain the same 170-byte dispatch
table at HuC6280 address `$410d`:

| Edition | Track 02 MD5 | Stage-two user-data start | Table file offset | Table FNV-1a |
|---|---|---:|---:|---:|
| US | `f23601102138f87c33025877767ebf76` | `0x2bed90` | `0x2bee9d` | `0x7f6a7f04` |
| JP Rev. 1 | `b7afb338ad31be1025b53f9aff12d73a` | `0x2be460` | `0x2be56d` | `0x7f6a7f04` |

The stage-two user-data starts are raw sectors 1224 (US) and 1223 (JP), plus
the 16-byte MODE1/2352 sector header. They load at `$4000`; therefore CPU
`$410d` is file offset `$10d` from those starts. These offsets and the
17-sector load/entry are recorded in
`docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md`.

At `$40dc`, the interpreter reads one byte through `($1c)`, doubles it, and
uses it as the X offset of `JMP ($410d,x)`. `$410d..$41b6` is consequently an
85-entry little-endian pointer table, ending immediately before the helper at
`$41b7`. The listing in
`docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm` now emits
this range as `.word` data instead of misleading linear instructions.

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

## Verification boundary

`theron_v1_huc6280_disassembly_read_file()` now verifies the complete table
against both raw BIN identities, checks all 85 decoded little-endian targets
against the listing, and confirms each target lies in the loaded `$4000..$7fff`
window. The receipt is static-source evidence only. The table alone does not
prove which indices occur in valid stream data, the meaning of any handler,
operand/stream advancement for every entry, or which real level/resource uses
the interpreter. It does not authorize a host bytecode interpreter or gameplay
semantics.
