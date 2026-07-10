/*
 * test_nexus_v1_script_vm.c
 *
 * Data-free Nexus V1 provisional trigger-dispatch regression.
 * Source: docs/nexus_triggers.md and docs/nexus_sensors.md classify
 * SDDRVS.TSK as Saturn sound-driver data. Synthetic manual rules lock runtime
 * operand matching; the bounded SLEV envelope test proves parser-gated dispatch
 * without enabling fallback rules for unknown real SLEV*.BIN candidates.
 */

#include "nexus_v1_script_vm.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int count;
    Nexus_WorldOpcode last_opcode;
    int last_x;
    int last_y;
    int last_value;
    int last_level;
} Receipt;

static int g_failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        g_failures++; \
    } \
} while (0)

static void put_u16_le(uint8_t *p, int value) {
    unsigned int v = (unsigned int)(uint16_t)value;
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static void receipt_handler(const Nexus_ScriptAction *action, void *user_data) {
    Receipt *r = (Receipt *)user_data;
    if (!r || !action) return;
    r->count++;
    r->last_opcode = action->opcode;
    r->last_x = action->x;
    r->last_y = action->y;
    r->last_value = action->value;
    r->last_level = action->level;
}

static Nexus_ScriptRule *append_rule(Nexus_ScriptVM *vm,
                                     int rule_id,
                                     Nexus_WorldOpcode cond_opcode,
                                     int cond_x,
                                     int cond_y,
                                     int cond_value,
                                     Nexus_WorldOpcode action_opcode) {
    Nexus_ScriptRule *r;

    if (!vm || vm->rule_count >= NEXUS_SCRIPT_MAX_RULES) return NULL;

    r = &vm->rules[vm->rule_count++];
    memset(r, 0, sizeof(*r));
    r->rule_id = rule_id;
    r->enabled = 1;
    r->cond.opcode = cond_opcode;
    r->cond.x = cond_x;
    r->cond.y = cond_y;
    r->cond.value = cond_value;
    r->action.opcode = action_opcode;
    r->action.x = cond_x + 1;
    r->action.y = cond_y + 1;
    r->action.value = cond_value + 10;
    r->action.level = cond_value;
    return r;
}

static void test_vm_local_handlers(void) {
    Nexus_ScriptVM a;
    Nexus_ScriptVM b;
    Receipt ra;
    Receipt rb;

    memset(&ra, 0, sizeof(ra));
    memset(&rb, 0, sizeof(rb));
    nexus_script_vm_init(&a);
    nexus_script_vm_init(&b);
    nexus_script_vm_set_handler(&a, receipt_handler, &ra);
    nexus_script_vm_set_handler(&b, receipt_handler, &rb);

    (void)append_rule(&a, 101, NEXUS_OP_WHEN_LEVEL_LOADED,
                      0, 0, 3, NEXUS_OP_DISPLAY_MESSAGE);
    (void)append_rule(&b, 201, NEXUS_OP_WHEN_LEVEL_LOADED,
                      0, 0, 7, NEXUS_OP_SOUND);

    nexus_script_on_level_load(&a, 3);
    CHECK(ra.count == 1, "vm A fires its own level-load rule");
    CHECK(rb.count == 0, "vm B handler is not called by vm A");
    CHECK(ra.last_opcode == NEXUS_OP_DISPLAY_MESSAGE,
          "vm A dispatches its configured action");

    nexus_script_on_level_load(&b, 3);
    CHECK(rb.count == 0, "vm B ignores wrong level operand");

    nexus_script_on_level_load(&b, 7);
    CHECK(rb.count == 1, "vm B fires matching level operand");
    CHECK(rb.last_opcode == NEXUS_OP_SOUND,
          "vm B dispatches its configured action");
}

static void test_event_operand_matching(void) {
    Nexus_ScriptVM vm;
    Receipt r;

    memset(&r, 0, sizeof(r));
    nexus_script_vm_init(&vm);
    nexus_script_vm_set_handler(&vm, receipt_handler, &r);

    (void)append_rule(&vm, 1, NEXUS_OP_WHEN_PARTY_ON_XY,
                      4, 5, 2, NEXUS_OP_TELEPORT);
    (void)append_rule(&vm, 2, NEXUS_OP_WHEN_CHAMPION_HAS,
                      0, 0, 44, NEXUS_OP_GIVE_ITEM);
    (void)append_rule(&vm, 3, NEXUS_OP_WHEN_CREATURE_DEAD,
                      0, 0, 9, NEXUS_OP_AWARD_XP);
    (void)append_rule(&vm, 4, NEXUS_OP_WHEN_DOOR_OPEN,
                      8, 9, 0, NEXUS_OP_TRIGGER_DOOR);
    (void)append_rule(&vm, 5, NEXUS_OP_WHEN_ITEM_USED,
                      0, 0, 12, NEXUS_OP_SET_FLAG);

    nexus_script_on_party_move(&vm, 4, 5, 1);
    nexus_script_on_party_move(&vm, 4, 6, 2);
    CHECK(r.count == 0, "party XY rule rejects wrong level and y");
    nexus_script_on_party_move(&vm, 4, 5, 2);
    CHECK(r.count == 1 && r.last_opcode == NEXUS_OP_TELEPORT,
          "party XY rule matches x/y/level");

    nexus_script_on_champion_item(&vm, 0, 43);
    CHECK(r.count == 1, "champion item rule rejects wrong item");
    nexus_script_on_champion_item(&vm, 0, 44);
    CHECK(r.count == 2 && r.last_opcode == NEXUS_OP_GIVE_ITEM,
          "champion item rule matches item id");

    nexus_script_on_creature_dead(&vm, 8);
    CHECK(r.count == 2, "creature-dead rule rejects wrong type");
    nexus_script_on_creature_dead(&vm, 9);
    CHECK(r.count == 3 && r.last_opcode == NEXUS_OP_AWARD_XP,
          "creature-dead rule matches creature type");

    nexus_script_on_door_change(&vm, 8, 9, 0);
    nexus_script_on_door_change(&vm, 8, 8, 1);
    CHECK(r.count == 3, "door-open rule rejects closed or wrong position");
    nexus_script_on_door_change(&vm, 8, 9, 1);
    CHECK(r.count == 4 && r.last_opcode == NEXUS_OP_TRIGGER_DOOR,
          "door-open rule matches open state and x/y");

    nexus_script_on_item_used(&vm, 11);
    CHECK(r.count == 4, "item-used rule rejects wrong item");
    nexus_script_on_item_used(&vm, 12);
    CHECK(r.count == 5 && r.last_opcode == NEXUS_OP_SET_FLAG,
          "item-used rule matches item id");
}

static void test_once_only_manual_fire_and_unload(void) {
    Nexus_ScriptVM vm;
    Nexus_ScriptRule *r0;
    Receipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    nexus_script_vm_init(&vm);
    nexus_script_vm_set_handler(&vm, receipt_handler, &receipt);

    r0 = append_rule(&vm, 99, NEXUS_OP_WHEN_PARTY_ON_XY,
                     2, 3, 4, NEXUS_OP_END_GAME);
    CHECK(r0 != NULL, "rule allocation succeeds");
    if (!r0) return;

    r0->once_only = 1;
    CHECK(nexus_script_vm_fire_rule(&vm, 99) == 1,
          "manual fire bypasses condition and dispatches");
    CHECK(receipt.count == 1 && receipt.last_opcode == NEXUS_OP_END_GAME,
          "manual fire records action");
    CHECK(nexus_script_vm_fire_rule(&vm, 99) == 0,
          "manual fire honors once-only after first dispatch");

    r0->enabled = 0;
    r0->once_only = 0;
    r0->fired_count = 0;
    CHECK(nexus_script_vm_fire_rule(&vm, 99) == 0,
          "manual fire honors disabled rule");

    nexus_script_vm_unload(&vm);
    CHECK(vm.rule_count == 0, "unload clears rules");
    CHECK(vm.current_level == -1, "unload clears current level");
    CHECK(nexus_script_vm_fire_rule(&vm, 99) == 0,
          "manual fire misses unloaded rule");
}

static void test_runtime_receipts_block_unparsed_real_source(void) {
    static const uint8_t fake_slev[] = { 'S', 'L', 'E', 'V', 0, 1, 2, 3 };
    Nexus_ScriptVM vm;
    Nexus_ScriptRuntimeReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    nexus_script_vm_init(&vm);
    CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0,
          "empty script VM emits runtime receipt");
    CHECK(receipt.status == NEXUS_SCRIPT_RUNTIME_NO_SOURCE &&
          receipt.fallback_visuals_permitted == 0,
          "empty script VM does not allow fallback dispatch");

    CHECK(nexus_script_vm_load_level(&vm, 2, fake_slev, (int)sizeof(fake_slev)) == 0,
          "script VM accepts bounded real candidate bytes");
    CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0,
          "script VM emits receipt after candidate load");
    CHECK(receipt.status == NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT,
          "unparsed real script source is blocked");
    CHECK(receipt.candidate_source_loaded == 1 &&
          receipt.candidate_source_bytes == (int)sizeof(fake_slev) &&
          receipt.rules_loaded == 0 &&
          receipt.dispatch_enabled == 0,
          "script receipt preserves source bytes without synthetic rules");
    CHECK(receipt.blocks_real_script_dispatch == 1 &&
          receipt.fallback_visuals_permitted == 0,
          "script receipt forbids fallback dispatch for unsupported real source");
    CHECK(strcmp(nexus_script_runtime_status_name(receipt.status),
                 "blocked-unsupported-format") == 0,
          "script receipt has stable blocked status name");
}

