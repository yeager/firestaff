# JP System Card controller-poll boundary

This note binds the scripted RUN input in the retained JP cold-start capture
to the authentic System Card controller-poll loop. It does not show that the
Theron's Quest title or game code consumed RUN, nor that the game advanced.

## Source identity and address mapping

The System Card image on TRV2 has size 262,656 bytes, MD5
`ff1a674273fe3540ccef576376407d1d`, and SHA-256
`df4f75feebb95e53dfef72dea0787df743e3474ba046ff5a4cb88e34dda93ff1`.
These identities match the capture receipt. The BIOS image remains on TRV2
and is not part of the repository.

Mednafen 1.32.1 `source/src/pce/huc.cpp`, `HuC_Load(Stream* s, bool
DisableBRAM, SysCardType syscard)`, lines 281-287, detects and skips a
512-byte copier header when the file length has a 512-byte remainder. For this
image, logical BIOS address `$E4B4` maps to payload offset `$04B4`, or file
offset `$06B4` including that header.

## Disassembly and capture witness

The verified System Card bytes decode as follows:

```text
$E4A4  CLY
$E4A5  ... controller scan loop ...
$E4B4  LDA $1000
$E4B7  ASL A
$E4B8  ASL A
$E4B9  ASL A
$E4BA  ASL A
$E4BB  STA $2228,Y
$E4BE  STZ $1000
$E4C1  PHA
$E4C2  PLA
$E4C3  NOP
$E4C4  NOP
$E4C5  LDA $1000
$E4C8  AND #$0F
$E4CA  ORA $2228,Y
$E4CD  EOR #$FF
$E4CF  STA $2228,Y
$E4D2  EOR $2232,Y
$E4D5  AND $2228,Y
$E4D8  STA $222D,Y
$E4DB  INY
$E4DC  CPY #$05
$E4DE  BCC $E4A5
```

The held RUN input appears in the capture at `$E4B4` (`SEL=1`, returned
`$3F`) and `$E4C5` (`SEL=0`, returned `$37`), where the active-low RUN wire
bit `$08` is cleared in the returned value. The `run@450:300` script produced
300 application rows over scheduler indices 450-749. Its consumption sidecar
classifies `$E4C5` as the witness, with one event witness, zero
non-System-Card poll reads, and
`game_or_non_system_card_poll_boundary=not_observed`.

The trace contains 40,938 controller-read rows, including 414 `$0008` rows
(207 paired reads at `$E4B4` and `$E4C5`). The branch at `$E4DE` returns to
the controller scan; this is evidence for BIOS polling, not game input
handling.

## Remaining boundary

The authenticated Drator menu-route evidence uses game-owned PCs `$7552`,
`$6E3E`, `$5C8C`, and `$6DBA`. None appears in this capture. The transition
receipt remains `transition=missing`, with no CD IRQ, raw-sector span, or SCSI
read. The System Card loop is now disassembled through its observed poll and
back edge, but the exit from BIOS polling to the game-owned title/menu path is
unproven. Do not treat this capture as title input, a game transition, or
gameplay evidence.
