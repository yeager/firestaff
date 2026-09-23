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
#if !defined(FIRESTAFF_THERON_PRODUCTION)
#include "theron_v1_track02_champion_roster.h"
#endif
#include "theron_v1_track02_jp_roster_receipt.h"
#include "theron_v1_track02_us_roster_receipt.h"
#include "theron_v1_track02.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if !defined(FIRESTAFF_THERON_PRODUCTION)
static size_t bounded_text_len(const char *text, size_t capacity) {
    size_t n = 0;
    if (!text) return 0;
    while (n < capacity && text[n] != '\0') ++n;
    return n;
}
#endif

static void apply_jp_record_to_champion(
    Theron_V1_Champion *champion,
    const Theron_Track02JpRosterReceipt *record) {
    uint8_t fighter = 0u;
    uint8_t ninja = 0u;
    uint8_t priest = 0u;
    uint8_t wizard = 0u;

    if (!champion || !record || !record->valid) return;

    if (record->name[0] != '\0') {
        snprintf(champion->name, sizeof(champion->name), "%s", record->name);
    }
    champion->portrait_index = THERON_PORTRAIT_UNAVAILABLE;
    champion->alive = 1;
    champion->health = (int16_t)record->hp;
    champion->max_health = (int16_t)record->hp;
    champion->stamina = (int16_t)record->stamina;
    champion->max_stamina = (int16_t)record->stamina;
    champion->mana = (int16_t)record->mana;
    champion->max_mana = (int16_t)record->mana;
    champion->luck = (int16_t)record->attributes[0];
    champion->strength = (int16_t)record->attributes[1];
    champion->dexterity = (int16_t)record->attributes[2];
    champion->wisdom = (int16_t)record->attributes[3];
    champion->vitality = (int16_t)record->attributes[4];
    champion->anti_magic = (int16_t)record->attributes[5];
    champion->anti_fire = (int16_t)record->attributes[6];
    for (int i = 0; i < 4; ++i) {
        if (record->skills[i] > fighter) fighter = record->skills[i];
        if (record->skills[4 + i] > ninja) ninja = record->skills[4 + i];
        if (record->skills[8 + i] > priest) priest = record->skills[8 + i];
        if (record->skills[12 + i] > wizard) wizard = record->skills[12 + i];
    }
    champion->fighter_level = fighter;
    champion->ninja_level = ninja;
    champion->priest_level = priest;
    champion->wizard_level = wizard;
    champion->primary_class = THERON_CLASS_FIGHTER;
    {
        uint8_t best = fighter;
        if (ninja > best) {
            champion->primary_class = THERON_CLASS_NINJA;
            best = ninja;
        }
        if (priest > best) {
            champion->primary_class = THERON_CLASS_PRIEST;
            best = priest;
        }
        if (wizard > best) champion->primary_class = THERON_CLASS_WIZARD;
    }
}

/* ── Block size (must match save format) ─────────────────────────────── */

size_t theron_v1_champion_block_size(void) {
    return sizeof(Theron_V1_Champion);
}

/* ── Party initialisation ────────────────────────────────────────────── */

#if !defined(FIRESTAFF_THERON_PRODUCTION)
static void init_champion_from_roster(Theron_V1_Champion *c,
                                      int slot,
                                      unsigned int roster_index) {
    if (!c) return;
    memset(c, 0, sizeof(*c));

    const Theron_ChampionRecord *rec = theron_v1_track02_us_champion(roster_index);
    if (!rec) return;

    /* US champion text is not yet joined to an authenticated text consumer.
     * Keep the numeric/source record usable for the forcefield handoff while
     * leaving its unavailable name empty instead of dereferencing NULL. */
    if (rec->name) {
        size_t len = bounded_text_len(rec->name, sizeof(c->name));
        if (len > 23) len = 23;
        memcpy(c->name, rec->name, len);
        c->name[23] = '\0';
    }

    /* Track 02 roster records do not bind portrait pixels or portrait IDs.
     * Do not derive a visual index from the host party slot. The explicit
     * startup fixture/asset path may fill this field after its own evidence
     * gate; the source-bound roster path keeps it unavailable. */
    (void)slot;
    c->portrait_index = THERON_PORTRAIT_UNAVAILABLE;
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

    /* Starting equipment from the DMWeb roster is useful for fixture probes,
     * but it is not a Track 02 object-record/T900 binding. Production must
     * not publish it as real inventory while the original start-object
     * consumer is unavailable. */
#if !defined(FIRESTAFF_THERON_PRODUCTION)
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
#else
    c->load     = 0;
#endif
    {
        int ml = ((int)rec->strength * 625 + 12500) / 1000;
        c->max_load = (int16_t)(ml > 6 ? ml : 6);
    }
    c->food     = 0;
    c->water    = 0;
}
#endif

