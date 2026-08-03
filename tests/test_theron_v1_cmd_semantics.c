#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "theron_v1_track02.h"

static uint8_t *load_track02(size_t *out_size) {
    const char *home = getenv("HOME");
    char path[1024];
    FILE *f;
    long len;
    uint8_t *buf;

    if (!home) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin", home);
    f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    if (len <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    buf = malloc((size_t)len);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)len;
    return buf;
}

static void test_cmd_semantic_classification(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    Theron_Track02SignalStatus status;

    memset(&receipt, 0, sizeof(receipt));
    status = theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid);
    assert(receipt.handler_count == 10);
    assert(receipt.all_handlers_classified);
    assert(receipt.stream_vm_complete);
    printf("  PASS: cmd_semantic_classification (10 handlers classified)\n");
}

static void test_handler_roles(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.handlers[0].role == THERON_CMD_ROLE_UNCONDITIONAL_ACTION);
    assert(receipt.handlers[1].role == THERON_CMD_ROLE_CONDITIONAL_EQ_ACTION);
    assert(receipt.handlers[2].role == THERON_CMD_ROLE_CONDITIONAL_EQ_BRANCH);
    assert(receipt.handlers[3].role == THERON_CMD_ROLE_CONDITIONAL_GT_BRANCH);
    assert(receipt.handlers[4].role == THERON_CMD_ROLE_CONDITIONAL_LT_BRANCH);
    assert(receipt.handlers[5].role == THERON_CMD_ROLE_TWO_OPERAND_DISPATCH_A);
    assert(receipt.handlers[6].role == THERON_CMD_ROLE_TWO_OPERAND_DISPATCH_B);
    assert(receipt.handlers[7].role == THERON_CMD_ROLE_TWO_OPERAND_DISPATCH_C);
    assert(receipt.handlers[8].role == THERON_CMD_ROLE_RENDER_DISPATCH);
    assert(receipt.handlers[9].role == THERON_CMD_ROLE_STREAM_END);
    printf("  PASS: handler_roles (all 10 roles match)\n");
}

static void test_handler_addresses(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.handlers[0].handler_cpu_address == 0x41C5);
    assert(receipt.handlers[1].handler_cpu_address == 0x41CB);
    assert(receipt.handlers[2].handler_cpu_address == 0x41D8);
    assert(receipt.handlers[3].handler_cpu_address == 0x41DE);
    assert(receipt.handlers[4].handler_cpu_address == 0x41E6);
    assert(receipt.handlers[5].handler_cpu_address == 0x41EC);
    assert(receipt.handlers[6].handler_cpu_address == 0x41F0);
    assert(receipt.handlers[7].handler_cpu_address == 0x41F4);
    assert(receipt.handlers[8].handler_cpu_address == 0x4214);
    assert(receipt.handlers[9].handler_cpu_address == 0x4253);
    printf("  PASS: handler_addresses (10 CPU addresses match jump table)\n");
}

static void test_advance_bytes(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.handlers[0].advance_bytes == 1);
    assert(receipt.handlers[1].advance_bytes == 2);
    assert(receipt.handlers[2].advance_bytes == 3);
    assert(receipt.handlers[3].advance_bytes == 4);
    assert(receipt.handlers[4].advance_bytes == 5);
    assert(receipt.handlers[5].advance_bytes == 3);
    assert(receipt.handlers[6].advance_bytes == 3);
    assert(receipt.handlers[7].advance_bytes == 3);
    assert(receipt.handlers[8].advance_bytes == 9);
    assert(receipt.handlers[9].advance_bytes == 0);
    assert(receipt.total_advance_bytes_sum == 1+2+3+4+5+3+3+3+9+0);
    printf("  PASS: advance_bytes (sum=%zu)\n",
           receipt.total_advance_bytes_sum);
}

static void test_operand_counts(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.handlers[0].operand_count == 0);
    assert(receipt.handlers[1].operand_count == 1);
    assert(receipt.handlers[2].operand_count == 1);
    assert(receipt.handlers[3].operand_count == 1);
    assert(receipt.handlers[4].operand_count == 1);
    assert(receipt.handlers[5].operand_count == 2);
    assert(receipt.handlers[6].operand_count == 2);
    assert(receipt.handlers[7].operand_count == 2);
    assert(receipt.handlers[8].operand_count == 1);
    assert(receipt.handlers[9].operand_count == 0);
    printf("  PASS: operand_counts\n");
}

static void test_state_table(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    size_t i, state_readers = 0;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.state_table_base_proven);
    assert(receipt.state_table_base == 0x2780);
    for (i = 0; i < 10; i++) {
        if (receipt.handlers[i].reads_state_table) state_readers++;
    }
    assert(state_readers == 7);
    printf("  PASS: state_table ($%04X, %zu readers)\n",
           receipt.state_table_base, state_readers);
}

static void test_render_chain(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    size_t i, render_count = 0;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    assert(receipt.render_chain_proven);
    for (i = 0; i < 10; i++) {
        if (receipt.handlers[i].calls_render_chain) render_count++;
    }
    assert(render_count == 1);
    assert(receipt.handlers[8].calls_render_chain);
    printf("  PASS: render_chain (handler 9 is sole render dispatcher)\n");
}

