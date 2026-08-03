#include "dm2_v1_object_transfer_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

#define HERO_ITEM_SLOTS DM2_V1_OBJECT_TRANSFER_HERO_ITEM_SLOTS
#define HAND_SLOTS DM2_V1_OBJECT_TRANSFER_HAND_CONTAINER_SLOTS

typedef struct {
    uint16_t items[HERO_ITEM_SLOTS];
    uint16_t hands[HAND_SLOTS];
} TestParty;

static void party_init(TestParty *party, DM2_V1_PossessionSlots *slots)
{
    size_t i;

    for (i = 0u; i < HERO_ITEM_SLOTS; ++i) {
        party->items[i] = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    for (i = 0u; i < HAND_SLOTS; ++i) {
        party->hands[i] = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    slots->hero_items = party->items;
    slots->hero_item_count = HERO_ITEM_SLOTS;
    slots->hand_container = party->hands;
    slots->hand_container_count = HAND_SLOTS;
    slots->hero_index = 0;
    slots->cur_act_hero = 1; /* curacthero - 1 == hero 0 */
    slots->cur_act_mode = 0;
}

/* ------------------------------------------------------------------ */
/* DM2_REMOVE_POSSESSION                                               */
/* ------------------------------------------------------------------ */

static void test_remove_possession(void)
{
    TestParty party;
    DM2_V1_PossessionSlots slots;
    DM2_V1_ObjectTransferReceipt receipt;

    party_init(&party, &slots);
    party.items[0] = 0x1001u;
    party.items[12] = 0x1002u;
    party.hands[3] = 0x2003u;

    expect_true(dm2_v1_REMOVE_POSSESSION(&slots, 12, &receipt) == 0x1002u,
                "REMOVE_POSSESSION returns the item slot content");
    expect_true(party.items[12] == DM2_V1_OBJECT_TRANSFER_NULL,
                "REMOVE_POSSESSION clears the item slot");
    expect_true(receipt.valid && receipt.mutated &&
                    receipt.removed_ref == 0x1002u &&
                    receipt.slot_index == 12 &&
                    receipt.item_bonus_pending &&
                    !receipt.ui_refresh_needed &&
                    strcmp(receipt.symbol, "REMOVE_POSSESSION") == 0,
                "REMOVE_POSSESSION receipt records the item-bonus follow-up");

    /* Slot 30+ addresses party.hand_container[slot - 30]. */
    expect_true(dm2_v1_REMOVE_POSSESSION(&slots, 33, &receipt) == 0x2003u,
                "slot 33 resolves hand_container[3]");
    expect_true(party.hands[3] == DM2_V1_OBJECT_TRANSFER_NULL,
                "REMOVE_POSSESSION clears the hand container slot");

    /* An already-empty slot returns with no side effects. */
    expect_true(dm2_v1_REMOVE_POSSESSION(&slots, 12, &receipt) ==
                    DM2_V1_OBJECT_TRANSFER_NULL,
                "an empty slot returns the null ref");
    expect_true(receipt.valid && !receipt.mutated &&
                    !receipt.item_bonus_pending &&
                    !receipt.ui_refresh_needed,
                "an empty slot removal has no side effects");

    /* Hand slot 0 on the active hero with matching curactmode
     * requests the squad-hands panel refresh. */
    expect_true(dm2_v1_REMOVE_POSSESSION(&slots, 0, &receipt) == 0x1001u,
                "hand slot 0 removal returns the held item");
    expect_true(receipt.ui_refresh_needed,
                "the active hero's active hand requests a panel refresh");

    /* Out-of-range slots fail closed. */
    expect_true(dm2_v1_REMOVE_POSSESSION(&slots, 99, &receipt) ==
                    DM2_V1_OBJECT_TRANSFER_NULL,
                "an out-of-range slot fails closed");
    expect_true(receipt.blocked && !receipt.valid,
                "out-of-range removal is a blocked receipt");
    expect_true(dm2_v1_REMOVE_POSSESSION(0, 0, &receipt) ==
                    DM2_V1_OBJECT_TRANSFER_NULL,
                "null slots fail closed");
}

/* ------------------------------------------------------------------ */
/* DM2_PUT_OBJECT_INTO_CONTAINER                                       */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t container;
    uint16_t appended[HAND_SLOTS];
    int count;
    int fail_after;
} TestAppendLog;

static int test_append(void *context, uint16_t container_ref,
                       uint16_t object_ref)
{
    TestAppendLog *log = (TestAppendLog *)context;

    if (log->fail_after >= 0 && log->count >= log->fail_after) {
        return 0;
    }
    log->container = container_ref;
    if (log->count < (int)HAND_SLOTS) {
        log->appended[log->count] = object_ref;
    }
    log->count++;
    return 1;
}

static void test_put_object_into_container(void)
{
    TestParty party;
    DM2_V1_PossessionSlots slots;
    DM2_V1_ObjectTransferReceipt receipt;
    TestAppendLog log;
    uint16_t pending;

    party_init(&party, &slots);
    party.hands[0] = 0x5001u;
    party.hands[2] = 0x5002u;
    party.hands[7] = 0x5003u;
    memset(&log, 0, sizeof(log));
    log.fail_after = -1;
    pending = 0x3000u;

    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(&pending, party.hands,
                                                 HAND_SLOTS, test_append,
                                                 &log, &receipt) == 1,
                "PUT_OBJECT_INTO_CONTAINER drains the hand containers");
    expect_true(pending == DM2_V1_OBJECT_TRANSFER_NULL,
                "the pending drop target is consumed");
    expect_true(log.count == 3 && log.container == 0x3000u &&
                    log.appended[0] == 0x5001u &&
                    log.appended[1] == 0x5002u &&
                    log.appended[2] == 0x5003u,
                "every occupied hand slot is appended in slot order");
    expect_true(party.hands[0] == DM2_V1_OBJECT_TRANSFER_NULL &&
                    party.hands[2] == DM2_V1_OBJECT_TRANSFER_NULL &&
                    party.hands[7] == DM2_V1_OBJECT_TRANSFER_NULL,
                "moved hand slots are cleared");
    expect_true(receipt.valid && receipt.mutated &&
                    receipt.moved_count == 3 &&
                    receipt.container_ref == 0x3000u &&
                    receipt.moved_slot_mask == 0x85u &&
                    strcmp(receipt.symbol,
                           "PUT_OBJECT_INTO_CONTAINER") == 0,
                "the batch receipt records the moved slot mask");

    /* No pending target: the source returns immediately. */
    memset(&log, 0, sizeof(log));
    log.fail_after = -1;
    pending = DM2_V1_OBJECT_TRANSFER_NULL;
    party.hands[1] = 0x5004u;
    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(&pending, party.hands,
                                                 HAND_SLOTS, test_append,
                                                 &log, &receipt) == 1,
                "no pending target is a bounded no-op");
    expect_true(log.count == 0 && party.hands[1] == 0x5004u,
                "no pending target leaves the hand containers alone");

    /* A failing append fails the batch closed. */
    memset(&log, 0, sizeof(log));
    log.fail_after = 0;
    pending = 0x3000u;
    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(&pending, party.hands,
                                                 HAND_SLOTS, test_append,
                                                 &log, &receipt) == 0,
                "a failing append fails the batch closed");
    expect_true(receipt.blocked, "a failing append is a blocked receipt");

    expect_true(dm2_v1_PUT_OBJECT_INTO_CONTAINER(0, party.hands,
                                                 HAND_SLOTS, test_append,
                                                 &log, &receipt) == 0,
                "a missing pending target pointer fails closed");
}

