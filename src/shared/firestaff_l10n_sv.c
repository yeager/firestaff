
#include "firestaff_l10n.h"
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 * Swedish (sv) in-game text — all DM1/CSB/DM2 translated strings
 *
 * Source: ReDMCSB hardcoded strings (COMMAND.C, CHAMPION.C, PANEL.C)
 * These are the strings that were #ifdef'd per language at compile time
 * in the original. Firestaff selects them at runtime.
 *
 * Swedish was never an official DM1 language. This is a new translation
 * for Firestaff, following the style of the German/French originals.
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Base skill names (CHAMPION.C:55-64) ──────────────────────────── */
/* EN: FIGHTER, NINJA, PRIEST, WIZARD
 * DE: KAMPFER, NINJA, PRIESTER, MAGIER
 * FR: GUERRIER, NINJA, PRETRE, SORCIER */
static const char *g_sv_base_skill_names[4] = {
    "KRIGARE", "NINJA", "PRÄST", "TROLLKARL"
};

/* ── Skill level titles (PANEL.C:26-82) ───────────────────────────── */
/* EN: NEOPHYTE, NOVICE, APPRENTICE, JOURNEYMAN, CRAFTSMAN, ARTISAN,
 *     ADEPT, EXPERT, (LO/UM/ON/EE)MASTER, ARCHMASTER
 * DE: ANFANGER, NEULING, LEHRLING, ARBEITER, GESELLE, HANDWERKR,
 *     FACHMANN, EXPERTE, ...MEISTER, ERZMEISTR */
static const char *g_sv_skill_levels[16] = {
    "NYBÖRJARE",    /* NEOPHYTE */
    "NOVICE",       /* NOVICE */
    "LÄRLING",      /* APPRENTICE */
    "GESALL",       /* JOURNEYMAN */
    "HANTVERKARE",   /* CRAFTSMAN */
    "MÄSTARE",       /* ARTISAN (short for display) */
    "ADEPT",        /* ADEPT */
    "EXPERT",       /* EXPERT */
    "LO MÄSTARE",   /* LO MASTER */
    "UM MÄSTARE",   /* UM MASTER */
    "ON MÄSTARE",   /* ON MASTER */
    "EE MÄSTARE",   /* EE MASTER */
    "STORMÄSTARE",   /* ARCHMASTER */
    NULL, NULL, NULL
};

/* ── Direction names (PANEL.C:1100-1106) ──────────────────────────── */
/* EN: NORTH, EAST, SOUTH, WEST
 * DE: NORDEN, OSTEN, SUEDEN, WESTEN
 * FR: AU NORD, A L'EST, AU SUD, A L'OUEST */
static const char *g_sv_direction_names[4] = {
    "NORR", "ÖSTER", "SÖDER", "VÄSTER"
};

/* ── Item attribute strings (PANEL.C:1215-1260) ───────────────────── */
/* EN: CONSUMABLE, POISONED, BROKEN, CURSED
 * DE: ESSBAR, VERGIFTET, DEFEKT, VERFLUCHT
 * FR: COMESTIBLE, EMPOISONNE, BRISE, MAUDIT */
static const char *g_sv_item_attributes[4] = {
    "ÄTBAR", "FÖRGIFTAD", "TRASIG", "FÖRBANNAD"
};

/* ── Viewport text (COMMAND.C:2009-2399) ──────────────────────────── */
static const char *g_sv_viewport_text[] = {
    "VAKNA UPP",         /* WAKE UP / WECKEN / REVEILLEZ-VOUS */
    "SPELET FRUSET",     /* GAME FROZEN / SPIEL ANGEHALTEN / JEU BLOQUE */
    "VILA I FRID",       /* REST IN PEACE */
    "GRATTIS",           /* CONGRATULATIONS */
    "(UTBRAND)",         /* (BURNT OUT) / (AUSGEBRANNT) / (CONSUME) */
    "INGET MONSTER",     /* NO CREATURE */
    NULL
};

/* ── Food/water status ────────────────────────────────────────────── */
static const char *g_sv_food_water[] = {
    "HUNGRIG",    /* STARVING */
    "TÖRSTIG",    /* DEHYDRATED */
    "MAT",        /* FOOD */
    "VATTEN",     /* WATER */
    "FÖRGIFTAD",  /* POISONED */
    NULL
};

/* ── Champion actions (from GRAPHICS.DAT graphic #699) ────────────── */
/* These are bitmaps in original but we provide text equivalents for V2 */
static const char *g_sv_action_names[] = {
    "HUGG",       /* SWING / N */
    "STÖT",       /* THRUST / N */
    "SKJUT",       /* SHOOT / SCHIESSEN / TIRER */
    "KAST",       /* THROW / WERFEN / LANCER */
    "HUGGA",       /* CHOP / HACK / TAILLER */
    "SLAG",       /* BASH / SCHLAG / FRAPPER */
    "BLOCKERA",     /* BLOCK / BLOCK / BLOQUER */
    "SPAR",       /* JAB / STOSSEN / PIQUER */
    "SPARK",      /* KICK / TRITT / COUP DE PIED */
    "VRID",       /* WRENCH / DREHEN */
    "BRÄCK",       /* PRISE / HEBELN */
    "BESVÄRJELSE",      /* INCANTATION */
    "TROLLDOM",   /* DISPELL / BANNEN */
    "BLESS",      /* BLESS / SEGNEN */
    "FÖRBANNA",       /* CURSE / FLUCHEN */
    "HELA",       /* HEAL / HEILEN */
    "GIFT",       /* POISON / VERGIFTEN */
    "BLÅSA",      /* BLOW HORN / HORN BLASEN */
    "SPELA",      /* PLAY FLUTE */
    "LÅS UPP",        /* OPEN LOCK */
    "STÄNG",      /* CLOSE LOCK */
    NULL
};

/* ── Entrance hall / champion selection ────────────────────────────── */
static const char *g_sv_entrance[] = {
    "VÄLJ EN CHAMPION",      /* SELECT A CHAMPION */
    "TRYCK PÅ SPEGELN",      /* CLICK ON THE MIRROR */
    "ÅTERUPPVÄCK",           /* RESURRECT */
    "ÅTERFÖDD",               /* REINCARNATE */
    "FRYST",                 /* FROZEN */
    NULL
};

/* ── Save/load ────────────────────────────────────────────────────── */
static const char *g_sv_save_load[] = {
    "SPARA",          /* SAVE */
    "LADDA",          /* LOAD */
    "SPELET SPARAT",  /* GAME SAVED */
    "LADDAR...",      /* LOADING... */
    NULL
};

/* ── Public API ───────────────────────────────────────────────────── */

const char *fs_sv_base_skill_name(int index) {
    if (index < 0 || index > 3) return "???";
    return g_sv_base_skill_names[index];
}

const char *fs_sv_skill_level(int level) {
    if (level < 0 || level > 12) return "???";
    return g_sv_skill_levels[level];
}

const char *fs_sv_direction(int dir) {
    return g_sv_direction_names[dir & 3];
}

const char *fs_sv_item_attribute(int attr) {
    if (attr < 0 || attr > 3) return "???";
    return g_sv_item_attributes[attr];
}

const char *fs_sv_viewport_text(int id) {
    if (id < 0 || id > 5) return "???";
    return g_sv_viewport_text[id];
}

const char *fs_sv_action_name(int id) {
    if (id < 0 || id > 20 || !g_sv_action_names[id]) return "???";
    return g_sv_action_names[id];
}

const char *fs_sv_entrance(int id) {
    if (id < 0 || id > 4) return "???";
    return g_sv_entrance[id];
}
