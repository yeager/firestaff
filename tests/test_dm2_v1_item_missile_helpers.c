#include "dm2_v1_item_missile_helpers.h"

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

/* ------------------------------------------------------------------ */
/* DM2_IS_MISSILE_VALID_TO_LAUNCHER                                    */
/* ------------------------------------------------------------------ */

static void test_missile_valid_to_launcher(void)
{
    DM2_V1_MissileLauncherFacts facts;
    DM2_V1_ItemMissileReceipt receipt;

    /* Launcher word carries the 0x8000 launcher bit plus ammo class
     * bit 0x0004; the arrow shares class bit 0x0004. */
    facts.launcher_ref = 0x3001u;
    facts.missile_ref = 0x4001u;
    facts.launcher_dbspec_word5 = 0x8004u;
    facts.missile_dbspec_word5 = 0x0004u;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 1,
                "IS_MISSILE_VALID_TO_LAUNCHER accepts overlapping class");
    expect_true(receipt.valid && receipt.result == 1 && !receipt.blocked &&
                    strcmp(receipt.symbol,
                           "IS_MISSILE_VALID_TO_LAUNCHER") == 0,
                "launcher predicate receipt records source symbol");

    /* Disjoint ammo class bits reject. */
    facts.missile_dbspec_word5 = 0x0008u;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 0,
                "disjoint ammunition class bits reject");

    /* A launcher without the 0x8000 bit is not a launcher at all. */
    facts.missile_dbspec_word5 = 0x0004u;
    facts.launcher_dbspec_word5 = 0x0004u;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 0,
                "held item without the 0x8000 launcher bit rejects");

    /* A candidate that is itself a launcher rejects. */
    facts.launcher_dbspec_word5 = 0x8004u;
    facts.missile_dbspec_word5 = 0x8004u;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 0,
                "a launcher candidate is not valid ammunition");

    facts.missile_dbspec_word5 = 0x0004u;
    facts.launcher_ref = DM2_V1_ITEM_MISSILE_NULL_REF;
    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(&facts, &receipt) == 0,
                "IS_MISSILE_VALID_TO_LAUNCHER blocks empty hand slot");
    expect_true(receipt.blocked, "empty hand slot is fail-closed");

    expect_true(dm2_v1_IS_MISSILE_VALID_TO_LAUNCHER(0, &receipt) == 0,
                "null facts reject");
    expect_true(receipt.blocked && !receipt.valid, "null facts fail closed");
}

/* ------------------------------------------------------------------ */
/* DM2_RETRIEVE_ITEM_BONUS                                             */
/* ------------------------------------------------------------------ */

static void test_retrieve_item_bonus(void)
{
    DM2_V1_ItemBonusFacts facts;
    DM2_V1_ItemMissileReceipt receipt;

    facts.item_ref = 0x2001u;
    facts.dbspec_index = 0x33u;
    facts.dbspec_word = 0x800cu; /* bit 0x4000 clear, 0x8000 gate set */
    facts.select_flag = 0;
    facts.mode = 2;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 12,
                "RETRIEVE_ITEM_BONUS returns the sign-extended low byte");
    expect_true(receipt.valid && receipt.result == 12 &&
                    strcmp(receipt.symbol, "RETRIEVE_ITEM_BONUS") == 0,
                "item bonus receipt records source symbol");

    /* Negative low byte sign-extends. */
    facts.dbspec_word = 0x80f8u;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == -8,
                "negative low byte sign-extends");

    /* A negative mode word flips the sign of the result. */
    facts.dbspec_word = 0x800cu;
    facts.mode = -2;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == -12,
                "negative mode negates the bonus");

    /* Zero DBSPEC word yields zero. */
    facts.dbspec_word = 0u;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 0,
                "zero DBSPEC word yields zero");

    /* Bit 0x4000 clear with a zero selector requires the 0x8000 bit. */
    facts.dbspec_word = 0x000cu;
    facts.mode = 2;
    facts.select_flag = 0;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 0,
                "zero selector requires the 0x8000 bit");
    facts.select_flag = 1;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 12,
                "a nonzero selector bypasses the 0x8000 gate");

    /* Bit 0x4000 set gates the mode word to -2, 2 or 3. */
    facts.dbspec_word = 0x400cu;
    facts.select_flag = 0;
    facts.mode = 1;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 0,
                "0x4000 bonus rejects an ungated mode");
    facts.mode = 3;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 12,
                "0x4000 bonus accepts mode 3");
    facts.mode = -2;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == -12,
                "0x4000 bonus accepts mode -2 and negates");

    facts.item_ref = DM2_V1_ITEM_MISSILE_NULL_REF;
    expect_true(dm2_v1_RETRIEVE_ITEM_BONUS(&facts, &receipt) == 0,
                "RETRIEVE_ITEM_BONUS blocks null object");
    expect_true(receipt.blocked && !receipt.valid,
                "null bonus facts are fail-closed");
}