static void test_append_to_chain_primitive(void)
{
    DM2_V1_ObjectTransferLink links[] = {
        {0x2001u, 0x2002u},
        {0x2002u, DM2_V1_OBJECT_TRANSFER_NULL}
    };
    DM2_V1_ObjectTransferReceipt receipt;
    uint16_t new_head;
    uint16_t previous_tail;

    expect_true(dm2_v1_object_transfer_append_to_chain(
                    links, 2u, 0x3000u, 0x2001u, 0x2222u, &new_head,
                    &previous_tail, &receipt) == 1,
                "the append primitive appends to a non-empty chain");
    expect_true(new_head == 0x2001u && previous_tail == 0x2002u &&
                    receipt.valid && receipt.previous_ref == 0x2002u,
                "the append primitive records tail and head");

    expect_true(dm2_v1_object_transfer_append_to_chain(
                    links, 0u, 0x3000u, DM2_V1_OBJECT_TRANSFER_NULL,
                    0x2222u, &new_head, &previous_tail, &receipt) == 1,
                "the append primitive seeds an empty container");
    expect_true(new_head == 0x2222u, "empty container head becomes the object");

    expect_true(dm2_v1_object_transfer_append_to_chain(
                    links, 2u, 0x3000u, 0x2001u, 0x2002u, &new_head,
                    &previous_tail, &receipt) == 0,
                "the append primitive blocks a duplicate chain object");
    expect_true(receipt.blocked && !receipt.valid,
                "duplicate container insertion is fail-closed");
}

