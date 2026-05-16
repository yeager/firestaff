#include "spell_reference_m12.h"
#include <stddef.h>
#include <string.h>
#include <ctype.h>

/* ── DM1 Spell Database ──────────────────────────────────────────
 *
 * All 25 spells from the canonical DM1 spell table (MENU.C:50..77,
 * cross-referenced with memory_magic_pc34_compat Phase 14).
 *
 * Mana costs are base values at LO power. Actual cost scales with
 * power level via the SymbolManaCostMultiplier table:
 *   LO=8, UM=12, ON=16, EE=20, PAL=24, MON=28
 * Each rune in the sequence contributes its multiplier to the total.
 *
 * Skill names follow the original game's champion skill system:
 *   Fighter(0), Ninja(1), Priest(2), Wizard(3)
 *   and sub-skills indexed 13-19:
 *     13=Heal, 14=Influence, 15=Defend, 16=Fire, 17=Air,
 *     18=Earth, 19=Water
 *
 * Spell kinds from DEFS.H:
 *   1=Potion, 2=Projectile, 3=Other, 4=Magic Map (Zo Kath Ra only)
 */

static const M12_SpellReferenceEntry g_spellDatabase[] = {
    /* ── FIRE SCHOOL (Ful-rooted) ────────────────────────────────── */
    {
        "TORCH",
        M12_SPELL_SCHOOL_FIRE,
        M12_SPELL_KIND_OTHER,
        "FUL",
        1,              /* 1 rune */
        8,              /* base mana at LO */
        1,              /* base skill level */
        "FIRE",
        "CREATES A MAGICAL LIGHT SOURCE",
        "THE SIMPLEST SPELL IN THE DUNGEON. CONJURES\n"
        "A HOVERING LIGHT THAT ILLUMINATES YOUR PARTY.\n"
        "DURATION AND BRIGHTNESS SCALE WITH POWER LEVEL.\n"
        "ESSENTIAL FOR EARLY DUNGEON EXPLORATION BEFORE\n"
        "FINDING THE ILLUMULET OR OTHER LIGHT SOURCES.\n"
        "EVERY ASPIRING WIZARD LEARNS THIS FIRST.",
        -1
    },
    {
        "FIREBALL",
        M12_SPELL_SCHOOL_FIRE,
        M12_SPELL_KIND_PROJECTILE,
        "FUL IR",
        2,              /* 2 runes */
        20,             /* base mana */
        3,              /* base skill level */
        "FIRE",
        "HURLS AN EXPLOSIVE BALL OF FIRE",
        "THE QUINTESSENTIAL COMBAT SPELL. LAUNCHES A\n"
        "BLAZING PROJECTILE THAT EXPLODES ON IMPACT.\n"
        "DAMAGE SCALES STRONGLY WITH POWER LEVEL AND\n"
        "FIRE SKILL. DEVASTATING AGAINST MUMMIES AND\n"
        "OTHER FIRE-VULNERABLE CREATURES. BEWARE THE\n"
        "MANA COST AT HIGHER POWER LEVELS.",
        -1
    },
    {
        "STRENGTH POTION",
        M12_SPELL_SCHOOL_FIRE,
        M12_SPELL_KIND_POTION,
        "FUL BRO KU",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "HEAL",
        "BREWS A POTION OF INCREASED STRENGTH",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING POTION TEMPORARILY BOOSTS THE\n"
        "DRINKER'S STRENGTH STAT. HIGHER POWER LEVELS\n"
        "PRODUCE STRONGER AND LONGER-LASTING EFFECTS.\n"
        "INVALUABLE BEFORE FACING STONE GOLEMS OR THE\n"
        "RED DRAGON.",
        -1
    },
    {
        "FIRE SHIELD",
        M12_SPELL_SCHOOL_FIRE,
        M12_SPELL_KIND_OTHER,
        "FUL BRO NETA",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "DEFEND",
        "SURROUNDS THE PARTY WITH A FIRE SHIELD",
        "CREATES A PROTECTIVE AURA OF FLAME AROUND THE\n"
        "PARTY. REDUCES FIRE DAMAGE FROM DRAGON BREATH\n"
        "AND FIRE-BASED ATTACKS. DURATION AND PROTECTION\n"
        "SCALE WITH POWER LEVEL. ESSENTIAL PREPARATION\n"
        "FOR THE DEEP DUNGEON WHERE VEXIRKS AND THE RED\n"
        "DRAGON AWAIT.",
        -1
    },
    /* ── AIR SCHOOL (Oh-rooted) ──────────────────────────────────── */
    {
        "LIGHTNING BOLT",
        M12_SPELL_SCHOOL_AIR,
        M12_SPELL_KIND_PROJECTILE,
        "OH KATH RA",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "AIR",
        "LAUNCHES A BOLT OF LIGHTNING",
        "A POWERFUL PROJECTILE SPELL THAT HURLS AN\n"
        "ELECTRICAL DISCHARGE AT THE TARGET. ESPECIALLY\n"
        "EFFECTIVE AGAINST WATER ELEMENTALS AND ANIMATED\n"
        "ARMOUR. THE BOLT TRAVELS INSTANTLY AND CAN\n"
        "STRIKE DISTANT TARGETS IN LONG CORRIDORS.\n"
        "ONE OF THE STRONGEST COMBAT SPELLS AVAILABLE.",
        -1
    },
    {
        "LIGHT",
        M12_SPELL_SCHOOL_AIR,
        M12_SPELL_KIND_OTHER,
        "OH IR RA",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "AIR",
        "CREATES A POWERFUL LASTING LIGHT",
        "AN ADVANCED VERSION OF THE TORCH SPELL.\n"
        "PRODUCES A BRIGHTER AND LONGER-LASTING MAGICAL\n"
        "ILLUMINATION. AT HIGH POWER LEVELS THE LIGHT\n"
        "CAN LAST FOR EXTENDED DUNGEON EXPLORATION\n"
        "SESSIONS. PREFERRED BY EXPERIENCED PARTIES\n"
        "OVER THE BASIC FUL TORCH.",
        -1
    },
    {
        "INVISIBILITY",
        M12_SPELL_SCHOOL_AIR,
        M12_SPELL_KIND_OTHER,
        "OH EW SAR",
        3,              /* 3 runes */
        36,             /* base mana */
        3,              /* base skill level */
        "AIR",
        "RENDERS THE PARTY INVISIBLE",
        "BENDS LIGHT AROUND THE PARTY, MAKING THEM\n"
        "UNSEEN BY MOST CREATURES. DURATION SCALES\n"
        "WITH POWER LEVEL AND AIR SKILL. BROKEN BY\n"
        "ATTACKING OR CASTING OTHER SPELLS. USEFUL\n"
        "FOR BYPASSING DANGEROUS ENCOUNTERS OR\n"
        "POSITIONING FOR A SURPRISE ATTACK.",
        -1
    },
    {
        "THIEVES EYE",
        M12_SPELL_SCHOOL_AIR,
        M12_SPELL_KIND_OTHER,
        "OH EW RA",
        3,              /* 3 runes */
        36,             /* base mana */
        3,              /* base skill level */
        "EARTH",
        "REVEALS HIDDEN DETAILS AND TRAPS",
        "GRANTS THE PARTY ENHANCED PERCEPTION FOR A\n"
        "LIMITED TIME. SHOWS HIDDEN PRESSURE PLATES,\n"
        "SECRET SWITCHES, AND OTHER CONCEALED DUNGEON\n"
        "FEATURES. INVALUABLE ON PUZZLE-HEAVY LEVELS.\n"
        "DURATION INCREASES WITH POWER LEVEL.",
        -1
    },
    {
        "POISON CLOUD",
        M12_SPELL_SCHOOL_AIR,
        M12_SPELL_KIND_PROJECTILE,
        "OH VEN",
        2,              /* 2 runes */
        20,             /* base mana */
        3,              /* base skill level */
        "WATER",
        "LAUNCHES A CLOUD OF POISONOUS GAS",
        "FIRES A TOXIC PROJECTILE THAT POISONS CREATURES\n"
        "ON IMPACT. THE POISON DEALS DAMAGE OVER TIME,\n"
        "WEAKENING ENEMIES GRADUALLY. LESS IMMEDIATE\n"
        "DAMAGE THAN FIREBALL BUT THE ONGOING EFFECT\n"
        "CAN BE MORE EFFICIENT AGAINST TOUGH TARGETS\n"
        "WITH HIGH HIT POINTS.",
        -1
    },
    {
        "DEXTERITY POTION",
        M12_SPELL_SCHOOL_AIR,
        M12_SPELL_KIND_POTION,
        "OH BRO ROS",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "HEAL",
        "BREWS A POTION OF INCREASED DEXTERITY",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING POTION TEMPORARILY ENHANCES THE\n"
        "DRINKER'S DEXTERITY, IMPROVING DODGE CHANCE\n"
        "AND ATTACK ACCURACY. PARTICULARLY USEFUL FOR\n"
        "NINJA-CLASS CHAMPIONS WHO RELY ON AGILITY.\n"
        "EFFECT DURATION SCALES WITH POWER LEVEL.",
        -1
    },
    /* ── EARTH SCHOOL (Des-rooted) ───────────────────────────────── */
    {
        "WEAKEN NON-MATERIAL",
        M12_SPELL_SCHOOL_EARTH,
        M12_SPELL_KIND_OTHER,
        "DES EW",
        2,              /* 2 runes */
        20,             /* base mana */
        1,              /* base skill level */
        "EARTH",
        "HARMS NON-MATERIAL BEINGS",
        "A SPECIALISED SPELL TARGETING GHOSTS, SPECTRES,\n"
        "AND OTHER INCORPOREAL ENTITIES. PHYSICAL WEAPONS\n"
        "PASS THROUGH SUCH CREATURES, BUT THIS SPELL\n"
        "DISRUPTS THEIR ESSENCE DIRECTLY. ESSENTIAL FOR\n"
        "CLEARING THE GHOST-INFESTED LEVELS. DAMAGE\n"
        "SCALES WITH EARTH SKILL AND POWER LEVEL.",
        -1
    },
    {
        "POISON BOLT",
        M12_SPELL_SCHOOL_EARTH,
        M12_SPELL_KIND_PROJECTILE,
        "DES VEN",
        2,              /* 2 runes */
        20,             /* base mana */
        1,              /* base skill level */
        "WATER",
        "FIRES A CONCENTRATED BOLT OF POISON",
        "A FOCUSED POISON PROJECTILE THAT STRIKES A\n"
        "SINGLE TARGET WITH VENOMOUS FORCE. LESS AREA\n"
        "COVERAGE THAN THE POISON CLOUD BUT MORE DIRECT\n"
        "DAMAGE. USEFUL AS A LOW-COST COMBAT SPELL FOR\n"
        "WATER-SKILLED CHAMPIONS. THE ONGOING POISON\n"
        "EFFECT STACKS WITH OTHER SOURCES.",
        -1
    },
    {
        "DARKNESS",
        M12_SPELL_SCHOOL_EARTH,
        M12_SPELL_KIND_OTHER,
        "DES IR SAR",
        3,              /* 3 runes */
        36,             /* base mana */
        1,              /* base skill level */
        "DEFEND",
        "PLUNGES THE AREA INTO MAGICAL DARKNESS",
        "EXTINGUISHES ALL LIGHT SOURCES IN THE VICINITY,\n"
        "INCLUDING MAGICAL ONES. CAN BE USED TACTICALLY\n"
        "TO CONFUSE MONSTERS THAT RELY ON SIGHT, THOUGH\n"
        "YOUR PARTY WILL ALSO BE BLINDED. COUNTERS THE\n"
        "LIGHT AND TORCH SPELLS. DURATION SCALES WITH\n"
        "POWER LEVEL.",
        -1
    },
    /* ── WATER SCHOOL (Zo-rooted) ────────────────────────────────── */
    {
        "OPEN DOOR",
        M12_SPELL_SCHOOL_WATER,
        M12_SPELL_KIND_OTHER,
        "ZO",
        1,              /* 1 rune */
        8,              /* base mana */
        1,              /* base skill level */
        "AIR",
        "MAGICALLY OPENS A DOOR",
        "ATTEMPTS TO OPEN A DOOR IN FRONT OF THE PARTY\n"
        "USING MAGICAL FORCE. SUCCESS DEPENDS ON THE\n"
        "CASTER'S SKILL AND THE DOOR'S RESISTANCE.\n"
        "SOME DOORS ARE MAGICALLY SEALED AND REQUIRE\n"
        "HIGHER POWER LEVELS OR SPECIFIC KEYS INSTEAD.\n"
        "ONE OF THE MOST FREQUENTLY CAST SPELLS.",
        -1
    },
    {
        "ZOKATHRA",
        M12_SPELL_SCHOOL_WATER,
        M12_SPELL_KIND_MAGIC_MAP,
        "ZO KATH RA",
        3,              /* 3 runes */
        36,             /* base mana */
        0,              /* base skill level — accessible early */
        "PRIEST",
        "CREATES THE FABLED ZO KATH RA MATERIAL",
        "THE MOST MYSTERIOUS SPELL IN THE DUNGEON.\n"
        "CREATES AN ENCHANTED MATERIAL THAT FUSES WITH\n"
        "THE FIRESTAFF TO FORM THE ULTIMATE WEAPON\n"
        "AGAINST LORD CHAOS. THIS IS THE KEY TO THE\n"
        "ENDGAME. WITHOUT IT, THE FIRESTAFF CANNOT\n"
        "CONTAIN THE POWER OF THE GREY LORD'S CREATION.",
        -1
    },
    {
        "MANA POTION",
        M12_SPELL_SCHOOL_WATER,
        M12_SPELL_KIND_POTION,
        "ZO BRO RA",
        3,              /* 3 runes */
        36,             /* base mana */
        3,              /* base skill level */
        "PRIEST",
        "BREWS A POTION THAT RESTORES MANA",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING BLUE ELIXIR REPLENISHES MAGICAL\n"
        "MANA RESERVES WHEN CONSUMED. PARADOXICALLY\n"
        "REQUIRES MANA TO CREATE, BUT AT HIGH SKILL THE\n"
        "RETURN EXCEEDS THE INVESTMENT. CRITICAL FOR\n"
        "SUSTAINED SPELL-CASTING IN LONG DUNGEON RUNS.",
        -1
    },
    {
        "POISON POTION",
        M12_SPELL_SCHOOL_WATER,
        M12_SPELL_KIND_POTION,
        "ZO VEN",
        2,              /* 2 runes */
        20,             /* base mana */
        2,              /* base skill level */
        "WATER",
        "BREWS A POTION OF POISON",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "CREATES A VIAL OF CONCENTRATED POISON. CAN BE\n"
        "THROWN AT ENEMIES AS A MAKESHIFT WEAPON OR USED\n"
        "IN CREATIVE PUZZLE SOLUTIONS. HANDLE WITH CARE\n"
        "— DRINKING IT YOURSELF WOULD BE DECIDEDLY\n"
        "UNWISE.",
        -1
    },
    /* ── BODY SCHOOL (Ya-rooted) ─────────────────────────────────── */
    {
        "STAMINA POTION",
        M12_SPELL_SCHOOL_BODY,
        M12_SPELL_KIND_POTION,
        "YA",
        1,              /* 1 rune */
        8,              /* base mana */
        2,              /* base skill level */
        "HEAL",
        "BREWS A STAMINA RESTORATION POTION",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING POTION RESTORES STAMINA WHEN\n"
        "CONSUMED. STAMINA GOVERNS PHYSICAL ACTIONS —\n"
        "WITHOUT IT, CHAMPIONS CANNOT SWING WEAPONS OR\n"
        "RUN. A RELIABLE SUPPLY OF THESE KEEPS YOUR\n"
        "PARTY FIGHTING FIT ON LONG EXPEDITIONS.",
        -1
    },
    {
        "SHIELD POTION",
        M12_SPELL_SCHOOL_BODY,
        M12_SPELL_KIND_POTION,
        "YA BRO",
        2,              /* 2 runes */
        20,             /* base mana */
        2,              /* base skill level */
        "DEFEND",
        "BREWS A DEFENSIVE SHIELD POTION",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING POTION TEMPORARILY ENHANCES THE\n"
        "DRINKER'S DEFENSIVE CAPABILITIES, REDUCING ALL\n"
        "INCOMING DAMAGE. EFFECT STACKS WITH WORN ARMOUR.\n"
        "STRONGER THAN THE PARTY SHIELD SPELL FOR A\n"
        "SINGLE CHAMPION BUT ONLY AFFECTS THE DRINKER.",
        -1
    },
    {
        "PARTY SHIELD",
        M12_SPELL_SCHOOL_BODY,
        M12_SPELL_KIND_OTHER,
        "YA IR",
        2,              /* 2 runes */
        20,             /* base mana */
        2,              /* base skill level */
        "DEFEND",
        "RAISES A MAGICAL SHIELD AROUND THE PARTY",
        "CREATES A PROTECTIVE BARRIER THAT REDUCES\n"
        "INCOMING DAMAGE FOR ALL PARTY MEMBERS. LESS\n"
        "POTENT PER-CHAMPION THAN THE SHIELD POTION BUT\n"
        "COVERS EVERYONE SIMULTANEOUSLY. DURATION AND\n"
        "STRENGTH SCALE WITH POWER LEVEL AND DEFEND\n"
        "SKILL. A STAPLE OF DEFENSIVE SPELL-CASTING.",
        -1
    },
    {
        "MAGIC FOOTPRINTS",
        M12_SPELL_SCHOOL_BODY,
        M12_SPELL_KIND_OTHER,
        "YA BRO ROS",
        3,              /* 3 runes */
        36,             /* base mana */
        1,              /* base skill level */
        "EARTH",
        "LEAVES VISIBLE FOOTPRINTS AS YOU WALK",
        "ENCHANTS THE FLOOR TO SHOW YOUR PARTY'S PATH\n"
        "AS GLOWING FOOTPRINTS. INVALUABLE FOR MAPPING\n"
        "COMPLEX MAZES AND AVOIDING WALKING IN CIRCLES.\n"
        "DURATION SCALES WITH POWER LEVEL. ESPECIALLY\n"
        "USEFUL IN THE LABYRINTHINE LOWER DUNGEON LEVELS\n"
        "WHERE ORIENTATION IS EASILY LOST.",
        -1
    },
    {
        "WISDOM POTION",
        M12_SPELL_SCHOOL_BODY,
        M12_SPELL_KIND_POTION,
        "YA BRO DAIN",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "HEAL",
        "BREWS A POTION OF INCREASED WISDOM",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING POTION TEMPORARILY BOOSTS THE\n"
        "DRINKER'S WISDOM STAT, IMPROVING SPELL SUCCESS\n"
        "RATE AND MANA REGENERATION. MOST BENEFICIAL\n"
        "FOR WIZARD AND PRIEST CHAMPIONS WHO RELY\n"
        "HEAVILY ON MAGICAL ABILITIES.",
        -1
    },
    {
        "VITALITY POTION",
        M12_SPELL_SCHOOL_BODY,
        M12_SPELL_KIND_POTION,
        "YA BRO NETA",
        3,              /* 3 runes */
        36,             /* base mana */
        4,              /* base skill level */
        "HEAL",
        "BREWS A POTION OF INCREASED VITALITY",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING POTION TEMPORARILY ENHANCES THE\n"
        "DRINKER'S VITALITY, INCREASING MAXIMUM HEALTH\n"
        "AND RESISTANCE TO POISON AND DISEASE. STACKS\n"
        "WITH NATURAL VITALITY. EXCELLENT PREPARATION\n"
        "BEFORE ENTERING POISONOUS DUNGEON ZONES.",
        -1
    },
    /* ── MIND SCHOOL (Vi-rooted) ─────────────────────────────────── */
    {
        "HEALTH POTION",
        M12_SPELL_SCHOOL_MIND,
        M12_SPELL_KIND_POTION,
        "VI",
        1,              /* 1 rune */
        8,              /* base mana */
        1,              /* base skill level */
        "HEAL",
        "BREWS A POTION THAT RESTORES HEALTH",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE MOST FUNDAMENTAL HEALING SPELL. THE GREEN\n"
        "POTION RESTORES HIT POINTS WHEN CONSUMED. AT\n"
        "LOW SKILL THE HEALING IS MODEST, BUT HIGH-LEVEL\n"
        "CASTERS PRODUCE POTENT ELIXIRS. ALWAYS KEEP A\n"
        "STOCK — DUNGEON COMBAT IS UNFORGIVING.",
        -1
    },
    {
        "CURE POISON",
        M12_SPELL_SCHOOL_MIND,
        M12_SPELL_KIND_POTION,
        "VI BRO",
        2,              /* 2 runes */
        20,             /* base mana */
        1,              /* base skill level */
        "HEAL",
        "BREWS A POTION THAT CURES POISON",
        "REQUIRES AN EMPTY FLASK IN THE CASTER'S HAND.\n"
        "THE RESULTING ANTIDOTE NEUTRALISES POISON\n"
        "AFFLICTING THE DRINKER. CRITICAL AFTER GIANT\n"
        "SCORPION ENCOUNTERS AND IN THE TOXIC SWAMP\n"
        "LEVELS. KEEP SEVERAL ON HAND — POISON KILLS\n"
        "SLOWLY BUT INEXORABLY WITHOUT TREATMENT.",
        -1
    },
};