void theron_v1_party_init(Theron_V1_Party *party, int dungeon_index) {
    (void)dungeon_index;
    if (!party) return;
    memset(party, 0, sizeof(*party));

#if defined(FIRESTAFF_THERON_PRODUCTION)
    /* A production party starts empty.  The authenticated regional Track 02
     * roster reader is the only owner of champion stats and skills.  Zero is
     * a real host item index for equipment slots, so explicitly retain the
     * unavailable sentinel until a T900 source consumer binds equipment. */
    for (int champion = 0; champion < THERON_MAX_CHAMPIONS; ++champion) {
        for (int equip = 0; equip < THERON_EQUIP_SLOT_COUNT; ++equip) {
            party->champions[champion].slots[equip] = -1;
        }
    }
    party->champion_count = 0;
    return;
#else
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
#endif
}

int theron_v1_party_refresh_jp_source_records(
    Theron_V1_Party *party,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex) {
    Theron_Track02JpRosterReceipt records[THERON_TRACK02_JP_ROSTER_COUNT];
    int matches[THERON_MAX_CHAMPIONS];

    if (!party || !track02_data || !md5_hex ||
        strcmp(md5_hex, THERON_TRACK02_MD5_JP_BIN) != 0 ||
        !theron_v1_track02_jp_roster_read(
            track02_data, track02_size, md5_hex, records)) {
        return 0;
    }
    if (party->champion_count < 0 ||
        party->champion_count > THERON_MAX_CHAMPIONS) {
        return 0;
    }
    for (int slot = 0; slot < party->champion_count; ++slot) {
        int found = -1;
        if (party->champions[slot].name[0] != '\0') {
            for (unsigned int index = 0u;
                 index < THERON_TRACK02_JP_ROSTER_COUNT; ++index) {
                if (strcmp(party->champions[slot].name,
                           records[index].name) == 0) {
                    found = (int)index;
                    break;
                }
            }
        } else if (slot < (int)THERON_TRACK02_JP_ROSTER_COUNT) {
            found = slot;
        }
        if (found < 0) return 0;
        matches[slot] = found;
    }
    for (int slot = 0; slot < party->champion_count; ++slot) {
        apply_jp_record_to_champion(&party->champions[slot],
                                    &records[(unsigned int)matches[slot]]);
    }
    return 1;
}

int theron_v1_party_refresh_us_source_records(
    Theron_V1_Party *party,
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex) {
    Theron_Track02UsRosterReceipt records[THERON_TRACK02_US_ROSTER_COUNT];
    int matches[THERON_MAX_CHAMPIONS];

    if (!party || !track02_data || !md5_hex ||
        strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) != 0 ||
        !theron_v1_track02_us_roster_read(
            track02_data, track02_size, md5_hex, records)) return 0;
    if (party->champion_count < 0 ||
        party->champion_count > THERON_MAX_CHAMPIONS) return 0;
    for (int slot = 0; slot < party->champion_count; ++slot) {
        int found = -1;
        for (unsigned int index = 0u;
             index < THERON_TRACK02_US_ROSTER_COUNT; ++index) {
            if (party->champions[slot].name[0] != '\0' &&
                strcmp(party->champions[slot].name,
                       records[index].name) == 0) {
                found = (int)index;
                break;
            }
        }
        if (found < 0 && party->champions[slot].name[0] == '\0' &&
            slot < (int)THERON_TRACK02_US_ROSTER_COUNT) found = slot;
        if (found < 0) return 0;
        matches[slot] = found;
    }
    for (int slot = 0; slot < party->champion_count; ++slot) {
        Theron_Track02JpRosterReceipt common;
        const Theron_Track02UsRosterReceipt *source =
            &records[(unsigned int)matches[slot]];
        memset(&common, 0, sizeof(common));
        common.valid = source->valid;
        snprintf(common.name, sizeof(common.name), "%s", source->name);
        common.sex = source->sex;
        common.hp = source->hp;
        common.stamina = source->stamina;
        common.mana = source->mana;
        memcpy(common.attributes, source->attributes,
               sizeof(common.attributes));
        memcpy(common.skills, source->skills, sizeof(common.skills));
        apply_jp_record_to_champion(&party->champions[slot], &common);
    }
    return 1;
}

