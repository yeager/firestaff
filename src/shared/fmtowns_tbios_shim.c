#include "fmtowns_tbios_shim.h"
#include <stddef.h>
#include <string.h>

/*
 * FMT_FNT.ROM is the separate 256 KiB FM Towns font device:
 *   * 8192 native glyph slots
 *   * 32 bytes per 16x16 glyph (two big-endian bytes per row)
 *
 * The slot mapping is not a linear JIS row/column table.  It is the exact
 * mapping used by Tsugaru's TownsPhysicalMemory::KanjiROMAccess::FontROMCode
 * after converting Shift-JIS to JIS X 0208.  FMT_F20.ROM is a different,
 * 512 KiB system ROM and contains no complete substitute font table.
 */

#define FMTOWNS_FNT_GLYPH_COUNT 8192u
#define FMTOWNS_FNT_GLYPH_BYTES 32u
#define FMTOWNS_FNT_ROM_BYTES \
    (FMTOWNS_FNT_GLYPH_COUNT * FMTOWNS_FNT_GLYPH_BYTES)

static const uint8_t *g_rom = NULL;
static size_t         g_rom_size = 0;

int fmtowns_tbios_shim_load_rom_pc34(const uint8_t *font_rom_bytes,
                                     size_t font_rom_size) {
    if (font_rom_bytes == NULL) {
        g_rom = NULL;
        g_rom_size = 0;
        return 1;
    }
    if (font_rom_size != FMTOWNS_FNT_ROM_BYTES) return 0;
    g_rom = font_rom_bytes;
    g_rom_size = font_rom_size;
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

/* Exact Tsugaru FontROMCode mapping for the JIS pair most recently written
 * to CFF94/CFF95.  It returns one of FMT_FNT.ROM's 8192 16x16 glyph slots. */
static unsigned int fnt_rom_code(uint8_t jis_high, uint8_t jis_low) {
    unsigned int block;
    unsigned int x;
    unsigned int y;
    if (jis_high < 0x28u) {
        block = ((unsigned int)jis_low - 0x20u) >> 5u;
        x = (unsigned int)jis_low & 0x1fu;
        y = (unsigned int)jis_high & 7u;
        if (block == 1u) block = 2u;
        else if (block == 2u) block = 1u;
        return block * 32u * 8u + y * 32u + x;
    }
    block = ((((unsigned int)jis_high - 0x30u) >> 4u) * 3u) +
        (((unsigned int)jis_low - 0x20u) >> 5u);
    x = (unsigned int)jis_low & 0x1fu;
    y = (unsigned int)jis_high & 0x0fu;
    return 0x400u + block * 32u * 16u + y * 32u + x;
}

static fmtowns_bios_status_t shim_fetch_sjis(
        void *ctx, uint8_t lead, uint8_t trail,
        uint8_t *out, size_t cap, size_t *n,
        uint8_t *gw, uint8_t *gh) {
    (void)ctx;
    if (!g_rom) return FMTOWNS_BIOS_HOST_UNBOUND;
    if (!out || cap == 0) return FMTOWNS_BIOS_HOST_BAD_ARGS;

    uint8_t row, col;
    if (!sjis_to_jis(lead, trail, &row, &col)) return FMTOWNS_BIOS_HOST_FAILED;
    if (cap < FMTOWNS_FNT_GLYPH_BYTES) return FMTOWNS_BIOS_HOST_BAD_ARGS;
    unsigned int idx = fnt_rom_code(row, col);
    size_t src = (size_t)idx * FMTOWNS_FNT_GLYPH_BYTES;
    if (idx >= FMTOWNS_FNT_GLYPH_COUNT ||
        src + FMTOWNS_FNT_GLYPH_BYTES > g_rom_size)
        return FMTOWNS_BIOS_HOST_FAILED;
    memcpy(out, g_rom + src, FMTOWNS_FNT_GLYPH_BYTES);
    if (n)  *n  = FMTOWNS_FNT_GLYPH_BYTES;
    if (gw) *gw = 16u;
    if (gh) *gh = 16u;
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