#define G_SPELL_DATABASE_COUNT \
    ((int)(sizeof(g_spellDatabase) / sizeof(g_spellDatabase[0])))

/* ── School display names ────────────────────────────────────────── */
static const char* const g_schoolNames[M12_SPELL_SCHOOL_COUNT] = {
    "ALL",
    "FIRE",
    "AIR",
    "EARTH",
    "WATER",
    "BODY",
    "MIND"
};

/* ── Case-insensitive substring search ───────────────────────────── */
static int m12_spell_strcasestr(const char* haystack, const char* needle) {
    int hLen, nLen, i, j;
    if (!needle || !needle[0]) return 1;
    if (!haystack) return 0;
    hLen = (int)strlen(haystack);
    nLen = (int)strlen(needle);
    if (nLen > hLen) return 0;
    for (i = 0; i <= hLen - nLen; i++) {
        for (j = 0; j < nLen; j++) {
            if (toupper((unsigned char)haystack[i + j]) !=
                toupper((unsigned char)needle[j])) {
                break;
            }
        }
        if (j == nLen) return 1;
    }
    return 0;
}

/* ── Check if a spell matches current filter + search ────────────── */
static int m12_spell_matches(const M12_SpellReferenceEntry* spell,
                              M12_SpellSchool schoolFilter,
                              const char* searchQuery) {
    if (schoolFilter != M12_SPELL_SCHOOL_ALL &&
        spell->school != schoolFilter) {
        return 0;
    }
    if (searchQuery && searchQuery[0]) {
        /* Search matches against name OR rune sequence */
        if (!m12_spell_strcasestr(spell->name, searchQuery) &&
            !m12_spell_strcasestr(spell->runeSequence, searchQuery)) {
            return 0;
        }
    }
    return 1;
}

