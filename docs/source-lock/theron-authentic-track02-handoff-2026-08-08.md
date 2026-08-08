# Theron authentic Track 02 startup handoff

**Verified: 2026-08-08.** This receipt records a real PC Engine CD startup
through Mednafen using the supplied US Track 02 media. It is deliberately
separate from Firestaff's semantic loader and screenshot-promotion gates.

## Bound inputs

| Input | Identity |
|---|---|
| US CUE | MD5 `bae1628ca5bc9a0808d711a6de051071` |
| US Track 02 MODE1/2048 ISO consumed by the capture | MD5 `ceb02343868f80cec899e9b239aff2da` |
| System Card 3.0 | MD5 `ff1a674273fe3540ccef576376407d1d` |
| Instrumented Mednafen executable | MD5 `06ac100b11449da433d9b7abf670b240` |
| Tracked menu image | SHA-256 `c9e0752019bcdf8d32bc9ba4b77f80667ec2c02fbd035c38eefa6c0d60498204` |

The game data and System Card remain user-supplied and are not committed to
the repository. The image is a crop of the real Mednafen window, not generated
art and not a Firestaff renderer frame.

## Positive evidence

The host event path received a real macOS Return key pair (`SDL scancode 40`),
Mednafen exposed the configured PCE Run bit as `raw=0008`, and the CD trace
recorded 56 SCSI reads and 175 raw-sector records. The first authenticated
Track 02 reads include LBA 3234 followed by LBA 4165 and later startup reads.
The resulting image shows the authentic Theron's Quest title/menu screen with
`NEW GAME`, `LOAD GAME`, the title logo, and the original copyright line.

This proves the following narrow boundary:

```text
System Card + US CUE/Track 02
        -> real Mednafen PCE input
        -> BIOS Run handoff
        -> CD Track 02 sector reads
        -> authentic Theron's Quest title/menu frame
```

## Explicitly not proven

This receipt does not prove Firestaff's `$2600` main-RAM consumer, semantic
level/object/tile binding, square-to-tile perspective, VCE palette ownership,
HUD binding, RNG consumption, creature AI/attacks/damage/loot, generator
timing, T700 stat cadence, or T900 object rules. Those remain fail-closed until
their own authenticated runtime consumer and disassembly receipts exist.

The corresponding tracked image is
`verification-screens/theron-quest-us-main-menu.png`. Its README caption
identifies it as an original-media Mednafen capture so it cannot be confused
with a completed Firestaff runtime screenshot.
