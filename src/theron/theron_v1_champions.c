/*
 * theron_v1_champions.c — Theron's Quest V1 Phase 7: Champion State & Pack/Unpack
 *
 * Champion structs for Theron's Quest: Theron + up to 3 companions.
 * Companion persistence: THQUEST.ASM T800.
 *
 * Phase 7 additions:
 *   - theron_v1_party_pack / unpack / pack_size — wire 4×128-byte champion
 *     blocks into the save/load path (THERON_SAVE_CHAMPION_COUNT × 128 bytes).
 *   - Full party lifecycle helpers: init, dungeon entry/exit reset,
 *     leader management, gold tracking, load recalculation.
 *
 * Source references:
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T800  — champion persistence + inventory reset per dungeon
 *   THQUEST.ASM T560  — dungeon loading (header + dungeon_seed)
 */

#include "theron_v1_champions.h"
#include "theron_v1_track02_champion_roster.h"
#include <string.h>
#include <stdlib.h>

/* ── Block size (must match save format) ─────────────────────────────── */

size_t theron_v1_champion_block_size(void) {
    return sizeof(Theron_V1_Champion);
}

/* ── Party initialisation ────────────────────────────────────────────── */

static void init_champion_from_roster(Theron_V1_Champion *c,
                                      int slot,
                                      unsigned int roster_index) {
    if (!c) return;
    memset(c, 0, sizeof(*c));

    const Theron_ChampionRecord *rec = theron_v1_track02_us_champion(roster_index);
    if (!rec) return;

    size_t len = strlen(rec->name);
    if (len > 23) len = 23;
    memcpy(c->name, rec->name, len);
    c->name[23] = '\0';

    c->portrait_index = (uint8_t)(slot);
    c->alive          = 1;

    c->health      = (int16_t)rec->hp;
    c->max_health  = (int16_t)rec->hp;
    c->stamina     = (int16_t)rec->stamina;
    c->max_stamina = (int16_t)rec->stamina;
    c->mana        = (int16_t)rec->mana;
    c->max_mana    = (int16_t)rec->mana;

    c->luck        = (int16_t)rec->luck;
    c->strength    = (int16_t)rec->strength;
    c->dexterity   = (int16_t)rec->dexterity;
    c->wisdom      = (int16_t)rec->wisdom;
    c->vitality    = (int16_t)rec->vitality;
    c->anti_magic  = (int16_t)rec->anti_magic;
    c->anti_fire   = (int16_t)rec->anti_fire;

    /* Skill levels from Track 02 roster — highest sub-skill per class */
    uint8_t fl = 0, nl = 0, pl = 0, wl = 0;
    for (int i = 0; i < 4; i++) {
        if (rec->fighter_skills[i] > fl) fl = rec->fighter_skills[i];
        if (rec->ninja_skills[i]   > nl) nl = rec->ninja_skills[i];
        if (rec->priest_skills[i]  > pl) pl = rec->priest_skills[i];
        if (rec->wizard_skills[i]  > wl) wl = rec->wizard_skills[i];
    }
    c->fighter_level = fl;
    c->ninja_level   = nl;
    c->priest_level  = pl;
    c->wizard_level  = wl;

    /* Primary class = highest class level (Fighter wins ties) */
    c->primary_class = THERON_CLASS_FIGHTER;
    uint8_t best = fl;
    if (nl > best) { c->primary_class = THERON_CLASS_NINJA;  best = nl; }
    if (pl > best) { c->primary_class = THERON_CLASS_PRIEST; best = pl; }
    if (wl > best) { c->primary_class = THERON_CLASS_WIZARD; }

    c->wounds     = 0;
    c->attributes = 0;
    memset(c->inventory, 0, sizeof(c->inventory));
    for (int i = 0; i < THERON_EQUIP_SLOT_COUNT; i++) c->slots[i] = -1;

    /* Starting equipment from DMWeb roster */
    int inv_next = 0;
    for (int i = 0; i < (int)rec->start_equip_count && i < 12; i++) {
        int8_t item = rec->start_equip_item[i];
        int8_t eslot = rec->start_equip_slot[i];
        if (item < 0) break;
        if (eslot >= 0 && eslot < THERON_EQUIP_SLOT_COUNT) {
            c->slots[eslot] = (int16_t)item;
        }
        if (inv_next < THERON_INVENTORY_SLOTS) {
            c->inventory[inv_next++] = (uint8_t)item;
        }
    }

    c->load     = (int16_t)inv_next;
    c->max_load = (int16_t)((rec->strength << 3) + 100);
    c->food     = 0;
    c->water    = 0;
}

