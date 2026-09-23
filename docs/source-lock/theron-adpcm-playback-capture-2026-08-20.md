# Theron ADPCM playback capture (2026-08-20)

## Scope

The authenticated Mednafen capture now records PC Engine CD ADPCM control
register `$180D` and rate register `$180E` independently of the existing
FIFO-to-ADPCM-RAM transport trace. Each record includes the logical HuC6280
PC, its MPR-derived physical PC, current ADPCM address/read address/length,
rate, control value and the resulting play-state transition.

This was run against the verified US Track 02 raw image
`f23601102138f87c33025877767ebf76`, System Card 3.0
`ff1a674273fe3540ccef576376407d1d`, and the complete real 19-track disc.
No media or capture output is checked into the repository.

## Authentic cold-start result

The run retained 2,048 byte-exact FIFO reads and 2,048 matching writes to
ADPCM RAM. The first source span begins at Track 02 LBA 4719, byte offset 16.
The ADPCM playback sidecar contains 140 register records after its provenance
header. System Card physical PCs `$001715`, `$00171A` and `$001728` repeatedly
wrote control values `$03`, `$02` and `$00` while filling/configuring the
bank. `$001387/$001392` performed initial reset/rate setup.

No control result had bit `$20` set, and the capture contains zero
`playback_start=1` records. The observed transfer is therefore an authentic
ADPCM bank load/configuration, not proof that a decoded sample was played.

## Authentic savestate replay result

The existing Mednafen state `f17f377df210b4a3ae904a13fb85a7f0` was replayed
with the checked seven-event PCE plan
`i, up, run, i, ii, left, right`. It produced 19,070 input transactions but
no `$180D` or `$180E` record at all. That sequence cannot be used to assign a
door, pickup, attack or other gameplay event to an ADPCM sample.

## Production boundary

`theron_v1_play_sound()` and every `Theron_SoundID` remain fail-closed. A
future promotion requires one same-session receipt that contains:

1. an isolated, source-identifiable original gameplay event;
2. its game-owned physical HuC6280 caller PC;
3. a `$180D` stop-to-play transition;
4. the selected ADPCM address and length;
5. byte provenance joining that interval to authenticated Track 02 data.

The new capture closes the instrumentation gap but deliberately does not
invent the missing event-to-sample mapping.
