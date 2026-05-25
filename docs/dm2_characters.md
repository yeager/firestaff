# DM2 V1 — Characters: Lord Dragoth, NPCs, Companions

**Audit task:** FS-N2-DM2-STORY  
**Status:** COMPLETE  
**Sources:** CRPG Addict (crpgaddict.blogspot.com), DMweb walkthrough (dmweb.free.fr), skproject source (SKULL.ASM, SKWin), existing docs/dm2_champ_types.md, docs/dm2_intro.md, docs/dm2_ai_creatures.md

---

## Player Character: Torham Zed

**Torham Zed** is the protagonist and a mandatory party member.

- **Background:** Aspiring hero sent by his uncle Mylius (World Council member) to investigate Skullkeep
- **Classes:** Fighter (primary, apprentice level), Priest and Wizard (secondary, novice), Ninja (tertiary, neophyte)
- **Starting equipment:** Dagger, leather armor (jerkin, pants, boots), canteen, 3 gold coins, 1 silver coin, 1 gem
- **Personality:** Noble, reluctant hero thrust into an inter-dimensional conflict

---

## Companion Champions (15 Available)

The **Hall of Champions** contains 15 potential champions in cryo-stasis machines.
Players select three to accompany Torham.

### Named Champions (from CRPG Addict playthrough)

| Champion | Primary Class | Notes |
|----------|---------------|-------|
| **Cletus** | Fighter | Monster with facial horns and a third eye/gem in the center of his head |
| **Seri Flamehair** | Priest | — |
| **Saros Shadow Follower** | Wizard | — |
| **Het Farvil** | — | Plant-like; weird hair suggests plant DNA |
| **Equus** | — | Bull-like with horns pointed in wrong direction |
| **Bane Blade Cleaver** | — | Goblin |
| **Anders Light Wielder** | — | Pointy ears |
| **Torham Zed** | Fighter | Mandatory; player can rename |

Champions have six attributes: **Strength, Dexterity, Wisdom, Vitality, Anti-Magic, Anti-Fire**.
Class advancement: **Neophyte → Novice → Apprentice → Journeyman** and beyond.

---

## NPCs

### The Moon Clan Headwoman
- **Role:** First major NPC; initially mistakes Torham for his uncle Mylius
- **Fate:** Killed by a Void orb with her dying breath she gives Torham the quest
- **Quote:** "Attack him there before he attacks us here!"

### Shopkeepers
- Multiple shops in the outpost area
- Left table = sell; right table = buy
- Place cash on table to transact; do NOT throw items at shopkeeper
- A guard protects the shops — cannot be defeated

### Companion NPCs
- Some NPCs join as combat companions (distinct from champion selection)
- Governed by `include/dm2_v1_companion.h`
- Companions fight alongside the party in battle

---

## Enemies & Bosses

### Lord Dragoth — Final Boss
- **Type:** AI index 30 — "LORD DRAGOTH" (skproject/SKWIN/_4976_03a2.h:38)
- **Appearance:** Huge, blue with a red tabard and big teeth
- **Location:** Endgame dungeon (DUNGEON_LEVEL_14); in the Void
- **Abilities:**
  - Unique multi-spell casting (AI script driven)
  - Casts **Reflector spell** (0x55) — unique among all creatures
  - Summons **Dragoth Attack Minions** (AI type 34)
  - High health, very high power, high threat
- **Strategy:** Use the **Numenstaff** to deal damage; teleport out to heal when low, return and repeat
- **Post-defeat:** Lord **Delos** appears, turns Dragoth into an insect, challenges party to a "re-match"

### Dragon (Skullkeep Guardian)
- **Type:** "Second clan door type (0x0A)" — triggers dragon encounter (SKWin.GDAT2.InternalCodes.txt)
- **Location:** Green door with two golden dragons at Skullkeep entrance
- Requires 4 blue clan key pieces to reach

### Void Orbs
- **Type:** Harbingers of Dragoth; phase in from the Void; can kill NPCs
- **Behavior:** Fly directly into Skullkeep

### Red Bearded Men (Goblins)
- **Location:** Lightning area
- **Behavior:** Hit and **steal weapons**, then run away
- **Counter:** Kill quickly or follow to retrieve gear

### Glops, Worms, Bats, Tornadoes
- Standard creatures found throughout zones
- Bats are poisonous — kill before they hit
- Worms drop sellable worm food

### Fireballs
- AI index 51; patched with fireball attack in Skullkeep dungeon

---

## Character System

### Champion Classes (docs/dm2_champ_classes.md)
- Four classes: **Fighter, Ninja, Wizard, Priest** — leveled independently
- Levels 1-4: Neophyte, Novice, Apprentice, Journeyman
- Levels 5+: Geomaster, Water lord, etc.

### Champion Death (docs/dm2_champ_death.md)
- Dead champions can be revived at the resurrection altar (place bones on altar)
- Permadeath mechanics

### Equipment
- Dual-wielding supported (DM2 new feature)
- New weapon types: crossbow, gun, bomb (ranged)
- Weapon GDAT fields include projectile flags

---

## Source References

- Torham Zed / prologue: DMweb walkthrough (dmweb.free.fr)
- Champions list: CRPG Addict blog (crpgaddict.blogspot.com)
- Lord Dragoth AI: skproject/SKWIN/_4976_03a2.h:38
- AI types: skproject/SKWIN/SkGlobal.h CREATURE_AI_TAB_SIZE 64
- Companion system: include/dm2_v1_companion.h
- Champion classes: docs/dm2_champ_classes.md
- Champion death: docs/dm2_champ_death.md