void theron_v1_party_init(Theron_V1_Party *party, int dungeon_index) {
    (void)dungeon_index;
    if (!party) return;
    memset(party, 0, sizeof(*party));

    /* Slot 0: THERON (roster index 0) — always in party */
    init_champion_from_roster(&party->champions[0], 0, 0);
    /* Slots 1-3: default to MARA/LINOS/HEXA (roster 1/2/3);
     * actual selection happens via Soul Room mirrors per dungeon. */
    init_champion_from_roster(&party->champions[1], 1, 1);
    init_champion_from_roster(&party->champions[2], 2, 2);
    init_champion_from_roster(&party->champions[3], 3, 3);

    party->champion_count = 4;
    party->active_slot    = 0;
    party->gold           = 0;
}

void theron_v1_party_clear_fixture_defaults(Theron_V1_Party *party) {
    if (!party) return;
    for (int i = 0; i < THERON_MAX_CHAMPIONS; ++i) {
        Theron_V1_Champion *c = &party->champions[i];
        memset(c->name, 0, sizeof(c->name));
        c->portrait_index = 0;
        c->primary_class = THERON_CLASS_FIGHTER;
        c->health = 0;
        c->max_health = 0;
        c->stamina = 0;
        c->max_stamina = 0;
        c->mana = 0;
        c->max_mana = 0;
        c->strength = 0;
        c->dexterity = 0;
        c->wisdom = 0;
        c->vitality = 0;
        c->anti_magic = 0;
        c->anti_fire = 0;
        c->fighter_level = 0;
        c->ninja_level = 0;
        c->priest_level = 0;
        c->wizard_level = 0;
        c->wounds = 0;
        c->attributes = 0;
        memset(c->inventory, 0, sizeof(c->inventory));
        for (int slot = 0; slot < THERON_EQUIP_SLOT_COUNT; ++slot) {
            c->slots[slot] = -1;
        }
        c->load = 0;
        c->max_load = 0;
        c->food = 0;
        c->water = 0;
        c->alive = 0;
    }
    party->champion_count = 0;
    party->active_slot = 0;
    party->gold = 0;
}

/* ── Soul Room companion selection ──────────────────────────────────── */

int theron_v1_party_set_companion(Theron_V1_Party *party,
                                  int slot,
                                  unsigned int roster_index) {
    if (!party) return -1;
    if (slot < 1 || slot > 3) return -1;
    if (roster_index >= theron_v1_track02_us_champion_count()) return -1;
    if (roster_index == 0) return -1;
    init_champion_from_roster(&party->champions[slot],
                              slot, roster_index);
    return 0;
}

/* ── Dungeon entry/exit reset ─────────────────────────────────────────── */

void theron_v1_party_dungeon_entry_reset(Theron_V1_Party *party) {
    if (!party) return;
    /* Companions (slots 1-3): clear inventories and equipment */
    for (int i = 1; i < THERON_MAX_CHAMPIONS; i++) {
        theron_v1_champion_reset_inventory(&party->champions[i]);
    }
    /* Theron (slot 0): keep everything — no reset needed */

    /* Recalculate loads for all champions */
    theron_v1_party_recalculate_loads(party);
}

void theron_v1_party_dungeon_exit(Theron_V1_Party *party) {
    (void)party;
    /* No state change on dungeon exit for Phase 7.
     * All persistent state is captured in the between-dungeon save. */
}

/* ── Champion accessors ──────────────────────────────────────────────── */

Theron_V1_Champion *theron_v1_party_getChampion(Theron_V1_Party *party, int slot) {
    if (!party) return NULL;
    if (slot < 0 || slot >= THERON_MAX_CHAMPIONS) return NULL;
    return &party->champions[slot];
}

Theron_V1_Champion *theron_v1_party_leader(Theron_V1_Party *party) {
    if (!party) return NULL;
    return &party->champions[party->active_slot];
}