/* ── Recount filtered spells ─────────────────────────────────────── */
static int m12_spell_recount(M12_SpellSchool schoolFilter,
                              const char* searchQuery) {
    int count = 0;
    int i;
    for (i = 0; i < G_SPELL_DATABASE_COUNT; i++) {
        if (m12_spell_matches(&g_spellDatabase[i], schoolFilter,
                               searchQuery)) {
            count++;
        }
    }
    return count;
}

/* ── Get nth matching spell ──────────────────────────────────────── */
static const M12_SpellReferenceEntry* m12_spell_get_nth(
        M12_SpellSchool schoolFilter,
        const char* searchQuery,
        int n) {
    int count = 0;
    int i;
    for (i = 0; i < G_SPELL_DATABASE_COUNT; i++) {
        if (m12_spell_matches(&g_spellDatabase[i], schoolFilter,
                               searchQuery)) {
            if (count == n) return &g_spellDatabase[i];
            count++;
        }
    }
    return NULL;
}

/* ── Public API ──────────────────────────────────────────────────── */

void M12_SpellReference_Init(M12_SpellReferenceState* sr) {
    if (!sr) return;
    sr->scrollOffset = 0;
    sr->selectedIndex = 0;
    sr->schoolFilter = M12_SPELL_SCHOOL_ALL;
    sr->filteredCount = G_SPELL_DATABASE_COUNT;
    memset(sr->searchQuery, 0, M12_SPELL_SEARCH_MAX);
    sr->searchLen = 0;
}

