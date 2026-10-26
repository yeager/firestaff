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
  --vdp1-command-trace /path/to/vdp1-command-trace.csv \
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

## Atomic one-frame follow-up

An isolated Ymir profile loaded the retained title-prompt state, ran one
deterministic frame with `--debug --dump-memory --load-state 2
--capture-frames 1 --capture-no-present`, and emitted its VDP1 VRAM, registers
and command trace in the same emulator-thread capture event. The Japanese IPL,
retail disc and title state were the only inputs. The derived artifact hashes
were:

| Artifact | SHA-256 |
| --- | --- |
| VDP1 VRAM | `62e33308e29fbff3adbab16738cc0be04e1d7a9dadd4a26780ff6445ee9f5682` |
| VDP1 registers | `00477de68c306db0fabcbfef46b9d24c8815c3069ab2223ef2908c975be37368` |
| VDP1 command trace | `6346986721f5a9204cbf00e67b7053f9909806ff763b46bb58a3af838c54ac67` |

It reproduced the same 86/162 whole-surface matches. The frame-local trace has
168 commands, 135 of which are textured sprite commands (VDP1 command types
0, 1 or 2). None has a texture-source start in any matched `MENU.BPK` range.
The apparent earlier overlap was a type-4 polyline command, not a textured
sprite, and therefore has no texture source. This rules out the previous
coarse overlap claim for this captured frame.

The title-menu capture consequently shows that the resident `MENU.BPK` bytes
are not enough to establish that this menu reads them. The production BPK
gate must not be interpreted as a title-menu renderer requirement until a
separate source-to-draw receipt identifies a frame that does consume those
surfaces. This remains non-admission evidence: it does not identify the
actual title/menu assets, their palette, placement, or compositor path.
