# Nexus `MENU.BPK` to VDP1 snapshot join

This receipt retains a byte-level observation from the Japanese retail title
menu. It is deliberately narrower than a renderer admission: it proves a
subset of real `MENU.BPK` output was present in one VDP1 VRAM snapshot, not
which VDP1 commands consumed it or how the Saturn palette/compositor presented
it.

## Inputs

| Input | SHA-256 |
| --- | --- |
| Japanese retail `MENU.BPK` | `740ab2a864f04b89cddb172ce2560044fcc8c6a7f98ae2fe50461aa8da886636` |
| Paused retail-title-menu `vdp1-vram.bin` | `968e9d0f05de45ff8c97ea562988cd0aea7c125b54822983ca52c12edc88b895` |

The 512 KiB VDP1 image belongs to
`nexus_v1_ymir_title_menu_replay_20261020`. Its accompanying receipt identifies
the state as a Japanese retail-disc boot followed by one Start press at the
title prompt. It is an emulator observation, not a hardware capture.
The snapshot has no command trace from the same capture, so do not join it
with a trace from another run.

## Reproduction

```sh
python3 scripts/analyze_nexus_menu_bpk_vdp1_snapshot_join.py \
  /home/trv2/.firestaff/data/nexus/MENU.BPK \
  /home/trv2/work/firestaff-parity-debug-20261024/parity-evidence/verification/nexus_v1_ymir_title_menu_replay_20261020/dumps/vdp1-vram.bin \
  --expected-vdp1-sha256 968e9d0f05de45ff8c97ea562988cd0aea7c125b54822983ca52c12edc88b895
```

Only add `--vdp1-command-trace` when the trace and VRAM dump are from one
atomic capture. This retained replay has no such command trace.

The tool rejects a different `MENU.BPK` identity, malformed BPPK/PRS3 framing,
an image that is not a 512 KiB VDP1 dump, or a supplied snapshot hash mismatch.
It decodes the documented bounded DMWeb PRS3 stream and searches for every
possibly overlapping occurrence of each whole decoded surface, directly and
with 16-bit word order swapped. A supplied command trace must come from the
same capture as the VRAM snapshot.

## Result

The hash-pinned rerun decodes all 162 declared PRS3 surfaces. Entries **77
through 162** occur byte-exactly in VDP1 VRAM (86/162 surfaces); none of those
joins requires word swapping. Their first direct-match offsets descend from
`0x6c700` (entry 77) to `0x61d60` (entry 162). The other 76 declared surfaces
do not have a whole-surface direct or word-swapped match in this snapshot.

This is byte-level evidence that authentic menu-package surfaces were present
in VDP1 VRAM. It does not prove which command consumed them or authorize a host
menu renderer. The following independent evidence is still required before
changing presentation admission:

- a command-chain receipt tying each admitted source range to rendered commands;
- Saturn CLUT/palette ownership and pixel-mode evidence;
- destination placement/order evidence; and
- an end-to-end captured comparison of the admitted route.

Until then, the tool emits `semantic_admission=blocked` and no decoded pixels
are handed to the production renderer.

## Command trace status

A previous draft of this note reported a one-frame VRAM/command-trace pair and
concluded that no textured command started in a matched `MENU.BPK` range. The
two raw files are not present together in the retained evidence, so that join
cannot be independently reproduced. Withdraw the 168-command, 135-textured,
zero-source-hit result and do not use it as evidence. The retained 2026-10-25
command history and the 2026-10-20 VRAM snapshot are separate captures;
pairing them would be invalid. A new atomic capture with both outputs is
required before making any command-consumption claim.

The confirmed VRAM snapshot proves only that 86 real `MENU.BPK` surfaces were
resident in that snapshot. It does not establish whether the title menu reads
them. This remains non-admission evidence and does not identify the active
menu assets, palette, placement or compositor path.

## Five-frame Start write trace

A separate isolated capture loaded the same retained Japanese title-prompt
state, held Control Pad 1 Start for capture frame zero, and ran five frames with
`--debug --trace-sh2-writes-begin 0x05c00000 --trace-sh2-writes-end
0x05c80000`. Ymir wrote a complete 1,082-row VDP1 SH-2 trace without dropped
rows. It is intentionally a post-state-load observation: it can identify work
caused by Start, but cannot claim to contain the earlier pre-load that produced
the title-prompt textures. Its trace is not paired with the 2026-10-20 VRAM
snapshot above.

The most frequent writer was master SH-2 PC `0x0608ba46`, with 1,004 writes in
`0x05c000c0..0x05c18710`. Disassembly of the captured WRAM bytes at that PC is
a zero-fill loop (`mov.l r5,@r4`, then `add #16,r4` and branch). It is therefore
a VDP1 clear, not a texture upload. No traced write covers the title-frame
texture starts `0x05c02460`, `0x05c02660`, or `0x05c02740`, nor the resident
`MENU.BPK` match interval.

Use the retained deterministic trace with:

```sh
python3 scripts/analyze_nexus_vdp1_sh2_write_trace.py \
  /path/to/sh2-memory-write-trace.csv \
  --source-address 0x05c02460 \
  --source-address 0x05c02660 \
  --source-address 0x05c02740
```

It retains only the observed PC/address facts and always leaves semantic
admission blocked.
