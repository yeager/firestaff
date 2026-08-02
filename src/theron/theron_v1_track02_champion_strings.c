#include "theron_v1_track02_champion_strings.h"

/* Source: US Track 02 BIN, hash-verified MD5 f23601102138f87c33025877767ebf76.
 * Evidence: parity-evidence/pass215_theron_track02_binary_analysis.md */

/* UD 0x1C9A32: 4 champion classes */
static const char *const g_classes[THERON_TRACK02_CLASS_COUNT] = {
    "FIGHTER", "NINJA", "PRIEST", "WIZARD",
};

/* UD 0x1C9B15: 6 stat names */
static const char *const g_stats[THERON_TRACK02_STAT_COUNT] = {
    "STRENGTH", "DEXTERITY", "WISDOM", "VITALITY", "ANTI-MAGIC", "ANTI-FIRE",
};

/* UD 0x1CFD85: 3 resource bar names */
static const char *const g_resources[THERON_TRACK02_RESOURCE_COUNT] = {
    "HEALTH", "STAMINA", "MANA",
};

/* UD 0x1C9B6B: 16 skill level names.
 * Levels 8-13 have prefix glyphs 0x60-0x65 in the original binary
 * (custom font indices for rank icons). We store only the text. */
static const char *const g_skill_levels[THERON_TRACK02_SKILL_LEVEL_COUNT] = {
    "NEOPHYTE",    /* 0 */
    "NOVICE",      /* 1 */
    "APPRENTICE",  /* 2 */
    "JOURNEYMAN",  /* 3 */
    "CRAFTSMAN",   /* 4 */
    "ARTISAN",     /* 5 */
    "ADEPT",       /* 6 */
    "EXPERT",      /* 7 */
    "MASTER",      /* 8  — prefix glyph 0x60 */
    "MASTER",      /* 9  — prefix glyph 0x61 */
    "MASTER",      /* 10 — prefix glyph 0x62 */
    "MASTER",      /* 11 — prefix glyph 0x63 */
    "MASTER",      /* 12 — prefix glyph 0x64 */
    "MASTER",      /* 13 — prefix glyph 0x65 */
    "ARCHMASTER",  /* 14 */
    "ARCHMASTER",  /* 15 — guard entry */
};

/* UD 0x1DEEA5: 4 object/shield actions (precede hand actions in binary) */
static const char *const g_object_actions[THERON_TRACK02_OBJECT_ACTION_COUNT] = {
    "BLOCK",     /* 0 — UD 0x1DEEA5 */
    "CHOP",      /* 1 — UD 0x1DEEAB */
    "BLOW HORN", /* 2 — UD 0x1DEEB2 */
    "FLIP",      /* 3 — UD 0x1DEEBC */
};

/* UD 0x1DEEC1: 5 hand-to-hand/movement actions */
static const char *const g_hand_actions[THERON_TRACK02_HAND_ACTION_COUNT] = {
    "PUNCH",       /* 0 */
    "KICK",        /* 1 */
    "WAR CRY",     /* 2 */
    "STAB",        /* 3 */
    "CLIMB DOWN",  /* 4 */
};

/* UD 0x1DEEE4: 30 combat/spell action names */
static const char *const g_actions[THERON_TRACK02_ACTION_COUNT] = {
    "FREEZE LIFE",  /*  0 — stored as single entry in binary */
    "HIT",         /*  1 */
    "SWING",       /*  2 */
    "STAB",        /*  3 */
    "THRUST",      /*  4 */
    "JAB",         /*  5 */
    "PARRY",       /*  6 */
    "HACK",        /*  7 */
    "BERZERK",     /*  8 */
    "FIREBALL",    /*  9 */
    "DISPELL",     /* 10 */
    "CONFUSE",     /* 11 */
    "LIGHTNING",   /* 12 */
    "DISRUPT",     /* 13 */
    "MELEE",       /* 14 */
    "X",           /* 15 */
    "INVOKE",      /* 16 */
    "SLASH",       /* 17 */
    "CLEAVE",      /* 18 */
    "BASH",        /* 19 */
    "STUN",        /* 20 */
    "SHOOT",       /* 21 */
    "SPELLSHIELD", /* 22 */
    "FIRESHIELD",  /* 23 */
    "HEAL",        /* 24 */
    "CALM",        /* 25 */
    "LIGHT",       /* 26 */
    "SPIT",        /* 27 */
    "BRANDISH",    /* 28 */
    "THROW",       /* 29 */
};

const char *theron_v1_track02_us_class_name(unsigned int index) {
    if (index >= THERON_TRACK02_CLASS_COUNT) return NULL;
    return g_classes[index];
}

const char *theron_v1_track02_us_stat_name(unsigned int index) {
    if (index >= THERON_TRACK02_STAT_COUNT) return NULL;
    return g_stats[index];
}

const char *theron_v1_track02_us_resource_name(unsigned int index) {
    if (index >= THERON_TRACK02_RESOURCE_COUNT) return NULL;
    return g_resources[index];
}

const char *theron_v1_track02_us_skill_level_name(unsigned int index) {
    if (index >= THERON_TRACK02_SKILL_LEVEL_COUNT) return NULL;
    return g_skill_levels[index];
}

const char *theron_v1_track02_us_object_action_name(unsigned int index) {
    if (index >= THERON_TRACK02_OBJECT_ACTION_COUNT) return NULL;
    return g_object_actions[index];
}

const char *theron_v1_track02_us_hand_action_name(unsigned int index) {
    if (index >= THERON_TRACK02_HAND_ACTION_COUNT) return NULL;
    return g_hand_actions[index];
}

const char *theron_v1_track02_us_action_name(unsigned int index) {
    if (index >= THERON_TRACK02_ACTION_COUNT) return NULL;
    return g_actions[index];
}

/* UD 0x1C9AF1 */
const char *theron_v1_track02_us_cant_reach(void) { return "CAN'T REACH"; }

/* UD 0x1C9AFD */
const char *theron_v1_track02_us_need_ammo(void)  { return "NEED AMMO"; }

/* UD 0x1C9AE3 — Theron-unique coin flip mechanic */
const char *theron_v1_track02_us_heads(void) { return "HEADS."; }
const char *theron_v1_track02_us_tails(void) { return "TAILS."; }

/* UD 0x1C9A4E: 7 UI interaction messages */
static const char *const g_ui_messages[THERON_TRACK02_UI_MESSAGE_COUNT] = {
    " NEEDS MORE PRACTICE WITH THIS ",   /* 0 — UD 0x1C9A4E */
    " MUMBLES A MEANINGLESS SPELL.",     /* 1 — UD 0x1C9A6E */
    " NEEDS AN EMPTY FLASK IN HAND FOR POTION.", /* 2 — UD 0x1C9A8C */
    " SPELL.",                            /* 3 — UD 0x1C9AB6 */
    " JUST GAINED A ",                   /* 4 — UD 0x1C9ABE */
    " LEVEL!",                           /* 5 — UD 0x1C9ACE */
    "IT COMES UP ",                      /* 6 — UD 0x1C9AD6 */
};

const char *theron_v1_track02_us_ui_message(unsigned int index) {
    if (index >= THERON_TRACK02_UI_MESSAGE_COUNT) return NULL;
    return g_ui_messages[index];
}

/* UD 0x1CBBBC */
const char *theron_v1_track02_us_go_away_resurrect(void) {
    return "GO AWAY AND RESURRECT THERON";
}
