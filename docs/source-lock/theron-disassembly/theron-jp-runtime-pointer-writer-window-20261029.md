# Japanese runtime pointer-writer window

Date: 2026-10-29

## Runtime capture

An isolated 25-second Mednafen 1.32.1 `pce_fast` capture used authentic
Japanese Rev. 1 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`), System
Card 3.0 (MD5 `ff1a674273fe3540ccef576376407d1d`), and the Japanese Ak-Tu-Ba
save state (MD5 `14dec90b96ec3e14622ec0fab535a92f`). The new instrumented
binary MD5 was `31851be2c34048d64c59e6d65947c9e0`. The code-window sidecar
SHA-256 is
`faed82c17027dc6ae0db3e7f71eaffbecbedfa05f86e27579cd720ec07a49f33`.

The candidate-write hook read a 32-byte instruction window around each
changed write through the active HuCPU MPR mapping. The capture records MPR
`$6D` and physical PCs `$0DACDD` and `$0DACE2`. Its input was Button I only at
mouse-script updates 245–247 and 432–434; other updates were zero. Four
changed writes were recorded:

| Logical address | Value | Writer PC |
| --- | --- | --- |
| `$28FE` | `$3C` | `$CCDD` |
| `$2902` | `$78` | `$CCE2` |
| `$28FF` | `$3C` | `$CCDD` |
| `$2903` | `$78` | `$CCE2` |

The capture wrapper withheld dynamic admission: `transition=missing`, zero
non-System-Card CD reads, and zero authenticated CD-to-RAM receipts. The save
state bypasses the original loader, so the MPR window is not dynamically
bound to a specific Track 02 copy.

## Authentic Track 02 comparison and disassembly

The two captured 32-byte windows match the authentic `TQJP02.bin` at seven
offsets each. The raw image is 8,102,640 bytes. The `$CCD5` window SHA-256 is
`4f9e9481e2f06b6e4db40126fb6456248a6d7ae3d0f39783bd75f5613401c130`; its
matching offsets are `659813`, `960869`, `1261925`, `1562981`, `1864037`,
`2165093`, and `2466149`. The `$CCDA` window SHA-256 is
`60f5160e442eb75508c7623698b0e7769deb819a7c41f938ed75c75228b4012f`; its
matching offsets are `659818`, `960874`, `1261930`, `1562986`, `1864042`,
`2165098`, and `2466154`.

MAME `unidasm -arch h6280` on the first exact match decodes the relevant
instructions as:

```asm
00CCD5: AD 11 29    LDA $2911
00CCD8: 9D 04 29    STA $2904,X
00CCDB: A5 C7       LDA $C7
00CCDD: 9D FC 28    STA $28FC,X
00CCE0: A5 C9       LDA $C9
00CCE2: 9D 00 29    STA $2900,X
00CCE5: E8          INX
00CCE6: E0 04       CPX #$04
00CCE8: 90 01       BCC $CCEB
00CCEA: 82          CLX
00CCEB: 8E 09 29    STX $2909
00CCEE: 60          RTS
```

This is a four-entry indexed write loop. The captured `$CCDD` and `$CCE2`
writes at `$28FE/$2902` and `$28FF/$2903` correspond to X values 2 and 3.
The instructions establish data flow from `$2911`, `$C7`, and `$C9` into
indexed RAM ranges; they do not establish the ranges' pointer, coordinate,
hover, or hitbox meaning. Seven identical retail source copies plus a
save-state runtime window are static correlation, not source-LBA provenance.

## Reproduction limit

The older mixed capture `r7` logged additional writes from `$CCD8` and
`$D39D`, including `$2904/$2905`. Repeating the same scheduled movement and
Button I samples with the new binary (`r12`) and the earlier binary (`r13`)
produced only the four writes above; both captures had 2,988 input
transactions, while `r7` had 1,909. The additional `$2904/$2905` result is
therefore not yet reproducible and must not be treated as click semantics.
The static `$D395` window is now decoded in both regional binaries, but the
older `$D39D` logical-PC label is not bound to that candidate code. It still
needs a same-session trace with executed opcode bytes, active MPRs, physical
PC and source-LBA provenance; see the
[`$D39D` source-disassembly note](theron-d39d-static-source-disassembly-20261029.md).

Private sidecars and save-state remain on TRV2. No game media, save-state,
firmware, or raw RAM snapshot was added to Git. This reference disassembly
does not change the native Firestaff renderer or authorize world-coordinate
mouse behavior.
