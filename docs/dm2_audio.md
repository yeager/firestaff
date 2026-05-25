# DM2 V1 — Audio System

## Overview

DM2's audio system handles both music and sound effects through an abstracted layer (`c_sound` / `c_midi` / `c_sfx`) that supports multiple backends. Music was originally HMP/MIDI format; the Firestaff SDL port adds WAV/OGG playback as a replacement. Sound effects use a GDAT-based lookup system with per-category sound IDs.

## Architecture

### Core Classes
- `c_sound` — master audio class, inherits from `c_sfx` and `c_midi`
  - Contains all music and SFX state
  - Initialized via `dm2sound.init()`
- `c_midi` — MIDI/HMP music playback (Allegro4 MIDI backend)
  - `do_music(i32 songnr)` — play music track by number
  - `stop_music()` — stop current music
  - `set_midi_volume(i16 wv)` — volume control
- `c_sfx` — sound effect system
- `c_music_wav` — WAV/OGG music loader (Firestaff SDL addition)
  - `do_music_wav(i16 nr)` — play `./DATA/sk%02d.ogg` looped
  - `do_music_stop()` — stop all samples via `al_stop_samples()`

### Playback Frequency
- `PLAYBACK_FREQUENCY = 5500` Hz (DOS)
- `SKWIN_PLAYBACK_FREQUENCY = 6000` Hz (Windows SDL)
- Sound card detection sets `glbSoundFreq_13ce` based on card type

## Music System

### Original (HMP/MIDI)
- `DM2_PLAY_MUSIC(i8 eaxb, bool bForceMusic)` — play music by track number
- `v1dff8a` (i8) — current music track number
- `v1d1512` — previous music track (for change detection)
- Music stored as HMP (HMIDI) format, played via `do_music(songnr)`
- `dm2sound.v1dd1d1` — master volume (default 90)

### Firestaff SDL Port (WAV/OGG)
- `do_music_wav(i16 nr)` loads `DATA/sk%02d.ogg` (e.g., sk00.ogg, sk01.ogg)
- Looped via `ALLEGRO_PLAYMODE_LOOP`
- `do_music_stop()` stops all samples
- Files expected in `DATA/` directory relative to executable

### Music vs DM1
- DM1: simple PC speaker / AdLib music
- DM2: MIDI/HMP with track numbers, separate GDAT music category
- DM2 supports force-play flag (`bForceMusic`)
- Firestaff SDL port replaces MIDI with OGG (same track numbering)

## Sound Effects

### Sound Definition System
- Sounds defined via GDAT entries with category/index/sound ID
- `SoundEntryInfo` struct: `glbSoundList[64]` — sound effect table
- Sound ID mapping: `DM2_QUERY_SND_ENTRY_INDEX(cls1, cls2, cls3)` — resolve sound index

### Sound Categories (from defines.h)

#### General/Standard Sounds
| Constant | Value | Description |
|---|---|---|
| `SOUND_STD_EXPLOSION` | 0x81 | Explosion (GDAT2 V5) |
| `SOUND_STD_DEFAULT` | 0x84 | Punch, fall, test wall, gethit |
| `SOUND_STD_KNOCK` | 0x85 | Falling item, punch knock |
| `SOUND_STD_THROW` | 0x86 | Throw/shoot item |
| `SOUND_STD_ACTIVATION` | 0x88 | GDAT2 V5 activation |
| `SOUND_STD_TELEPORT` | 0x89 | GDAT2 V5 teleport |
| `SOUND_STD_ACTIVATION_MESSAGE` | 0x00 | Message tick (PC9821 only) |
| `SOUND_STD_SPELL_MESSAGE` | 0x01 | Spell message (PC9821 only) |
| `SOUND_STD_TELEPORT_MESSAGE` | 0x02 | Teleporter message (PC9821 only) |

#### Champion Sounds
| Constant | Value | Description |
|---|---|---|
| `SOUND_CHAMPION_ATTACK` | 0x00 | Attack swing |
| `SOUND_CHAMPION_SHOOT` | 0x01 | Ranged attack |
| `SOUND_CHAMPION_GETHIT` | 0x82 | Champion takes damage |
| `SOUND_CHAMPION_EAT_DRINK` | 0x83 | Consuming food/water |
| `SOUND_CHAMPION_SCREAM` | 0x87 | Death scream |
| `SOUND_CHAMPION_BUMP` | 0x8A | Collision/bump |
| `SOUND_CHAMPION_FOOTSTEP` | 0x92 | SPX custom (not in retail DM2) |

