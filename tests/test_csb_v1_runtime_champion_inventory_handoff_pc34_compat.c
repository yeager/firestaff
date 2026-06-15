/*
 * CSB V1 runtime champion inventory handoff regression.
 *
 * This is a narrow runtime boundary test, not an end-to-end CSB playability
 * claim.  It imports a synthetic DM1 champion snapshot into CSB runtime,
 * applies one F0302-style hand/slot exchange to the runtime-owned party,
 * then proves the changed action hand survives one V1 tick and one leader
 * switch boundary.
 *
 * Source-locks:
 *   ReDMCSB CHAMPION.C F0297 lines 243-266
 *   ReDMCSB CHAMPION.C F0298 lines 270-295
 *   ReDMCSB CHAMPION.C F0302 lines 662-710
 *   ReDMCSB COMMAND.C F0380 lines 1964-1986 and 2175-2176
 *   ReDMCSB LOADSAVE.C F0435 lines 2733-2744
 *   ReDMCSB DEFS.H lines 541, 547, 5324, 5861-5863, 7921-7946
 */

#include "csb_v1_runtime_champion_inventory_handoff_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define THING_NONE 0xFFFFu
#define LEADER_HAND_SEED 0x1234u
#define ACTION_HAND_SEED 0x0234u
#define READY_HAND_SEED 0x0456u
#define LOAD_FLAG 0x0200u

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_INT(got, want, msg) do { \
    if ((got) == (want)) { \
        passed++; printf("  PASS: %s == %d\n", msg, (int)(want)); \
    } else { \
        failed++; printf("  FAIL: %s got=%d want=%d\n", msg, (int)(got), (int)(want)); \
    } \
} while (0)

#define CHECK_U16(got, want, msg) do { \
    if ((uint16_t)(got) == (uint16_t)(want)) { \
        passed++; printf("  PASS: %s == 0x%04X\n", msg, (unsigned)(uint16_t)(want)); \
    } else { \
        failed++; printf("  FAIL: %s got=0x%04X want=0x%04X\n", msg, \
                        (unsigned)(uint16_t)(got), (unsigned)(uint16_t)(want)); \
    } \
} while (0)

static void put16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

static int build_synthetic_dm1_party_buffer(uint8_t *buf, size_t buf_size)
{
    int i;
    int slot;
    if (!buf || buf_size < 1024) return -1;
    memset(buf, 0, buf_size);
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 2;

    for (i = 0; i < 2; i++) {
        size_t off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START +
                     (size_t)i * (size_t)CSB_V1_DM1_CHAMP_SIZE;
        size_t equip_off = off + (size_t)CSB_V1_DM1_CHAMP_OFF_EQUIP;
        memcpy((char *)buf + off + CSB_V1_DM1_CHAMP_OFF_NAME,
               i == 0 ? "ALPHA   " : "BETA    ", 8);
        put16(buf + off + CSB_V1_DM1_CHAMP_OFF_HEALTH, (uint16_t)(80 + i));
        put16(buf + off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH, (uint16_t)(100 + i));
        put16(buf + off + CSB_V1_DM1_CHAMP_OFF_STAMINA, (uint16_t)(60 + i));
        put16(buf + off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA, (uint16_t)(100 + i));
        put16(buf + off + CSB_V1_DM1_CHAMP_OFF_MANA, (uint16_t)(30 + i));
        put16(buf + off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA, (uint16_t)(50 + i));
        buf[off + CSB_V1_DM1_CHAMP_OFF_STR] = (uint8_t)(55 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_DEX] = (uint8_t)(66 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_WIS] = (uint8_t)(77 + i);
        buf[off + CSB_V1_DM1_CHAMP_OFF_VIT] = (uint8_t)(88 + i);

        for (slot = 0; slot < CSB_V1_SLOT_COUNT; slot++) {
            put16(buf + equip_off + (size_t)slot * 2u, THING_NONE);
        }
    }

    put16(buf + CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_EQUIP +
              (size_t)CSB_V1_SLOT_ACTION_HAND * 2u,
          ACTION_HAND_SEED);
    put16(buf + CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_EQUIP +
              (size_t)CSB_V1_SLOT_READY_HAND * 2u,
          READY_HAND_SEED);

    return 0;
}

