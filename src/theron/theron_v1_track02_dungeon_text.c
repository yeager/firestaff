#include "theron_v1_track02_dungeon_text.h"
#include <stddef.h>

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * Creature names at UD 0x2741EF, 8-byte fixed-width null-terminated.
 * Dungeon intro stories at UD 0x27613E-0x276CCC.
 *   Control bytes: 0x01=newline, 0x02=paragraph break, 0x03=section start.
 * Retrieval messages at UD 0x27713F-0x277288.
 * GAME SPEED label at UD 0x274227. */

static const char *const g_dungeon_names[THERON_TRACK02_DUNGEON_COUNT] = {
    "AKUTUBA",  /* UD 0x2741EF */
    "DRATOR",   /* UD 0x2741F7 */
    "FORMIC",   /* UD 0x2741FF */
    "SARMON",   /* UD 0x274207 */
    "SHADO",    /* UD 0x27420F */
    "THIEF",    /* UD 0x274217 */
    "DEMON",    /* UD 0x27421F */
};

static const char *const g_treasure_names[THERON_TRACK02_DUNGEON_COUNT] = {
    "Shield Defiant",  /* UD 0x27715B */
    "Taza Boots",      /* UD 0x277189 */
    "Taza Poleyn",     /* UD 0x2771B8 */
    "Soulcage",        /* UD 0x2771E8 */
    "Taza Armour",     /* UD 0x277215 */
    "Tazahelm",        /* UD 0x277245 */
    "Retaliator",      /* UD 0x277272 */
};

/* Stories verbatim from binary. 0x01=newline, 0x02=para break, 0x03=section. */
static const char *const g_stories[THERON_TRACK02_DUNGEON_COUNT] = {
    /* 0: AKUTUBA — UD 0x27613E, 388 bytes */
    "Long ago, a great\x01"
    "sorcerer named Alaphalon"
    "created a fortress and\x01"
    "levitated it high above "
    "in the air.\x01"
    "\x02"
    "                        \x03"
    "This fortress, Ak-Tu-Ba,"
    "is now inhabited by the "
    "Mummies who stole the\x01"
    "Shield Defiant at the\x01"
    "end of the destruction  "
    "of Ya-Brodin. \x01"
    "The Mummies seem to have"
    "some evil plan using\x01"
    "the Shield Defiant,\x01"
    "which is able to\x01"
    "withdraw energy directly"
    "from the sun.           ",

    /* 1: DRATOR — UD 0x2762C4, 364 bytes */
    "Drator was once a\x01"
    "warrior and an evil\x01"
    "wizard.\x01"
    "He is the last  survivor"
    "of the Cult of Deaths,  "
    "which he founded. \x01"
    "He now sleeps in his\x01"
    "tower with Taza Boots,\x01"
    "one of the Ya-Brodin's\x01"
    "treasures on the top\x01"
    "level. \x01"
    "\x02"
    "                        \x03"
    "To keep intruders out of"
    "his tower, Drator has\x01"
    "filled each level with\x01"
    "cunning traps and fierce"
    "monsters. \x01"
    "\x02"
    "                        ",

    /* 2: FORMIC — UD 0x276432, 561 bytes */
    "The city of Formicia is "
    "the underground dwelling"
    "place of the Trolins.   \x01"
    "\x02"
    " \x01"
    "                        \x03"
    "It was the Trolins who\x01"
    "made the capture of the "
    "Brotherhood of\x01"
    "Enlightenment possible.  \x01"
    "\x02"
    "                        \x03"
    "Tunneling beneath the\x01"
    "monastery of Ya-Brodin, "
    "they bypassed the\x01"
    "defenses of the\x01"
    "Brotherhood and unlocked"
    "the gates. \x01"
    "The Trolins'share of\x01"
    "the loot from Ya-Brodin "
    "was the Taza Poleyn.\x01"
    " \x01"
    "\x02"
    " \x01"
    "                        \x03"
    "They now plot to rule\x01"
    "the world above, through"
    "a vast network of\x01"
    "underground tunnels they"
    "dig with the help of\x01"
    "the Taza Poleyn.        ",

    /* 3: SARMON — UD 0x276665, 379 bytes */
    "Sarmon was a powerful\x01"
    "but an evil wizard who  "
    "helped to destroy the\x01"
    "Brotherhood of\x01"
    "Enlightenment.\x01"
    "\x02"
    "                        \x03"
    "His minions stole the\x01"
    "Soulcage for him from\x01"
    "the treasures being held"
    "in a vault at Ya-Brodin. \x01"
    "\x02"
    "                        \x03"
    "Sarmon eventually died, "
    "but his powerful magic  "
    "still lives in the form "
    "of a spirit, and\x01"
    "continues to fiercely\x01"
    "defend the Soulcage.    ",

    /* 4: SHADO — UD 0x2767E2, 372 bytes */
    "Shadodan is a very old\x01"
    "dragon. Some say he is\x01"
    "actually a wizard who\x01"
    "was defeated long ago by"
    "a witch and turned into "
    "a dragon. \x01"
    "In his den, Shadodan has"
    "hidden the Taza Armour, "
    "perhaps the only defense"
    "against a dragon's fire "
    "and made finding it\x01"
    "very difficult. \x01"
    "Many have been caught by"
    "the traps that have\x01"
    "filled his den and\x01"
    "killed by his breath.\x01"
    " \x01"
    "\x02"
    "                        ",

    /* 5: THIEF — UD 0x276958, 489 bytes */
    "Underground of the\x01"
    "swamps of Nordoor is the"
    "\"Village of Thieves\".\x01"
    "The denizens of this\x01"
    "treacherous place are\x01"
    "Gigglers. \x01"
    "They are natural-born\x01"
    "thieves, who first\x01"
    "conceived of the plan to"
    "loot the Brotherhood of "
    "Enlightenment of its\x01"
    "treasures. \x01"
    "They left much of the \x01"
    "dirty work to the others"
    "they enlisted in this\x01"
    "evil campaign.\x01"
    " \x01"
    "\x02"
    "                        \x03"
    "But they were careful to"
    "keep the Tazahelm\x01"
    "because it boosts its\x01"
    "wearers dexterity, which"
    "is everything to a thief"
    "like a Giggler.         ",

    /* 6: DEMON — UD 0x276B43, 392 bytes */
    "The Demon's Gate is the "
    "place where all the\x01"
    "things created by evil\x01"
    "magic return when their "
    "masters have no more\x01"
    "need of them. \x01"
    "The inhabitants of this "
    "place, from time to\x01"
    "time, sense the work of "
    "great evil elsewhere and"
    "thus participated in the"
    "ruin of Ya-Brodin. \x01"
    "And one of them named\x01"
    "Sargoth managed to\x01"
    "find the sword, the\x01"
    "Retaliator, and took it "
    "back to the Demon's\x01"
    "Gate.                   ",
};

