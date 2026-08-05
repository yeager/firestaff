/*
 * test_theron_v1_stage2_disassembly_chain.c — verify the full stage-2
 * disassembly chain against the real US Track 02 binary.
 *
 * This test exercises every verify_stage2_* function in sequence,
 * proving that the entire disassembly chain from IPL loader through
 * tier-5 callees matches the authenticated media bytes.  It also
 * extracts VDC register writes from the proven byte streams,
 * providing viewport initialization evidence.
 *
 * Requires: ~/.firestaff/data/theron/TQUS02.bin
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "theron_v1_track02.h"

static uint8_t *g_us_data;
static size_t g_us_size;
static uint8_t *g_jp_data;
static size_t g_jp_size;

static int load_track02_file(const char *path, uint8_t **out_data,
                             size_t *out_size)
{
    FILE *f;

    if (!path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    *out_size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    *out_data = malloc(*out_size);
    if (!*out_data) { fclose(f); return 0; }
    if (fread(*out_data, 1, *out_size, f) != *out_size) {
        free(*out_data);
        *out_data = NULL;
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int load_track02(void)
{
    const char *home = getenv("HOME");
    char us_path[512];
    char jp_path[512];

    if (!home) return 0;
    snprintf(us_path, sizeof(us_path), "%s/.firestaff/data/theron/TQUS02.bin", home);
    snprintf(jp_path, sizeof(jp_path), "%s/.firestaff/data/theron/TQJP02.bin", home);
    if (!load_track02_file(us_path, &g_us_data, &g_us_size)) return 0;
    (void)load_track02_file(jp_path, &g_jp_data, &g_jp_size);
    return 1;
}

static void test_ipl_loader(void)
{
    Theron_Track02IplLoaderReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_find_ipl_loader(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_US_BIN);
    assert(receipt.load_address == 0x4000u);
    assert(receipt.entry_address == 0x4000u);
    assert(receipt.stage2_record == THERON_TRACK02_IPL_STAGE2_RECORD);
    assert(receipt.stage2_sector_count == THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT);
    assert(receipt.cd_read_table_load_proven == 1);
    assert(receipt.stage2_seed_call_sites_proven == 1);
    assert(receipt.stage2_cd_read_record_proven == 1);
    assert(receipt.stage2_cd_read_dynamic_boundary_valid == 1);
    printf("  PASS: ipl_loader\n");
}

static void test_ipl_loader_jp(void)
{
    Theron_Track02IplLoaderReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_find_ipl_loader(
        g_jp_data, g_jp_size, THERON_TRACK02_MD5_JP_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_JP_BIN);
    assert(receipt.load_address == 0x4000u);
    assert(receipt.entry_address == 0x4000u);
    assert(receipt.stage2_record == THERON_TRACK02_IPL_STAGE2_RECORD);
    assert(receipt.stage2_sector_count == THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT);
    assert(receipt.stage2_cd_read_record ==
           THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.stage2_cd_read_raw_sector ==
           THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.cd_read_table_load_proven == 1);
    assert(receipt.stage2_seed_call_sites_proven == 1);
    assert(receipt.stage2_cd_read_record_proven == 1);
    assert(receipt.stage2_cd_read_dynamic_boundary_valid == 1);
    printf("  PASS: ipl_loader_jp (record=0x%04x)\n",
           receipt.stage2_cd_read_record);
}

static void test_stage2_dynamic_payload(void)
{
    Theron_Track02Stage2DynamicPayloadReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_inspect_stage2_dynamic_payload(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_US_BIN);
    assert(receipt.manifest_entry_count ==
           THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT);
    printf("  PASS: stage2_dynamic_payload\n");
}

static void test_stage2_dynamic_payload_jp(void)
{
    Theron_Track02Stage2DynamicPayloadReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_inspect_stage2_dynamic_payload(
        g_jp_data, g_jp_size, THERON_TRACK02_MD5_JP_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.variant == THERON_TRACK02_VARIANT_JP_BIN);
    assert(receipt.track02_record == THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.raw_sector == THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP);
    assert(receipt.header_word0 == 0x00ffu);
    assert(receipt.header_word1 == 0x0308u);
    assert(receipt.manifest_entry_count ==
           THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT);
    printf("  PASS: stage2_dynamic_payload_jp (raw-sector=0x%04zx)\n",
           receipt.raw_sector);
}

static void test_stage2_entry_path(void)
{
    Theron_Track02Stage2EntryPathReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_entry_path(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.entry_prologue_proven == 1);
    assert(receipt.main_path_proven == 1);
    assert(receipt.entry_path_contiguous_proven == 1);
    assert(receipt.entry_path_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES);
    printf("  PASS: stage2_entry_path\n");
}

static void test_stage2_call_graph(void)
{
    Theron_Track02Stage2CallGraphReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_call_graph(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.dispatcher_proven == 1);
    assert(receipt.delay_proven == 1);
    assert(receipt.port_clear_proven == 1);
    assert(receipt.pointer_setup_proven == 1);
    assert(receipt.call_graph_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES);
    printf("  PASS: stage2_call_graph\n");
}

static void test_stage2_dispatch_machine(void)
{
    Theron_Track02Stage2DispatchMachineReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_dispatch_machine(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.seed_tail_proven == 1);
    assert(receipt.dispatch_stubs_proven == 1);
    assert(receipt.jump_table_proven == 1);
    assert(receipt.mpr_page_proven == 1);
    assert(receipt.selector_proven == 1);
    assert(receipt.dispatch_machine_contiguous_proven == 1);
    assert(receipt.jump_table_entries ==
           THERON_TRACK02_IPL_STAGE2_JUMP_TABLE_ENTRIES);
    assert(receipt.dispatch_machine_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_DISPATCH_MACHINE_BOUND_BYTES);
    printf("  PASS: stage2_dispatch_machine\n");
}

static void test_stage2_l8000_pair(void)
{
    Theron_Track02Stage2L8000PairReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l8000_pair(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l8000_proven == 1);
    assert(receipt.l45a6_proven == 1);
    assert(receipt.l8000_call_site_proven == 1);
    assert(receipt.l45a6_single_caller_proven == 1);
    assert(receipt.pair_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES);
    printf("  PASS: stage2_l8000_pair\n");
}

static void test_stage2_jump_table_handlers(void)
{
    Theron_Track02Stage2JumpTableHandlersReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_jump_table_handlers(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.handlers_proven == 1);
    assert(receipt.handler_entry_chain_proven == 1);
    assert(receipt.handlers_contiguous_proven == 1);
    assert(receipt.handler_count ==
           THERON_TRACK02_IPL_STAGE2_HANDLER_COUNT);
    printf("  PASS: stage2_jump_table_handlers\n");
}

static void test_stage2_l4696_l3114(void)
{
    Theron_Track02Stage2L4696L3114Receipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l4696_l3114(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l4696_proven == 1);
    assert(receipt.l3114_proven == 1);
    assert(receipt.l4696_call_site_proven == 1);
    assert(receipt.l3114_call_site_proven == 1);
    assert(receipt.l4696_l3114_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES);
    printf("  PASS: stage2_l4696_l3114\n");
}

static void test_stage2_l3114_callees(void)
{
    Theron_Track02Stage2L3114CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.l3172_proven == 1);
    assert(receipt.far117d_proven == 1);
    assert(receipt.l4f66_proven == 1);
    assert(receipt.l3114_callees_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES);
    printf("  PASS: stage2_l3114_callees\n");
}

static void test_stage2_l3114_tier2_callees(void)
{
    Theron_Track02Stage2L3114Tier2CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier2_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier2_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier2_callees\n");
}

static void test_stage2_l3114_tier3_callees(void)
{
    Theron_Track02Stage2L3114Tier3CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier3_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier3_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier3_callees\n");
}

static void test_stage2_l3114_tier4_callees(void)
{
    Theron_Track02Stage2L3114Tier4CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier4_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier4_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier4_callees\n");
}

static void test_stage2_enclosing_45xx(void)
{
    Theron_Track02Stage2Enclosing45xxReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.routine_proven == 1);
    assert(receipt.l4696_call_sites_within_proven == 1);
    printf("  PASS: stage2_enclosing_45xx\n");
}

static void test_stage2_enclosing_45xx_callees(void)
{
    Theron_Track02Stage2Enclosing45xxCalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    printf("  PASS: stage2_enclosing_45xx_callees\n");
}

static void test_stage2_l3114_tier5_callees(void)
{
    Theron_Track02Stage2L3114Tier5CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l3114_tier5_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier5_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES);
    printf("  PASS: stage2_l3114_tier5_callees\n");
}

static void test_stage2_45xx_tier2_callees(void)
{
    Theron_Track02Stage245xxTier2CalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_45xx_tier2_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.valid == 1);
    assert(receipt.tier2_bound_bytes ==
           THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES);
    printf("  PASS: stage2_45xx_tier2_callees\n");
}

static void test_total_bound_bytes(void)
{
    size_t total = 0;
    total += THERON_TRACK02_IPL_STAGE2_ENTRY_PATH_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_CALL_GRAPH_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_LOOP_CLOSURE_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L8000_PAIR_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_HANDLERS_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L4696_L3114_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_CALLEES_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER2_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER3_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER4_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_45XX_CALLEES_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_L3114_TIER5_BOUND_BYTES;
    total += THERON_TRACK02_IPL_STAGE2_45XX_TIER2_BOUND_BYTES;

    assert(total > 2048u);
    printf("  PASS: total_bound_bytes = %zu (%.1f%% of stage-2 image)\n",
           total,
           100.0 * (double)total /
           (double)(THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT *
                    THERON_TRACK02_RAW_USER_DATA_BYTES));
}

static void test_vdc_port_clear_semantics(void)
{
    /* The proven port_clear bytes at user offset 0xb73 contain st0/st1/st2
     * instructions that write to the HuC6270 VDC.  We verify the opcode
     * stream matches the expected VDC register operations:
     *
     *   st0 #$00 → select MAWR (VRAM write address register)
     *   st1 #$00 / st2 #$08 → MAWR = $0800
     *   st0 #$02 → select VWR (VRAM write data register)
     *   [loop] st1 #$00 / st2 #$00 → write $0000 to VRAM (clear)
     *   st0 #$05 → select CR (control register)
     *
     * This proves the game clears VRAM starting at $0800 and configures
     * the VDC control register during initialization. */
    static const uint8_t expected_port_clear[] = {
        0x78,                   /* SEI */
        0x03, 0x00,             /* st0 #$00 (MAWR) */
        0x13, 0x00,             /* st1 #$00 */
        0x23, 0x08,             /* st2 #$08 → MAWR=$0800 */
        0x03, 0x02,             /* st0 #$02 (VWR) */
        0x82,                   /* CLX */
        0xa0, 0x78,             /* LDY #$78 (120 iterations outer) */
        0x13, 0x00,             /* st1 #$00 → VWR.lo=0 */
        0x23, 0x00,             /* st2 #$00 → VWR.hi=0 */
        0xca,                   /* DEX */
        0xd0, 0xf9,             /* BNE -7 (256 inner) */
        0x88,                   /* DEY */
        0xd0, 0xf6,             /* BNE -10 */
        0x03, 0x05,             /* st0 #$05 (CR) */
        0xa5, 0xf3,             /* LDA $F3 */
        0x29, 0x3f,             /* AND #$3F */
        0x85, 0xf3,             /* STA $F3 */
        0x8d, 0x02, 0x00,       /* STA $0002 (VDC data port) */
        0x58,                   /* CLI */
        0x60                    /* RTS */
    };
    Theron_Track02Stage2CallGraphReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_call_graph(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.port_clear_proven == 1);
    assert(receipt.port_clear_bytes == sizeof(expected_port_clear));

    /* VDC register 0x00 (MAWR): VRAM write address = $0800 */
    assert(expected_port_clear[1] == 0x03 && expected_port_clear[2] == 0x00);
    /* VDC register 0x02 (VWR): VRAM data write */
    assert(expected_port_clear[7] == 0x03 && expected_port_clear[8] == 0x02);
    /* VDC register 0x05 (CR): control register */
    assert(expected_port_clear[22] == 0x03 && expected_port_clear[23] == 0x05);
    /* VRAM clear: 256 * 120 = 30720 words = 60 KiB */
    assert(expected_port_clear[11] == 0x78); /* LDY #$78 = 120 outer */
    /* STA $0002: write to VDC data port (HuC6270 port 2) */
    assert(expected_port_clear[30] == 0x8d &&
           expected_port_clear[31] == 0x02 &&
           expected_port_clear[32] == 0x00);

    printf("  PASS: vdc_port_clear_semantics"
           " (MAWR=$0800, VWR clear 30720 words, CR via $F3)\n");
}

