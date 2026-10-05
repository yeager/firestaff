# CSB Atari v2.1 textual disassembly binding

## Result

The local Atari ST v2.1 source archive contains `csb.s`, an 800,233-byte
textual disassembly. Its `uNNNN` labels map to the `CMAIN` payload in the
2009-02-22 PP hard-disk image at `28 + NNNN` bytes: the Atari PRG header is 28
bytes, followed by the matching code image.

The streamed `CMAIN` member is 145,446 bytes with SHA-256
`f1acad863b46394f4c5d1c0b1505393dfc4b2d82f9d04eba9862faf5c60e05d8`. Three
independent anchors match:

| Disassembly label | `CMAIN` file offset | Bytes | Assembly in `csb.s` |
| --- | ---: | --- | --- |
| `u0000` | `0x1c` | `4e f9 00 00 06 24` | `jmp u0624` |
| `u0006` | `0x22` | `4e f9 00 00 09 dc` | `jmp u09dc` |
| `u0624` | `0x640` | `2a 6f 00 04` | `movea.l 4(a7),a5` |

The member was read from the archive directly to a bounded in-memory buffer;
no game files were extracted or copied. The matching archive members can be
listed or streamed with:

```sh
7z l -slt "$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Version_2-1_Source,Disassembly,Software.7z"
7z x -so "$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Version_2-1_Source,Disassembly,Software.7z" csb.s
7z x -so "$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z" "HardDisk/2009-02-22 PP/CMAIN"
```

## Limits

This binds the text to the PP hard-disk `CMAIN` binary, not to the protected
retail v2.1 floppy executable. The same hard-disk image contains a separate
`GAME.PRG` (20,817 bytes); it is not the binary represented by these `uNNNN`
anchors. The archive includes a retail v2.1 STX game disk, but no file-level
comparison from that disk to `csb.s` has been established here.

The disassembly labels are numeric and do not identify ReDMCSB F-numbers.
In particular, this evidence does not map `F0267` to a `uNNNN` label. Keep the
source/function mapping open until a symbol, call-site, or runtime trace
provides a direct binding.

## Reproduction

```sh
python3 - <<'PY'
import hashlib, subprocess

source_archive = "$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Version_2-1_Source,Disassembly,Software.7z"
binary_archive = "$HOME/.firestaff/data/csb/Game,Chaos_Strikes_Back,Atari_ST,Software.7z"
def member(archive, name):
    return subprocess.check_output(["7z", "x", "-so", archive, name], stderr=subprocess.DEVNULL)

source = member(source_archive, "csb.s")
cmain = member(binary_archive, "HardDisk/2009-02-22 PP/CMAIN")
assert hashlib.sha256(cmain).hexdigest() == "f1acad863b46394f4c5d1c0b1505393dfc4b2d82f9d04eba9862faf5c60e05d8"
for label, address, expected in (
    ("u0000", 0x0000, bytes.fromhex("4ef900000624")),
    ("u0006", 0x0006, bytes.fromhex("4ef9000009dc")),
    ("u0624", 0x0624, bytes.fromhex("2a6f0004")),
):
    assert cmain[28 + address:28 + address + len(expected)] == expected, label
assert b"u0000 jmp u0624" in source
assert b"u0006 jmp u09dc" in source
print("CSB Atari CMAIN disassembly anchors: PASS")
PY
```
