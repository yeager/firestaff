# DM1 original XP transaction

Source review and local verification: 2026-09-06. This is numeric runtime
coverage, not complete game, platform or emulator parity.

## Ownership

ReDMCSB CHAMPION.C F0304:865-989 owns XP scaling, the base-skill threshold,
level-up bonuses and RNG before publishing champion state. The world-level
`F0884_WORLD_AwardSkillExperience_Compat` now connects those operations.
Lifecycle skill rows own XP; party maxima supply the current champion values
used for bonuses. Updated maximum health/stamina/mana and attribute maxima
are published to both representations. Current vitals are not refilled.

The PC34 rule uses two bits for antimagic. Authenticated F20 startup binds
modulo three as non-save edition configuration. The master RNG is advanced
once per original draw; resting practice does not grant level bonuses.

| Live award path | Numeric owner |
| --- | --- |
| Successful F0412 spell | M10, after effect and before action disable |
| Failed F0412 practice | M10, before failure return |
| F0230 parry | M10, before later attack RNG |
| F0231 melee side effects | M10 |
| Steal sensor effect | M10 |
| F0328 throw | M11 calls the world transaction |
| F0407 action tail | M11 calls the world transaction |
| F0401 influence and other M11 magic awards | M11 calls the world transaction |

Successful spell emissions no longer award XP in M11. M10 emits level-up
notifications for presentation without repeating the mutation. Existing M11
direct-award logging remains separate from original message-area parity.

## Evidence

- Independent bounded unit fixtures check both antimagic rules and default
  PC34 behavior, maxima publication, unchanged current vitals, invalid-call
  immutability, resting practice and no repeated bonus below the next threshold.
- Original I34E archive `ee7b83cdb88c39c441a319f9610e97d6` supplies the actual
  Mon Light spell and map difficulty. A clearly artificial RAM mastery setup
  crosses the Wizard threshold through the public rune/cast API in Original,
  V2.0 and V2.1. Base XP changes from 499 to 925 exactly once; maxima reach
  the live party, and animation/redraw preserves champion, lifecycle and RNG.
- Production build and five final skill, original-XP, failed-practice and
  pending-damage regressions pass (0.49s). Earlier action/F20 media checks
  pass (16.30s). No game data was created, modified or extracted to disk.

## Remaining proof

Natural-play emulator traces, each attack/sensor/throw boundary crossing,
earlier Atari/Amiga edition admission, original recently-upgraded flags and
localized original message-area timing remain open. Existing signed XP
saturation and maximum-stat edge behavior also need a separate source audit.
Savegame format work remains deferred; edition configuration is not game data.
