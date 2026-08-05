/*
 * test_nexus_v1_script_vm.c
 *
 * Data-free Nexus V1 provisional trigger-dispatch regression.
 * Source: docs/nexus_triggers.md and docs/nexus_sensors.md classify
 * SDDRVS.TSK as Saturn sound-driver data. Manual condition/action fixtures
 * must remain inert; canonical SLEV files are parsed only as bounded SH-2
 * task headers and never enable fallback dispatch.
 */

#include "nexus_v1_engine.h"

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

static void test_manual_rules_cannot_enable_runtime_dispatch(void) {
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
    CHECK(ra.count == 0, "manual level-load rule stays inert");
    CHECK(rb.count == 0, "vm B handler is not called by vm A");

    nexus_script_on_level_load(&b, 3);
    CHECK(rb.count == 0, "vm B ignores wrong level operand");

    nexus_script_on_level_load(&b, 7);
    CHECK(rb.count == 0, "matching manual rule cannot dispatch audio");
}

static void test_forged_parser_flags_cannot_enable_dispatch(void) {
    Nexus_ScriptVM vm;
    Nexus_ScriptRule *rule;
    Receipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    nexus_script_vm_init(&vm);
    nexus_script_vm_set_handler(&vm, receipt_handler, &receipt);
    rule = append_rule(&vm, 301, NEXUS_OP_WHEN_LEVEL_LOADED,
                       0, 0, 4, NEXUS_OP_DISPLAY_MESSAGE);
    CHECK(rule != NULL, "forged-dispatch fixture allocates");
    if (!rule) return;

    /* Even a caller that forges every existing parser/profile bit still lacks
     * the authenticated Saturn event trace and must remain inert. */
    vm.candidate_source_loaded = 1;
    vm.canonical_source_verified = 1;
    vm.real_task_profile_supported = 1;
    vm.parser_supported = 1;
    vm.dispatch_enabled = 1;
    vm.parsed_rule_count = 1;
    nexus_script_on_level_load(&vm, 4);
    CHECK(vm.source_dispatch_trace_verified == 0 && receipt.count == 0,
          "forged parser flags cannot dispatch without Saturn trace proof");
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
    CHECK(r.count == 0, "party XY fixture cannot teleport at runtime");

    nexus_script_on_champion_item(&vm, 0, 43);
    CHECK(r.count == 0, "champion item rule rejects wrong item");
    nexus_script_on_champion_item(&vm, 0, 44);
    CHECK(r.count == 0, "item fixture cannot grant an unproven item");

    nexus_script_on_creature_dead(&vm, 8);
    CHECK(r.count == 0, "creature-dead rule rejects wrong type");
    nexus_script_on_creature_dead(&vm, 9);
    CHECK(r.count == 0, "creature fixture cannot award unproven XP");

    nexus_script_on_door_change(&vm, 8, 9, 0);
    nexus_script_on_door_change(&vm, 8, 8, 1);
    CHECK(r.count == 0, "door-open rule rejects closed or wrong position");
    nexus_script_on_door_change(&vm, 8, 9, 1);
    CHECK(r.count == 0, "door fixture cannot change a square at runtime");

    nexus_script_on_item_used(&vm, 11);
    CHECK(r.count == 0, "item-used rule rejects wrong item");
    nexus_script_on_item_used(&vm, 12);
    CHECK(r.count == 0, "item fixture cannot set an unproven flag");
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
    CHECK(nexus_script_vm_fire_rule(&vm, 99) == 0,
          "manual fire cannot bypass missing Saturn dispatch proof");
    CHECK(receipt.count == 0,
          "manual fire does not record a synthetic action");
    CHECK(nexus_script_vm_fire_rule(&vm, 99) == 0,
          "manual fire remains inert after an attempted dispatch");

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
    CHECK(receipt.source_dispatch_trace_verified == 0,
          "real SLEV receipt exposes missing Saturn dispatch trace");
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
          receipt.real_task_primary_literal_instruction_offset == 4 &&
          receipt.real_task_primary_literal_displacement == 0x0e &&
          receipt.real_task_primary_literal_offset == 64 &&
          receipt.real_task_primary_literal_address == 0x00202734 &&
          receipt.real_task_primary_literal_provenance ==
              NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R3 &&
          receipt.real_task_aux_literal_instruction_offset == 32 &&
          receipt.real_task_aux_literal_displacement == 0x08 &&
          receipt.real_task_aux_literal_offset == 68 &&
          receipt.real_task_aux_literal_address == 0x00202840 &&
          receipt.real_task_aux_literal_provenance ==
              NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R0 &&
          receipt.real_task_rts_instruction_offset == 28 &&
          receipt.real_task_post_rts_instruction_offset == 30 &&
          receipt.real_task_post_rts_instruction_word == 0x6ef6,
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

static void test_engine_owned_slev_profile_route_stays_blocked(void) {
    Nexus_V1_Engine engine;
    Nexus_V1_LevelScriptRouteReceipt route;
    uint8_t slev[96];
    static const uint8_t task_header[] = {
        0x2f, 0xe6, 0xe2, 0x1a, 0xd3, 0x0e, 0x34, 0x23,
        0x4f, 0x22, 0x7f, 0xfc, 0x2f, 0x52, 0x8d, 0x02,
        0x23, 0x42, 0x44, 0x11, 0x89, 0x04, 0xe0, 0xff,
        0x7f, 0x04, 0x4f, 0x26, 0x00, 0x0b, 0x6e, 0xf6,
        0xd0, 0x08, 0x4e, 0x08
    };

    memset(&engine, 0, sizeof(engine));
    memset(slev, 0, sizeof(slev));
    memcpy(slev, task_header, sizeof(task_header));
    slev[52] = 0xa0;
    slev[53] = 0x10;
    slev[36] = 0x43;
    slev[37] = 0x0b;
    slev[40] = 0x41;
    slev[41] = 0x23;
    slev[48] = 0xe0;
    slev[49] = 0xff;
    slev[64] = 0x00;
    slev[65] = 0x20;
    slev[66] = 0x27;
    slev[67] = 0x34;
    slev[68] = 0x00;
    slev[69] = 0x20;
    slev[70] = 0x28;
    slev[71] = 0x40;

    engine.level_loaded = 1;
    engine.level_aux_runtime_receipt.level_index = 0;
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 1;
    nexus_script_vm_init(&engine.script_vm);
    CHECK(nexus_script_vm_load_canonical_level(&engine.script_vm, 0, slev,
                                               (int)sizeof(slev), 1) == 0,
          "engine-owned canonical SLEV profile loads");
    memset(&route, 0, sizeof(route));
    CHECK(nexus_v1_current_level_script_route_receipt(&engine, &route) == 1 &&
          route.status == NEXUS_V1_LEVEL_SCRIPT_ROUTE_BOUND_TASK_PROFILE &&
          route.level_index == 0 && route.canonical_slev_source_verified &&
          route.candidate_source_bytes == (int)sizeof(slev) &&
          route.task_header_size == 36 && route.task_word_count == 48 &&
          route.first_opcode == 0x2fe6 && route.setup_immediate == 0x1a &&
          route.setup_immediate_provenance ==
              NEXUS_SLEV_SETUP_IMMEDIATE_SH2_MOV_R2 &&
          route.primary_literal_offset == 64 &&
          route.primary_literal_address == 0x00202734 &&
          route.primary_literal_provenance ==
              NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R3 &&
          route.auxiliary_literal_offset == 68 &&
          route.auxiliary_literal_address == 0x00202840 &&
          route.auxiliary_literal_provenance ==
              NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R0 &&
          route.task_header_profile_bound && !route.saturn_task_dispatch_proven &&
          !route.dispatch_permitted && route.blocks_real_script_dispatch &&
          !route.fallback_visuals_permitted,
          "engine binds only the active verified SLEV task profile");
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 0;
    CHECK(nexus_v1_current_level_script_route_receipt(&engine, &route) == 0 &&
          route.status == NEXUS_V1_LEVEL_SCRIPT_ROUTE_BLOCKED_SOURCE &&
          route.blocks_real_script_dispatch,
          "unverified SLEV bytes cannot reach the engine route");
}

static void test_engine_slev_capture_target_stays_source_bound(void) {
    Nexus_V1_Engine engine;
    Nexus_V1_LevelScriptCaptureTargetReceipt target;
    uint8_t slev[96];
    char text[1024];
    FILE *file;
    const char *path = "/private/tmp/firestaff-nexus-slev-capture-target.txt";
    static const uint8_t task_header[] = {
        0x2f, 0xe6, 0xe2, 0x1a, 0xd3, 0x0e, 0x34, 0x23,
        0x4f, 0x22, 0x7f, 0xfc, 0x2f, 0x52, 0x8d, 0x02,
        0x23, 0x42, 0x44, 0x11, 0x89, 0x04, 0xe0, 0xff,
        0x7f, 0x04, 0x4f, 0x26, 0x00, 0x0b, 0x6e, 0xf6,
        0xd0, 0x08, 0x4e, 0x08
    };

    memset(&engine, 0, sizeof(engine));
    memset(slev, 0, sizeof(slev));
    memcpy(slev, task_header, sizeof(task_header));
    slev[36] = 0x43; slev[37] = 0x0b;
    slev[52] = 0xa0; slev[53] = 0x10;
    slev[64] = 0x00; slev[65] = 0x20; slev[66] = 0x27; slev[67] = 0x34;
    slev[68] = 0x00; slev[69] = 0x20; slev[70] = 0x28; slev[71] = 0x40;
    engine.level_loaded = 1;
    engine.level_aux_runtime_receipt.level_index = 0;
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 1;
    snprintf(engine.level_aux_runtime_receipt.slev.canonical_name,
             sizeof(engine.level_aux_runtime_receipt.slev.canonical_name),
             "SLEV00.BIN");
    snprintf(engine.level_aux_runtime_receipt.slev.canonical_md5,
             sizeof(engine.level_aux_runtime_receipt.slev.canonical_md5),
             "59c01cbdd224152a6176687cdebeea9e");
    nexus_script_vm_init(&engine.script_vm);
    CHECK(nexus_script_vm_load_canonical_level(&engine.script_vm, 0, slev,
                                               (int)sizeof(slev), 1) == 0,
          "capture target loads canonical SLEV profile");
    remove(path);
    memset(&target, 0, sizeof(target));
    CHECK(nexus_v1_engine_write_slev_capture_target(&engine, path, &target) == 1 &&
          target.valid && target.level_index == 0 &&
          strcmp(target.canonical_slev_name, "SLEV00.BIN") == 0 &&
          strcmp(target.canonical_slev_md5,
                 "59c01cbdd224152a6176687cdebeea9e") == 0 &&
          target.source_byte_count == (int)sizeof(slev) &&
          target.primary_literal_address == 0x00202734 &&
          target.auxiliary_literal_address == 0x00202840 &&
          target.original_saturn_execution_required &&
          !target.task_body_dispatch_proven && target.no_dispatch_only &&
          !target.fallback_visuals_permitted,
          "active verified SLEV writes only an authentic execution-capture target");
    file = fopen(path, "rb");
    CHECK(file != NULL, "SLEV capture target is written");
    if (file) {
        size_t read_count;
        read_count = fread(text, 1, sizeof(text) - 1, file);
        text[read_count] = '\0';
        fclose(file);
        CHECK(strstr(text, NEXUS_V1_SLEV_CAPTURE_TARGET_MAGIC) != NULL &&
              strstr(text, "capture_kind=original_saturn_sh2_execution") != NULL &&
              strstr(text, "task_body_dispatch_proven=0") != NULL &&
              strstr(text, "no_dispatch_only=1") != NULL,
              "capture target requests proof without enabling a dispatcher");
    }
    remove(path);
}

static void test_engine_slev_trace_admission_stays_no_dispatch(void) {
    Nexus_V1_Engine engine;
    Nexus_V1_LevelScriptTraceAdmissionReceipt receipt;
    Nexus_V1_LevelScriptTraceAdmissionReceipt stored;
    Nexus_V1_LevelScriptTraceHostReceipt host_receipt;
    Nexus_V1_LevelScriptTraceHostReceipt stored_host;
    Nexus_V1_SlevDispatchEvidenceReceipt evidence;
    uint8_t slev[96];
    char trace[2048];
    uint64_t raw_trace_hash = 1469598103934665603ULL;
    size_t raw_index;
    static const uint8_t raw_trace[] =
        "mednafen debugger raw SH-2 trace\n"
        "pc=06001200 opcode=2fe6\n"
        "literal=00202734\n"
        "pc=06001224 opcode=430b\n"
        "literal=00202840\n"
        "pc=06001280 kind=write\n";
    static const uint8_t task_header[] = {
        0x2f, 0xe6, 0xe2, 0x1a, 0xd3, 0x0e, 0x34, 0x23,
        0x4f, 0x22, 0x7f, 0xfc, 0x2f, 0x52, 0x8d, 0x02,
        0x23, 0x42, 0x44, 0x11, 0x89, 0x04, 0xe0, 0xff,
        0x7f, 0x04, 0x4f, 0x26, 0x00, 0x0b, 0x6e, 0xf6,
        0xd0, 0x08, 0x4e, 0x08
    };
    memset(&engine, 0, sizeof(engine));
    memset(slev, 0, sizeof(slev));
    memcpy(slev, task_header, sizeof(task_header));
    slev[36] = 0x43; slev[37] = 0x0b;
    slev[52] = 0xa0; slev[53] = 0x10;
    slev[64] = 0x00; slev[65] = 0x20; slev[66] = 0x27; slev[67] = 0x34;
    slev[68] = 0x00; slev[69] = 0x20; slev[70] = 0x28; slev[71] = 0x40;
    engine.level_loaded = 1;
    engine.level_aux_runtime_receipt.level_index = 0;
    engine.level_aux_runtime_receipt.slev.canonical_hash_verified = 1;
    snprintf(engine.level_aux_runtime_receipt.slev.canonical_name,
             sizeof(engine.level_aux_runtime_receipt.slev.canonical_name),
             "SLEV00.BIN");
    snprintf(engine.level_aux_runtime_receipt.slev.canonical_md5,
             sizeof(engine.level_aux_runtime_receipt.slev.canonical_md5),
             "59c01cbdd224152a6176687cdebeea9e");
    nexus_script_vm_init(&engine.script_vm);
    CHECK(nexus_script_vm_load_canonical_level(&engine.script_vm, 0, slev,
                                               (int)sizeof(slev), 1) == 0,
          "trace admission loads canonical SLEV profile");
    for (raw_index = 0; raw_index < sizeof(raw_trace) - 1; ++raw_index) {
        raw_trace_hash ^= raw_trace[raw_index];
        raw_trace_hash *= 1099511628211ULL;
    }
    /* 96fb749ee/2026db6f8 bind the admitted trace to the loaded source
       bytes: the trace must carry the candidate source fnv1a64.  All
       numeric trace fields are hex-encoded (source_byte_count=0x60=96,
       task_header_size=0x24=36 for the synthetic profile below). */
    {
        uint64_t slev_hash = 1469598103934665603ULL;
        size_t slev_index;
        for (slev_index = 0; slev_index < sizeof(slev); ++slev_index) {
            slev_hash ^= slev[slev_index];
            slev_hash *= 1099511628211ULL;
        }
    snprintf(trace, sizeof(trace),
             "magic=FIRESTAFF_NEXUS_SLEV_SH2_TRACE_V1\n"
             "producer=mednafen-debugger\n"
             "trace_sha256=0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef\n"
             "raw_trace_fnv1a64=%llx\nlevel_index=0\n"
             "canonical_slev_name=SLEV00.BIN\n"
             "canonical_slev_md5=59c01cbdd224152a6176687cdebeea9e\n"
             "source_byte_count=%x\nsource_fnv1a64=%llx\ntask_header_size=24\nentry_opcode=2fe6\n"
             "primary_literal_address=00202734\nauxiliary_literal_address=00202840\n"
             "entry_pc=06001200\n"
             "task_body_pc=06001224\ntask_body_opcode=430b\n"
             "callback_or_write_pc=06001280\ncallback_or_write_kind=write\n"
             "original_saturn_execution=1\n",
             (unsigned long long)raw_trace_hash,
             (int)sizeof(slev),
             (unsigned long long)slev_hash);
    }
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_engine_admit_slev_execution_trace_with_raw(
              &engine, trace, strlen(trace), raw_trace, sizeof(raw_trace) - 1,
              &receipt) == 1 &&
          receipt.status == NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE &&
          receipt.level_index == 0 && receipt.capture_target_bound &&
          receipt.mednafen_debugger_provenance &&
          receipt.original_saturn_execution_claimed && receipt.trace_sha256_present &&
          receipt.raw_trace_bytes_bound && receipt.raw_trace_fnv1a64 == raw_trace_hash &&
          receipt.raw_trace_byte_count == sizeof(raw_trace) - 1 &&
          receipt.entry_pc == 0x06001200u && receipt.task_body_pc == 0x06001224u &&
          receipt.task_body_opcode == 0x430bu &&
          receipt.callback_or_write_pc == 0x06001280u &&
          receipt.callback_or_write_is_write && receipt.trace_chain_complete &&
          !receipt.task_body_dispatch_proven && !receipt.dispatch_permitted &&
          receipt.blocks_real_script_dispatch && !receipt.fallback_visuals_permitted,
          "matched Mednafen trace is retained only as opaque dispatch evidence");
    CHECK(nexus_v1_current_level_slev_trace_admission_receipt(&engine, &stored) == 0 &&
          stored.status == NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE &&
          stored.entry_pc == receipt.entry_pc &&
          !stored.dispatch_permitted,
          "engine owns the admitted trace receipt without enabling dispatch");
    CHECK(nexus_v1_build_slev_dispatch_evidence(
              &engine, raw_trace, sizeof(raw_trace) - 1, &evidence) == 1 &&
          evidence.status == NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED &&
          evidence.raw_trace_bound && evidence.entry_observed &&
          evidence.task_body_observed && evidence.callback_or_write_observed &&
          evidence.primary_literal_observed && evidence.auxiliary_literal_observed &&
          evidence.entry_raw_offset < evidence.task_body_raw_offset &&
          evidence.task_body_raw_offset < evidence.callback_or_write_raw_offset &&
          evidence.observation_order_proven &&
          evidence.literal_observation_proven &&
          evidence.callback_or_write_is_write &&
          !evidence.task_body_dispatch_proven && !evidence.dispatch_permitted &&
          evidence.blocks_real_script_dispatch &&
          !evidence.fallback_visuals_permitted,
          "raw trace proves observation order without promoting task semantics");
    CHECK(nexus_v1_engine_consume_slev_execution_trace(&engine,
                                                       &host_receipt) == 1 &&
          host_receipt.status == NEXUS_V1_SLEV_TRACE_HOST_CONSUMED_OPAQUE &&
          host_receipt.active_slev_target_revalidated &&
          host_receipt.admitted_trace_bound && host_receipt.host_consumed &&
          host_receipt.entry_pc == receipt.entry_pc &&
          host_receipt.task_body_pc == receipt.task_body_pc &&
          host_receipt.task_body_opcode == receipt.task_body_opcode &&
          host_receipt.callback_or_write_pc == receipt.callback_or_write_pc &&
          host_receipt.callback_or_write_is_write &&
          !host_receipt.task_body_dispatch_proven &&
          !host_receipt.dispatch_permitted &&
          host_receipt.blocks_real_script_dispatch &&
          !host_receipt.fallback_visuals_permitted,
          "admitted SLEV trace reaches the active host route without dispatch");
    CHECK(nexus_v1_current_level_slev_trace_host_receipt(&engine,
                                                         &stored_host) == 0 &&
          stored_host.status == NEXUS_V1_SLEV_TRACE_HOST_CONSUMED_OPAQUE &&
          stored_host.entry_pc == receipt.entry_pc &&
          !stored_host.dispatch_permitted,
          "engine retains the consumed opaque trace receipt");
    {
        char mismatched_trace[sizeof(trace)];
        strcpy(mismatched_trace, trace);
        memcpy(strstr(mismatched_trace, "59c01"), "00000", 5);
        CHECK(nexus_v1_engine_admit_slev_execution_trace_with_raw(
                  &engine, mismatched_trace, strlen(mismatched_trace), raw_trace,
                  sizeof(raw_trace) - 1,
                  &receipt) == 0 &&
              receipt.status == NEXUS_V1_SLEV_TRACE_BLOCKED_TARGET_MISMATCH &&
              nexus_v1_current_level_slev_trace_admission_receipt(
                  &engine, &stored) == 0 &&
              stored.status == NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE,
              "wrong SLEV identity rejects without replacing admitted evidence");
    }
    engine.script_vm.current_level = 1;
    CHECK(nexus_v1_engine_consume_slev_execution_trace(&engine,
                                                       &host_receipt) == 0 &&
          host_receipt.status == NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_ACTIVE_ROUTE &&
          nexus_v1_current_level_slev_trace_host_receipt(&engine,
                                                         &stored_host) == 0 &&
          stored_host.status == NEXUS_V1_SLEV_TRACE_HOST_CONSUMED_OPAQUE,
          "active SLEV mismatch cannot replace the prior host receipt");
}