int M12_SpellReference_TotalCount(void) {
    return G_SPELL_DATABASE_COUNT;
}

int M12_SpellReference_FilteredCount(const M12_SpellReferenceState* sr) {
    if (!sr) return 0;
    return sr->filteredCount;
}

const M12_SpellReferenceEntry* M12_SpellReference_GetFiltered(
        const M12_SpellReferenceState* sr, int index) {
    if (!sr || index < 0 || index >= sr->filteredCount) return NULL;
    return m12_spell_get_nth(sr->schoolFilter, sr->searchQuery, index);
}

const M12_SpellReferenceEntry* M12_SpellReference_GetSelected(
        const M12_SpellReferenceState* sr) {
    if (!sr) return NULL;
    return M12_SpellReference_GetFiltered(sr, sr->selectedIndex);
}

void M12_SpellReference_Scroll(M12_SpellReferenceState* sr, int delta) {
    int maxOffset;
    if (!sr) return;
    sr->selectedIndex += delta;
    if (sr->selectedIndex < 0) {
        sr->selectedIndex = 0;
    }
    if (sr->selectedIndex >= sr->filteredCount) {
        sr->selectedIndex = sr->filteredCount > 0
                          ? sr->filteredCount - 1
                          : 0;
    }
    /* Adjust scroll window to keep selection visible */
    if (sr->selectedIndex < sr->scrollOffset) {
        sr->scrollOffset = sr->selectedIndex;
    }
    if (sr->selectedIndex >= sr->scrollOffset + M12_SPELL_VISIBLE_LINES) {
        sr->scrollOffset = sr->selectedIndex - M12_SPELL_VISIBLE_LINES + 1;
    }
    maxOffset = sr->filteredCount - M12_SPELL_VISIBLE_LINES;
    if (maxOffset < 0) maxOffset = 0;
    if (sr->scrollOffset > maxOffset) {
        sr->scrollOffset = maxOffset;
    }
    if (sr->scrollOffset < 0) {
        sr->scrollOffset = 0;
    }
}

