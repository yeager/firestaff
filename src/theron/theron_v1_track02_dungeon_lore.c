/*
 * theron_v1_track02_dungeon_lore.c — TQ US Track 02 dungeon lore texts
 *
 * Binary-verified from hash-locked US Track 02 BIN
 * (MD5: f23601102138f87c33025877767ebf76).
 *
 * 7 dungeon description/lore blocks at UD 0x27613E–0x276CCB.
 * Each block is multi-line narrative text separated by 0x01 (line break)
 * and terminated by 0x00.  The texts describe the backstory and quest
 * objective for each dungeon, corresponding 1:1 with the 7 creature
 * types (AKUTUBA through DEMON) and 7 quest items.
 *
 * Additional save/load UI strings found nearby:
 *   "THAT FILE ALREADY EXISTS!" at UD 0x2751D3
 *   "REPLACE" at UD 0x275229
 *   "  NO   " at UD 0x275232
 */

#include "theron_v1_track02_dungeon_lore.h"
#include <stddef.h>

static const char *const g_dungeon_lore[THERON_TRACK02_DUNGEON_COUNT] = {
    /* 0: AKUTUBA — Shield Defiant (UD 0x27613E) */
    "Long ago, a great sorcerer named Alaphalon"
    "created a fortress and levitated it high above in the air. "
    "This fortress, Ak-Tu-Ba, is now inhabited by the Mummies who stole the "
    "Shield Defiant at the end of the destruction of Ya-Brodin. "
    "The Mummies seem to have some evil plan using the Shield Defiant, "
    "which is able to withdraw energy directly from the sun.",

    /* 1: DRATOR — Taza Boots (UD 0x2762C4) */
    "Drator was once a warrior and an evil wizard. "
    "He is the last survivor of the Cult of Deaths, which he founded. "
    "He now sleeps in his tower with Taza Boots, "
    "one of the Ya-Brodin's treasures on the top level. "
    "To keep intruders out of his tower, Drator has "
    "filled each level with cunning traps and fierce monsters.",

    /* 2: FORMIC — Taza Poleyn (UD 0x276432) */
    "The city of Formicia is the underground dwelling"
    "place of the Trolins. "
    "It was the Trolins who made the capture of the Brotherhood of "
    "Enlightenment possible. "
    "Tunneling beneath the monastery of Ya-Brodin, they bypassed the "
    "defenses of the Brotherhood and unlocked the gates. "
    "The Trolins' share of the loot from Ya-Brodin was the Taza Poleyn. "
    "They now plot to rule the world above, through"
    "a vast network of underground tunnels they"
    "dig with the help of the Taza Poleyn.",

    /* 3: SARMON — Soulcage (UD 0x276665) */
    "Sarmon was a powerful but an evil wizard who "
    "helped to destroy the Brotherhood of Enlightenment. "
    "His minions stole the Soulcage for him from "
    "the treasures being held in a vault at Ya-Brodin. "
    "Sarmon eventually died, but his powerful magic "
    "still lives in the form of a spirit, and "
    "continues to fiercely defend the Soulcage.",

    /* 4: SHADO — Taza Armour (UD 0x2767E2) */
    "Shadodan is a very old dragon. Some say he is "
    "actually a wizard who was defeated long ago by"
    "a witch and turned into a dragon. "
    "In his den, Shadodan has hidden the Taza Armour, "
    "perhaps the only defense against a dragon's fire "
    "and made finding it very difficult. "
    "Many have been caught by the traps that have "
    "filled his den and killed by his breath.",

    /* 5: THIEF — Tazahelm (UD 0x276958) */
    "Underground of the swamps of Nordoor is the "
    "\"Village of Thieves\". "
    "The denizens of this treacherous place are Gigglers. "
    "They are natural-born thieves, who first "
    "conceived of the plan to loot the Brotherhood of "
    "Enlightenment of its treasures. "
    "They left much of the dirty work to the others"
    "they enlisted in this evil campaign. "
    "But they were careful to keep the Tazahelm "
    "because it boosts its wearers dexterity, "
    "which is everything to a thief like a Giggler.",

    /* 6: DEMON — Retaliator (UD 0x276B43) */
    "The Demon's Gate is the place where all the "
    "things created by evil magic return when their "
    "masters have no more need of them. "
    "The inhabitants of this place, from time to time, "
    "sense the work of great evil elsewhere and"
    "thus participated in the ruin of Ya-Brodin. "
    "And one of them named Sargoth managed to "
    "find the sword, the Retaliator, and took it "
    "back to the Demon's Gate.",
};

const char *theron_v1_track02_us_dungeon_lore(unsigned int dungeon_index)
{
    if (dungeon_index >= THERON_TRACK02_DUNGEON_COUNT) return NULL;
    return g_dungeon_lore[dungeon_index];
}

const char *theron_v1_track02_us_file_exists_warning(void)
{
    return "THAT FILE ALREADY EXISTS!";
}

const char *theron_v1_track02_us_replace_label(void)
{
    return "REPLACE";
}

const char *theron_v1_track02_us_no_label(void)
{
    return "NO";
}
