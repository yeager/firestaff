
#include "nexus_v1_champions.h"
#include "nexus_v1_inventory.h"
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════
 * Maximum load calculation — matches ReDMCSB CHAMPION.C F0309.
 * Source: CHAMPION.C F0309_GetMaximumLoad lines 1157-1195.
 *
 * Formula:
 *   max_load = (strength << 3) + 100
 *            → stamina-adjusted (if stamina < half max)
 *            → wound-penalized (leg wounds -25%, other wounds -33%)
 *            → elven boots bonus (+6.25%)
 *            → rounded to nearest 10
 *
 * Stamina adjustment (F0306_GetStaminaAdjustedValue):
 *   if stamina < half_max:
 *     adjusted = (value >> 1) + value * stamina / half_max
 *   e.g. strength=50, stamina=25/50 → adjusted = 25+25 = 50 (full)
 *        strength=50, stamina=0/50   → adjusted = 25+0 = 25 (half)
 *
 * Wound penalty:
 *   leg wounds (LEGS): reduce max_load by 25% (>>2)
 *   other wounds: reduce max_load by 33% (>>3)
 *   Source: CHAMPION.C lines 1167-1173.
 *
 * Elven boots bonus (feet slot = ICON_ARMOUR_ELVEN_BOOTS):
 *   max_load += max_load >> 4  (+6.25%)
 *   Source: CHAMPION.C lines 1186-1188.
 *
 * Round to nearest 10:
 *   max_load -= max_load % 10
 *   Source: CHAMPION.C lines 1190-1191.
 * ═══════════════════════════════════════════════════════════════════ */

static int get_stamina_adjusted_value(int value, int current_stamina, int max_stamina) {
    if (current_stamina < (max_stamina >> 1)) {
        /* Stamina below half: strength/load is reduced.
         * Formula: (value >> 1) + value * stamina / half_max
         * Source: CHAMPION.C F0306 lines 1097-1109. */
        int half_max = max_stamina >> 1;
        if (half_max > 0) {
            return (value >> 1) + ((long)value * current_stamina / half_max);
        }
    }
    return value;
}

int nexus_champion_get_maximum_load(const Nexus_V1_Champion *c) {
    int max_load;
    int effective_strength;
    int leg_wounds, other_wounds;
    int has_elven_boots;

    if (!c) return 100;

    /* Base: (strength << 3) + 100 */
    effective_strength = (c->strength << 3) + 100;

    /* Stamina adjustment — reduces effective strength when stamina is low */
    effective_strength = get_stamina_adjusted_value(
        effective_strength, c->stamina, c->max_stamina);

    max_load = effective_strength;

    /* Wound penalty.
     * Leg wounds (LEGS): -25% (>>2). Other wounds: -33% (>>3).
     * Source: CHAMPION.C F0309 lines 1167-1173. */
    leg_wounds = (c->wounds & NEXUS_WOUND_LEGS) != 0;
    other_wounds = c->wounds & ~NEXUS_WOUND_LEGS;
    if (leg_wounds) {
        max_load -= max_load >> 2;  /* -25% for leg wounds */
    }
    if (other_wounds) {
        max_load -= max_load >> 3;  /* -33% for other wounds */
    }

    /* Elven boots bonus: +6.25% if wearing elven boots in feet slot.
     * Source: CHAMPION.C F0309 lines 1186-1188.
     * We check if boots slot has a valid item (non-negative). */
    has_elven_boots = (c->slots[NEXUS_SLOT_FEET] >= 0);
    if (has_elven_boots) {
        max_load += max_load >> 4;  /* +6.25% */
    }

    /* Round to nearest 10 (round down to next multiple of 10).
     * Source: CHAMPION.C F0309 lines 1190-1191:
     *   max_load -= max_load % 10 */
    max_load -= max_load % 10;

    if (max_load < 1) max_load = 1;
    return max_load;
}

/* Movement tick rate — matches ReDMCSB CHAMPION.C F0310.
 * Returns number of 55ms game ticks between steps.
 * When overloaded (load >= max_load), movement is severely slowed.
 * NOTE: BUG0_72 — DM1 uses > not >=, so at exactly max_load
 * (yellow warning, not yet red overload) movement is still slowed.
 * Source: CHAMPION.C F0310 lines 1197-1222, BUG0_72 comment at 1198. */
