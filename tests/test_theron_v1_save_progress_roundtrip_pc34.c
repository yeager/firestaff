/*
 * Theron V1 save/load progress round-trip regression.
 *
 * Source-locked against:
 *   THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon saves)
 *   THQUEST.ASM T800  — champion persistence between dungeons
 *   include/theron_v1_save_load.h           — slotN.tqsv layout
 *   include/theron_v1_champions.h           — Theron_V1_Party + Theron_V1_Champion
 *   include/theron_v1_dungeon_progression.h — Theron_DungeonProgression
 *
 * Scope: a single dungeon-2 mid-quest between-dungeon save/restore with a
 * real Theron_V1_Party. The party is built via theron_v1_party_init,
 * mutated into a realistic mid-quest snapshot, packed through the public
 * theron_v1_party_pack API into a slot-sized opaque buffer, written to
 * a slot via theron_v1_save_to_slot, read back via
 * theron_v1_save_load_from_slot, and unpacked into a fresh party via
 * theron_v1_party_unpack. The test then asserts every champion field
 * that the documented pack/unpack path is supposed to preserve is
 * byte-equal across the round-trip, plus a re-save overwrite round trip.
 *
 * Disjoint from:
 *   tests/theron_v1_save_load_test.c
 *     — that suite uses dummy champion bytes (`memset 0; champ_data[0]
 *       = 0x42`) and never exercises theron_v1_party_pack/unpack with a
 *       real Theron_V1_Party. It also doesn't reach the slot-checksum
 *       boundary with a populated struct.
 *   tests/test_theron_v1_save_header_rejection.c
 *     — that suite only mutates one header byte per fixture and
 *       verifies that mutated headers stay non-launchable. It does not
 *       exercise the champion stream at all.
 *
 * The size of the champion buffer is taken from
 * theron_v1_party_pack_size(), so the test stays in lock-step with the
 * real pack/unpack API even if Theron_V1_Champion grows or shrinks.
 *
 * The test also documents the fact that map-level fields
 * (leader_x/y/dir, levitating, door_state_override, gold) live in
 * Theron_V1_Party but are outside the 4×Theron_V1_Champion pack
 * stream. The pack/unpack path explicitly resets them and forces
 * champion_count/active_slot to TQR canonical defaults; pinning that
 * behaviour here means a future struct-shape regression cannot silently
 * start writing those fields into the pack stream without the test
 * flagging the change.
 */

#include "theron_v1_champions.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#define TST_MKDIR(path) _mkdir(path)
#define TST_RMDIR(path) _rmdir(path)
#else
#include <unistd.h>
#define TST_MKDIR(path) mkdir(path, 0700)
#define TST_RMDIR(path) rmdir(path)
#endif

static int g_failures = 0;