/* ------------------------------------------------------------------ */
/* DM2_GET_MISSILE_REF_OF_MINION                                       */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t ref;
    uint16_t word0;
    uint16_t word2;
    uint16_t next;
} TestRecord;

static TestRecord g_records[] = {
    /* minion: word0 non-null, word2 = chain head */
    {0x0100u, 0x0001u, 0x3801u, DM2_V1_ITEM_MISSILE_END_MARKER},
    /* 0x3801: DB index bits 10-13 = 0xe, word2 = 0x55 */
    {0x3801u, 0x0001u, 0x0055u, 0x1802u},
    /* 0x1802: DB index 6 -> skipped */
    {0x1802u, 0x0001u, 0x0099u, 0x3803u},
    /* 0x3803: DB index 0xe, word2 = 0x77 */
    {0x3803u, 0x0001u, 0x0077u, DM2_V1_ITEM_MISSILE_END_MARKER},
    /* empty minion */
    {0x0200u, 0xffffu, 0x3801u, DM2_V1_ITEM_MISSILE_END_MARKER}
};

static TestRecord *test_lookup(uint16_t ref)
{
    size_t i;

    for (i = 0u; i < sizeof(g_records) / sizeof(g_records[0]); ++i) {
        if (g_records[i].ref == ref) {
            return &g_records[i];
        }
    }
    return 0;
}

static int test_record_word(void *context, uint16_t ref, unsigned offset,
                            uint16_t *out_word)
{
    TestRecord *record = test_lookup(ref);

    (void)context;
    if (!record) {
        return 0;
    }
    *out_word = offset == 0u ? record->word0 : record->word2;
    return 1;
}

static int test_next_link(void *context, uint16_t ref, uint16_t *out_ref)
{
    TestRecord *record = test_lookup(ref);

    (void)context;
    if (!record) {
        return 0;
    }
    *out_ref = record->next;
    return 1;
}

static int test_self_loop_next(void *context, uint16_t ref,
                               uint16_t *out_ref)
{
    (void)context;
    *out_ref = ref;
    return 1;
}

static void test_get_missile_ref_of_minion(void)
{
    DM2_V1_RecordChainAccess chain;
    DM2_V1_ItemMissileReceipt receipt;

    chain.record_word = test_record_word;
    chain.next_link = test_next_link;
    chain.context = 0;
    chain.max_steps = 0u;

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(
                    &chain, 0x0100u, DM2_V1_ITEM_MISSILE_NULL_REF,
                    &receipt) == 0x3801u,
                "wildcard filter returns the first DB-14 chain node");
    expect_true(receipt.valid && receipt.result == 0x3801 &&
                    strcmp(receipt.symbol,
                           "GET_MISSILE_REF_OF_MINION") == 0,
                "minion missile receipt records the resolved record");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(&chain, 0x0100u, 0x0077u,
                                                 &receipt) == 0x3803u,
                "an exact filter skips non-matching DB-14 nodes");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(&chain, 0x0100u, 0x1234u,
                                                 &receipt) ==
                    DM2_V1_ITEM_MISSILE_NULL_REF,
                "an unmatched filter walks to the end marker");
    expect_true(receipt.valid && !receipt.blocked,
                "an exhausted chain is a bounded negative result");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(
                    &chain, 0x0200u, DM2_V1_ITEM_MISSILE_NULL_REF,
                    &receipt) == DM2_V1_ITEM_MISSILE_NULL_REF,
                "an empty minion record yields no missile");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(
                    &chain, DM2_V1_ITEM_MISSILE_END_MARKER,
                    DM2_V1_ITEM_MISSILE_NULL_REF, &receipt) ==
                    DM2_V1_ITEM_MISSILE_NULL_REF,
                "the end-marker minion ref yields no missile");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(
                    &chain, 0x0999u, DM2_V1_ITEM_MISSILE_NULL_REF,
                    &receipt) == DM2_V1_ITEM_MISSILE_NULL_REF,
                "an unresolvable minion record fails closed");
    expect_true(receipt.blocked && !receipt.valid,
                "unresolvable records are fail-closed");

    /* A self-looping chain is bounded. */
    chain.next_link = test_self_loop_next;
    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(&chain, 0x0100u, 0x1234u,
                                                 &receipt) ==
                    DM2_V1_ITEM_MISSILE_NULL_REF,
                "a self-looping chain terminates");
    expect_true(receipt.blocked &&
                    receipt.steps <= (int)DM2_V1_ITEM_MISSILE_MAX_CHAIN + 1,
                "a corrupt chain is bounded and fail-closed");

    expect_true(dm2_v1_GET_MISSILE_REF_OF_MINION(0, 0x0100u, 0xffffu,
                                                 &receipt) ==
                    DM2_V1_ITEM_MISSILE_NULL_REF,
                "a missing chain access fails closed");
}