void M12_SpellReference_CycleSchool(M12_SpellReferenceState* sr,
                                     int direction) {
    int school;
    if (!sr) return;
    school = (int)sr->schoolFilter + direction;
    if (school < 0) school = M12_SPELL_SCHOOL_COUNT - 1;
    if (school >= M12_SPELL_SCHOOL_COUNT) school = 0;
    sr->schoolFilter = (M12_SpellSchool)school;
    sr->scrollOffset = 0;
    sr->selectedIndex = 0;
    sr->filteredCount = m12_spell_recount(sr->schoolFilter,
                                           sr->searchQuery);
}

const char* M12_SpellReference_SchoolName(M12_SpellSchool school) {
    if (school < 0 || school >= M12_SPELL_SCHOOL_COUNT) {
        return "UNKNOWN";
    }
    return g_schoolNames[school];
}

const char* M12_SpellReference_KindName(M12_SpellKind kind) {
    switch (kind) {
        case M12_SPELL_KIND_POTION:     return "POTION";
        case M12_SPELL_KIND_PROJECTILE: return "PROJECTILE";
        case M12_SPELL_KIND_OTHER:      return "OTHER";
        case M12_SPELL_KIND_MAGIC_MAP:  return "MAGIC MAP";
        default:                        return "UNKNOWN";
    }
}

