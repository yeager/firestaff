# DM2 V1 — sound and music

## Original data and current boundary

The PC-DOS edition has 29 HMP music records, indexed `00` through `1c`, in the
`MUSICS` GDAT category of the hash-verified `GRAPHICS.DAT` file. The requested
record is selected by the authentic `SONGLIST.DAT` mapping. Firestaff reads
this data only from the approved GDAT loader; a similar file beside the game is
not a substitute.

DMWeb documents MIDI conversions for these HMP records. SKProject's
`SKWIN/SkWinMIDI.cpp` is a Windows enhancement that plays such external,
already converted `.hmp.mid` files. It is **not** a raw-HMP decoder and does
not prove that an arbitrary side file or generated MIDI result is original game
data.

## Firestaff behavior

- GDAT SFX is decoded from verified original records and can be sent to SDL3
  when the audio device is ready.
- DM2's original queue and positional ordering follow the bounded
  `c_sound.cpp` and `c_sfx.cpp` paths.
- The HMP record can be identified and structurally inspected from
  `GRAPHICS.DAT`, but it is not played. An approved HMP header is not the same
  as correct timing, multichannel track interpretation, or audio output.
- Firestaff does not create WAV, OGG, or MIDI replacements to make a silent
  original music record appear playable.

This is intentionally fail-closed. Music requests can be recorded by startup
and map transitions, but are not reported as played until the entire
source-anchored decoding and output chain is verified.

## Sources

- Original PC-DOS `GRAPHICS.DAT`, GDAT `MUSICS/<track>/dtHMP/0`.
- Original PC-DOS `SONGLIST.DAT`.
- SKProject `SKWINSPX/src/v5/sfxsnd.cpp::DM2_PLAY_MUSIC`, som kontrollerar
  the GDAT record before music is requested.
- SKProject `SKWIN/SkWinMIDI.cpp`, which shows that its MIDI support requires
  an external converted Standard MIDI file.
- [DMWeb: the DM2 PC edition](http://dmweb.free.fr/games/dungeon-master-ii/editions/pc/),
  which lists 29 embedded HMP records and MIDI conversions.

## Remaining work

A future solution must use the verified raw HMP bytes directly and prove all
track boundaries, delta times, MIDI events, and output. It must not rely on an
external converted file, assumed track semantics, or a synthetic audio buffer.