/* ------------------------------------------------------------------ */
/* DM2_IS_ITEM_HAND_ACTIVABLE                                          */
/* ------------------------------------------------------------------ */

typedef struct {
    int moneybox;
    int chest;
    int map;
    uint8_t cls1;
    uint8_t cls2;
    uint8_t herotype;
    int loadable_cmd;      /* only this cmd is loadable (-1 = none) */
    int16_t action_word;   /* cmdstr field 2 */
    int16_t gate_word;     /* cmdstr field 0x11 */
    int16_t requirement;   /* cmdstr field 8 */
    int16_t skill;         /* cmdstr field 0 */
    int16_t min_level;     /* cmdstr field 1 */
    int16_t charge;
    int16_t pouch_pos;
    int16_t skill_level;
    int action_applies;
} TestHandFacts;

static int hf_moneybox(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestHandFacts *)ctx)->moneybox;
}

static int hf_chest(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestHandFacts *)ctx)->chest;
}

static int hf_map(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestHandFacts *)ctx)->map;
}

static uint8_t hf_cls1(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestHandFacts *)ctx)->cls1;
}

static uint8_t hf_cls2(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestHandFacts *)ctx)->cls2;
}

static uint8_t hf_herotype(void *ctx, int16_t hero)
{
    (void)hero;
    return ((TestHandFacts *)ctx)->herotype;
}

static int hf_loadable(void *ctx, uint8_t cls1, uint8_t cls2,
                       uint8_t group, uint8_t cmd)
{
    (void)cls1;
    (void)cls2;
    (void)group;
    return ((TestHandFacts *)ctx)->loadable_cmd == (int)cmd;
}

static int16_t hf_cmdstr(void *ctx, uint8_t cls1, uint8_t cls2,
                         uint8_t cmd, uint8_t field)
{
    TestHandFacts *facts = (TestHandFacts *)ctx;

    (void)cls1;
    (void)cls2;
    (void)cmd;
    switch (field) {
    case 0u:
        return facts->skill;
    case 1u:
        return facts->min_level;
    case 2u:
        return facts->action_word;
    case 8u:
        return facts->requirement;
    case 0x11u:
        return facts->gate_word;
    default:
        return 0;
    }
}

static int hf_applies(void *ctx, int16_t action_word, uint16_t ref)
{
    (void)action_word;
    (void)ref;
    return ((TestHandFacts *)ctx)->action_applies;
}

static int16_t hf_charge(void *ctx, uint16_t ref)
{
    (void)ref;
    return ((TestHandFacts *)ctx)->charge;
}

static int16_t hf_pouch(void *ctx, int16_t hero, int16_t slot)
{
    (void)hero;
    (void)slot;
    return ((TestHandFacts *)ctx)->pouch_pos;
}

static int16_t hf_skill_level(void *ctx, int16_t hero, int16_t skill,
                              int16_t mode)
{
    (void)hero;
    (void)skill;
    (void)mode;
    return ((TestHandFacts *)ctx)->skill_level;
}

static void hand_callbacks_init(DM2_V1_HandActivableCallbacks *cb,
                                TestHandFacts *facts)
{
    cb->is_container_moneybox = hf_moneybox;
    cb->is_container_chest = hf_chest;
    cb->is_container_map = hf_map;
    cb->query_cls1 = hf_cls1;
    cb->query_cls2 = hf_cls2;
    cb->hero_type = hf_herotype;
    cb->gdat_entry_if_loadable = hf_loadable;
    cb->cmdstr_entry = hf_cmdstr;
    cb->action_applies_to_item = hf_applies;
    cb->item_charge = hf_charge;
    cb->find_pouch_or_scabbard_pos = hf_pouch;
    cb->player_skill_level = hf_skill_level;
    cb->context = facts;
}

