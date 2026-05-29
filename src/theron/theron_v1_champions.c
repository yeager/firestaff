/*
 * theron_v1_champions.c — Theron's Quest V1 Phase 7: Champion State & Pack/Unpack
 *
 * Champion structs for Theron's Quest: Theron + 3 companions.
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
#include <string.h>
#include <stdlib.h>

/* ── Block size (must match save format) ─────────────────────────────── */

size_t theron_v1_champion_block_size(void) {
    return sizeof(Theron_V1_Champion);
}

/* ── Party initialisation ────────────────────────────────────────────── */

static void init_champion_slot(Theron_V1_Champion *c,
                                int slot,
                                const char *name,
                                Theron_ChampionClass cls) {
    if (!c) return;
    memset(c, 0, sizeof(*c));

    if (name) {
        size_t len = strlen(name);
        if (len > 23) len = 23;
        memcpy(c->name, name, len);
        c->name[23] = '\0';
    }

    c->portrait_index = (uint8_t)(slot);
    c->primary_class  = cls;
    c->alive           = 1;

    c->health      = 10;  c->max_health  = 10;
    c->stamina     = 10;  c->max_stamina = 10;
    c->mana        =  0;  c->max_mana    =  0;

    c->strength    = 10;
    c->dexterity   = 10;
    c->wisdom      = 10;
    c->vitality    = 10;
    c->anti_magic  =  0;
    c->anti_fire   =  0;

    c->fighter_level = (cls == THERON_CLASS_FIGHTER) ? 1 : 0;
    c->ninja_level   = (cls == THERON_CLASS_NINJA)   ? 1 : 0;
    c->priest_level  = (cls == THERON_CLASS_PRIEST)  ? 1 : 0;
    c->wizard_level  = (cls == THERON_CLASS_WIZARD)  ? 1 : 0;

    c->wounds     = 0;
    c->attributes = 0;
    memset(c->inventory, 0, sizeof(c->inventory));
    for (int i = 0; i < THERON_EQUIP_SLOT_COUNT; i++) c->slots[i] = -1;
    c->load     = 0;
    c->max_load = 180;   /* (10 << 3) + 100 */
    c->food     = 0;
    c->water    = 0;
    /* padding[6] already zeroed by memset */
}

void theron_v1_party_init(Theron_V1_Party *party, int dungeon_index) {
    (void)dungeon_index;   /* unused — dungeon_index for future Phase 2 seed */
    if (!party) return;
    memset(party, 0, sizeof(*party));

    /* Slot 0: Theron */
    init_champion_slot(&party->champions[0], 0, "Theron", THERON_CLASS_FIGHTER);
    /* Slot 1-3: companions (blank until player-named) */
    init_champion_slot(&party->champions[1], 1, "Companion 1", THERON_CLASS_FIGHTER);
    init_champion_slot(&party->champions[2], 2, "Companion 2", THERON_CLASS_PRIEST);
    init_champion_slot(&party->champions[3], 3, "Companion 3", THERON_CLASS_WIZARD);

    party->champion_count = 4;
    party->active_slot    = 0;
    party->gold           = 0;
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
    /* champion_count is always 4 for TQR; active_slot is always 0 */
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