static void expect_true(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_temp_dir(char out[512]) {
#if defined(_WIN32)
    const char *tmp = getenv("TEMP");
    snprintf(out, 512, "%s\\firestaff_theron_save_progress_%lu",
             tmp ? tmp : ".", (unsigned long)time(NULL));
    return TST_MKDIR(out) == 0;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_save_progress_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

static void remove_temp_dir(const char *root) {
    int slot;
    for (slot = 0; slot < THERON_SAVE_SLOT_COUNT; ++slot) {
        char path[512];
        theron_v1_save_slot_path(root, slot, path, sizeof(path));
        if (path[0]) {
            remove(path);
        }
    }
    TST_RMDIR(root);
}

/*
 * Build a non-trivial Theron_V1_Party representing a mid-quest
 * checkpoint (Crypt of Shadows entered, first quest item found, party
 * carries gold, Theron is kitted out, one companion is wounded, one
 * companion is downed).
 */
static void populate_party_at_progress(Theron_V1_Party *party) {
    if (!party) return;

    theron_v1_party_init(party, THERON_DUNGEON_2_CRYPT_OF_SHADOWS);

    /* Map-level fields — set to distinctive values so we can detect
     * if a future change starts packing them. The pack/unpack stream
     * does not include them today. */
    party->leader_x = 11;
    party->leader_y = 7;
    party->leader_dir = 1; /* East */
    party->levitating = 0;
    party->door_state_override = 0;
    party->gold = 4321u;

    /* Theron (slot 0): kitted fighter with grown stats. */
    Theron_V1_Champion *theron = &party->champions[0];
    theron->health = 47;   theron->max_health = 60;
    theron->stamina = 39;  theron->max_stamina = 50;
    theron->mana = 12;     theron->max_mana = 24;
    theron->strength = 17;
    theron->dexterity = 14;
    theron->wisdom = 11;
    theron->vitality = 16;
    theron->anti_magic = 3;
    theron->anti_fire = 2;
    theron->fighter_level = 3;
    theron->wounds = 0;
    theron->alive = 1;
    strncpy(theron->name, "Theron", sizeof(theron->name) - 1);
    theron->portrait_index = 4;
    theron->primary_class = THERON_CLASS_FIGHTER;
    /* Inventory: a few quest-relevant items. */
    theron->inventory[0] = THERON_ITEM_POTION;
    theron->inventory[1] = THERON_ITEM_POTION;
    theron->inventory[2] = THERON_ITEM_FOOD;
    theron->inventory[3] = THERON_ITEM_KEY;
    theron->food = 6;
    theron->water = 4;
    /* Equip: weapon + armor + shield. */
    theron->slots[THERON_ESLOT_WEAPON] = THERON_ITEM_WEAPON;
    theron->slots[THERON_ESLOT_ARMOR] = THERON_ITEM_ARMOR;
    theron->slots[THERON_ESLOT_SHIELD] = THERON_ITEM_SHIELD;
    /* Other equip slots intentionally left at -1. */

    /* Companion 1: priest with a head wound, mid-level, partial inventory. */
    Theron_V1_Champion *c1 = &party->champions[1];
    strncpy(c1->name, "Mara", sizeof(c1->name) - 1);
    c1->portrait_index = 2;
    c1->primary_class = THERON_CLASS_PRIEST;
    c1->alive = 1;
    c1->health = 22;  c1->max_health = 35;
    c1->stamina = 28; c1->max_stamina = 40;
    c1->mana = 18;    c1->max_mana = 30;
    c1->strength = 9;
    c1->dexterity = 12;
    c1->wisdom = 15;
    c1->vitality = 10;
    c1->priest_level = 2;
    c1->wounds = THERON_WOUND_HEAD;
    c1->attributes = THERON_ATTR_STATISTICS;
    c1->inventory[0] = THERON_ITEM_SCROLL;
    c1->inventory[5] = THERON_ITEM_ANTIDOTE;
    c1->slots[THERON_ESLOT_HELM] = THERON_ITEM_HELM;
    c1->food = 3;
    c1->water = 5;

    /* Companion 2: wizard, no wounds, carries scrolls. */
    Theron_V1_Champion *c2 = &party->champions[2];
    strncpy(c2->name, "Vey", sizeof(c2->name) - 1);
    c2->portrait_index = 6;
    c2->primary_class = THERON_CLASS_WIZARD;
    c2->alive = 1;
    c2->health = 18;  c2->max_health = 28;
    c2->stamina = 22; c2->max_stamina = 35;
    c2->mana = 30;    c2->max_mana = 42;
    c2->strength = 8;
    c2->dexterity = 11;
    c2->wisdom = 17;
    c2->vitality = 9;
    c2->wizard_level = 2;
    c2->inventory[2] = THERON_ITEM_SCROLL;
    c2->inventory[3] = THERON_ITEM_SCROLL;
    c2->food = 4;
    c2->water = 4;

    /* Companion 3: downed fighter, must round-trip with alive=0 and
     * negative health preserved as-is. */
    Theron_V1_Champion *c3 = &party->champions[3];
    strncpy(c3->name, "Ruk", sizeof(c3->name) - 1);
    c3->portrait_index = 5;
    c3->primary_class = THERON_CLASS_FIGHTER;
    c3->alive = 0;
    c3->health = -1;
    c3->max_health = 45;
    c3->stamina = 0; c3->max_stamina = 50;
    c3->mana = 0;    c3->max_mana = 0;
    c3->strength = 14;
    c3->dexterity = 13;
    c3->wisdom = 8;
    c3->vitality = 14;
    c3->fighter_level = 2;
    c3->wounds = THERON_WOUND_BODY | THERON_WOUND_ARMS | THERON_WOUND_LEGS;

    /* Recalculate loads so the load field is not stale. */
    theron_v1_party_recalculate_loads(party);
}

/*
 * Build a Theron_DungeonProgression that matches the same checkpoint:
 *   - dungeon 2 in progress (Crypt of Shadows, entered)
 *   - first quest item collected
 *   - dungeon_states: 1=COMPLETE, 2=IN_PROGRESS
 *   - per-dungeon seeds distinct
 *   - playtime non-zero
 */
static void populate_progression(Theron_DungeonProgression *prog) {
    if (!prog) return;
    theron_v1_dungeon_progression_init(prog);

    prog->current_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;
    prog->current_level = 1;
    prog->dungeon_states[THERON_DUNGEON_1_HALL_OF_RECORDS - 1] =
        THERON_DUNGEON_STATE_COMPLETE;
    prog->dungeon_states[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] =
        THERON_DUNGEON_STATE_IN_PROGRESS;
    prog->quest_items_collected = THERON_QUEST_ITEM_1_SACRED_AMPLIFIER;
    prog->quest_items_in_current_dungeon = 1;
    prog->item_reset_mode = THERON_ITEM_RESET_MODE_CHAMPION;
    prog->item_reset_applied = 1;
    prog->dungeon_playtime_seconds = 0x0BADF00Du;
    prog->champion_stats_persist = 1;
    prog->champion_inv_persist = 0;
    for (int i = 0; i < THERON_DUNGEON_COUNT; ++i) {
        prog->dungeon_seeds[i] = (uint32_t)(0xC0DE0000u + (unsigned)i);
    }
}

int main(void) {
    char temp_dir[512];
    uint8_t *champion_buffer;
    uint8_t *champion_read;
    size_t packed;
    int rc;

    if (!make_temp_dir(temp_dir)) {
        perror("make temp dir");
        return 1;
    }

    /* ── 1. Build the source party and progression at the checkpoint. */
    Theron_V1_Party party_before;
    Theron_DungeonProgression prog_before;
    populate_party_at_progress(&party_before);
    populate_progression(&prog_before);

    /* Sanity: the source party is well-formed before we even hit the
     * save path. Catches fixture mistakes early. */
    expect_true(party_before.champions[0].alive == 1,
                "fixture: Theron alive");
    expect_true(party_before.champions[1].wounds == THERON_WOUND_HEAD,
                "fixture: companion 1 has head wound");
    expect_true(party_before.champions[3].alive == 0,
                "fixture: companion 3 is downed");
    expect_true(prog_before.quest_items_collected ==
                THERON_QUEST_ITEM_1_SACRED_AMPLIFIER,
                "fixture: only first quest item collected");
    expect_true(prog_before.dungeon_playtime_seconds == 0x0BADF00Du,
                "fixture: dungeon playtime sentinel");
    expect_true(party_before.gold == 4321u,
                "fixture: party gold set");

    /* ── 2. Pack party into the opaque champion buffer and save. */
    champion_buffer = (uint8_t *)malloc(theron_v1_party_pack_size());
    if (!champion_buffer) {
        fprintf(stderr, "FAIL: malloc(%zu) for champion stream\n",
                theron_v1_party_pack_size());
        ++g_failures;
        remove_temp_dir(temp_dir);
        return 1;
    }
    packed = theron_v1_party_pack(&party_before,
                                   champion_buffer,
                                   theron_v1_party_pack_size());
    expect_true(packed == theron_v1_party_pack_size(),
                "theron_v1_party_pack filled the full stream");
    expect_true(theron_v1_party_pack_size() ==
                THERON_MAX_CHAMPIONS * theron_v1_champion_block_size(),
                "pack_size matches 4×block_size");

    rc = theron_v1_save_to_slot(temp_dir,
                                /* slot_index = */ 4,
                                champion_buffer,
                                packed,
                                &prog_before,
                                "After Crypt of Shadows entrance");
    expect_true(rc == 0, "save to slot 4 succeeded");

    /* ── 3. Load from slot into fresh buffers and unpack. */
    champion_read = (uint8_t *)malloc(theron_v1_party_pack_size());
    if (!champion_read) {
        fprintf(stderr, "FAIL: malloc(%zu) for read buffer\n",
                theron_v1_party_pack_size());
        ++g_failures;
        free(champion_buffer);
        remove_temp_dir(temp_dir);
        return 1;
    }
    Theron_DungeonProgression prog_read;
    Theron_V1_Party party_after;
    Theron_SaveSlot slot_info;

    memset(champion_read, 0xCD, theron_v1_party_pack_size());
    memset(&prog_read, 0, sizeof(prog_read));
    memset(&party_after, 0xCD, sizeof(party_after));
    memset(&slot_info, 0, sizeof(slot_info));

    rc = theron_v1_save_load_from_slot(temp_dir,
                                        4,
                                        champion_read,
                                        theron_v1_party_pack_size(),
                                        &prog_read,
                                        sizeof(prog_read),
                                        &slot_info);
    expect_true(rc == 0, "load from slot 4 succeeded");
    expect_true(slot_info.valid == 1,
                "loaded slot is marked valid");
    expect_true(slot_info.slot_index == 4,
                "loaded slot index matches");
    expect_true(slot_info.current_dungeon ==
                THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                "loaded slot header: current dungeon");
    expect_true(slot_info.quest_items ==
                THERON_QUEST_ITEM_1_SACRED_AMPLIFIER,
                "loaded slot header: quest marker");
    expect_true(strcmp(slot_info.label,
                       "After Crypt of Shadows entrance") == 0,
                "loaded slot label matches");

    /* The champion stream is what the runtime has to drive from, so
     * it must reach party_after byte-identical to the packed stream. */
    expect_true(memcmp(champion_read, champion_buffer,
                       theron_v1_party_pack_size()) == 0,
                "champion buffer round-trips byte-identical");

    int uprc = theron_v1_party_unpack(&party_after,
                                      champion_read,
                                      theron_v1_party_pack_size());
    expect_true(uprc == 0, "theron_v1_party_unpack succeeded");

    /* ── 4. Champion array must byte-equal the source party (the
     *      pack/unpack stream IS the entire champion array). */
    expect_true(memcmp(&party_after.champions,
                       &party_before.champions,
                       sizeof(party_before.champions)) == 0,
                "champion array after pack+save+load+unpack byte-identical");

    /* ── 5. Spot-check the documented invariants field-by-field so a
     *      regression points at the right field instead of dumping a
     *      full struct mismatch. */
    expect_true(party_after.champions[0].alive == 1,
                "Theron alive flag preserved");
    expect_true(strcmp(party_after.champions[0].name, "Theron") == 0,
                "Theron name preserved");
    expect_true(party_after.champions[0].health == 47 &&
                party_after.champions[0].max_health == 60,
                "Theron health/max_health preserved");
    expect_true(party_after.champions[0].mana == 12 &&
                party_after.champions[0].max_mana == 24,
                "Theron mana/max_mana preserved");
    expect_true(party_after.champions[0].fighter_level == 3,
                "Theron fighter_level preserved");
    expect_true(party_after.champions[0].strength == 17 &&
                party_after.champions[0].dexterity == 14 &&
                party_after.champions[0].wisdom == 11 &&
                party_after.champions[0].vitality == 16,
                "Theron stat block preserved");
    expect_true(party_after.champions[0].slots[THERON_ESLOT_WEAPON] ==
                THERON_ITEM_WEAPON,
                "Theron weapon equip preserved");
    expect_true(party_after.champions[0].slots[THERON_ESLOT_ARMOR] ==
                THERON_ITEM_ARMOR,
                "Theron armor equip preserved");
    expect_true(party_after.champions[0].slots[THERON_ESLOT_SHIELD] ==
                THERON_ITEM_SHIELD,
                "Theron shield equip preserved");
    expect_true(party_after.champions[0].inventory[0] ==
                THERON_ITEM_POTION,
                "Theron inventory[0] preserved");
    expect_true(party_after.champions[0].inventory[3] ==
                THERON_ITEM_KEY,
                "Theron inventory[3] preserved");
    expect_true(party_after.champions[0].food == 6 &&
                party_after.champions[0].water == 4,
                "Theron food/water preserved");

    expect_true(party_after.champions[1].alive == 1,
                "companion 1 alive flag preserved");
    expect_true(strcmp(party_after.champions[1].name, "Mara") == 0,
                "companion 1 name preserved");
    expect_true(party_after.champions[1].wounds == THERON_WOUND_HEAD,
                "companion 1 wound bitmask preserved");
    expect_true(party_after.champions[1].priest_level == 2,
                "companion 1 priest_level preserved");
    expect_true(party_after.champions[1].inventory[0] ==
                THERON_ITEM_SCROLL,
                "companion 1 inventory[0] preserved");
    expect_true(party_after.champions[1].inventory[5] ==
                THERON_ITEM_ANTIDOTE,
                "companion 1 inventory[5] preserved");
    expect_true(party_after.champions[1].slots[THERON_ESLOT_HELM] ==
                THERON_ITEM_HELM,
                "companion 1 helm equip preserved");

    expect_true(party_after.champions[2].alive == 1,
                "companion 2 alive flag preserved");
    expect_true(strcmp(party_after.champions[2].name, "Vey") == 0,
                "companion 2 name preserved");
    expect_true(party_after.champions[2].mana == 30 &&
                party_after.champions[2].max_mana == 42,
                "companion 2 mana block preserved");
    expect_true(party_after.champions[2].wizard_level == 2,
                "companion 2 wizard_level preserved");

    expect_true(party_after.champions[3].alive == 0,
                "companion 3 downed flag preserved");
    expect_true(strcmp(party_after.champions[3].name, "Ruk") == 0,
                "companion 3 name preserved");
    expect_true(party_after.champions[3].health == -1,
                "companion 3 negative health preserved");
    expect_true(party_after.champions[3].max_health == 45,
                "companion 3 max_health preserved");
    expect_true(party_after.champions[3].wounds ==
                (THERON_WOUND_BODY | THERON_WOUND_ARMS |
                 THERON_WOUND_LEGS),
                "companion 3 multi-wound bitmask preserved");

    /* ── 6. Documented pack/unpack side-effects:
     *      champion_count and active_slot are forced to canonical TQR
     *      defaults because they are not part of the save stream
     *      (TQR is always 4 champions, leader is always Theron at
     *      slot 0). A future struct-shape regression that starts
     *      packing them will change these post-unpack values, which
     *      the next two assertions will then fail. */
    expect_true(party_after.champion_count == THERON_MAX_CHAMPIONS,
                "champion_count forced to 4 (TQR canonical)");
    expect_true(party_after.active_slot == THERON_CHAMPION_SLOT_THERON,
                "active_slot forced to slot 0 (TQR canonical)");

    /* ── 7. Dungeon progression survives the save/load independently
     *      of the champion pack stream. The runtime is expected to
     *      re-load the progression via the Theron_DungeonProgression
     *      memcpy, not via party pack. */
    expect_true(prog_read.current_dungeon == prog_before.current_dungeon,
                "progression current_dungeon preserved");
    expect_true(prog_read.current_level == prog_before.current_level,
                "progression current_level preserved");
    expect_true(prog_read.dungeon_states[THERON_DUNGEON_1_HALL_OF_RECORDS - 1] ==
                THERON_DUNGEON_STATE_COMPLETE,
                "progression dungeon 1 state preserved");
    expect_true(prog_read.dungeon_states[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] ==
                THERON_DUNGEON_STATE_IN_PROGRESS,
                "progression dungeon 2 state preserved");
    expect_true(prog_read.quest_items_collected ==
                prog_before.quest_items_collected,
                "progression quest_items_collected preserved");
    expect_true(prog_read.quest_items_in_current_dungeon ==
                prog_before.quest_items_in_current_dungeon,
                "progression quest_items_in_current_dungeon preserved");
    expect_true(prog_read.item_reset_mode == prog_before.item_reset_mode,
                "progression item_reset_mode preserved");
    expect_true(prog_read.item_reset_applied == prog_before.item_reset_applied,
                "progression item_reset_applied preserved");
    expect_true(prog_read.dungeon_playtime_seconds ==
                prog_before.dungeon_playtime_seconds,
                "progression dungeon_playtime_seconds preserved");
    expect_true(prog_read.champion_stats_persist == 1,
                "progression champion_stats_persist flag preserved");
    expect_true(prog_read.champion_inv_persist == 0,
                "progression champion_inv_persist flag preserved");
    {
        int i;
        for (i = 0; i < THERON_DUNGEON_COUNT; ++i) {
            if (prog_read.dungeon_seeds[i] !=
                prog_before.dungeon_seeds[i]) {
                fprintf(stderr,
                        "FAIL: progression dungeon_seeds[%d] mismatch "
                        "(got 0x%08x expected 0x%08x)\n",
                        i, prog_read.dungeon_seeds[i],
                        prog_before.dungeon_seeds[i]);
                ++g_failures;
                break;
            }
            if (prog_read.dungeon_states[i] !=
                prog_before.dungeon_states[i]) {
                fprintf(stderr,
                        "FAIL: progression dungeon_states[%d] mismatch "
                        "(got %d expected %d)\n",
                        i, prog_read.dungeon_states[i],
                        prog_before.dungeon_states[i]);
                ++g_failures;
                break;
            }
        }
    }

    free(champion_buffer);
    free(champion_read);

    /* ── 8. Cross-check: enum + verify + load-from-slot produces
     *      exactly one valid entry on the same slot. */
    {
        Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
        int count = theron_v1_save_enum_slots(temp_dir, slots,
                                              THERON_SAVE_SLOT_COUNT);
        expect_true(count == THERON_SAVE_SLOT_COUNT,
                    "enum reports full 8-slot table");
        expect_true(slots[4].valid == 1,
                    "enum: slot 4 reported valid");
        expect_true(theron_v1_save_verify_slot(temp_dir, 4) == 1,
                    "verify: slot 4 is intact");
    }

    /* ── 9. Re-save (overwrite) round-trip: a second save with new
     *      progress must replace the old slot's champion data and
     *      progression exactly. Catches any stale-tail or partial-
     *      write bug in the production code path. */
    {
        Theron_V1_Party party_v2;
        Theron_DungeonProgression prog_v2;
        Theron_V1_Party party_after;
        Theron_DungeonProgression prog_read;
        Theron_SaveSlot slot_info;

        memset(&party_v2, 0, sizeof(party_v2));
        memset(&prog_v2, 0, sizeof(prog_v2));
        theron_v1_party_init(&party_v2, THERON_DUNGEON_3_ABYSS_OF_FLAMES);
        party_v2.champions[0].health = 9;
        party_v2.champions[0].max_health = 70;
        party_v2.champions[1].alive = 0;
        populate_progression(&prog_v2);
        prog_v2.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
        prog_v2.current_level = 2;
        prog_v2.dungeon_states[THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] =
            THERON_DUNGEON_STATE_COMPLETE;
        prog_v2.quest_items_collected =
            THERON_QUEST_ITEM_1_SACRED_AMPLIFIER |
            THERON_QUEST_ITEM_2_SHADOW_KEY;

        champion_buffer = (uint8_t *)malloc(theron_v1_party_pack_size());
        if (!champion_buffer) {
            fprintf(stderr, "FAIL: re-save malloc failed\n");
            ++g_failures;
            remove_temp_dir(temp_dir);
            return 1;
        }
        size_t packed_v2 = theron_v1_party_pack(&party_v2,
                                                champion_buffer,
                                                theron_v1_party_pack_size());
        expect_true(packed_v2 == theron_v1_party_pack_size(),
                    "re-save pack filled the full stream");

        rc = theron_v1_save_to_slot(temp_dir,
                                    4,
                                    champion_buffer,
                                    packed_v2,
                                    &prog_v2,
                                    "Abyss of Flames level 2");
        expect_true(rc == 0, "re-save to slot 4 succeeded");

        champion_read = (uint8_t *)malloc(theron_v1_party_pack_size());
        if (!champion_read) {
            fprintf(stderr, "FAIL: re-save read malloc failed\n");
            ++g_failures;
            free(champion_buffer);
            remove_temp_dir(temp_dir);
            return 1;
        }
        memset(champion_read, 0, theron_v1_party_pack_size());
        memset(&prog_read, 0, sizeof(prog_read));
        memset(&party_after, 0, sizeof(party_after));
        memset(&slot_info, 0, sizeof(slot_info));

        rc = theron_v1_save_load_from_slot(temp_dir,
                                           4,
                                           champion_read,
                                           theron_v1_party_pack_size(),
                                           &prog_read,
                                           sizeof(prog_read),
                                           &slot_info);
        expect_true(rc == 0, "re-save load succeeded");
        expect_true(slot_info.current_dungeon ==
                    THERON_DUNGEON_3_ABYSS_OF_FLAMES,
                    "re-save header reflects new dungeon");
        expect_true(slot_info.quest_items ==
                    (THERON_QUEST_ITEM_1_SACRED_AMPLIFIER |
                     THERON_QUEST_ITEM_2_SHADOW_KEY),
                    "re-save header reflects two quest items");
        expect_true(strcmp(slot_info.label,
                           "Abyss of Flames level 2") == 0,
                    "re-save header label is updated");

        theron_v1_party_unpack(&party_after, champion_read,
                               theron_v1_party_pack_size());
        expect_true(party_after.champions[0].health == 9 &&
                    party_after.champions[0].max_health == 70,
                    "re-save: Theron health/max_health preserved");
        expect_true(party_after.champions[1].alive == 0,
                    "re-save: companion 1 downed flag preserved");

        free(champion_buffer);
        free(champion_read);
    }

    remove_temp_dir(temp_dir);

    if (g_failures) {
        return 1;
    }
    puts("ok: Theron V1 save/load progress round-trip preserves a real party "
         "and progression checkpoint across slot save+load+unpack");
    return 0;
}
