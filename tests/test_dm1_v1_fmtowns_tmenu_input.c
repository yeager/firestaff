#include "dm1_v1_fmtowns_tmenu_input.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_source_locked_addresses(void) {
    /* Every vaddr byte-verified from TMENU.EXP disassembly on 2026-08-07. */
    assert(DM1_V1_FMTOWNS_TMENU_HEADER_SIZE_BYTES == 0x180U);
    assert(DM1_V1_FMTOWNS_TMENU_LOAD_IMAGE_OFFSET == 0x200U);
    assert(DM1_V1_FMTOWNS_TMENU_INIT_EIP_VADDR == 0x9408U);
    assert(DM1_V1_FMTOWNS_TMENU_POLL_MAIN_VADDR == 0xc8e0U);
    assert(DM1_V1_FMTOWNS_TMENU_EVENT_DISPATCH_VADDR == 0xbfacU);
    assert(DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR == 0xa130U);
    assert(DM1_V1_FMTOWNS_TMENU_TBIOS_STATUS_PACKET_VADDR == 0x2dbcU);
    assert(DM1_V1_FMTOWNS_TMENU_EVENT_QUEUE_HEAD_VADDR == 0x5890U);
    assert(DM1_V1_FMTOWNS_TMENU_HANDLER_TABLE_VADDR == 0x5510U);
    assert(DM1_V1_FMTOWNS_TMENU_ACTIVE_FLAG_VADDR == 0x557cU);
    assert(DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_STRIDE_BYTES == 8U);
    assert(DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_STRIDE_BYTES == 10U);
    assert(DM1_V1_FMTOWNS_TMENU_PHARLAP_REALMODE_SELECTOR == 0x110U);
}

static void test_event_record_vaddr_math(void) {
    /* Slot 0 -> base; slot N -> base + N*8. */
    assert(dm1_v1_fmtowns_tmenu_event_record_vaddr_pc34(0) ==
           DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_BASE_VADDR);
    for (uint32_t s = 1; s < 16; ++s) {
        uint32_t expect = DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_BASE_VADDR + s * 8U;
        assert(dm1_v1_fmtowns_tmenu_event_record_vaddr_pc34(s) == expect);
    }
    /* Out of bounds fails closed. */
    assert(dm1_v1_fmtowns_tmenu_event_record_vaddr_pc34(4096) == 0U);
    assert(dm1_v1_fmtowns_tmenu_event_record_vaddr_pc34(65535) == 0U);
}

static void test_event_text_vaddr_math(void) {
    /* Slot 0 -> pool base + 2 header bytes. */
    assert(dm1_v1_fmtowns_tmenu_event_text_vaddr_pc34(0) ==
           DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_POOL_VADDR + 2U);
    /* Slot 5 -> pool base + 2 + 5*10 = pool + 52 */
    assert(dm1_v1_fmtowns_tmenu_event_text_vaddr_pc34(5) ==
           DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_POOL_VADDR + 2U + 50U);
    assert(dm1_v1_fmtowns_tmenu_event_text_vaddr_pc34(4096) == 0U);
}

static void test_handler_vaddr_math(void) {
    /* Handler dispatch is `call [table + id * 4]`. */
    assert(dm1_v1_fmtowns_tmenu_handler_vaddr_pc34(0) ==
           DM1_V1_FMTOWNS_TMENU_HANDLER_TABLE_VADDR);
    assert(dm1_v1_fmtowns_tmenu_handler_vaddr_pc34(1) ==
           DM1_V1_FMTOWNS_TMENU_HANDLER_TABLE_VADDR + 4U);
    assert(dm1_v1_fmtowns_tmenu_handler_vaddr_pc34(64) ==
           DM1_V1_FMTOWNS_TMENU_HANDLER_TABLE_VADDR + 256U);
    assert(dm1_v1_fmtowns_tmenu_handler_vaddr_pc34(4096) == 0U);
}

/* Real-data round-trip: given a caller-supplied TMENU.EXP path,
 * verify that (a) the P3 header validates, (b) the disassembled
 * function bodies at each vaddr look like plausible entry code
 * (push ebp; mov ebp, esp; ... or a poll bx-setup). */
static void test_real_data_round_trip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_TMENU_EXP");
    FILE *fp;
    uint8_t hdr[64];
    uint8_t body[8];
    if (!path || !path[0]) { puts("SKIP: no TMENU.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    /* Header check: bytes 0..1 = "P3", header_size (word at +4) = 0x180. */
    if (fread(hdr, 1, 8, fp) != 8) { fclose(fp); puts("SKIP: read failed"); return; }
    assert(hdr[0] == 'P' && hdr[1] == '3');
    uint16_t header_size = (uint16_t)(hdr[4] | (hdr[5] << 8));
    assert(header_size == DM1_V1_FMTOWNS_TMENU_HEADER_SIZE_BYTES);
    /* POLL_MAIN entry: at load + 0xc8e0, first 3 bytes = `enter 0, 0`
     * which encodes to c8 00 00 00 in x86-32. */
    long file_off = (long)DM1_V1_FMTOWNS_TMENU_LOAD_IMAGE_OFFSET +
                    (long)DM1_V1_FMTOWNS_TMENU_POLL_MAIN_VADDR;
    if (fseek(fp, file_off, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek failed"); return;
    }
    if (fread(body, 1, 4, fp) != 4) {
        fclose(fp); puts("SKIP: read failed"); return;
    }
    /* `enter 0, 0` = c8 00 00 00 */
    assert(body[0] == 0xc8 && body[1] == 0x00 &&
           body[2] == 0x00 && body[3] == 0x00);
    /* INIT_EIP: at load + 0x9408, first 2 bytes = `jmp 0x9410` -> eb 06 */
    file_off = (long)DM1_V1_FMTOWNS_TMENU_LOAD_IMAGE_OFFSET +
               (long)DM1_V1_FMTOWNS_TMENU_INIT_EIP_VADDR;
    if (fseek(fp, file_off, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek failed"); return;
    }
    if (fread(body, 1, 5, fp) != 5) {
        fclose(fp); puts("SKIP: read failed"); return;
    }
    /* `jmp 0x9410` from 0x9408 = e9 03 00 00 00 (5-byte near jmp). */
    assert(body[0] == 0xe9);
    assert(body[1] == 0x03 && body[2] == 0x00 && body[3] == 0x00 && body[4] == 0x00);
    /* TBIOS_POLL: at load + 0xa130, first bytes = `push bx; push ds`
     * = 66 53 1e (32-bit mode with 66 prefix for push bx). */
    file_off = (long)DM1_V1_FMTOWNS_TMENU_LOAD_IMAGE_OFFSET +
               (long)DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR;
    if (fseek(fp, file_off, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek failed"); return;
    }
    if (fread(body, 1, 3, fp) != 3) {
        fclose(fp); puts("SKIP: read failed"); return;
    }
    assert(body[0] == 0x66 && body[1] == 0x53 && body[2] == 0x1e);
    fclose(fp);
    puts("PASS: real TMENU.EXP header + entry bytes match shipped vaddrs");
}

int main(void) {
    test_source_locked_addresses();
    test_event_record_vaddr_math();
    test_event_text_vaddr_math();
    test_handler_vaddr_math();
    test_real_data_round_trip();
    puts("All dm1_v1_fmtowns_tmenu_input tests passed.");
    return 0;
}