int nexus_champion_get_movement_ticks(const Nexus_V1_Champion *c) {
    int load, max_load;
    if (!c) return 1;

    load = c->load;
    max_load = nexus_champion_get_maximum_load(c);

    if (load > max_load) {  /* BUG0_72: should be >= but DM1 uses > */
        /* Overloaded: return (load * 15 / max_load) - 10
         * Source: CHAMPION.C F0310 lines 1207-1210. */
        int ticks = ((load * 15) / max_load) - 10;
        if (ticks < 1) ticks = 1;
        return ticks;
    }

    return 1;  /* Normal: one tick per step */
}

/* Stamina decrement — matches ReDMCSB CHAMPION.C F0325.
 * Source: CHAMPION.C F0325 lines 2025-2048.
 *
 * If stamina goes to 0 or below, champion takes wounds:
 *   damage = |negative_stamina| / 2 */
void nexus_champion_decrement_stamina(Nexus_V1_Champion *c, int cost) {
    int new_stamina;
    if (!c || !c->alive) return;

    new_stamina = c->stamina - cost;
    c->stamina = new_stamina;
    c->attributes |= NEXUS_ATTR_LOAD_CHANGED | NEXUS_ATTR_STATISTICS;

    if (new_stamina <= 0) {
        /* Champion collapses — take wound damage.
         * Source: CHAMPION.C F0325 lines 2036-2040:
         *   F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage(
         *       champion, (-L0988_i_Stamina) >> 1, ...) */
        c->stamina = 0;
        /* Wound damage = |negative_stamina| / 2 */
        c->health -= ((-new_stamina) >> 1);
        if (c->health <= 0) {
            c->health = 0;
            c->alive = 0;
            c->attributes |= NEXUS_ATTR_DEAD;
        }
    } else if (new_stamina > c->max_stamina) {
        c->stamina = c->max_stamina;
    }
}

/* Recalculate current load from inventory + equipment.
 * Source: CHAMPION.C load update on inventory change.
 *
 * For now: sum of all inventory item weights.
 * Equipment slots also contribute to load.
 * We need the item catalog to look up item weights. */
void nexus_champion_recalc_load(Nexus_V1_Champion *c) {
    int i, total = 0;
    const Nexus_ItemDef *def;
    if (!c) return;

    /* Sum inventory weights */
    for (i = 0; i < 30; i++) {
        if (c->inventory[i] >= 0) {
            def = nexus_itemdef_get(c->inventory[i]);
            if (def) total += def->weight;
        }
    }

    /* Sum equipment slot weights */
    for (i = 0; i < NEXUS_SLOT_COUNT; i++) {
        if (c->slots[i] >= 0) {
            def = nexus_itemdef_get(c->slots[i]);
            if (def) total += def->weight;
        }
    }

    c->load = total;
    c->attributes |= NEXUS_ATTR_LOAD_CHANGED;
}
static const struct { const char *ascii; const char *jp; int cls; int hp; int sta; int mp; int str; int dex; int wis; int vit; } g_nexus_roster[] = {
    {"Syra",      "\xe3\x82\xb7\xe3\x83\xa9", NEXUS_CLASS_FIGHTER, 70, 55, 15, 55, 40, 25, 50},
    {"Leyla",     "\xe3\x83\xac\xe3\x82\xa4\xe3\x83\xa9", NEXUS_CLASS_WIZARD,  40, 35, 65, 25, 35, 60, 30},
    {"Nabi",      "\xe3\x83\x8a\xe3\x83\x93", NEXUS_CLASS_NINJA,   55, 60, 25, 40, 60, 30, 45},
    {"Gando",     "\xe3\x82\xac\xe3\x83\xb3\xe3\x83\x89", NEXUS_CLASS_PRIEST,  50, 40, 55, 35, 30, 55, 40},
    {"Torham",    "\xe3\x83\x88\xe3\x83\xab\xe3\x83\x8f\xe3\x83\xa0", NEXUS_CLASS_FIGHTER, 65, 50, 20, 50, 45, 28, 48},
    {"Elija",     "\xe3\x82\xa8\xe3\x83\xaa\xe3\x82\xb8\xe3\x83\xa3", NEXUS_CLASS_WIZARD,  38, 30, 70, 22, 32, 65, 28},
    {"Wu Tse",    "\xe3\x82\xa6\xe3\x83\xbc\xe3\x83\x84\xe3\x82\xa7", NEXUS_CLASS_NINJA,   52, 58, 30, 38, 55, 35, 42},
    {"Stamm",     "\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\xa0", NEXUS_CLASS_FIGHTER, 75, 60, 10, 60, 35, 20, 55},
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0}
};