static void test_role_names(void) {
    assert(strcmp(theron_v1_track02_cmd_role_name(
        THERON_CMD_ROLE_UNCONDITIONAL_ACTION), "unconditional-action") == 0);
    assert(strcmp(theron_v1_track02_cmd_role_name(
        THERON_CMD_ROLE_RENDER_DISPATCH), "render-dispatch") == 0);
    assert(strcmp(theron_v1_track02_cmd_role_name(
        THERON_CMD_ROLE_STREAM_END), "stream-end") == 0);
    assert(strcmp(theron_v1_track02_cmd_role_name(
        THERON_CMD_ROLE_UNKNOWN), "unknown") == 0);
    printf("  PASS: role_names\n");
}

static void test_syscard_catalog(const uint8_t *data, size_t size) {
    Theron_Track02SyscardCatalogReceipt receipt;
    Theron_Track02SignalStatus status;

    memset(&receipt, 0, sizeof(receipt));
    status = theron_v1_track02_catalog_syscard_calls(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid);
    assert(receipt.total_call_sites > 0);
    assert(receipt.cd_play_count > 0);
    assert(receipt.ad_play_count > 0);
    assert(receipt.ad_cplay_count > 0);
    assert(receipt.cd_read_count > 0);
    assert(receipt.cd_fade_count > 0);
    assert(receipt.cd_boot_count > 0);
    printf("  PASS: syscard_catalog (total=%zu, CD_PLAY=%zu, AD_PLAY=%zu, "
           "AD_CPLAY=%zu, CD_READ=%zu, CD_FADE=%zu, CD_BOOT=%zu)\n",
           receipt.total_call_sites,
           receipt.cd_play_count, receipt.ad_play_count,
           receipt.ad_cplay_count, receipt.cd_read_count,
           receipt.cd_fade_count, receipt.cd_boot_count);
}

static void test_pce_io_catalog(const uint8_t *data, size_t size) {
    Theron_Track02PceIoCatalogReceipt receipt;
    Theron_Track02SignalStatus status;

    memset(&receipt, 0, sizeof(receipt));
    status = theron_v1_track02_catalog_pce_io(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid);
    assert(receipt.joypad_access_count > 100);
    assert(receipt.vce_palette_access_count > 200);
    assert(receipt.timer_access_count > 0);
    assert(receipt.irq_access_count > 0);
    assert(receipt.joypad_read_routine_proven);
    assert(receipt.vce_palette_write_proven);
    printf("  PASS: pce_io_catalog (joypad=%zu, VCE=%zu, timer=%zu, IRQ=%zu, "
           "joypad_routine=sector %u:0x%03x)\n",
           receipt.joypad_access_count, receipt.vce_palette_access_count,
           receipt.timer_access_count, receipt.irq_access_count,
           receipt.joypad_read_routine_sector,
           receipt.joypad_read_routine_user_offset);
}

static void test_syscard_cd_play_sites(const uint8_t *data, size_t size) {
    Theron_Track02SyscardCatalogReceipt receipt;
    size_t i, code_cd_play = 0;

    theron_v1_track02_catalog_syscard_calls(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    for (i = 0; i < receipt.call_site_count; i++) {
        if (receipt.call_sites[i].vector == THERON_SYSCARD_CD_PLAY &&
            receipt.call_sites[i].in_user_data) {
            code_cd_play++;
        }
    }
    assert(code_cd_play >= 2);
    printf("  PASS: syscard_cd_play_sites (%zu in-code CD_PLAY calls)\n",
           code_cd_play);
}

static void test_vm_structure_summary(const uint8_t *data, size_t size) {
    Theron_Track02CmdSemanticReceipt receipt;
    size_t i;
    theron_v1_track02_classify_cmd_semantics(
        data, size, THERON_TRACK02_MD5_US_BIN, &receipt);

    printf("\n  Command Stream VM Structure:\n");
    for (i = 0; i < receipt.handler_count; i++) {
        const Theron_Track02CmdHandlerSemantic *h = &receipt.handlers[i];
        printf("    Handler %2zu ($%04X): %-25s adv=%u ops=%u"
               " state=%d sub=%d render=%d\n",
               i + 1, h->handler_cpu_address,
               theron_v1_track02_cmd_role_name(h->role),
               h->advance_bytes, h->operand_count,
               h->reads_state_table, h->calls_subroutine,
               h->calls_render_chain);
    }
    printf("    State table: $%04X  |  Total advance sum: %zu\n",
           receipt.state_table_base, receipt.total_advance_bytes_sum);
    printf("  PASS: vm_structure_summary\n");
}

int main(void) {
    uint8_t *data;
    size_t size = 0;

    printf("test_theron_v1_cmd_semantics\n");

    test_role_names();

    data = load_track02(&size);
    if (!data) {
        printf("  SKIP: TQUS02.bin not available\n");
        printf("PASS (1 test, data-dependent tests skipped)\n");
        return 0;
    }

    test_cmd_semantic_classification(data, size);
    test_handler_roles(data, size);
    test_handler_addresses(data, size);
    test_advance_bytes(data, size);
    test_operand_counts(data, size);
    test_state_table(data, size);
    test_render_chain(data, size);
    test_pce_io_catalog(data, size);
    test_syscard_catalog(data, size);
    test_syscard_cd_play_sites(data, size);
    test_vm_structure_summary(data, size);

    free(data);
    printf("PASS (12 tests)\n");
    return 0;
}
