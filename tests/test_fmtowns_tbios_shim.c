#include "fmtowns_tbios_shim.h"
#include "fmtowns_bios_host.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FMT_FNT.ROM is 8192 native 16x16 glyphs at 32 bytes per glyph. */
#define FIX_ROM_SIZE (8192u * 32u)

static uint8_t *make_valid_rom_fixture(void) {
    uint8_t *rom = (uint8_t *)calloc(1, FIX_ROM_SIZE);
    /* JIS row 0x21/column 0x21 resolves to native FNT slot zero. */
    for (int i = 0; i < 32; ++i) rom[i] = (uint8_t)(0xb0 + i);
    return rom;
}

static uint32_t fnv1a(const uint8_t *bytes, size_t count) {
    uint32_t hash = UINT32_C(2166136261);
    size_t i;
    for (i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static void test_real_font_rom_if_configured(const fmtowns_bios_host_t *host) {
    const char *path = getenv("FIRESTAFF_FMTOWNS_FONT_ROM");
    FILE *file;
    uint8_t *bytes;
    uint8_t glyph[32];
    size_t size;
    size_t n;
    uint8_t width;
    uint8_t height;

    if (!path || !path[0]) return;
    file = fopen(path, "rb");
    assert(file != NULL);
    assert(fseek(file, 0L, SEEK_END) == 0);
    size = (size_t)ftell(file);
    assert(size == FIX_ROM_SIZE);
    assert(fseek(file, 0L, SEEK_SET) == 0);
    bytes = (uint8_t *)malloc(size);
    assert(bytes != NULL);
    assert(fread(bytes, 1u, size, file) == size);
    fclose(file);

    /* Tsugaru's FontROMCode maps the F31J chooser's first glyph, ど
     * (Shift-JIS 82c7 -> JIS 2449), to native slot 393.  The checked
     * FMT_FNT.ROM glyph has this independently captured FNV-1a receipt. */
    assert(fmtowns_tbios_shim_load_rom_pc34(bytes, size) == 1);
    assert(host->tbios_fetch_sjis_glyph(NULL, 0x82, 0xc7, glyph,
                                        sizeof(glyph), &n, &width, &height)
           == FMTOWNS_BIOS_HOST_OK);
    assert(n == sizeof(glyph) && width == 16u && height == 16u);
    assert(fnv1a(glyph, sizeof(glyph)) == UINT32_C(0xaf14d837));
    assert(fmtowns_tbios_shim_load_rom_pc34(NULL, 0) == 1);
    free(bytes);
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

    /* An incomplete FMT_FNT.ROM is rejected. */
    uint8_t small[1024] = {0};
    assert(fmtowns_tbios_shim_load_rom_pc34(small, sizeof(small)) == 0);

    /* Valid ROM accepted. */
    uint8_t *rom = make_valid_rom_fixture();
    assert(fmtowns_tbios_shim_load_rom_pc34(rom, FIX_ROM_SIZE) == 1);
    assert(fmtowns_tbios_shim_has_rom_pc34() == 1);

    /* SJIS path: (0x81, 0x40) maps to JIS row 0x21, col 0x21 and
     * therefore FMT_FNT native slot zero. */
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
    assert(fmtowns_tbios_shim_load_rom_pc34(NULL, 0) == 1);
    free(rom);
    test_real_font_rom_if_configured(host);
    puts("All fmtowns_tbios_shim tests passed.");
    return 0;
}