/* Const-correct versions */
const Theron_V1_Champion *theron_v1_party_getChampion_c(const Theron_V1_Party *party, int slot) {
    if (!party) return NULL;
    if (slot < 0 || slot >= THERON_MAX_CHAMPIONS) return NULL;
    return &party->champions[slot];
}

const Theron_V1_Champion *theron_v1_party_leader_c(const Theron_V1_Party *party) {
    if (!party) return NULL;
    return &party->champions[party->active_slot];
}

/* ── Pack / unpack ──────────────────────────────────────────────────── */

size_t theron_v1_party_pack_size(void) {
    return (size_t)THERON_MAX_CHAMPIONS * theron_v1_champion_block_size();
}

size_t theron_v1_party_pack(const Theron_V1_Party *party, void *buf, size_t bufsize) {
    if (!party || !buf) return 0;
    size_t needed = theron_v1_party_pack_size();
    if (bufsize < needed) return 0;

    size_t block = theron_v1_champion_block_size();
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        memcpy((uint8_t *)buf + i * block,
               &party->champions[i],
               block);
    }
    return needed;
}

int theron_v1_party_unpack(Theron_V1_Party *party, const void *buf, size_t bufsize) {
    if (!party || !buf) return -1;
    size_t needed = theron_v1_party_pack_size();
    if (bufsize < needed) return -1;

    size_t block = theron_v1_champion_block_size();
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        memcpy(&party->champions[i],
               (const uint8_t *)buf + i * block,
               block);
    }

    /* gold and champion_count are NOT in the champion block stream;
     * they are restored separately by the save/load caller via the
     * dungeon_progression or dedicated gold slot.  Here we only restore
     * the champion arrays. */
    /* Legacy save blocks do not carry the startup mirror selection, so keep
     * all decoded champion templates available until the startup/save path
     * supplies a narrower selected party count. */
    party->champion_count = THERON_MAX_CHAMPIONS;
    party->active_slot    = THERON_CHAMPION_SLOT_THERON;
    return 0;
}

/* ── Party-level predicates ─────────────────────────────────────────── */

int theron_v1_party_theron_alive(const Theron_V1_Party *party) {
    if (!party) return 0;
    return party->champions[THERON_CHAMPION_SLOT_THERON].alive != 0;
}

int16_t theron_v1_party_total_health(const Theron_V1_Party *party) {
    if (!party) return 0;
    int16_t total = 0;
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        if (party->champions[i].alive) {
            total += (int16_t)party->champions[i].health;
        }
    }
    return total;
}

/* ── Load recalculation ──────────────────────────────────────────────── */

void theron_v1_party_recalculate_loads(Theron_V1_Party *party) {
    if (!party) return;
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &party->champions[i];
        int load = 0;
        for (int j = 0; j < THERON_INVENTORY_SLOTS; j++) {
            if (c->inventory[j] != THERON_ITEM_NONE) load++;
        }
        c->load = (int16_t)load;
        /* max_load is a property of the champion's body (strength);
         * simplified: (strength << 3) + 100 */
        c->max_load = (int16_t)(((int)c->strength << 3) + 100);
    }
}

/* ── Low-level champion helpers (from Phase 3) ───────────────────────── */

size_t theron_v1_champion_block_size_PHASE3(void) {
    return theron_v1_champion_block_size();
}

int theron_v1_champion_is_alive(const Theron_V1_Champion *c) {
    return c && c->alive;
}

int theron_v1_champion_skill_level(const Theron_V1_Champion *c) {
    if (!c) return 0;
    switch (c->primary_class) {
        case THERON_CLASS_FIGHTER: return c->fighter_level;
        case THERON_CLASS_NINJA:   return c->ninja_level;
        case THERON_CLASS_PRIEST:  return c->priest_level;
        case THERON_CLASS_WIZARD:  return c->wizard_level;
        default: return 0;
    }
}

void theron_v1_champion_reset_inventory(Theron_V1_Champion *c) {
    if (!c) return;
    memset(c->inventory, 0, sizeof(c->inventory));
    for (int i = 0; i < THERON_EQUIP_SLOT_COUNT; i++) c->slots[i] = -1;
    c->load = 0;
}

const char *theron_v1_champions_source_evidence(void) {
    return "THQUEST.ASM T520/T560/T800  "
           "+ tqr_v1_phase0_provenance_gate_H2339.md";
}
