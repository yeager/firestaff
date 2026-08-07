#include "fmtowns_bios_host.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int called_tbios = 0;
static fmtowns_bios_status_t stub_tbios(void *ctx, fmtowns_bios_regs_t *r) {
    (void)ctx; (void)r; ++called_tbios; return FMTOWNS_BIOS_HOST_OK;
}

static int called_sjis = 0;
static fmtowns_bios_status_t stub_sjis(
        void *ctx, uint8_t l, uint8_t t,
        uint8_t *out, size_t cap, size_t *n,
        uint8_t *w, uint8_t *h) {
    (void)ctx; (void)l; (void)t; (void)cap;
    ++called_sjis;
    if (n) *n = 32;
    if (w) *w = 16;
    if (h) *h = 16;
    memset(out, 0xa5, 32);
    return FMTOWNS_BIOS_HOST_OK;
}

int main(void) {
    fmtowns_bios_regs_t regs = {0};
    /* Fail-closed default: no host bound. */
    assert(fmtowns_bios_host_current_pc34() == NULL);
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TBIOS, &regs)
           == FMTOWNS_BIOS_HOST_UNBOUND);
    uint8_t buf[32];
    size_t n; uint8_t w, h;
    assert(fmtowns_bios_host_fetch_sjis_glyph_pc34(
        0x93, 0xfa, buf, sizeof(buf), &n, &w, &h)
        == FMTOWNS_BIOS_HOST_UNBOUND);

    /* Bind a partial host. */
    fmtowns_bios_host_t host = {0};
    host.call_tbios = stub_tbios;
    host.tbios_fetch_sjis_glyph = stub_sjis;
    fmtowns_bios_host_bind_pc34(&host);
    assert(fmtowns_bios_host_current_pc34() == &host);

    /* Bound slot: dispatches. */
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TBIOS, &regs)
           == FMTOWNS_BIOS_HOST_OK);
    assert(called_tbios == 1);

    /* Unbound slot on partial host: UNSUPPORTED (not UNBOUND). */
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TIMING, &regs)
           == FMTOWNS_BIOS_HOST_UNSUPPORTED);

    /* Bad slot rejected. */
    assert(fmtowns_bios_host_call_pc34((fmtowns_bios_slot_t)0x99, &regs)
           == FMTOWNS_BIOS_HOST_BAD_SLOT);

    /* NULL regs rejected. */
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TBIOS, NULL)
           == FMTOWNS_BIOS_HOST_BAD_ARGS);

    /* SJIS glyph fetch through the host. */
    assert(fmtowns_bios_host_fetch_sjis_glyph_pc34(
        0x93, 0xfa, buf, sizeof(buf), &n, &w, &h)
        == FMTOWNS_BIOS_HOST_OK);
    assert(called_sjis == 1);
    assert(n == 32 && w == 16 && h == 16);
    assert(buf[0] == 0xa5);

    /* I/O read/write with no port fns: UNSUPPORTED. */
    uint8_t v;
    assert(fmtowns_bios_host_io_read_u8_pc34(0x04E9, &v)
           == FMTOWNS_BIOS_HOST_UNSUPPORTED);
    assert(fmtowns_bios_host_io_write_u8_pc34(0x04E9, 0)
           == FMTOWNS_BIOS_HOST_UNSUPPORTED);

    fmtowns_bios_host_unbind_pc34();
    assert(fmtowns_bios_host_current_pc34() == NULL);
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TBIOS, &regs)
           == FMTOWNS_BIOS_HOST_UNBOUND);

    puts("All fmtowns_bios_host tests passed.");
    return 0;
}