static void test_slev_rule_table_loads_and_dispatches(void) {
    enum { header_size = 8, record_size = 32, rule_count = 2 };
    uint8_t slev[header_size + record_size * rule_count];
    Nexus_ScriptVM vm;
    Nexus_ScriptRuntimeReceipt receipt;
    Receipt r;

    memset(slev, 0, sizeof(slev));
    memset(&receipt, 0, sizeof(receipt));
    memset(&r, 0, sizeof(r));

    slev[0] = 'S';
    slev[1] = 'L';
    slev[2] = 'E';
    slev[3] = 'V';
    slev[4] = 1;
    slev[5] = record_size;
    put_u16_le(&slev[6], rule_count);

    slev[8] = NEXUS_OP_WHEN_PARTY_ON_XY;
    slev[9] = NEXUS_OP_TELEPORT;
    slev[10] = 1; /* once-only */
    put_u16_le(&slev[12], 300);
    put_u16_le(&slev[14], 4);
    put_u16_le(&slev[16], 5);
    put_u16_le(&slev[18], 2);
    put_u16_le(&slev[26], 9);
    put_u16_le(&slev[28], 10);
    put_u16_le(&slev[32], 7);

    slev[40] = NEXUS_OP_WHEN_LEVEL_LOADED;
    slev[41] = NEXUS_OP_DISPLAY_MESSAGE;
    put_u16_le(&slev[44], 301);
    put_u16_le(&slev[50], 3);
    put_u16_le(&slev[68], 77);

    nexus_script_vm_init(&vm);
    nexus_script_vm_set_handler(&vm, receipt_handler, &r);
    CHECK(nexus_script_vm_load_level(&vm, 2, slev, (int)sizeof(slev)) == 0,
          "SLEV rule table loads");
    CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0,
          "SLEV rule table emits receipt");
    CHECK(receipt.status == NEXUS_SCRIPT_RUNTIME_READY_PARSED &&
          receipt.parser_supported == 1 &&
          receipt.dispatch_enabled == 1,
          "SLEV rule table receipt is ready and dispatch-enabled");
    CHECK(receipt.parsed_record_size == record_size &&
          receipt.parsed_rule_count == rule_count &&
          receipt.rules_loaded == rule_count,
          "SLEV receipt records parsed table shape");

    nexus_script_on_party_move(&vm, 4, 5, 2);
    CHECK(r.count == 1 && r.last_opcode == NEXUS_OP_TELEPORT &&
          r.last_x == 9 && r.last_y == 10 && r.last_level == 7,
          "parsed party-XY rule dispatches teleport action");
    nexus_script_on_party_move(&vm, 4, 5, 2);
    CHECK(r.count == 1, "parsed once-only rule fires once");

    nexus_script_on_level_load(&vm, 2);
    CHECK(r.count == 1, "parsed level-load rule rejects wrong level");
    nexus_script_on_level_load(&vm, 3);
    CHECK(r.count == 2 && r.last_opcode == NEXUS_OP_DISPLAY_MESSAGE,
          "parsed level-load rule dispatches message action");
}

int main(void) {
    test_vm_local_handlers();
    test_event_operand_matching();
    test_once_only_manual_fire_and_unload();
    test_runtime_receipts_block_unparsed_real_source();
    test_slev_rule_table_loads_and_dispatches();

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_script_vm: %d failure(s)\n", g_failures);
        return 1;
    }

    puts("test_nexus_v1_script_vm: PASS");
    return 0;
}