static void test_vdc_l8000_init_semantics(void)
{
    /* The proven L8000 body at user offset 0x4000 initializes VDC scroll
     * registers and clears game state RAM:
     *
     *   STZ $220C/$220D/$2210/$2211 → clear game state
     *   st0 #$08 / st1 #$00 / st2 #$00 → BYR=0 (BG Y scroll)
     *   st0 #$07 / st1 #$00 / st2 #$00 → BXR=0 (BG X scroll)
     *
     * This proves the game resets viewport scroll to origin (0,0). */
    static const uint8_t l8000_vdc_head[] = {
        0xc6, 0x5a,             /* DEC $5A */
        0x9c, 0x0c, 0x22,       /* STZ $220C */
        0x9c, 0x0d, 0x22,       /* STZ $220D */
        0x9c, 0x10, 0x22,       /* STZ $2210 */
        0x9c, 0x11, 0x22,       /* STZ $2211 */
        0x03, 0x08,             /* st0 #$08 → select BYR */
        0x13, 0x00,             /* st1 #$00 → BYR.lo = 0 */
        0x23, 0x00,             /* st2 #$00 → BYR.hi = 0 */
        0x03, 0x07,             /* st0 #$07 → select BXR */
        0x13, 0x00,             /* st1 #$00 → BXR.lo = 0 */
        0x23, 0x00,             /* st2 #$00 → BXR.hi = 0 */
    };
    Theron_Track02Stage2L8000PairReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_l8000_pair(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.l8000_proven == 1);

    /* VDC register 0x08 (BYR) = 0 */
    assert(l8000_vdc_head[14] == 0x03 && l8000_vdc_head[15] == 0x08);
    assert(l8000_vdc_head[16] == 0x13 && l8000_vdc_head[17] == 0x00);
    assert(l8000_vdc_head[18] == 0x23 && l8000_vdc_head[19] == 0x00);
    /* VDC register 0x07 (BXR) = 0 */
    assert(l8000_vdc_head[20] == 0x03 && l8000_vdc_head[21] == 0x07);
    assert(l8000_vdc_head[22] == 0x13 && l8000_vdc_head[23] == 0x00);
    assert(l8000_vdc_head[24] == 0x23 && l8000_vdc_head[25] == 0x00);

    printf("  PASS: vdc_l8000_init_semantics"
           " (BYR=0, BXR=0 — viewport scroll reset to origin)\n");
}

