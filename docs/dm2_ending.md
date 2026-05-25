# DM2 V1 — Ending: How to Complete Dungeon Master II

**Audit task:** FS-N2-DM2-STORY  
**Status:** COMPLETE  
**Sources:** CRPG Addict (crpgaddict.blogspot.com), DMweb walkthrough (dmweb.free.fr), GameFAQs (gamefaqs.gamespot.com), skproject source, existing docs/dm2_story.md, docs/dm2_intro.md

---

## How to Complete DM2: Full Walkthrough Summary

Completion requires: progress through all zones, collect key items, navigate the
fortress, repair the Zo Link, and defeat Lord Dragoth in his own dimension.

---

## Ending Sequence Steps

### Step 1: Start and Choose Party
- Begin in the Hall of Champions (cave with cryo-stasis machines)
- Select Torham Zed (mandatory) plus 3 champions from 15 available
- Balanced party: fighters up front, magic users behind

### Step 2: Explore Entry Stage
- Move table to reveal items behind sun-burst painting: key, money box, magic map
- Grab healing solutions from nearby chest
- Find resurrection altar
- Head outside to the shop area

### Step 3: Prepare at Shops
- Buy botas (water carriers) for every character
- Buy compass, rope (equip to go down holes)
- Buy swords/axes for fighters
- Sell excess items; exchange coins for gems (place money box on left shop table)
- Search fountain for money
- Stock up heavily on food

### Step 4: Explore the Overworld — Collect Four Keys
The main gate (green door with golden dragons) requires **four pieces of the blue clan key**.

**Key piece locations:**
- The **\* key** — on a table in the food shop (opens the \* gate in the overworld)
- **Clan key pieces** — found across overworld zones (Misty area stonehenge has one)
- Explore all areas: bat caves, misty area, lightning area, brown monster area

**Zone exploration order:**
1. Go through the **\* gate** (requires \* key from food shop)
2. Enter the **bat caves** (green gem, sellable bat remains)
3. Explore the **brown monster area** (worms drop worm food)
4. Enter the **misty area** via archway — find stonehenge with lightning key, scout map, one clan key piece
5. Proceed to **lightning area** — beware of red bearded men who steal weapons

### Step 5: Enter Skullkeep
- Use four blue clan key pieces on the green door with two golden dragons
- Navigate the multi-level fortress (outdoor + building + indoor dungeon)
- Press switches, pull levers, step on pressure plates; fight monsters

### Step 6: Find and Repair the Zo Link
- Locate the ancient **Zo Link** machine in the depths of Skullkeep
- Repair it using items/methods discovered in the dungeon

### Step 7: Activate and Cross into the Void
- Activate the repaired Zo Link → **Void** (Dragoth's dimension)
- Final dungeon level (DUNGEON_LEVEL_14)

### Step 8: Defeat Lord Dragoth
**Lord Dragoth encounter:**
- AI index 30: "LORD DRAGOTH" — huge, blue with red tabard, big teeth
- Unique abilities: Multi-spell casting, Reflector spell (0x55), summons Dragoth Attack Minions (AI type 34)
- High health, very high power, high threat

**Combat strategy:**
- Use the **Numenstaff** to deal damage
- When party health drops to ~50%, teleport back to heal, return and repeat
- Takes approximately half a dozen rounds with this method

### Step 9: Face Lord Delos (Endgame Cinematic)
- After defeating Dragoth, **Lord Delos** appears
- Delos is furious with Dragoth for losing
- Delos transforms Dragoth into an insect ("yum yum")
- Delos challenges the party: "it looks like there is going to be a re-match, hopefully with a new games..."
- **This is the final dialogue — implying a sequel hook**

---

## Credits / Game End

After the Lord Delos scene, the game ends with credits.
Lord Delos is a fully implemented boss character in the skproject source.

---

## Quick Reference: Completion Checklist

- [ ] Choose party: Torham Zed + 3 champions (Hall of Champions)
- [ ] Get magic map, key, money box (Entry Stage)
- [ ] Buy botas, compass, rope, weapons at shops
- [ ] Stock up on food
- [ ] Get \* key from food shop table
- [ ] Open \* gate in overworld
- [ ] Collect 4 blue clan key pieces from overworld zones
- [ ] Enter Skullkeep (green dragon door)
- [ ] Navigate fortress levels
- [ ] Find and repair Zo Link
- [ ] Activate Zo Link → cross into the Void
- [ ] Defeat Lord Dragoth
- [ ] Face Lord Delos → game ends

---

## What Happens After Winning

1. Lord Dragoth is defeated and turned into an insect by Lord Delos
2. Lord Delos challenges the party to a re-match — sequel hook
3. Credits roll
4. No further story content for DM2 itself

Zalk is saved from Dragoth's dimensional invasion — for now.

---

## Source References

- Ending sequence: GameFAQs (gamefaqs.gamespot.com) — "Lord Delos is rather unhappy with Dragoth for losing and turns him into a nasty but tasty insect (yum yum)"
- Lord Dragoth combat: CRPG Addict — "Numenstaff" tactic
- Zo Link: DMweb walkthrough — "fix the Zo Link machine"
- Prologue / mission: CRPG Addict blog
- Final boss AI: skproject/SKWIN/_4976_03a2.h:38; skproject/SkGlobal.h
- DUNGEON_LEVEL_14: skproject source; docs/dm2_intro.md
