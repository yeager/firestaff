/*
 * test_nexus_v1_script_vm.c
 *
 * Data-free Nexus V1 provisional trigger-dispatch regression.
 * Source: docs/nexus_triggers.md and docs/nexus_sensors.md classify
 * SDDRVS.TSK as Saturn sound-driver data. Synthetic manual rules lock runtime
 * operand matching; canonical SLEV files are parsed only as bounded SH-2 task
 * headers and never enable fallback dispatch.
 */

#include "nexus_v1_script_vm.h"

#include <stdio.h>
#include <stdlib.h>
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

static void test_real_slev_task_profile_blocks_dispatch(void) {
    uint8_t slev[96];
    static const uint8_t task_header[] = {
        0x2f, 0xe6, 0xe2, 0x1a, 0xd3, 0x0e, 0x34, 0x23,
        0x4f, 0x22, 0x7f, 0xfc, 0x2f, 0x52, 0x8d, 0x02,
        0x23, 0x42, 0x44, 0x11, 0x89, 0x04, 0xe0, 0xff,
        0x7f, 0x04, 0x4f, 0x26, 0x00, 0x0b, 0x6e, 0xf6,
        0xd0, 0x08, 0x4e, 0x08
    };
    Nexus_ScriptVM vm;
    Nexus_ScriptRuntimeReceipt receipt;

    memset(slev, 0, sizeof(slev));
    memset(&receipt, 0, sizeof(receipt));
    memcpy(slev, task_header, sizeof(task_header));
    slev[52] = 0xa0; slev[53] = 0x10; /* branch-like word */
    slev[36] = 0x43; slev[37] = 0x0b; /* JSR @R3 */
    slev[40] = 0x41; slev[41] = 0x23;
    slev[48] = 0xe0; slev[49] = 0xff; /* immediate-like word */
    slev[64] = 0x00; slev[65] = 0x20; slev[66] = 0x27; slev[67] = 0x34;
    slev[68] = 0x00; slev[69] = 0x20; slev[70] = 0x28; slev[71] = 0x40;

    nexus_script_vm_init(&vm);
    CHECK(nexus_script_vm_load_canonical_level(&vm, 0, slev,
                                               (int)sizeof(slev), 1) == 0,
          "real-shaped SLEV task profile loads");
    CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0,
          "real-shaped SLEV task profile emits receipt");
    CHECK(receipt.status == NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT &&
          receipt.blocks_real_script_dispatch == 1 &&
          receipt.dispatch_enabled == 0 &&
          receipt.rules_loaded == 0,
          "real-shaped SLEV task profile does not enable fallback dispatch");
    CHECK(receipt.real_task_profile_supported == 1 &&
          receipt.real_task_word_count == 48 &&
          receipt.real_task_first_opcode == 0x2fe6 &&
          receipt.real_task_rts_count == 1 &&
          receipt.real_task_branch_count == 1 &&
          receipt.real_task_immediate_count == 3 &&
          receipt.real_task_jsr_count == 1 &&
          receipt.real_task_pc_relative_load_count == 2,
          "real-shaped SLEV task profile records SH-2 opcode shape");
    CHECK(receipt.real_task_literal_pointer_count == 2 &&
          receipt.real_task_first_literal_offset == 64 &&
          receipt.real_task_first_literal_address == 0x00202734 &&
          receipt.real_task_last_literal_address == 0x00202840,
          "real-shaped SLEV task profile records literal call operands");
    CHECK(receipt.real_task_header_supported == 1 &&
          receipt.real_task_header_size == 36 &&
          receipt.real_task_setup_immediate == 0x1a &&
          receipt.real_task_setup_immediate_provenance ==
              NEXUS_SLEV_SETUP_IMMEDIATE_SH2_MOV_R2 &&
          receipt.real_task_primary_literal_offset == 64 &&
          receipt.real_task_primary_literal_address == 0x00202734 &&
          receipt.real_task_primary_literal_provenance ==
              NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R3 &&
          receipt.real_task_aux_literal_offset == 68 &&
          receipt.real_task_aux_literal_address == 0x00202840 &&
          receipt.real_task_aux_literal_provenance ==
              NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R0,
          "real-shaped SLEV task receipt classifies bounded entry operands");

    slev[0] = 0x00;
    nexus_script_vm_init(&vm);
    CHECK(nexus_script_vm_load_canonical_level(&vm, 0, slev,
                                               (int)sizeof(slev), 1) == 0,
          "malformed SLEV task bytes load as candidate");
    CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0 &&
          receipt.real_task_profile_supported == 0 &&
          receipt.status == NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT,
          "malformed task bytes are not promoted to real-profile evidence");
}