#### Creature Sounds
| Constant | Value | Description |
|---|---|---|
| `SOUND_CREATURE_MOVE` | 0x00 | Movement |
| `SOUND_CREATURE_TURN` | 0x01 | Turning (Minion) |
| `SOUND_CREATURE_GET_HIT` | 0x02 | Hit reaction |
| `SOUND_CREATURE_REFLECTOR` | 0x03 | Reflecting attack (Dragoth) |
| `SOUND_CREATURE_JUMP` | 0x04 | Jump (Rocky) |
| `SOUND_CREATURE_ATTACK_1` | 0x07 | Melee attack |
| `SOUND_CREATURE_PICK_STEAL` | 0x08 | Pick/steal (Thief) |
| `SOUND_CREATURE_SPAWN` | 0x10 | Spawn/appear |
| `SOUND_CREATURE_DEATH` | 0x11 | Death |
| `SOUND_CREATURE_ATTACK_2` | 0x12 | Secondary attack (Thorn Demon) |

#### Special
| Constant | Value | Description |
|---|---|---|
| `SOUND_ITEM_TAKE` | 0x60 | SPX custom |
| `SOUND_ITEM_PUT_DOWN` | 0x61 | SPX custom |
| `SOUND_DOOR_STEP` | 0x8E | Step on door |
| `SOUND_DOOR_OPENED` | 0x90 | SPX custom (not in retail) |
| `SOUND_DOOR_CLOSE` | 0x8F | Door closing |
| `SOUND_MINION_TRANSFORMS` | 0x8B | Minion transformation |
| `SOUND_ACTIVATION_LOOP` | 0xA0 | Looping activation sound |

### Sound Queue System
- `QUEUE_NOISE_GEN1()` / `QUEUE_NOISE_GEN2()` — enqueue ambient/world sounds
- Parameters: category, index, sound ID, volume, pitch, x, y, flags
- Sounds positioned in world space (distance attenuation)
- `DM2_PROCESS_SOUND(i16 eaxw)` — process one sound from queue

### Sound Playback Functions
- `DM2_PLAY_SOUND(i16 eaxw, s_sfx* edxp_s60)` — play sound with SFX struct
- `DM2_SOUND1()` through `DM2_SOUND9()` — various sound operations
- `DM2_SOUND7(i16 n)` — query sound entry
- `DM2_SOUND8(bool flag)` — sound toggle
- `DM2_SOUND9(i8 eaxb, i8 edxb, i8 ebxb)` — advanced sound query

### Sound vs DM1
| Feature | DM1 | DM2 |
|---|---|---|
| Sound system | AdLib/FM synthesis | SoundBlaster-compatible |
| Sound IDs | Limited set | Full GDAT lookup (64 entries) |
| Creature sounds | Generic | Per-creature-type sound IDs |
| Ambient sounds | Minimal | World-position queued sounds |
| Champion sounds | Attack only | HP/Stamina/Mana barks + reactions |

## Audio Initialization

`c_sound::init()`:
1. `init_sfx()` — initialize sound effect system
2. `init_midi()` — initialize MIDI subsystem
3. Sets default volume `v1dd1d1 = 90`
- `dtor()` calls `al_uninstall_audio()` (Allegro cleanup)

## Key Globals
- `dm2sound` — global sound object
- `glbSoundList[64]` — sound entry table
- `glbSoundCardType` — detected sound card (SoundBlaster=6, etc.)
- `glbSoundBlasterBasePort` — SB I/O port
- `v1dff8a` — current music track number
- `v1d1512` — previous music track
- `glbXAmbientSoundActivated` — ambient sound state

## Source Files
- `skproject/SKULLWIN/c_sound.h` / `.cpp` — master audio class
- `skproject/SKULLWIN/c_midi.h` — MIDI playback
- `skproject/SKULLWIN/c_music_wav.cpp` — WAV/OGG music (Firestaff SDL)
- `skproject/SKULLWIN/c_sfx.h` / `.cpp` — sound effects
- `skproject/SKWIN/defines.h` — SOUND_* constants
- `skproject/SKWIN/SkWinCore.h` — audio globals (volume, card type, frequency)
