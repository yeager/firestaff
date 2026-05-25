# DM2 V1 — Lore: World, Backstory, Setting

**Audit task:** FS-N2-DM2-STORY  
**Status:** COMPLETE  
**Sources:** CRPG Addict (crpgaddict.blogspot.com), DMweb walkthrough (dmweb.free.fr), Sega-16 (sega-16.com), Amiga Reviews (amigareviews.leveluphost.com), skproject source, existing docs/dm2_story.md, docs/dm2_intro.md

---

## World: Zalk

Dungeon Master II is set in the world of **Zalk** — governed by the **World Council**.
The game has **no continuity with Dungeon Master (1987)**; it is a self-contained story
in its own universe. Zalk is threatened by an inter-dimensional invasion led by Lord Dragoth.

---

## Skullkeep Fortress

**Skullkeep** is a long-abandoned fortress on a remote island — the central location of DM2.

- **History:** Ancient fortress, long abandoned; rumored to hold the remains of the Zo Link
- **Guardian:** A dragon guards the entrance (door type 0x0A "second clan door")
- **Plan:** Lord Dragoth has chosen Skullkeep as the site for his beachhead invasion of Zalk
- **Structure:** Multi-level — outdoor areas, multi-floor buildings, underground dungeon (up to 30 levels; DM2_LEVEL_OUTDOOR, DM2_LEVEL_INDOOR, DM2_LEVEL_BUILDING)
- **Access:** Sealed with a lock requiring **four pieces of the blue clan key**
- **Machinery:** Skullkeep contains machinery that runs the fortress

---

## The Void

The **Void** is an alternate dimension — the home territory of **Lord Dragoth**.

- **Nature:** Dark, hostile dimension; origin of the evil orbs that herald Dragoth's invasion
- **Orbs:** Sinister orbs materialize from the Void and fly directly into Skullkeep
- **Travel:** The **Zo Link** enables inter-dimensional travel between Zalk and the Void
- **Endgame:** Final confrontation with Lord Dragoth takes place within the Void

---

## The Zo Link

An ancient machine capable of inter-dimensional travel, located within Skullkeep.

- **Purpose:** Enable travel between Zalk and the Void
- **State:** Broken and decaying — must be repaired by the player
- **Plot function:** The dying Moon Clan headwoman begs Torham to "fix the Zo Link machine" and use it to attack Dragoth in his own dimension

German source (Amiga Reviews): *"vier Schlüsseln, um in die Ruinenfestung 'Skullkeep' zu gelangen, wo eine alte Maschine namens 'Zo Link' vor sich hin rostet — und allein mit ihrer Hilfe kann man zum großen Finale in Dragoths Dimension überwechseln"* ("four keys to get into the ruins fortress 'Skullkeep' where an old machine called the 'Zo Link' rusts away — only with its help can you reach the grand finale in Dragoth's dimension")

---

## Lord Dragoth

**Lord Dragoth** is the primary antagonist — an evil warlord from another dimension.

- **Origin:** The Void (another dimension); not originally from Zalk
- **Plan:** Use Skullkeep as a beachhead for his invasion of Zalk
- **Method:** Create portals to send minions through; uses Void orbs as heralds
- **Boss fight:** AI index 30 "LORD DRAGOTH" — huge, blue with red tabard, big teeth
- **Abilities:** Multi-spell casting, Reflector spell (0x55), summons Dragoth Attack Minions (AI type 34)
- **Fate:** After defeat, Lord Delos turns Dragoth into an insect

---

## The Moon Clan

The indigenous tribe native to the island where Skullkeep is located.

- **Leader:** The Moon Clan headwoman — first major NPC; killed by a Void orb
- **Last words:** Charges Torham to fix the Zo Link and attack Dragoth in the Void

---

## The World Council

Governing body of Zalk.

- **Member:** **Mylius** — Torham Zed's uncle; a World Council member
- **Role:** Sends Torham to investigate Skullkeep; concerned about Dragoth's beachhead plan

---

## Torham Zed's Mission

- **Sent by:** Mylius (World Council member)
- **Destination:** Remote island fortress (Skullkeep)
- **Reason:** Mylius is worried about Skullkeep and believes Dragoth has chosen it for his beachhead
- **Goal:** Investigate the fortress, stop Dragoth's plan, protect Zalk

---

## Environment & Atmosphere

### Weather Zones (dm2_v1_outdoor_renderer.h)

| Weather | Description |
|---------|-------------|
| 0 = Clear | Normal visibility |
| 1 = Rain | Reduced visibility; lightning risk |
| 2 = Fog | Impaired vision |
| 3 = Storm | Dangerous; lightning can hit party |

### Time of Day
DM2 has a day/night cycle (float 0.0–1.0) affecting sky color and ambient lighting.

### Overworld Zones
- **Shop area** — gloomy, rainy with lightning
- **The \* area** — rainy, lightning, grey road markers
- **Bat caves** — dark, poisonous bats, machinery
- **Brown monster area** — worms, monster-infested
- **Misty area** — foggy, tornadoes, stonehenge with stone circle
- **Lightning area** — worms and weapon-stealing goblins

---

## Magic System Lore

### Mana Flowers (Misty area)
- Combine with a staff to make it magical
- Eat to increase mana points
- Sell for loot

### Spell Types
- **Potion (SPELL_TYPE_POTION)** — consumable magical effects
- **Missile (SPELL_TYPE_MISSILE)** — targeted ranged magic
- **General (SPELL_TYPE_GENERAL)** — enchantments and buffs
- **Summon (SPELL_TYPE_SUMMON)** — creature summoning

---

## References

- Zalk / World Council / Mylius: CRPG Addict blog (Aug 2022)
- Skullkeep / Zo Link: Sega-16; Amiga Reviews
- Void / orbs / mission: CRPG Addict; DMweb walkthrough
- Moon Clan headwoman: DMweb walkthrough
- Weather / time-of-day: dm2_v1_outdoor_renderer.h; docs/dm2_story.md
- Dragon / door type 0x0A: SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt
