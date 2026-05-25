# DM2 V1 — Story: Plot, Lord Dragoth, Campaign Structure

**Audit task:** FS-N2-DM2-STORY  
**Status:** COMPLETE  
**Sources:** CRPG Addict (crpgaddict.blogspot.com), DMweb walkthrough (dmweb.free.fr), skproject source, SKULL.ASM disassembly, existing docs/dm2_story.md, docs/dm2_intro.md

---

## The Legend of Skullkeep

**Dungeon Master II: The Legend of Skullkeep** (FTL Games, 1993–1995) is set in a world
called **Zalk**. Unlike DM1, this sequel has a distinct, self-contained backstory with
no continuity to the original Dungeon Master.

The player takes the role of **Torham Zed**, a young warrior sent by his uncle **Mylius**
—a member of the governing **World Council** — to investigate a remote island fortress
known as **Skullkeep**, long rumored to hold the remains of a mysterious machine.

---

## Plot Summary

Evil is stirring in Skullkeep. Weird, sinister **orbs** keep materializing in the world
**"from the Void"** and flying directly into the castle. These orbs are heralds of a
greater threat: an evil warlord named **Lord Dragoth**, who hails from another dimension
and plans to use Skullkeep as a **beachhead** for his invasion of Zalk.

Torham meets the **headwoman of the Moon Clan** — an elderly woman who initially mistakes
him for his uncle Mylius. Just before one of the orbs kills her, she reveals Dragoth's
plan: he must create portals to send his minions through during his preparation. She
implores Torham with her dying breath to **fix the Zo Link machine** inside Skullkeep
and use it to cross the Void, **"attack him there before he attacks us here!"**

---

## Campaign Structure

### 1. Hall of Champions
Starting area. Torham walks up to cryo-stasis machines containing **15 potential
champion companions** to choose from.

### 2. Entry Stage
Stonework room with interactable objects. Behind a sun-burst painting: key, money box,
magic map. Nearby chest has healing solutions. Next room has the **resurrection altar**.

### 3. Shop Area / Outpost
Gloomy, rainy overworld with lightning. Multiple shops. Critical purchases: botas (water
carriers), compass, rope, weapons. Stock up heavily on food.

### 4. The \* Area (Main Overworld)
Rainy, lightning-filled overworld. Contains Skullkeep entrance, bat caves, brown monster
area, misty area, lightning area.

### 5. Skullkeep Fortress
Multi-level fortress (outdoor + building + indoor dungeon, up to 30 levels). The **Zo Link**
machine is inside. Requires 4 blue clan key pieces to enter.

### 6. Endgame: The Void
After fixing the Zo Link and activating it, the party crosses into the Void (Dragoth's
dimension) for the final confrontation.

---

## Key Campaign Milestones

| Phase | Objective |
|-------|-----------|
| Prologue | Choose party from Hall of Champions |
| Early | Explore outpost, buy supplies, find keys and map |
| Mid | Collect 4 blue clan key pieces; enter Skullkeep |
| Core | Navigate Skullkeep dungeon; find and repair Zo Link |
| Late | Activate Zo Link; enter the Void |
| Endgame | Defeat Lord Dragoth; face Lord Delos |

---

## World / Lore

- **World:** Zalk — governed by the **World Council** (Mylius is a member)
- **Skullkeep:** Long-abandoned fortress; contains the Zo Link machine
- **The Void:** Alternate dimension; Dragoth's home territory
- **Zo Link:** Ancient machine for inter-dimensional travel
- **Moon Clan:** Indigenous tribe; their headwoman gives the main quest
- **The orbs:** Harbingers of Dragoth's arrival; emerge from the Void

---

## References

- Prologue text: dmweb.free.fr walkthrough (J. Johnson-Appoo & Lu Richardson)
- World/lore: CRPG Addict blog (crpgaddict.blogspot.com, Aug 2022)
- Zone structure: Lemon Amiga solution (www.lemonamiga.com)
- Dungeon types: dm2_v1_dungeon_loader.h
- Lord Dragoth AI: skproject/SKWIN/_4976_03a2.h:38; AI index 30 "LORD DRAGOTH"
- Endgame sequence: DUNGEON_LEVEL_14; Dragoth AI type 30, Attacker minion AI type 34