#if defined(THERON_CHAMPION_FIXTURE_HELPERS)

/* Fixture-only reset. Production startup now keeps source-bound roster
 * records and must never expose this synthetic zero-party helper. */
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

#endif /* THERON_CHAMPION_FIXTURE_HELPERS */

/* ── Per-dungeon companion availability ───────────────────────────────
 * DOTAN (roster index 6) is absent from Dungeon 1 (AKUTUBA). */

int theron_v1_companion_available_in_dungeon(unsigned int roster_index,
                                             int dungeon_id) {
    if (roster_index == 0 || roster_index >= THERON_TRACK02_US_ROSTER_COUNT)
        return 0;
    if (roster_index == 6 && dungeon_id == 1) return 0;
    return 1;
}

/* ── Soul Room companion selection ──────────────────────────────────── */

int theron_v1_party_set_companion(Theron_V1_Party *party,
                                  int slot,
                                  unsigned int roster_index,
                                  int dungeon_id) {
    if (!party) return -1;
    if (slot < 1 || slot > 3) return -1;
    if (!theron_v1_companion_available_in_dungeon(roster_index, dungeon_id))
        return -1;
#if defined(FIRESTAFF_THERON_PRODUCTION)
    /* Identity is supplied by the authenticated roster-name catalog in the
     * caller.  Numeric state remains zero until the regional record reader
     * admits all selected records atomically. */
    memset(&party->champions[slot], 0, sizeof(party->champions[slot]));
    for (int equip = 0; equip < THERON_EQUIP_SLOT_COUNT; ++equip) {
        party->champions[slot].slots[equip] = -1;
    }
    party->champions[slot].portrait_index = THERON_PORTRAIT_UNAVAILABLE;
#else
    init_champion_from_roster(&party->champions[slot],
                              slot, roster_index);
#endif
    return 0;
}

/* ── Dungeon entry/exit reset ─────────────────────────────────────────── */

void theron_v1_party_dungeon_entry_reset(Theron_V1_Party *party) {
    if (!party) return;
    /* Companions (slots 1-3): clear inventories and equipment */
    for (int i = 1; i < party->champion_count &&
                        i < THERON_MAX_CHAMPIONS; i++) {
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
    if (slot < 0 || slot >= party->champion_count ||
        slot >= THERON_MAX_CHAMPIONS) return NULL;
    return &party->champions[slot];
}

Theron_V1_Champion *theron_v1_party_leader(Theron_V1_Party *party) {
    if (!party || party->active_slot < 0 ||
        party->active_slot >= party->champion_count ||
        party->active_slot >= THERON_MAX_CHAMPIONS) return NULL;
    return &party->champions[party->active_slot];
}

/* Const-correct versions */
const Theron_V1_Champion *theron_v1_party_getChampion_c(const Theron_V1_Party *party, int slot) {
    if (!party) return NULL;
    if (slot < 0 || slot >= party->champion_count ||
        slot >= THERON_MAX_CHAMPIONS) return NULL;
    return &party->champions[slot];
}

const Theron_V1_Champion *theron_v1_party_leader_c(const Theron_V1_Party *party) {
    if (!party || party->active_slot < 0 ||
        party->active_slot >= party->champion_count ||
        party->active_slot >= THERON_MAX_CHAMPIONS) return NULL;
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
    if (!party || party->champion_count <= THERON_CHAMPION_SLOT_THERON) return 0;
    return party->champions[THERON_CHAMPION_SLOT_THERON].alive != 0;
}

int16_t theron_v1_party_total_health(const Theron_V1_Party *party) {
    if (!party) return 0;
    int16_t total = 0;
    for (int i = 0; i < party->champion_count &&
                        i < THERON_MAX_CHAMPIONS; i++) {
        if (party->champions[i].alive) {
            total += (int16_t)party->champions[i].health;
        }
    }
    return total;
}

/* ── Load recalculation ──────────────────────────────────────────────── */

void theron_v1_party_recalculate_loads(Theron_V1_Party *party) {
    if (!party) return;
    for (int i = 0; i < party->champion_count &&
                        i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &party->champions[i];
        int load = 0;
        for (int j = 0; j < THERON_INVENTORY_SLOTS; j++) {
            if (c->inventory[j] != THERON_ITEM_NONE) load++;
        }
        c->load = (int16_t)load;
        /* ReDMCSB F0309: max_load = (strength * 625 + 12500) / 1000, min 6 */
        int ml = ((int)c->strength * 625 + 12500) / 1000;
        c->max_load = (int16_t)(ml > 6 ? ml : 6);
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