static void test_optional_real_slev_corpus_profile(void) {
    const char *home = getenv("HOME");
    int seen = 0;
    int profiled = 0;
    int level;

    if (!home || home[0] == '\0') return;

    for (level = 0; level < 16; ++level) {
        char path[512];
        FILE *fp;
        long size;
        uint8_t *data;
        Nexus_ScriptVM vm;
        Nexus_ScriptRuntimeReceipt receipt;

        snprintf(path, sizeof(path),
                 "%s/.firestaff/data/nexus/SLEV%02d.BIN", home, level);
        fp = fopen(path, "rb");
        if (!fp) continue;
        if (fseek(fp, 0, SEEK_END) != 0) {
            fclose(fp);
            continue;
        }
        size = ftell(fp);
        if (size <= 0 || size > 65536L || fseek(fp, 0, SEEK_SET) != 0) {
            fclose(fp);
            continue;
        }
        data = (uint8_t *)malloc((size_t)size);
        if (!data) {
            fclose(fp);
            continue;
        }
        if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
            free(data);
            fclose(fp);
            continue;
        }
        fclose(fp);

        seen++;
        nexus_script_vm_init(&vm);
        CHECK(nexus_script_vm_load_canonical_level(&vm, level, data,
                                                   (int)size, 1) == 0,
              "optional real SLEV corpus file loads");
        CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0,
              "optional real SLEV corpus file emits receipt");
        if (receipt.real_task_profile_supported) {
            profiled++;
            CHECK(receipt.status ==
                      NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT &&
                  receipt.blocks_real_script_dispatch == 1 &&
                  receipt.dispatch_enabled == 0,
                  "optional real SLEV profile stays no-dispatch");
            CHECK(receipt.real_task_word_count == (int)(size / 2) &&
                  receipt.real_task_first_opcode == 0x2fe6 &&
                  receipt.real_task_rts_count > 0 &&
                  receipt.real_task_jsr_count > 0 &&
                  receipt.real_task_pc_relative_load_count > 0 &&
                  receipt.real_task_checksum16 != 0,
                  "optional real SLEV profile records SH-2 call/operand shape");
            CHECK(receipt.real_task_header_supported == 1 &&
                  receipt.real_task_header_size == 36 &&
                  receipt.real_task_primary_literal_offset >= 36 &&
                  receipt.real_task_aux_literal_offset >= 36 &&
                  receipt.real_task_primary_literal_address >= 0x00200000 &&
                  receipt.real_task_aux_literal_address >= 0x00200000 &&
                  receipt.real_task_setup_immediate_provenance ==
                      NEXUS_SLEV_SETUP_IMMEDIATE_SH2_MOV_R2 &&
                  receipt.real_task_primary_literal_provenance ==
                      NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R3 &&
                  receipt.real_task_aux_literal_provenance ==
                      NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R0,
                  "optional real SLEV corpus matches bounded entry provenance");
            if (receipt.real_task_literal_pointer_count > 0) {
                CHECK(receipt.real_task_first_literal_offset >= 0 &&
                      receipt.real_task_first_literal_address >= 0x00200000 &&
                      receipt.real_task_last_literal_address >= 0x00200000,
                      "optional real SLEV profile records literal table operands");
            }
        }
        free(data);
    }

    if (seen > 0) {
        CHECK(profiled == seen,
              "all staged real SLEV corpus files match the task profile");
    }
}

int main(void) {
    test_vm_local_handlers();
    test_event_operand_matching();
    test_once_only_manual_fire_and_unload();
    test_runtime_receipts_block_unparsed_real_source();
    test_real_slev_task_profile_blocks_dispatch();
    test_optional_real_slev_corpus_profile();

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_script_vm: %d failure(s)\n", g_failures);
        return 1;
    }

    puts("test_nexus_v1_script_vm: PASS");
    return 0;
}