/* ------------------------------------------------------------------ */
/* DM2_LOAD_PROJECTILE_TO_HAND                                         */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t valid_ref;      /* the only ref the predicates accept */
    int chest;
    uint16_t chest_head;
    uint16_t chest_next;
    uint16_t cut_ref;
    uint16_t equipped_ref;
    int16_t equip_hand;
    int equips;
    int cuts;
} TestLoadFacts;

static int lp_missile_valid(void *ctx, int16_t hero, int16_t hand,
                            uint16_t ref)
{
    (void)hero;
    (void)hand;
    return ref != DM2_V1_OBJECT_TRANSFER_NULL &&
           ref == ((TestLoadFacts *)ctx)->valid_ref;
}

static int lp_command_valid(void *ctx, int16_t hero, uint16_t ref,
                            int16_t handcmd)
{
    (void)hero;
    (void)handcmd;
    return ref != DM2_V1_OBJECT_TRANSFER_NULL &&
           ref == ((TestLoadFacts *)ctx)->valid_ref;
}

static int lp_is_chest(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestLoadFacts *)ctx)->chest;
}

static int lp_chest_head(void *ctx, uint16_t chest, uint16_t *out_ref)
{
    (void)chest;
    *out_ref = ((TestLoadFacts *)ctx)->chest_head;
    return 1;
}

static int lp_next_link(void *ctx, uint16_t ref, uint16_t *out_ref)
{
    TestLoadFacts *facts = (TestLoadFacts *)ctx;

    *out_ref = ref == facts->chest_head ? facts->chest_next
                                        : DM2_V1_OBJECT_TRANSFER_END_MARKER;
    return 1;
}

static int lp_cut(void *ctx, uint16_t chest, uint16_t ref)
{
    TestLoadFacts *facts = (TestLoadFacts *)ctx;

    (void)chest;
    facts->cut_ref = ref;
    facts->cuts++;
    return 1;
}

static int lp_equip(void *ctx, int16_t hero, uint16_t ref, int16_t hand)
{
    TestLoadFacts *facts = (TestLoadFacts *)ctx;

    (void)hero;
    facts->equipped_ref = ref;
    facts->equip_hand = hand;
    facts->equips++;
    return 1;
}

static void load_callbacks_init(DM2_V1_LoadProjectileCallbacks *cb,
                                TestLoadFacts *facts)
{
    cb->is_missile_valid_to_launcher = lp_missile_valid;
    cb->is_item_valid_for_command = lp_command_valid;
    cb->is_container_chest = lp_is_chest;
    cb->chest_chain_head = lp_chest_head;
    cb->next_record_link = lp_next_link;
    cb->cut_record_from_chest = lp_cut;
    cb->equip_item_to_hand = lp_equip;
    cb->context = facts;
}