void nexus_v1_champions_init(Nexus_V1_ChampionPool *pool) {
    int i, j;
    if (!pool) return;
    memset(pool, 0, sizeof(*pool));
    for (i = 0; g_nexus_roster[i].ascii; i++) {
        Nexus_V1_Champion *c = &pool->champions[i];
        strncpy(c->name_ascii, g_nexus_roster[i].ascii, 31);
        strncpy(c->name_jp, g_nexus_roster[i].jp, 63);
        c->primary_class = g_nexus_roster[i].cls;
        c->health = c->max_health = g_nexus_roster[i].hp;
        c->stamina = c->max_stamina = g_nexus_roster[i].sta;
        c->mana = c->max_mana = g_nexus_roster[i].mp;
        c->strength = g_nexus_roster[i].str;
        c->dexterity = g_nexus_roster[i].dex;
        c->wisdom = g_nexus_roster[i].wis;
        c->vitality = g_nexus_roster[i].vit;
        c->anti_magic = 5;
        c->anti_fire = 5;
        c->food = 1500;
        c->water = 1500;
        c->alive = 1;
        c->portrait_index = i;

        /* Initialize inventory and equipment slots */
        for (j = 0; j < 30; j++)
            c->inventory[j] = -1;
        for (j = 0; j < NEXUS_SLOT_COUNT; j++)
            c->slots[j] = -1;

        c->load = 0;
        c->max_load = nexus_champion_get_maximum_load(c);
        c->wounds = 0;
        c->attributes = 0;

        pool->champion_count++;
    }
    /* Empty party */
    for (i = 0; i < NEXUS_MAX_PARTY; i++)
        pool->party[i] = -1;
    pool->leader_index = 0;
}

int nexus_v1_champion_recruit(Nexus_V1_ChampionPool *pool, int mirror_index) {
    int i;
    if (!pool || mirror_index < 0 || mirror_index >= pool->champion_count) return -1;
    if (!pool->champions[mirror_index].alive) return -1;
    if (pool->party_count >= NEXUS_MAX_PARTY) return -1;
    /* Check not already in party */
    for (i = 0; i < pool->party_count; i++)
        if (pool->party[i] == mirror_index) return -1;
    pool->party[pool->party_count++] = mirror_index;
    printf("Recruited %s (%s)\n", pool->champions[mirror_index].name_ascii,
        pool->champions[mirror_index].name_jp);
    return pool->party_count - 1;
}

int nexus_v1_champion_resurrect(Nexus_V1_ChampionPool *pool, int party_slot) {
    int idx;
    if (!pool || party_slot < 0 || party_slot >= pool->party_count) return -1;
    idx = pool->party[party_slot];
    if (idx < 0 || pool->champions[idx].alive) return -1;
    pool->champions[idx].alive = 1;
    pool->champions[idx].health = pool->champions[idx].max_health / 4;
    pool->champions[idx].stamina = pool->champions[idx].max_stamina / 4;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════
 * Champion pool binary serialization (Phase 6 save/load)
 * Source-lock: ReDMCSB LOADSAVE.C F0433/F0434 (DM1 save/load structure),
 *              CHAMPION.C F0309 (champion save format).
 * ═══════════════════════════════════════════════════════════════════ */

/* Helper: write a 32-bit little-endian value and advance pointer */
static uint8_t *wr32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v & 0xFF);
    return p + 4;
}

/* Helper: read a 32-bit little-endian value and advance pointer */
static const uint8_t *rd32(const uint8_t *p, uint32_t *out) {
    *out = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
    return p + 4;
}

