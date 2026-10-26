# Nexus startup PRS3-to-VDP1 trace boundary

This note records a reproducible, real-media observation made while locating
the producer of the Japanese retail Nexus title-menu data. It is deliberately
not a decoder admission, a menu-rendering authorization, or a substitute for
an original-Saturn provenance capture.

## Inputs

- Japanese retail CUE SHA-256:
  `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`
- Japanese Saturn IPL SHA-256:
  `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`
- Retained title-prompt state SHA-256:
  `11d88ef517c408b3bca5c078c0ccb8081dbf3342f308239dd73403c73f0c3774`
- Internal backup image SHA-256 before and after the run:
  `87d041baecca76a629b18b34e13b47adca1d5995a69bcac769aaca54c8f34035`

Ymir 0.4.2-dev was run with `--debug`, the title state was restored, Start was
held for the first ten of 120 captured frames, and SH-2 writes to physical VDP1
VRAM `[0x05c00000, 0x05c80000)` were recorded. The local evidence directory is
not committed because it contains raw commercial-game-derived trace data.

| Artifact | SHA-256 |
| --- | --- |
| `sh2-memory-write-trace.csv` | `78bbfc74bea7c292db16a4b954ec6702c7244307a58c913ba8788613b867e96c` |
| `sh2-memory-write-trace-bundle.txt` | `e0829832754af38ab2e5bf24cb3935fde713395f69c15c3e67af302f9311d66f` |

The completed bundle records 20,662 writes and zero dropped records.

## Result

The retail `DM.BIN` PRS3 routine has static entry address `0x06024d80`. Its
direct-byte side writes through the caller-provided output pointer; its
zero-side path reads the output window and has no direct output-store
instruction. During this particular title-to-menu transition, the bounded
VDP1 trace contains no writes with a program counter in the PRS3 routine's
`0x06024d80..0x06024e3e` corridor.

The dominant observed VDP1 writer was instead `0x0608ba46` (19,102 writes).
Disassembly of the authenticated `DM.BIN` shows it is a four-byte zero-fill
loop over a VDP1 address range, not the PRS3 routine:

```text
0608ba46: mov.l r5,@r4
0608ba48: add   #16,r4
0608ba4a: cmp/hs r6,r4
0608ba4c: bf    0608ba46
```

This rules out one tempting but unsupported shortcut: treating the PRS3
routine itself as the direct VDP1 producer during this captured transition.
It does not determine the real decompressed buffer owner, the transfer path,
the VDP1 command that consumes a resulting texture, palette interpretation,
or menu placement.

## Short high-WRAM trace

A second run used the same retail inputs, restored title-prompt state, and
held Start for all ten captured frames. It recorded SH-2 writes to high WRAM
`[0x06000000, 0x06100000)`. The completed receipt contains 175,530 writes and
zero dropped records:

| Artifact | SHA-256 |
| --- | --- |
| `sh2-memory-write-trace.csv` | `b0c67d8fa234ba37358ccefb759a52527012d88a48f88bb5690313dde30dd1fd` |
| `sh2-memory-write-trace-bundle.txt` | `40b9b7ad3a852c990f78f1a89580f626e3ce181eecdfdcfcaf5893a4fe506942` |

No row has a program counter inside the PRS3 routine's
`0x06024d80..0x06024e3e` corridor. This does not prove that PRS3 never runs
elsewhere in the game. It only establishes that it did not produce a traced
high-WRAM write during this ten-frame title Start interval. Therefore this
interval cannot be used to invent a PRS3 output-buffer ownership claim.

## Next evidence required

The startup menu remains capture-gated until one coherent real-media trace
bounds all of the following in the same transition:

1. the selected `MENU.BPK` PRS3 stream and the PRS3 output buffer;
2. the producer/transfer path from that buffer to the VDP1 texture window;
3. the VDP1 command consuming that window; and
4. the associated palette state and on-screen placement.

Until then Firestaff must keep the menu surface route blocked rather than
inventing a decoder, palette, or menu pixels.
