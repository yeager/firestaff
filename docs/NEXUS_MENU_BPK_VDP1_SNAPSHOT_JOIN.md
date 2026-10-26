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

## Reproduction

```sh
python3 scripts/analyze_nexus_menu_bpk_vdp1_snapshot_join.py \
  /home/trv2/.firestaff/data/nexus/MENU.BPK \
  /home/trv2/work/firestaff-parity-debug-20261024/parity-evidence/verification/nexus_v1_ymir_title_menu_replay_20261020/dumps/vdp1-vram.bin \
  --expected-vdp1-sha256 968e9d0f05de45ff8c97ea562988cd0aea7c125b54822983ca52c12edc88b895
```

The tool rejects a different `MENU.BPK` identity, malformed BPPK/PRS3 framing,
an image that is not a 512 KiB VDP1 dump, or a supplied snapshot hash mismatch.
It decodes the documented bounded DMWeb PRS3 stream and searches only for a
whole decoded surface, directly and with 16-bit word order swapped.

## Result

The receipt decodes all 162 declared PRS3 surfaces. Entries **77 through 162**
occur byte-exactly in VDP1 VRAM (86/162 surfaces); none of those joins requires
word swapping. Their observed offsets descend from `0x6c700` (entry 77) to
`0x61da0` (entry 162). The other 76 declared surfaces do not have a whole-
surface direct or word-swapped match in this snapshot.

This is useful positive evidence for an authentic menu upload, but it does not
authorize a host menu renderer. The following independent evidence is still
required before changing presentation admission:

- a command-chain receipt tying each admitted source range to rendered commands;
- Saturn CLUT/palette ownership and pixel-mode evidence;
- destination placement/order evidence; and
- an end-to-end captured comparison of the admitted route.

Until then, the tool emits `semantic_admission=blocked` and no decoded pixels
are handed to the production renderer.