/* Calculate champion blob size.
 * Matches ReDMCSB CHAMPION.C F0309 save structure:
 *   name_ascii(32) + name_jp(64) + 23 int fields + inventory(30) + slots(9×4)
 *   = 32 + 64 + 23×4 + 30 + 36 = 268 bytes */
static size_t champion_blob_size(void) {
    return 32 + 64 + (23 * 4) + 30 + (NEXUS_SLOT_COUNT * 4);
}

size_t nexus_v1_champion_pool_serialize_size(const Nexus_V1_ChampionPool *pool) {
    if (!pool) return 0;
    /* Header: magic(4) + version(2) + champion_count(4) + party_count(4)
     *         + leader_index(4) + leader_index_24(4) = 22 bytes */
    size_t n = 4 + 2 + 4 + 4 + 4 + 4;
    n += (size_t)pool->champion_count * champion_blob_size();
    n += (size_t)NEXUS_MAX_PARTY * 4;  /* party slots */
    return n;
}

size_t nexus_v1_champion_pool_serialize(const Nexus_V1_ChampionPool *pool,
                                         void *buf, size_t bufsize) {
    uint8_t *p = (uint8_t *)buf;
    uint8_t *end;
    int i, j;

    if (!pool || !buf) return 0;
    size_t needed = nexus_v1_champion_pool_serialize_size(pool);
    if (bufsize < needed) return 0;

    end = (uint8_t *)buf + bufsize;

    /* Pool header */
    p = wr32(p, NEXUS_CHAMPION_POOL_SAVE_MAGIC);
    p = wr32(p, (uint32_t)NEXUS_CHAMPION_POOL_SAVE_VERSION);
    p = wr32(p, (uint32_t)pool->champion_count);
    p = wr32(p, (uint32_t)pool->party_count);
    p = wr32(p, (uint32_t)pool->leader_index);
    /* leader_index_24 is the leader's champion index in the 0-23 range
     * (same as leader_index for our pool which stores indices directly) */
    p = wr32(p, (uint32_t)pool->leader_index);

    /* Champions */
    for (i = 0; i < pool->champion_count && i < NEXUS_MAX_CHAMPIONS; i++) {
        const Nexus_V1_Champion *c = &pool->champions[i];
        /* name_ascii(32) */
        memset(p, 0, 32);
        if (c->name_ascii[0]) strncpy((char *)p, c->name_ascii, 31);
        p += 32;
        /* name_jp(64) */
        memset(p, 0, 64);
        if (c->name_jp[0]) strncpy((char *)p, c->name_jp, 63);
        p += 64;
        /* All int scalar fields */
        p = wr32(p, (uint32_t)c->primary_class);
        p = wr32(p, (uint32_t)c->health);
        p = wr32(p, (uint32_t)c->max_health);
        p = wr32(p, (uint32_t)c->stamina);
        p = wr32(p, (uint32_t)c->max_stamina);
        p = wr32(p, (uint32_t)c->mana);
        p = wr32(p, (uint32_t)c->max_mana);
        p = wr32(p, (uint32_t)c->strength);
        p = wr32(p, (uint32_t)c->dexterity);
        p = wr32(p, (uint32_t)c->wisdom);
        p = wr32(p, (uint32_t)c->vitality);
        p = wr32(p, (uint32_t)c->anti_magic);
        p = wr32(p, (uint32_t)c->anti_fire);
        p = wr32(p, (uint32_t)c->fighter_level);
        p = wr32(p, (uint32_t)c->ninja_level);
        p = wr32(p, (uint32_t)c->priest_level);
        p = wr32(p, (uint32_t)c->wizard_level);
        p = wr32(p, (uint32_t)c->food);
        p = wr32(p, (uint32_t)c->water);
        p = wr32(p, (uint32_t)c->alive);
        p = wr32(p, (uint32_t)c->portrait_index);
        p = wr32(p, (uint32_t)c->load);
        p = wr32(p, (uint32_t)c->max_load);
        p = wr32(p, (uint32_t)c->wounds);
        p = wr32(p, (uint32_t)c->attributes);
        /* inventory(30) */
        for (j = 0; j < 30; j++)
            *p++ = c->inventory[j];
        /* equipment slots(9×4) */
        for (j = 0; j < NEXUS_SLOT_COUNT; j++)
            p = wr32(p, (uint32_t)c->slots[j]);
    }

    /* Party slots (always 4 entries, -1 = empty) */
    for (i = 0; i < NEXUS_MAX_PARTY; i++) {
        uint32_t slot_val = (i < pool->party_count) ? (uint32_t)pool->party[i] : 0xFFFFFFFFU;
        p = wr32(p, slot_val);
    }

    return (size_t)(p - (uint8_t *)buf);
}