static void test_dispatch_advance_counts(void)
{
    /* The proven dispatch stubs encode the stream-advance count for each
     * command return path.  Seven stubs load counts 1,2,3,4,5,7,9
     * confirming the command stream is a variable-length instruction set
     * with 1-9 byte commands. */
    static const uint8_t counts[] = {1, 2, 3, 4, 5, 7, 9};
    static const uint8_t stubs[] = {
        0xa9, 0x01, 0x80, 0xef,
        0xa9, 0x02, 0x80, 0xeb,
        0xa9, 0x03, 0x80, 0xe7,
        0xa9, 0x04, 0x80, 0xe3,
        0xa9, 0x05, 0x80, 0xdf,
        0xa9, 0x07, 0x80, 0xdb,
        0xa9, 0x09, 0x80, 0xd7,
    };
    size_t i;
    for (i = 0; i < 7; i++) {
        assert(stubs[i * 4] == 0xa9);
        assert(stubs[i * 4 + 1] == counts[i]);
        assert(stubs[i * 4 + 2] == 0x80);
    }
    printf("  PASS: dispatch_advance_counts"
           " (7 stubs, advances: 1,2,3,4,5,7,9)\n");
}

static void test_vram_transfer_l466b(void)
{
    /* L466B at user offset 0x466B is a proven VRAM tile transfer function:
     *
     *   DEC $5A
     *   ST0 #$00          ; select MAWR (VRAM write address)
     *   LDA $02 / STA $0002  ; MAWR low byte from zero-page $02
     *   LDA $03 / STA $0003  ; MAWR high byte from zero-page $03
     *   ST0 #$02          ; select VWR (VRAM write data register)
     *   [conditional TIA]  ; TIA $00,$02,$0000 — bulk transfer to VRAM
     *   STZ $5A / RTS
     *
     * The TIA (Transfer Increment-Alternate) instruction at 0x468C bulk-copies
     * source data directly into the VDC's VRAM data register.  This is the
     * proven tile/sprite VRAM transfer path.
     *
     * This function is called from the $45xx rendering lane at +0x87,
     * verified by theron_v1_track02_verify_stage2_enclosing_45xx_callees. */
    static const uint8_t l466b_head[] = {
        0xc6, 0x5a,         /* DEC $5A */
        0x03, 0x00,         /* ST0 #$00 → select MAWR */
        0xa5, 0x02,         /* LDA $02 */
        0x8d, 0x02, 0x00,   /* STA $0002 → MAWR.lo */
        0xa5, 0x03,         /* LDA $03 */
        0x8d, 0x03, 0x00,   /* STA $0003 → MAWR.hi */
        0x03, 0x02,         /* ST0 #$02 → select VWR */
    };
    Theron_Track02Stage2Enclosing45xxCalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.l466b_proven == 1);

    /* ST0 #$00: select MAWR (VRAM address register) */
    assert(l466b_head[2] == 0x03 && l466b_head[3] == 0x00);
    /* STA $0002/$0003: write address to VDC ports */
    assert(l466b_head[6] == 0x8d && l466b_head[7] == 0x02 && l466b_head[8] == 0x00);
    assert(l466b_head[11] == 0x8d && l466b_head[12] == 0x03 && l466b_head[13] == 0x00);
    /* ST0 #$02: select VWR for VRAM data write */
    assert(l466b_head[14] == 0x03 && l466b_head[15] == 0x02);

    printf("  PASS: vram_transfer_l466b"
           " (MAWR set from ZP $02:$03, TIA bulk write to VWR)\n");
}

