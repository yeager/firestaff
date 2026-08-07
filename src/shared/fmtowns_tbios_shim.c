#include "fmtowns_tbios_shim.h"
#include <stddef.h>
#include <string.h>

/*
 * FMT_F20.ROM font layout (Fujitsu FM Towns TBIOS ROM):
 *   * ANK 8x16 (256 narrow glyphs)   : offset 0x3d800, 16 bytes/glyph
 *   * JIS X 0208 16x16 (kanji/kana)  : offset 0x40000, 32 bytes/glyph
 *
 * JIS X 0208 glyph index inside the ROM is derived from a
 * Shift-JIS pair (lead, trail) by the standard SJIS -> JIS row/col
 * conversion, then indexed as ((row - 0x21) * 94 + (col - 0x21)) * 32.
 *
 * These offsets are public in the FM Towns technical literature and
 * are what a Tsugaru font-render path also consumes. The shim reads
 * them directly; no CPU emulation required for text-render service.
 */

#define ANK_OFFSET       0x3d800u
#define ANK_ENTRY_BYTES  16u
#define ANK_ENTRY_W       8u
#define ANK_ENTRY_H      16u

#define KANJI_OFFSET     0x40000u
#define KANJI_ENTRY_BYTES 32u
#define KANJI_ENTRY_W    16u
#define KANJI_ENTRY_H    16u

#define ROM_MIN_BYTES    (KANJI_OFFSET + 94u * 94u * KANJI_ENTRY_BYTES)

static const uint8_t *g_rom = NULL;
static size_t         g_rom_size = 0;

static int rom_signature_ok(const uint8_t *rom, size_t size) {
    if (!rom || size < ROM_MIN_BYTES) return 0;
    /* FMT_F20.ROM is 512 KiB and starts with "V31L" or a similar
     * TBIOS version fingerprint at offset 0. Do not enforce a
     * single fingerprint here (users may load a compatible dump);
     * only check the printable-ASCII "Vxx" prefix and the "towns"
     * / "tbios" strings that every FMT_F20 revision carries at
     * offsets 0x10 and 0x18 respectively. */
    if (rom[0] != 'V' || rom[1] < '0' || rom[1] > '9') return 0;
    if (memcmp(rom + 0x10, "towns", 5) != 0) return 0;
    if (memcmp(rom + 0x18, "tbios", 5) != 0) return 0;
    return 1;
}

int fmtowns_tbios_shim_load_rom_pc34(const uint8_t *rom_bytes,
                                     size_t rom_size) {
    if (rom_bytes == NULL) {
        g_rom = NULL;
        g_rom_size = 0;
        return 1;
    }
    if (!rom_signature_ok(rom_bytes, rom_size)) return 0;
    g_rom = rom_bytes;
    g_rom_size = rom_size;
    return 1;
}

int fmtowns_tbios_shim_has_rom_pc34(void) {
    return g_rom != NULL;
}

/* Convert a Shift-JIS (lead, trail) pair to a JIS X 0208 (row, col)
 * pair. Both row and col are in 0x21..0x7e range on success. Returns
 * 1 on success, 0 on out-of-range input. */
static int sjis_to_jis(uint8_t lead, uint8_t trail,
                       uint8_t *row_out, uint8_t *col_out) {
    unsigned int r, row, col;
    if (lead >= 0x81u && lead <= 0x9fu)      r = (unsigned int)lead - 0x81u;
    else if (lead >= 0xe0u && lead <= 0xfcu) r = (unsigned int)lead - 0xc1u;
    else return 0;
    if (trail < 0x40u || trail == 0x7fu || trail > 0xfcu) return 0;
    if (trail < 0x9fu) {
        row = 2u * r + 0x21u;
        col = (trail < 0x7fu ? (unsigned int)trail - 0x40u
                             : (unsigned int)trail - 0x41u) + 0x21u;
    } else {
        row = 2u * r + 0x22u;
        col = (unsigned int)trail - 0x9eu + 0x21u;
    }
    if (row < 0x21u || row > 0x7eu) return 0;
    if (col < 0x21u || col > 0x7eu) return 0;
    *row_out = (uint8_t)row;
    *col_out = (uint8_t)col;
    return 1;
}

static fmtowns_bios_status_t shim_fetch_sjis(
        void *ctx, uint8_t lead, uint8_t trail,
        uint8_t *out, size_t cap, size_t *n,
        uint8_t *gw, uint8_t *gh) {
    (void)ctx;
    if (!g_rom) return FMTOWNS_BIOS_HOST_UNBOUND;
    if (!out || cap == 0) return FMTOWNS_BIOS_HOST_BAD_ARGS;

    /* ANK 8x16 path when caller passed lead in the ASCII range and
     * trail == 0 (a convention this shim documents; JDM font module
     * only ever calls with true SJIS pairs, so this branch is here
     * mainly for symmetry and testability). */
    if (trail == 0u && lead < 0x80u) {
        if (cap < ANK_ENTRY_BYTES) return FMTOWNS_BIOS_HOST_BAD_ARGS;
        size_t src = ANK_OFFSET + (size_t)lead * ANK_ENTRY_BYTES;
        if (src + ANK_ENTRY_BYTES > g_rom_size) return FMTOWNS_BIOS_HOST_FAILED;
        memcpy(out, g_rom + src, ANK_ENTRY_BYTES);
        if (n)  *n  = ANK_ENTRY_BYTES;
        if (gw) *gw = (uint8_t)ANK_ENTRY_W;
        if (gh) *gh = (uint8_t)ANK_ENTRY_H;
        return FMTOWNS_BIOS_HOST_OK;
    }

    uint8_t row, col;
    if (!sjis_to_jis(lead, trail, &row, &col)) return FMTOWNS_BIOS_HOST_FAILED;
    if (cap < KANJI_ENTRY_BYTES) return FMTOWNS_BIOS_HOST_BAD_ARGS;
    unsigned int idx = ((unsigned int)row - 0x21u) * 94u + ((unsigned int)col - 0x21u);
    size_t src = KANJI_OFFSET + (size_t)idx * KANJI_ENTRY_BYTES;
    if (src + KANJI_ENTRY_BYTES > g_rom_size) return FMTOWNS_BIOS_HOST_FAILED;
    memcpy(out, g_rom + src, KANJI_ENTRY_BYTES);
    if (n)  *n  = KANJI_ENTRY_BYTES;
    if (gw) *gw = (uint8_t)KANJI_ENTRY_W;
    if (gh) *gh = (uint8_t)KANJI_ENTRY_H;
    return FMTOWNS_BIOS_HOST_OK;
}

static const fmtowns_bios_host_t g_host = {
    NULL,           /* ctx */
    NULL,           /* call_tbios */
    NULL,           /* call_secondary */
    NULL,           /* call_timing */
    NULL,           /* call_hardware_init */
    NULL,           /* io_port_read_u8 */
    NULL,           /* io_port_write_u8 */
    shim_fetch_sjis /* tbios_fetch_sjis_glyph */
};

const fmtowns_bios_host_t *fmtowns_tbios_shim_host_pc34(void) {
    return &g_host;
}
