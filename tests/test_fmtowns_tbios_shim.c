#include "fmtowns_tbios_shim.h"
#include "fmtowns_bios_host.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ROM_MIN_BYTES per the shim: KANJI_OFFSET + 94*94*32 = 0x40000 + 282752
 * = 0x84c00 = 543232 bytes. Round up to 640 KiB for the fixture. */
#define FIX_ROM_SIZE (640u * 1024u)

static uint8_t *make_valid_rom_fixture(void) {
    uint8_t *rom = (uint8_t *)calloc(1, FIX_ROM_SIZE);
    /* Signature required by rom_signature_ok: "V" + digit at [0..1],
     * "towns" at 0x10, "tbios" at 0x18. */
    memcpy(rom + 0x00, "V31L22A", 7);
    memcpy(rom + 0x10, "towns", 5);
    memcpy(rom + 0x18, "tbios", 5);
    /* Plant identifiable bytes for ANK 'A' (0x41) at ANK_OFFSET + 0x41*16. */
    size_t ank = 0x3d800u + 0x41u * 16u;
    for (int i = 0; i < 16; ++i) rom[ank + i] = (uint8_t)(0xa0 + i);
    /* Plant identifiable bytes for JIS row 0x21, col 0x21 at
     * KANJI_OFFSET + 0. */
    for (int i = 0; i < 32; ++i) rom[0x40000u + i] = (uint8_t)(0xb0 + i);
    return rom;
}

int main(void) {
    /* Fail-closed before ROM is loaded. */
    assert(fmtowns_tbios_shim_has_rom_pc34() == 0);
    const fmtowns_bios_host_t *host = fmtowns_tbios_shim_host_pc34();
    assert(host != NULL);
    assert(host->tbios_fetch_sjis_glyph != NULL);
    /* Every other slot is NULL. */
    assert(host->call_tbios == NULL);
    assert(host->call_timing == NULL);
    assert(host->io_port_read_u8 == NULL);

    uint8_t buf[32];
    size_t n; uint8_t gw, gh;
    /* Fail-closed: UNBOUND without ROM. */
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x93, 0xfa, buf, sizeof(buf), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_UNBOUND);

    /* Bad ROM rejected. */
    uint8_t bogus[128] = {0};
    assert(fmtowns_tbios_shim_load_rom_pc34(bogus, sizeof(bogus)) == 0);
    assert(fmtowns_tbios_shim_has_rom_pc34() == 0);

    /* Undersized ROM with good header still rejected. */
    uint8_t small[1024] = {0};
    memcpy(small + 0x00, "V31L22A", 7);
    memcpy(small + 0x10, "towns", 5);
    memcpy(small + 0x18, "tbios", 5);
    assert(fmtowns_tbios_shim_load_rom_pc34(small, sizeof(small)) == 0);

    /* Valid ROM accepted. */
    uint8_t *rom = make_valid_rom_fixture();
    assert(fmtowns_tbios_shim_load_rom_pc34(rom, FIX_ROM_SIZE) == 1);
    assert(fmtowns_tbios_shim_has_rom_pc34() == 1);

    /* ANK path: (0x41, 0) returns the 16 bytes we planted. */
    memset(buf, 0, sizeof(buf));
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x41, 0x00, buf, sizeof(buf), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_OK);
    assert(n == 16 && gw == 8 && gh == 16);
    for (int i = 0; i < 16; ++i) assert(buf[i] == (uint8_t)(0xa0 + i));

    /* SJIS path: (0x81, 0x40) maps to JIS row 0x21, col 0x21. */
    memset(buf, 0, sizeof(buf));
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x81, 0x40, buf, sizeof(buf), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_OK);
    assert(n == 32 && gw == 16 && gh == 16);
    for (int i = 0; i < 32; ++i) assert(buf[i] == (uint8_t)(0xb0 + i));

    /* Out-of-range SJIS trail: FAILED (ROM loaded but pair invalid). */
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x81, 0x3f, buf, sizeof(buf), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_FAILED);

    /* Buffer too small: BAD_ARGS. */
    uint8_t tiny[8];
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x81, 0x40, tiny, sizeof(tiny), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_BAD_ARGS);

    /* NULL buf: BAD_ARGS. */
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x81, 0x40, NULL, 32, &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_BAD_ARGS);

    /* Unload restores fail-closed. */
    fmtowns_tbios_shim_load_rom_pc34(NULL, 0);
    assert(fmtowns_tbios_shim_has_rom_pc34() == 0);
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x81, 0x40, buf, sizeof(buf), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_UNBOUND);

    /* End-to-end: bind the shim into the global BIOS host and go through
     * the dispatcher. */
    fmtowns_tbios_shim_load_rom_pc34(rom, FIX_ROM_SIZE);
    fmtowns_bios_host_bind_pc34(host);
    memset(buf, 0, sizeof(buf));
    assert(fmtowns_bios_host_fetch_sjis_glyph_pc34(0x81, 0x40, buf, sizeof(buf), &n, &gw, &gh)
           == FMTOWNS_BIOS_HOST_OK);
    assert(buf[0] == 0xb0);
    /* Unsupported slot through dispatcher: UNSUPPORTED. */
    fmtowns_bios_regs_t regs = {0};
    assert(fmtowns_bios_host_call_pc34(FMTOWNS_BIOS_SLOT_TBIOS, &regs)
           == FMTOWNS_BIOS_HOST_UNSUPPORTED);
    fmtowns_bios_host_unbind_pc34();

    free(rom);
    puts("All fmtowns_tbios_shim tests passed.");
    return 0;
}