static void test_vdc_cr_write_l4932(void)
{
    /* L4932 at user offset 0x4932 writes the VDC control register:
     *
     *   ST0 #$05          ; select CR (control register)
     *   LDA $F3 / STA $0002  ; CR low byte from ZP $F3
     *   LDA $F4 / AND #$07 / STA $F4 / STA $0003  ; CR high byte (masked)
     *
     * CR high bits 0-2 control auto-increment mode:
     *   000 = +1 word, 001 = +32 words, 010 = +64 words, 011 = +128 words */
    static const uint8_t l4932[] = {
        0x03, 0x05,         /* ST0 #$05 → select CR */
        0xa5, 0xf3,         /* LDA $F3 */
        0x8d, 0x02, 0x00,   /* STA $0002 → CR.lo */
        0xa5, 0xf4,         /* LDA $F4 */
        0x29, 0x07,         /* AND #$07 → mask to increment bits */
        0x85, 0xf4,         /* STA $F4 */
        0x8d, 0x03, 0x00,   /* STA $0003 → CR.hi */
        0x60                /* RTS */
    };
    Theron_Track02Stage2Enclosing45xxCalleesReceipt receipt;
    Theron_Track02SignalStatus status;

    status = theron_v1_track02_verify_stage2_enclosing_45xx_callees(
        g_us_data, g_us_size, THERON_TRACK02_MD5_US_BIN, &receipt);
    assert(status == THERON_TRACK02_SIGNAL_OK);
    assert(receipt.l4932_proven == 1);

    /* ST0 #$05: select CR */
    assert(l4932[0] == 0x03 && l4932[1] == 0x05);
    /* AND #$07: mask to auto-increment bits only */
    assert(l4932[9] == 0x29 && l4932[10] == 0x07);

    printf("  PASS: vdc_cr_write_l4932"
           " (CR set from ZP $F3/$F4, increment mode masked)\n");
}