static void test_real_slev_corpus_profile(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int seen = 0;
    int profiled = 0;
    int level;

    if (!data_dir || data_dir[0] == '\0') {
        puts("test_nexus_v1_script_vm: SKIP corpus (FIRESTAFF_NEXUS_DATA_DIR unset)");
        return;
    }

    for (level = 0; level < 16; ++level) {
        char path[512];
        FILE *fp;
        long size;
        uint8_t *data;
        Nexus_ScriptVM vm;
        Nexus_ScriptRuntimeReceipt receipt;

        snprintf(path, sizeof(path), "%s/SLEV%02d.BIN", data_dir, level);
        fp = fopen(path, "rb");
        CHECK(fp != NULL, "configured SLEV corpus file is present");
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
              "real SLEV corpus file loads");
        CHECK(nexus_script_vm_runtime_receipt(&vm, &receipt) == 0,
              "real SLEV corpus file emits receipt");
        if (receipt.real_task_profile_supported) {
            profiled++;
            CHECK(receipt.status ==
                      NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT &&
                  receipt.blocks_real_script_dispatch == 1 &&
                  receipt.dispatch_enabled == 0,
                  "real SLEV profile stays no-dispatch");
            CHECK(receipt.real_task_word_count == (int)(size / 2) &&
                  receipt.real_task_first_opcode == 0x2fe6 &&
                  receipt.real_task_rts_count > 0 &&
                  receipt.real_task_jsr_count > 0 &&
                  receipt.real_task_pc_relative_load_count > 0 &&
                  receipt.real_task_checksum16 != 0,
                  "real SLEV profile records SH-2 call/operand shape");
            CHECK(receipt.real_task_header_supported == 1 &&
                  receipt.real_task_header_size == 36 &&
                  receipt.real_task_primary_literal_instruction_offset == 4 &&
                  receipt.real_task_aux_literal_instruction_offset == 32 &&
                  receipt.real_task_primary_literal_offset ==
                      8 + receipt.real_task_primary_literal_displacement * 4 &&
                  receipt.real_task_aux_literal_offset ==
                      36 + receipt.real_task_aux_literal_displacement * 4 &&
                  receipt.real_task_primary_literal_offset >= 36 &&
                  receipt.real_task_aux_literal_offset >= 36 &&
                  receipt.real_task_primary_literal_address >= 0x00200000 &&
                  receipt.real_task_aux_literal_address >= 0x00200000 &&
                  receipt.real_task_setup_immediate_provenance ==
                      NEXUS_SLEV_SETUP_IMMEDIATE_SH2_MOV_R2 &&
                  receipt.real_task_primary_literal_provenance ==
                      NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R3 &&
                  receipt.real_task_aux_literal_provenance ==
                      NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R0 &&
                  receipt.real_task_rts_instruction_offset == 28 &&
                  receipt.real_task_post_rts_instruction_offset == 30 &&
                  receipt.real_task_post_rts_instruction_word == 0x6ef6,
                  "real SLEV corpus matches bounded entry provenance");
            if (receipt.real_task_literal_pointer_count > 0) {
                CHECK(receipt.real_task_first_literal_offset >= 0 &&
                      receipt.real_task_first_literal_address >= 0x00200000 &&
                      receipt.real_task_last_literal_address >= 0x00200000,
                      "real SLEV profile records literal table operands");
            }
        }
        free(data);
    }

    CHECK(seen == 16, "configured corpus contains all 16 SLEV files");
    CHECK(profiled == seen,
          "all real SLEV corpus files match the task profile");
}

int main(void) {
    test_manual_rules_cannot_enable_runtime_dispatch();
    test_forged_parser_flags_cannot_enable_dispatch();
    test_event_operand_matching();
    test_once_only_manual_fire_and_unload();
    test_runtime_receipts_block_unparsed_real_source();
    test_real_slev_task_profile_blocks_dispatch();
    test_engine_owned_slev_profile_route_stays_blocked();
    test_engine_slev_capture_target_stays_source_bound();
    test_engine_slev_trace_admission_stays_no_dispatch();
    test_real_slev_corpus_profile();

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_script_vm: %d failure(s)\n", g_failures);
        return 1;
    }

    puts("test_nexus_v1_script_vm: PASS");
    return 0;
}