void M12_SpellReference_SetSearch(M12_SpellReferenceState* sr,
                                   const char* query) {
    if (!sr) return;
    if (!query || !query[0]) {
        memset(sr->searchQuery, 0, M12_SPELL_SEARCH_MAX);
        sr->searchLen = 0;
    } else {
        int len = (int)strlen(query);
        if (len >= M12_SPELL_SEARCH_MAX) len = M12_SPELL_SEARCH_MAX - 1;
        memcpy(sr->searchQuery, query, (size_t)len);
        sr->searchQuery[len] = '\0';
        sr->searchLen = len;
    }
    sr->scrollOffset = 0;
    sr->selectedIndex = 0;
    sr->filteredCount = m12_spell_recount(sr->schoolFilter,
                                           sr->searchQuery);
}

void M12_SpellReference_SearchAppend(M12_SpellReferenceState* sr,
                                      char ch) {
    if (!sr) return;
    if (sr->searchLen >= M12_SPELL_SEARCH_MAX - 1) return;
    sr->searchQuery[sr->searchLen] = ch;
    sr->searchLen++;
    sr->searchQuery[sr->searchLen] = '\0';
    sr->scrollOffset = 0;
    sr->selectedIndex = 0;
    sr->filteredCount = m12_spell_recount(sr->schoolFilter,
                                           sr->searchQuery);
}

void M12_SpellReference_SearchBackspace(M12_SpellReferenceState* sr) {
    if (!sr || sr->searchLen <= 0) return;
    sr->searchLen--;
    sr->searchQuery[sr->searchLen] = '\0';
    sr->scrollOffset = 0;
    sr->selectedIndex = 0;
    sr->filteredCount = m12_spell_recount(sr->schoolFilter,
                                           sr->searchQuery);
}

void M12_SpellReference_SearchClear(M12_SpellReferenceState* sr) {
    if (!sr) return;
    memset(sr->searchQuery, 0, M12_SPELL_SEARCH_MAX);
    sr->searchLen = 0;
    sr->scrollOffset = 0;
    sr->selectedIndex = 0;
    sr->filteredCount = m12_spell_recount(sr->schoolFilter,
                                           sr->searchQuery);
}