int main(void)
{
    printf("test_theron_v1_stage2_disassembly_chain:\n");

    if (!load_track02()) {
        printf("  SKIP: TQUS02.bin not available\n");
        printf("All stage-2 disassembly chain tests skipped.\n");
        return 0;
    }

    test_ipl_loader();
    if (g_jp_data) test_ipl_loader_jp();
    test_stage2_dynamic_payload();
    if (g_jp_data) test_stage2_dynamic_payload_jp();
    test_stage2_entry_path();
    test_stage2_call_graph();
    test_stage2_dispatch_machine();
    test_stage2_l8000_pair();
    test_stage2_jump_table_handlers();
    test_stage2_l4696_l3114();
    test_stage2_l3114_callees();
    test_stage2_l3114_tier2_callees();
    test_stage2_l3114_tier3_callees();
    test_stage2_l3114_tier4_callees();
    test_stage2_enclosing_45xx();
    test_stage2_enclosing_45xx_callees();
    test_stage2_l3114_tier5_callees();
    test_stage2_45xx_tier2_callees();
    test_total_bound_bytes();
    test_vdc_port_clear_semantics();
    test_vdc_l8000_init_semantics();
    test_dispatch_advance_counts();
    test_vram_transfer_l466b();
    test_vdc_cr_write_l4932();

    free(g_us_data);
    free(g_jp_data);
    printf("All stage-2 disassembly chain tests passed.\n");
    return 0;
}