int main(void)
{
    uint8_t buf[1024];
    CSB_V1_PartyState imported;
    CSB_V1_PartyState after;
    CSB_V1_RuntimeProfile runtime;
    CSB_V1_RuntimeChampionInventoryHandoffResultPc34Compat handoff;
    uint64_t handoff_hash;
    uint64_t final_hash;
    const char *evidence =
        csb_v1_runtime_champion_inventory_handoff_source_evidence_pc34_compat();

    printf("probe=test_csb_v1_runtime_champion_inventory_handoff_pc34_compat\n");
    printf("sourceEvidence=%s\n", evidence);

    CHECK(strstr(evidence, "CHAMPION.C F0297 lines 243-266") != NULL,
          "source evidence cites F0297 leader-hand put");
    CHECK(strstr(evidence, "CHAMPION.C F0298 lines 270-295") != NULL,
          "source evidence cites F0298 leader-hand remove");
    CHECK(strstr(evidence, "CHAMPION.C F0302 lines 662-710") != NULL,
          "source evidence cites F0302 slot-box exchange");
    CHECK(strstr(evidence, "COMMAND.C F0380 lines 1964-1986") != NULL,
          "source evidence cites command dispatch into F0302");
    CHECK(strstr(evidence, "LOADSAVE.C F0435 lines 2733-2744") != NULL,
          "source evidence cites load/leader-hand restore");

    CHECK(build_synthetic_dm1_party_buffer(buf, sizeof(buf)) == 0,
          "synthetic DM1 import buffer created");
    memset(&imported, 0, sizeof(imported));
    CHECK_INT(csb_v1_character_import_dm1_buffer(&imported, buf, (int)sizeof(buf)),
              2, "DM1 buffer imports two champions");
    CHECK_INT(imported.ImportedFromDM1, 1, "party is marked imported from DM1");
    CHECK_INT(imported.LeaderIndex, 0, "import starts with champion 0 leader");
    CHECK_U16(imported.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND],
              THING_NONE, "CSB import clears champion 0 action hand");
    CHECK_U16(imported.Champions[0].Slots[CSB_V1_SLOT_READY_HAND],
              THING_NONE, "CSB import clears champion 0 ready hand");
    CHECK_U16(imported.Champions[1].Slots[CSB_V1_SLOT_ACTION_HAND],
              THING_NONE, "import leaves champion 1 action hand empty");

    /* The CSB import path intentionally clears DM1 equipment
     * (csb_v1_character_pc34_compat.c cites ReDMCSB CEDTINCI.C
     * F7090/F7020), so this runtime regression seeds one source-owned
     * post-import slot before handing the party to the CSB runtime. */
    imported.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND] = ACTION_HAND_SEED;
    imported.Champions[0].Slots[CSB_V1_SLOT_READY_HAND] = READY_HAND_SEED;

    csb_v1_runtime_init(&runtime, NULL);
    CHECK(csb_v1_runtime_set_party_state(&runtime, &imported) == 0,
          "runtime accepts imported party snapshot");
    CHECK_INT(runtime.party_state_valid, 1, "runtime party state is valid");
    CHECK_INT(runtime.leader_index, 0, "runtime leader is imported leader");

    memset(&handoff, 0, sizeof(handoff));
    CHECK_INT(csb_v1_runtime_champion_inventory_handoff_pc34_compat(
                  &runtime,
                  0,
                  CSB_V1_SLOT_ACTION_HAND,
                  LEADER_HAND_SEED,
                  &handoff),
              1, "F0302-style action-hand exchange applied");
    CHECK_INT(handoff.imported_from_dm1, 1,
              "handoff records imported-DM1 party boundary");
    CHECK_INT(handoff.party_state_valid, 1,
              "handoff runs on runtime-owned party snapshot");
    CHECK_INT(handoff.leader_index_before, 0,
              "handoff starts with champion 0 leader");
    CHECK_U16(handoff.slot_thing_before, ACTION_HAND_SEED,
              "handoff removes old action-hand thing");
    CHECK_U16(handoff.slot_thing_after, LEADER_HAND_SEED,
              "handoff writes old leader-hand thing to action hand");
    CHECK_U16(handoff.leader_hand_before, LEADER_HAND_SEED,
              "handoff starts with seeded transient leader hand");
    CHECK_U16(handoff.leader_hand_after, ACTION_HAND_SEED,
              "handoff returns old action-hand thing to transient leader hand");
    CHECK((handoff.attributes_after & LOAD_FLAG) != 0,
          "handoff marks champion load state dirty like F0297/F0298");

    handoff_hash =
        csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(&runtime);
    CHECK(handoff.state_hash == handoff_hash,
          "handoff result hash matches runtime hash after exchange");

    CHECK_INT(csb_v1_runtime_tick_v1(&runtime), 1,
              "one CSB V1 runtime tick fires after handoff");
    CHECK_INT(runtime.tick_count, 1, "runtime tick_count advanced once");
    CHECK_INT(runtime.game_time, 1, "runtime game_time advanced once");

    CHECK_INT(csb_v1_runtime_set_leader(&runtime, 1), 0,
              "leader switch to champion 1 succeeds after handoff");
    CHECK_INT(runtime.leader_index, 1,
              "runtime leader index is champion 1 after switch");

    memset(&after, 0, sizeof(after));
    CHECK_INT(csb_v1_runtime_get_party_state(&runtime, &after), 2,
              "runtime returns two-champion party after tick and leader switch");
    CHECK_INT(after.LeaderIndex, 1,
              "party snapshot records switched leader");
    CHECK_U16(after.Champions[0].Slots[CSB_V1_SLOT_ACTION_HAND],
              LEADER_HAND_SEED,
              "champion 0 action-hand mutation survives tick and leader switch");
    CHECK_U16(after.Champions[0].Slots[CSB_V1_SLOT_READY_HAND],
              READY_HAND_SEED,
              "champion 0 ready-hand slot is not disturbed");
    CHECK_U16(after.Champions[1].Slots[CSB_V1_SLOT_ACTION_HAND],
              THING_NONE,
              "leader switch does not leak item into champion 1 action hand");
    CHECK((after.Champions[0].Attributes & LOAD_FLAG) != 0,
          "champion 0 load-dirty status bit survives tick and leader switch");

    final_hash =
        csb_v1_runtime_champion_inventory_handoff_hash_pc34_compat(&runtime);
    printf("deterministicHash=0x%016llX\n", (unsigned long long)final_hash);
    printf("handoffHash=0x%016llX\n", (unsigned long long)handoff_hash);
    printf("assertions=%d failures=%d\n", passed, failed);

    if (failed == 0) {
        printf("ok: CSB V1 imported champion action-hand handoff survives one runtime tick and leader switch without claiming full CSB playability\n");
    }
    return failed ? 1 : 0;
}