static const char *const g_retrieval[THERON_TRACK02_DUNGEON_COUNT] = {
    "  THERON has retrieved\x01 the Shield Defiant.\x01    ",   /* UD 0x27713F */
    "  THERON has retrieved\x01 the Taza Boots.\x01    ",       /* UD 0x277172 */
    "  THERON has retrieved\x01 the Taza Poleyn.\x01    ",      /* UD 0x2771A1 */
    "  THERON has retrieved\x01 the Soulcage.\x01    ",         /* UD 0x2771D1 */
    "  THERON has retrieved\x01 the Taza Armour.\x01    ",      /* UD 0x2771FE */
    "  THERON has retrieved\x01 the Tazahelm.\x01    ",         /* UD 0x27722E */
    "  THERON has retrieved\x01 the Retaliator.\x01    ",       /* UD 0x27725B */
};

const char *theron_v1_track02_us_dungeon_name(unsigned int index) {
    if (index >= THERON_TRACK02_DUNGEON_COUNT) return NULL;
    return g_dungeon_names[index];
}

const char *theron_v1_track02_us_dungeon_story(unsigned int index) {
    if (index >= THERON_TRACK02_DUNGEON_COUNT) return NULL;
    return g_stories[index];
}

const char *theron_v1_track02_us_treasure_name(unsigned int index) {
    if (index >= THERON_TRACK02_DUNGEON_COUNT) return NULL;
    return g_treasure_names[index];
}

const char *theron_v1_track02_us_retrieval_message(unsigned int index) {
    if (index >= THERON_TRACK02_DUNGEON_COUNT) return NULL;
    return g_retrieval[index];
}

const char *theron_v1_track02_us_game_speed_label(void) {
    return "GAME SPEED";  /* UD 0x274227 */
}