static void test_item_hand_activable(void)
{
    DM2_V1_HandActivableCallbacks cb;
    DM2_V1_ItemMissileReceipt receipt;
    DM2_V1_HandActionEntry actions[DM2_V1_ITEM_HAND_ACTIVABLE_MAX_ACTIONS];
    size_t count = 0u;
    TestHandFacts facts;

    memset(&facts, 0, sizeof(facts));
    facts.cls1 = 0x20u;
    facts.cls2 = 0x03u;
    facts.herotype = 0x05u;
    facts.loadable_cmd = 9;
    facts.action_word = 0x0a;
    facts.gate_word = 0;
    facts.requirement = 0;
    facts.skill = 4;
    facts.min_level = 2;
    facts.charge = 3;
    facts.pouch_pos = 0;
    facts.skill_level = 5;
    facts.action_applies = 1;
    hand_callbacks_init(&cb, &facts);

    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 1,
                "IS_ITEM_HAND_ACTIVABLE collects a qualifying command");
    expect_true(count == 1u && actions[0].cls1 == 0x20u &&
                    actions[0].cls2 == 0x03u && actions[0].cmd == 9u,
                "the collected entry mirrors the ddat.v1e0b40 triple");
    expect_true(receipt.valid && receipt.result == 1 &&
                    receipt.action_count == 1 &&
                    strcmp(receipt.symbol, "IS_ITEM_HAND_ACTIVABLE") == 0,
                "hand activable receipt records the collected count");

    /* Skill level below the command's requirement drops the entry. */
    facts.skill_level = 1;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 0,
                "insufficient skill level drops the command");
    expect_true(count == 0u, "no actions collected below the skill gate");
    facts.skill_level = 5;

    /* Field 0x11 must be unset or name this very slot. */
    facts.gate_word = 3;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 0,
                "a slot-gated command is dropped for another slot");
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 2, actions,
                                              3u, &count, &receipt) == 1,
                "a slot-gated command is kept for its own slot");
    facts.gate_word = 0;

    /* Charge gating: requirement 0x12 needs an empty item. */
    facts.requirement = 0x12;
    facts.charge = 3;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 0,
                "the 0x12 requirement needs a discharged item");
    facts.charge = 0;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 1,
                "a discharged item satisfies the 0x12 requirement");

    /* 0x10/0x11 requirements collapse to a single charge. */
    facts.requirement = 0x10;
    facts.charge = 0;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 0,
                "a 0x10 requirement needs at least one charge");
    facts.charge = 1;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 1,
                "one charge satisfies the 0x10 requirement");
    facts.requirement = 0;

    /* Containers short-circuit to activable. */
    facts.moneybox = 1;
    facts.loadable_cmd = -1;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 1,
                "a moneybox container is always activable");
    facts.moneybox = 0;
    facts.chest = 1;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 1,
                "a chest container is always activable");
    facts.chest = 0;
    facts.map = 1;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 1,
                "a map container is always activable");
    facts.map = 0;

    /* No loadable command at all: not activable. */
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(&cb, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 0,
                "an item with no loadable command is not activable");

    /* The 0xffff sentinel switches to the hero's own action set with
     * cls1 = 0x16 and cls2 = herotype. */
    facts.loadable_cmd = 10;
    facts.action_word = 0x11;
    facts.pouch_pos = 0;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(
                    &cb, 0, DM2_V1_ITEM_MISSILE_NULL_REF, 0, actions, 3u,
                    &count, &receipt) == 1,
                "the hero-action mode collects the pouch command");
    expect_true(count == 1u &&
                    actions[0].cls1 ==
                        (uint8_t)DM2_V1_ITEM_HAND_ACTIVABLE_HERO_CLS1 &&
                    actions[0].cls2 == 0x05u,
                "hero-action mode substitutes cls1 0x16 and the herotype");
    facts.pouch_pos = -1;
    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(
                    &cb, 0, DM2_V1_ITEM_MISSILE_NULL_REF, 0, actions, 3u,
                    &count, &receipt) == 0,
                "an absent pouch/scabbard drops the 0x11 command");

    expect_true(dm2_v1_IS_ITEM_HAND_ACTIVABLE(0, 0, 0x1234u, 0, actions,
                                              3u, &count, &receipt) == 0,
                "missing callbacks fail closed");
    expect_true(receipt.blocked && !receipt.valid,
                "missing callbacks are a blocked receipt");
}

int main(void)
{
    test_missile_valid_to_launcher();
    test_retrieve_item_bonus();
    test_get_missile_ref_of_minion();
    test_item_hand_activable();
    expect_true(strstr(dm2_v1_item_missile_helpers_source_evidence(),
                       "IS_ITEM_HAND_ACTIVABLE:4562") != 0,
                "source evidence includes hand activable symbol");
    expect_true(strstr(dm2_v1_item_missile_helpers_source_evidence(),
                       "GET_MISSILE_REF_OF_MINION:1449") != 0,
                "source evidence includes minion missile symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 item/missile helpers: ok");
    return 0;
}