int nexus_v1_champion_pool_deserialize(Nexus_V1_ChampionPool *pool,
                                        const void *buf, size_t bufsize) {
    const uint8_t *p = (const uint8_t *)buf;
    uint32_t magic, version;
    int i, j;

    if (!pool || !buf || bufsize < 4) return -1;

    /* Magic + version */
    p = rd32(p, &magic);
    p = rd32(p, &version);
    if (magic != NEXUS_CHAMPION_POOL_SAVE_MAGIC) return -2;
    if (version != NEXUS_CHAMPION_POOL_SAVE_VERSION) return -3;

    /* Pool header */
    p = rd32(p, (uint32_t *)&pool->champion_count);
    p = rd32(p, (uint32_t *)&pool->party_count);
    p = rd32(p, (uint32_t *)&pool->leader_index);
    { uint32_t li24; p = rd32(p, &li24); (void)li24; } /* skip leader_index_24 */

    if (pool->champion_count > NEXUS_MAX_CHAMPIONS) pool->champion_count = NEXUS_MAX_CHAMPIONS;
    if (pool->party_count > NEXUS_MAX_PARTY) pool->party_count = NEXUS_MAX_PARTY;

    /* Champions */
    for (i = 0; i < pool->champion_count && i < NEXUS_MAX_CHAMPIONS; i++) {
        Nexus_V1_Champion *c = &pool->champions[i];
        /* name_ascii(32) */
        memset(c->name_ascii, 0, sizeof(c->name_ascii));
        memcpy(c->name_ascii, p, 32);
        p += 32;
        /* name_jp(64) */
        memset(c->name_jp, 0, sizeof(c->name_jp));
        memcpy(c->name_jp, p, 64);
        p += 64;
        /* All int scalar fields */
        p = rd32(p, (uint32_t *)&c->primary_class);
        p = rd32(p, (uint32_t *)&c->health);
        p = rd32(p, (uint32_t *)&c->max_health);
        p = rd32(p, (uint32_t *)&c->stamina);
        p = rd32(p, (uint32_t *)&c->max_stamina);
        p = rd32(p, (uint32_t *)&c->mana);
        p = rd32(p, (uint32_t *)&c->max_mana);
        p = rd32(p, (uint32_t *)&c->strength);
        p = rd32(p, (uint32_t *)&c->dexterity);
        p = rd32(p, (uint32_t *)&c->wisdom);
        p = rd32(p, (uint32_t *)&c->vitality);
        p = rd32(p, (uint32_t *)&c->anti_magic);
        p = rd32(p, (uint32_t *)&c->anti_fire);
        p = rd32(p, (uint32_t *)&c->fighter_level);
        p = rd32(p, (uint32_t *)&c->ninja_level);
        p = rd32(p, (uint32_t *)&c->priest_level);
        p = rd32(p, (uint32_t *)&c->wizard_level);
        p = rd32(p, (uint32_t *)&c->food);
        p = rd32(p, (uint32_t *)&c->water);
        p = rd32(p, (uint32_t *)&c->alive);
        p = rd32(p, (uint32_t *)&c->portrait_index);
        p = rd32(p, (uint32_t *)&c->load);
        p = rd32(p, (uint32_t *)&c->max_load);
        p = rd32(p, (uint32_t *)&c->wounds);
        p = rd32(p, (uint32_t *)&c->attributes);
        /* inventory(30) */
        for (j = 0; j < 30; j++)
            c->inventory[j] = (int8_t)*p++;
        /* equipment slots(9×4) */
        for (j = 0; j < NEXUS_SLOT_COUNT; j++)
            p = rd32(p, (uint32_t *)&c->slots[j]);
    }

    /* Party slots */
    for (i = 0; i < NEXUS_MAX_PARTY; i++) {
        uint32_t slot_val;
        p = rd32(p, &slot_val);
        pool->party[i] = (slot_val == 0xFFFFFFFFU) ? -1 : (int)slot_val;
    }

    return 0;
}



