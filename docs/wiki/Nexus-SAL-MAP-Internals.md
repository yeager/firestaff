# Nexus SAL/MAP Bounded Route

## DONE

The retail Japanese corpus at `FIRESTAFF_NEXUS_DATA_DIR` contains sixteen
`SNDLEV00` through `SNDLEV15` SAL/MAP pairs. Each MAP has a 24-byte leading
region, a sequence of eight-byte records, and an `ffff` terminator. The
implemented record route retains only the observed fields: raw selector byte,
raw attribute byte, big-endian SAL offset, and big-endian SAL size. It accepts
a selector only when it occurs exactly once and its complete SAL window is in
bounds. The focused corpus test covers all sixteen pairs.

This is a container route, not an audio route: the selector and attribute have
no asserted Saturn event meaning, returned SAL bytes are never decoded, and
no playback is requested from this result.

## Host-Side Event Dispatch (Still Unmapped)

`nexus_sound_set_event_selector()` lets the host engine bind a
`Nexus_SoundEvent` to a raw MAP selector, and a per-event dispatch table
(`g_event_selector`, default `-1`/unmapped) looks up the bound selector at
playback time. This is purely a host-owned binding mechanism — it does not
assign Saturn event meaning to any selector value. Until original Saturn
event→selector evidence is available, every entry stays unmapped and the
dispatch table is fail-closed.

## TODO

Establish the Saturn sound-driver call ABI and the SAL codec from executable
trace or disassembly evidence. Only that evidence can assign selector meaning,
interpret the attribute byte, or permit audio playback.

## Verification

```bash
cc -std=c99 -Wall -Wextra -O2 -Iinclude \
  tests/test_nexus_v1_sal_map_corpus.c \
  src/nexus/nexus_v1_sound.c src/nexus/nexus_v1_audio_receipt.c \
  -o /tmp/test_nexus_v1_sal_map_corpus
FIRESTAFF_NEXUS_DATA_DIR=/Users/bosse/.firestaff/data/nexus \
  /tmp/test_nexus_v1_sal_map_corpus
```