static void load_input_init(DM2_V1_LoadProjectileToHandInput *input,
                            TestParty *party,
                            int16_t *cooldown,
                            int16_t *cmd,
                            int16_t *defclass)
{
    party_init(party, &input->slots);
    input->hand_slot = 0;
    input->hero_cur_hp = 20;
    cooldown[0] = 9;
    cooldown[1] = 9;
    cmd[0] = DM2_V1_OBJECT_TRANSFER_HANDCMD_MISSILE;
    cmd[1] = -1;
    defclass[0] = 7;
    defclass[1] = 7;
    input->handcooldown = cooldown;
    input->handcmd = cmd;
    input->handdefenseclass = defclass;
}

static void test_load_projectile_to_hand(void)
{
    DM2_V1_LoadProjectileToHandInput input;
    DM2_V1_LoadProjectileCallbacks cb;
    DM2_V1_ObjectTransferReceipt receipt;
    TestParty party;
    TestLoadFacts facts;
    int16_t cooldown[2];
    int16_t cmd[2];
    int16_t defclass[2];

    /* 1. the quiver slot (item[12]) itself holds the ammunition. */
    memset(&facts, 0, sizeof(facts));
    facts.valid_ref = 0x4400u;
    facts.chest_head = DM2_V1_OBJECT_TRANSFER_END_MARKER;
    facts.chest_next = DM2_V1_OBJECT_TRANSFER_END_MARKER;
    load_callbacks_init(&cb, &facts);
    load_input_init(&input, &party, cooldown, cmd, defclass);
    party.items[12] = 0x4400u;

    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 1,
                "LOAD_PROJECTILE_TO_HAND reloads from the quiver slot");
    expect_true(facts.equipped_ref == 0x4400u && facts.equip_hand == 1,
                "the missile reload equips into the opposite hand");
    expect_true(party.items[12] == DM2_V1_OBJECT_TRANSFER_NULL,
                "the quiver slot is emptied by REMOVE_POSSESSION");
    expect_true(cooldown[0] == 0 && cmd[0] == -1 && defclass[0] == 0,
                "the hand bookkeeping fields are cleared");
    expect_true(receipt.valid && receipt.equipped &&
                    receipt.path ==
                        DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_SLOT &&
                    receipt.handcmd ==
                        DM2_V1_OBJECT_TRANSFER_HANDCMD_MISSILE &&
                    strcmp(receipt.symbol,
                           "LOAD_PROJECTILE_TO_HAND") == 0,
                "the reload receipt records the source path");

    /* 2. the ammunition sits inside the chest held in item[12]. */
    memset(&facts, 0, sizeof(facts));
    facts.valid_ref = 0x4501u;
    facts.chest = 1;
    facts.chest_head = 0x4500u;
    facts.chest_next = 0x4501u;
    load_callbacks_init(&cb, &facts);
    load_input_init(&input, &party, cooldown, cmd, defclass);
    party.items[12] = 0x7000u;

    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 1,
                "LOAD_PROJECTILE_TO_HAND reloads from the chest chain");
    expect_true(facts.cuts == 1 && facts.cut_ref == 0x4501u &&
                    facts.equipped_ref == 0x4501u,
                "the chest record is cut out and equipped");
    expect_true(receipt.path ==
                    DM2_V1_LOAD_PROJECTILE_PATH_FROM_CHEST_CHAIN,
                "the chest-chain path is receipted");

    /* 3. the pouch slots 7..9 are the last missile resort. */
    memset(&facts, 0, sizeof(facts));
    facts.valid_ref = 0x4601u;
    facts.chest_head = DM2_V1_OBJECT_TRANSFER_END_MARKER;
    load_callbacks_init(&cb, &facts);
    load_input_init(&input, &party, cooldown, cmd, defclass);
    party.items[12] = 0x7000u;
    party.items[8] = 0x4601u;

    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 1,
                "LOAD_PROJECTILE_TO_HAND reloads from the pouch slots");
    expect_true(facts.equipped_ref == 0x4601u &&
                    party.items[8] == DM2_V1_OBJECT_TRANSFER_NULL &&
                    receipt.path == DM2_V1_LOAD_PROJECTILE_PATH_FROM_POUCH,
                "the pouch slot is emptied and equipped");

    /* 4. nothing matches: the missile path simply gives up. */
    memset(&facts, 0, sizeof(facts));
    facts.valid_ref = 0x9999u;
    facts.chest_head = DM2_V1_OBJECT_TRANSFER_END_MARKER;
    load_callbacks_init(&cb, &facts);
    load_input_init(&input, &party, cooldown, cmd, defclass);
    party.items[12] = 0x7000u;
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 0,
                "an exhausted missile search equips nothing");
    expect_true(facts.equips == 0 &&
                    receipt.path == DM2_V1_LOAD_PROJECTILE_PATH_EXHAUSTED,
                "the exhausted path is receipted");

    /* 5. the 0x2a command equips into this very hand and falls back to
     * removing item[12] itself. */
    memset(&facts, 0, sizeof(facts));
    facts.valid_ref = 0x9999u;
    facts.chest_head = DM2_V1_OBJECT_TRANSFER_END_MARKER;
    load_callbacks_init(&cb, &facts);
    load_input_init(&input, &party, cooldown, cmd, defclass);
    cmd[0] = DM2_V1_OBJECT_TRANSFER_HANDCMD_SPELL;
    party.items[12] = 0x7000u;
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 1,
                "the 0x2a path falls back to the quiver slot itself");
    expect_true(facts.equipped_ref == 0x7000u && facts.equip_hand == 0,
                "the 0x2a reload equips into the acting hand");

    /* 6. an occupied target hand aborts. */
    load_input_init(&input, &party, cooldown, cmd, defclass);
    party.items[1] = 0x1111u;
    party.items[12] = 0x4400u;
    memset(&facts, 0, sizeof(facts));
    facts.valid_ref = 0x4400u;
    facts.chest_head = DM2_V1_OBJECT_TRANSFER_END_MARKER;
    load_callbacks_init(&cb, &facts);
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 0,
                "an occupied target hand aborts the reload");
    expect_true(receipt.path ==
                    DM2_V1_LOAD_PROJECTILE_PATH_TARGET_HAND_BUSY,
                "the busy target hand is receipted");

    /* 7. a dead hero returns after clearing the cooldown. */
    load_input_init(&input, &party, cooldown, cmd, defclass);
    input.hero_cur_hp = 0;
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 0,
                "a dead hero does not reload");
    expect_true(cooldown[0] == 0 && cmd[0] != -1 &&
                    receipt.path ==
                        DM2_V1_LOAD_PROJECTILE_PATH_HERO_DEAD,
                "the dead-hero return still cleared the hand cooldown");

    /* 8. a hand index outside 0..1 returns. */
    load_input_init(&input, &party, cooldown, cmd, defclass);
    input.hand_slot = 4;
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 0,
                "a hand index outside 0..1 does not reload");
    expect_true(receipt.path == DM2_V1_LOAD_PROJECTILE_PATH_BAD_HAND,
                "the out-of-range hand is receipted");

    /* 9. an unhandled command is a bounded no-op. */
    load_input_init(&input, &party, cooldown, cmd, defclass);
    cmd[0] = 0x11;
    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(&input, &cb, &receipt) == 0,
                "an unhandled hand command does not reload");
    expect_true(cmd[0] == -1 &&
                    receipt.path ==
                        DM2_V1_LOAD_PROJECTILE_PATH_UNHANDLED_CMD,
                "the captured command is still cleared");

    expect_true(dm2_v1_LOAD_PROJECTILE_TO_HAND(0, &cb, &receipt) == 0,
                "null input fails closed");
    expect_true(receipt.blocked, "null input is a blocked receipt");
}

int main(void)
{
    test_remove_possession();
    test_put_object_into_container();
    test_append_to_chain_primitive();
    test_load_projectile_to_hand();
    expect_true(strstr(dm2_v1_object_transfer_helpers_source_evidence(),
                       "REMOVE_POSSESSION:2485") != 0,
                "source evidence includes removal symbol");
    expect_true(strstr(dm2_v1_object_transfer_helpers_source_evidence(),
                       "LOAD_PROJECTILE_TO_HAND:3643") != 0,
                "source evidence includes projectile symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 object transfer helpers: ok");
    return 0;
}