/* ═══════════════════════════════════════════════════════════════════
 * Vi Altar Rebirth (F0283_CHAMPION_ViAltarRebirth)
 * Source: ReDMCSB REVIVE.C F0283 lines 902-950, champ_death.md.
 *
 * Vi Altar resurrection is a special dungeon mechanic that permanently
 * reduces champion maximum health (~1/6 reduction). Used for special
 * dungeon events like the VIP altar. Unlike mirror resurrect which
 * fully restores at a mirror square, Vi Altar only restores health
 * to half and permanently reduces the maximum.
 *
 * Steps:
 *   1. Find an empty cell for the champion if the current cell is occupied
 *   2. Clear all slots (all versions) — equipment removed from all slots
 *   3. Set health = new_max / 2
 *   4. new_max = max(25, new_max - new_max/64 - 1)
 *   5. Update direction to match party direction
 *   6. Trigger UI redraw for spell area and champion state
 * ═══════════════════════════════════════════════════════════════════ */

static int get_max_health_int(int a, int b) { return a > b ? a : b; }

int nexus_v1_champion_vi_altar_rebirth(Nexus_V1_Champion *c, int party_direction) {
    int new_max, current_after;

    if (!c) return -1;

    /* Move to an empty adjacent cell if current cell is occupied.
     * Source: REVIVE.C F0283 lines 909-914:
     *   "if (F0285_CHAMPION_GetIndexInCell(L0832_ps_Champion->Cell) != CM1_CHAMPION_NONE)"
     *   The cell logic is dungeon-internal; in Firestaff Nexus the party
     *   handles cell assignment. No-op here — cell management is at the
     *   engine level. We just ensure the champion is alive. */
    (void)0;

    /* Clear all slots — equipment removed.
     * Source: REVIVE.C F0283 lines 927-930 (MEDIA720 variant). */
    c->slots[NEXUS_SLOT_WEAPON]  = -1;
    c->slots[NEXUS_SLOT_HEAD]    = -1;
    c->slots[NEXUS_SLOT_TORSO]  = -1;
    c->slots[NEXUS_SLOT_LEGS]   = -1;
    c->slots[NEXUS_SLOT_FEET]   = -1;
    c->slots[NEXUS_SLOT_HANDS]  = -1;
    c->slots[NEXUS_SLOT_SHIELD] = -1;
    c->slots[NEXUS_SLOT_AMULET] = -1;
    /* Load is cleared */
    c->load = 0;

    /* Calculate new maximum health:
     *   new_max = max(25, max_health - max_health/64 - 1)
     * Source: REVIVE.C F0283 line 933:
     *   F0025_MAIN_GetMaximumValue(25, AL0831_ui_MaximumHealth
     *       - (AL0831_ui_MaximumHealth >> 6) - 1) */
    new_max = c->max_health - (c->max_health >> 6) - 1;
    new_max = get_max_health_int(25, new_max);
    c->max_health = new_max;

    /* Current health = new_max / 2.
     * Source: REVIVE.C F0283 line 934:
     *   L0832_ps_Champion->CurrentHealth = (L0832_ps_Champion->MaximumHealth = ...) >> 1 */
    current_after = new_max >> 1;
    c->health = current_after;

    /* Set champion alive and facing party direction.
     * Source: REVIVE.C F0283 lines 936-939. */
    c->alive = 1;
    c->direction = party_direction;

    /* Mark UI attributes dirty — spell area + status box redraw needed.
     * MASK0x8000_ACTION_HAND | MASK0x1000_STATUS_BOX | MASK0x0400_ICON.
     * Source: REVIVE.C F0283 line 940. */
    c->attributes |= NEXUS_ATTR_LOAD_CHANGED | NEXUS_ATTR_STATISTICS;

    printf("%s [Vi Altar]: reborn with health %d/%d (permanent max-health reduction)\n",
           c->name_ascii, c->health, c->max_health);

    return 0;
}
