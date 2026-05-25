# DM2 V1 Story/Lore — Source-Locked

## Sources

- SKULL.ASM (522,128 lines IDA disassembly, sha256: a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
- skproject (github.com/gbsphenx/skproject) HEAD a962896
- SKWIN/README.md
- SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt

---

## The Legend of Skullkeep

Dungeon Master II: The Legend of Skullkeep (1993) is set centuries after the
events of Dungeon Master. The party must infiltrate the Skullkeep — a massive
fortress guarded by a dragon — to recover a legendary artifact of immense power.

---

## Main Quest: Recover the Artifact

**Goal:** Enter the Skullkeep, defeat or bypass its guardian dragon, and claim
the artifact hidden within the fortress.

**Key elements:**
- Skullkeep is a multi-level fortress with outdoor areas, buildings, and
  underground dungeon levels (include/dm2_v1_dungeon_loader.h)
- The dragon (door type 0x0A, "second clan door type") guards a critical path
  (SKWin.GDAT2.InternalCodes.txt)
- Party must progress through outdoor exploration and indoor dungeon levels
- Shops and NPCs available for trading and information (include/dm2_v1_game.h)

---

## How It Differs from DM1

| Element | DM1 | DM2 |
|---------|-----|-----|
| Setting | The Chaos Horde dungeon | Skullkeep fortress |
| Goal | Find the Firestaff | Recover the artifact |
| World | Single dungeon | Fortress + outdoor + dungeon |
| NPCs | None | Shops, companions |
| End boss | Cyclon (champion guardian) | Dragon |
| Progression | Linear dungeon floors | Non-linear exploration |

---

## Dungeon Structure

DM2 combines three level types (dm2_v1_dungeon_loader.h):
1. DM2_LEVEL_OUTDOOR (0) — sky, ground, trees, buildings
2. DM2_LEVEL_INDOOR (1) — standard first-person dungeon
3. DM2_LEVEL_BUILDING (2) — multi-floor buildings within outdoor areas

DM2 supports up to 30 levels (DM2_V1_MAX_LEVELS = 30), more than DM1.

---

## Dragon Encounter

The "second clan door type (0x0A)" is specifically the Skullkeep Dragon door
(SKWin.GDAT2.InternalCodes.txt). This is a unique door type that triggers the
dragon encounter, distinct from the standard clan door (0x09 or similar).

---

## NPC Interactions

DM2 introduces:
- **Companion NPCs** — party members who fight alongside the party
  (include/dm2_v1_companion.h)
- **Shops** — buy/sell equipment, supplies
- **Reputation system** — NPC interaction tracks party standing
  (include/dm2_v1_game.h: "reputation")

Source: include/dm2_v1_game.h — "DM2 has a different engine with outdoor areas, shops, NPCs."

---

## Day/Night and Weather

DM2 implements a time-of-day cycle (0.0-1.0 float) affecting sky color
(dm2_v1_outdoor_renderer.h). Weather zones add atmosphere:
- 0 = clear
- 1 = rain
- 2 = fog
- 3 = storm

---

## STATUS: SOURCE-LOCKED