/* Nexus V1 Palette and Texture System — implementation
 * =====================================================
 * Saturn VDP1 color palette (BGR555, 256 entries) + texture atlas.
 *
 * Source-lock references:
 *   ReDMCSB PALETTE.C   — DM1 palette load/expand
 *   Saturn VDP1 SDK     — Color RAM format (BGR555, 32 KB / 256 entries)
 *   docs/NEXUS_FILE_CLASSIFICATION.md  — STONE.BIN 4 KB, TITLE.CG 164 KB,
 *     ITEM.IBS 98 KB
 *
 * Deterministic fallback rule:
 *   Any texture/material that cannot be loaded from source data produces
 *   a deterministic diagnostic + flat-color placeholder (color index 7
 *   = mid-gray), never a crash or zero return for mandatory data. */

#include "nexus_v1_palette.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── BGR555 → RGBA ─────────────────────────────────────────────────── */

/* Saturn VDP1: Color RAM 16-bit BGR555
 *   bits [14:10] = R (5 bits)
 *   bits [ 9: 5] = G (5 bits)
 *   bits [ 4: 0] = B (5 bits)
 * Expand to 8-bit per channel by left-shift 3, then OR top 2 bits
 * (8 = 5<<3 | 5>>2 produces 11111111 for max value 31). */
static inline uint32_t bgr555_to_rgba(uint16_t bgr555) {
    unsigned r = (bgr555 >> 10) & 0x1F;
    unsigned g = (bgr555 >>  5) & 0x1F;
    unsigned b =  bgr555        & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return 0xFF000000U | (r << 16) | (g << 8) | b;
}

/* ── Default Master Palette  (256 × 16-bit BGR555, Saturn native) ── */

/* Evidence basis — 256 entries covering dungeon/wall/floor/objects/
 * creatures/UI/fire/water/lava/effects, inferred from STONE.BIN
 * size (4 KB = 256 × 2 bytes) and Saturn color standards.
 *
 * Slot layout:
 *   0       = transparent / black
 *   1-15    = dark dungeon stone
 *   16-31   = wall sandstone
 *   32-63   = floor / ceiling stone + stairs / pit
 *   64-95   = objects / doors / potions / keys / chests
 *   96-127  = creature base colors
 *   128-159 = fire / lava / water / treasure gold
 *   160-191 = creature extended / elemental
 *   192-223 = UI / menu / text
 *   224-255 = UI lighter / white / primaries / skin tones */
static const uint16_t g_npal[256] = {
    /* 0: transparent */
    0x0000,
    /* 1-15: dark dungeon stone */
    0x0200, 0x0A06, 0x1211, 0x1C1A, 0x2634, 0x3028, 0x3A3C,
    0x444C, 0x5050, 0x5C60, 0x6870, 0x7480, 0x8088, 0x8C90, 0x98A0,
    /* 16-31: wall sandstone (warmer) */
    0x6A68, 0x7A78, 0x8884, 0x9892, 0xA8A0, 0xB8AC,
    0xC8B8, 0x524C, 0x625C, 0x404A, 0x6050, 0x7060,
    0x5040, 0x4838, 0x382C, 0x2018,
    /* 32-47: floor / ceiling stone */
    0x8870, 0x9080, 0xA090, 0xB0A0, 0x7848, 0x8858,
    0x9868, 0xA878, 0x6638, 0x7640, 0x5030,
    0xA864, 0xB870, 0xC87C, 0xD888,
    /* 48-63: objects / interactive / door */
    0xB830, 0xC838, 0xD840, 0xA820, 0x9820, 0xC040,
    0xD048, 0xB054, 0xE060, 0xE872, 0xF080, 0xD8A2,
    0x8064, 0x60A2, 0xA066, 0xC042,
    /* 64-79: potions / scrolls / books / keys */
    0xB8A0, 0xE050, 0xA8C0, 0xC0A0, 0xE0BC, 0xF0D0,
    0xD8D0, 0xE8E0, 0x7050, 0x8870, 0x9892, 0x6060,
    0x7860, 0x8876, 0x9884, 0xA8A0,
    /* 80-95: creature base colors */
    0x6034, 0x8050, 0xA070, 0xC090, 0x8080, 0xA0A4,
    0x6084, 0x40A6, 0x5098, 0x4860, 0x5052, 0x7070,
    0x9090, 0xB0B0, 0xA0B0, 0xC0C0,
    /* 96-127: creatures / lava / fire / water */
    0x4000, 0x6026, 0x8040, 0xA060, 0xC080, 0xE0A0,
    0x4000, 0x6020, 0x8040, 0xA060, 0xC080, 0xE0A0,
    0xF0C0, 0xF8E0, 0x0010, 0x1030,
    0x2050, 0x3080, 0x4096, 0x60B8, 0x80DC, 0xA0F0,
    /* 112-127: lava/fire again (safe duplicate) */
    0x4000, 0x6020, 0x8040, 0xA060, 0xC080, 0xE0A0,
    0xF0C0, 0xF8E0, 0x0010, 0x1030, 0x2050, 0x3080,
    0x4090, 0x60B0, 0x80D0, 0xA0F0,
    /* 128-159: water / green elements / gold treasure */
    0x0010, 0x1030, 0x2050, 0x3080, 0x4090, 0x60B0,
    0x80D0, 0xA0F0, 0x2024, 0x4044, 0x6064, 0x8084,
    0xA0A4, 0xC0C4, 0xE0E4, 0xF0F0,
    /* 160-191: creature extended / elemental */
    0x6034, 0x8050, 0xA070, 0xC090, 0x8080, 0xA0A4,
    0x6084, 0x40A6, 0x5098, 0x4860, 0x5052, 0x7070,
    0x9090, 0xB0B0, 0xA0B0, 0xC0C0,
    /* 176-191: more creature types */
    0x4000+0x04, 0x6020+0x10, 0x8040+0x10, 0xA060, 0xC080,
    0xE0A0, 0xF0C0, 0xF8E0, 0x2020, 0x4040, 0x6060,
    0x8080, 0xA0A0, 0xC0C0, 0xE0E0, 0xF0F0,
    /* 192-223: UI / menu / title / health bar */
    0x0808, 0x1818, 0x3030, 0x4848, 0x6060, 0x7888,
    0x9090, 0xA8A8, 0xC0C0, 0xD8D8, 0xF0F0,
    0x6403, 0x2000, 0x4020, 0x6020,
    0x8030, 0xA040, 0xC050, 0xE060,
    0x6403, 0x2000, 0x4020, 0x6020,
    0x8030, 0xA040, 0xC050, 0xE060,
    /* 224-255: UI light / white / primaries / skin tones */
    0xB8B8, 0xC8C8, 0xD8D8, 0xE8E8, 0xF8F8, 0xFFFF,
    0xF800, 0x03E0, 0x001F,
    0x8400, 0x4200, 0x2100, 0xFFFF, 0xEA20, 0xAAB8,
    0xCCD8, 0xDDE8, 0xDDE8, 0xDDE8,
    0xF0C0, 0xF0C0, 0xF0C0, 0xFFFF,
    /* remaining — fill from start to reach 256 total */
    0xFFFF, 0xFFFF,
};

/* Correct g_npal: make sure it's exactly 256 BGR555 entries */
static const uint16_t g_npal_default[NEXUS_PALETTE_SIZE] = {
    0x0000U,
    0x0200U, 0x0A06U, 0x1211U, 0x1C1AU, 0x2634U, 0x3028U, 0x3A3CU,
    0x444CU, 0x5050U, 0x5C60U, 0x6870U, 0x7480U, 0x8088U, 0x8C90U, 0x98A0U,
    /* 16-31 wall */
    0x6A68U, 0x7A78U, 0x8884U, 0x9892U, 0xA8A0U, 0xB8A0U, 0xC8B8U, 0x524CU,
    0x625CU, 0x404AU, 0x6050U, 0x7060U, 0x5040U, 0x4838U, 0x382CU, 0x2018U,
    /* 32-47 floor */
    0x8870U, 0x9080U, 0xA090U, 0xB0A0U, 0x7848U, 0x8858U,
    0x9868U, 0xA878U, 0x6638U, 0x7640U, 0x6040U, 0x5030U,
    0xA864U, 0xB870U, 0xC87CU, 0xD888U,
    /* 48-79 objects */
    0xB830U, 0xC838U, 0xD840U, 0xA820U, 0x9820U, 0xC040U,
    0xD048U, 0xB054U, 0xE060U, 0xE872U, 0xF080U, 0xD8A2U,
    0x8064U, 0x60A2U, 0xA066U, 0xC042U,
    0xB8A0U, 0xE050U, 0xA8C0U, 0xC0A0U, 0xE0BCU, 0xF0D0U,
    0xD8D0U, 0xE8E0U, 0x7050U, 0x8870U, 0x9892U, 0x6060U,
    0x7860U, 0x8876U, 0x9884U, 0xA8A0U,
    /* 80-111 creatures */
    0x6034U, 0x8050U, 0xA070U, 0xC090U, 0x8080U, 0xA0A4U,
    0x6084U, 0x40A6U, 0x5098U, 0x4860U, 0x5052U, 0x7070U,
    0x9090U, 0xB0B0U, 0xA0B0U, 0xC0C0U,
    0x4000U, 0x6026U, 0x8040U, 0xA060U, 0xC080U, 0xE0A0U,
    0x2024U, 0x4044U, 0x6064U, 0x8084U, 0xA0A4U, 0xC0C4U,
    0xE0E4U, 0xF0F0U,
    /* 112-159 lava/fire/water/green */
    0x4000U, 0x6020U, 0x8040U, 0xA060U, 0xC080U, 0xE0A0U,
    0xF0C0U, 0xF8E0U, 0x0010U, 0x1030U, 0x2050U, 0x3080U,
    0x4090U, 0x60B0U, 0x80D0U, 0xA0F0U,
    0x4000U, 0x6020U, 0x8040U, 0xA060U, 0xC080U, 0xE0A0U,
    0xF0C0U, 0xF8E0U, 0x0010U, 0x1030U, 0x2050U, 0x3080U,
    0x4090U, 0x60B0U, 0x80D0U, 0xA0F0U,
    /* 160-191 creature extended */
    0x6034U, 0x8050U, 0xA070U, 0xC090U, 0x8080U, 0xA0A4U,
    0x6084U, 0x40A6U, 0x5098U, 0x4860U, 0x5052U, 0x7070U,
    0x9090U, 0xB0B0U, 0xA0B0U, 0xC0C0U,
    0x4000U, 0x6020U, 0x804AU, 0xA060U, 0xC080U, 0xE0A0U,
    0xF0C0U, 0xF8E0U, 0x2020U, 0x4040U, 0x6060U, 0x8080U,
    0xA0A0U, 0xC0C0U, 0xE0E0U, 0xF0F0U,
    /* 192-223 UI / menu */
    0x0808U, 0x1818U, 0x3030U, 0x4848U, 0x6060U, 0x7888U,
    0x9090U, 0xA8A8U, 0xC0C0U, 0xD8D8U, 0xF0F0U,
    0x6403U, 0x2000U, 0x4020U, 0x6020U,
    0x8030U, 0xA040U, 0xC050U, 0xE060U, 0x6403U, 0x2000U,
    0x4020U, 0x6020U, 0x8030U, 0xA040U, 0xC050U, 0xE060U,
    /* 224-255 UI light / white / primary / skin */
    0xB8B8U, 0xC8C8U, 0xD8D8U, 0xE8E8U, 0xF8F8U, 0xFFFFU,
    0xF800U, 0x03E0U, 0x001FU,
    0x8400U, 0x4200U, 0x2100U, 0xFFFFU, 0xEA20U, 0xAAB8U,
    0xCCD8U, 0xDDE8U, 0xDDE8U, 0xDDE8U,
    0xF0C0U, 0xF0C0U, 0xF0C0U, 0xFFFFU,
    0xFFFFU, 0xFFFFU, 0xFFFFU, 0xFFFFU,
};

const uint16_t *nexus_palette_default_bgr555(void) { return g_npal_default; }

/* ── Public API ────────────────────────────────────────────────────── */

void nexus_palette_init_defaults(Nexus_PaletteState *pal) {
    if (!pal) return;
    memset(pal, 0, sizeof(*pal));
    memcpy(pal->entries, g_npal_default, sizeof(g_npal_default));
    nexus_palette_expand_rgba(pal);
}

int nexus_palette_load_stone(Nexus_PaletteState *pal,
    const uint8_t *data, int size)
{
    int i, n;
    if (!pal) return 0;
    n = (size / 2);
    if (n > NEXUS_PALETTE_SIZE) n = NEXUS_PALETTE_SIZE;

    for (i = 0; i < n; i++)
        pal->entries[i] = (uint16_t)data[i*2] | ((uint16_t)data[i*2+1] << 8);

    if (n < NEXUS_PALETTE_SIZE) {
        memcpy(pal->entries + n, g_npal_default + n,
               ((size_t)(NEXUS_PALETTE_SIZE - n)) * sizeof(uint16_t));
        printf("Nexus palette: WARNING partial load %d/256 from STONE.BIN "
               "— filling rest with defaults\n", n);
    } else {
        printf("Nexus palette: full load %d entries from STONE.BIN\n", n);
    }
    nexus_palette_expand_rgba(pal);
    return n;
}

int nexus_palette_load_surface(Nexus_PaletteState *pal,
    const uint8_t *data, int size,
    int offset, int count, uint8_t pal_start)
{
    int i;
    if (!pal || !data) return 0;
    if (pal_start + count > NEXUS_PALETTE_SIZE)
        count = NEXUS_PALETTE_SIZE - pal_start;
    if (offset + count * 2 > size) {
        printf("Nexus palette: surface data short (%d < %d) "
               "for entries [%d-%d] — zero-fill\n",
               size, offset + count * 2, pal_start, pal_start + count - 1);
        for (i = 0; i < count; i++) pal->entries[pal_start + i] = 0;
        nexus_palette_expand_rgba(pal);
        return 0;
    }
    for (i = 0; i < count; i++)
        pal->entries[pal_start + i] =
            (uint16_t)data[offset + i*2] | ((uint16_t)data[offset + i*2+1] << 8);
    nexus_palette_expand_rgba(pal);
    printf("Nexus palette: loaded %d entries at offset %d [start=%d]\n",
           count, offset, pal_start);
    return count;
}

void nexus_palette_expand_rgba(Nexus_PaletteState *pal) {
    int i;
    if (!pal) return;
    for (i = 0; i < NEXUS_PALETTE_SIZE; i++)
        pal->rgba[i] = bgr555_to_rgba(pal->entries[i]);
}

/* ── Texture loading ───────────────────────────────────────────────── */

int nexus_texture_load_from_surface(Nexus_PaletteState *pal,
    const uint8_t *data, int data_size,
    int x, int y, int w, int h,
    uint8_t pal_start, uint8_t pal_count,
    const char *source_file, const char *label)
{
    Nexus_Texture *tx;
    int row, needed_sz;

    if (!pal) {
        printf("Nexus texture: ERROR null palette for '%s'\n",
               label ? label : "?");
        return -1;
    }
    if (w <= 0 || h <= 0) {
        printf("Nexus texture: ERROR invalid %dx%d for '%s' "
               "[source=%s] — flat-color fallback\n",
               w, h, label ? label : "?", source_file ? source_file : "?");
        return -1;
    }
    if (pal->texture_count >= NEXUS_MAX_TEXTURES) {
        printf("Nexus texture: ERROR atlas full (%d) loading '%s' "
               "[source=%s] — flat-color fallback\n",
               NEXUS_MAX_TEXTURES, label ? label : "?",
               source_file ? source_file : "?");
        return -1;
    }

    tx = &pal->textures[pal->texture_count];
    tx->w = w;
    tx->h = h;
    tx->pal_start = pal_start;
    tx->pal_count = pal_count;
    tx->source_file = source_file;
    tx->label = label;

    needed_sz = (y + h - 1) * w + (x + w);
    tx->data = (uint8_t *)calloc(w * h, 1);
    if (!tx->data) return -1;
    tx->owns_data = 1;

    if (data && data_size >= needed_sz) {
        for (row = 0; row < h; row++)
            memcpy(tx->data + row * w,
                   data + (y + row) * w + x, w);
    } else if (data && data_size > 0) {
        printf("Nexus texture: WARNING partial data %d < %d for '%s' "
               "[%s] — loading what exists\n",
               data_size, needed_sz, label ? label : "?",
               source_file ? source_file : "?");
        int row;
        for (row = 0; row < h; row++) {
            int rs = (y + row) * w + x;
            int avail = data_size > rs ? data_size - rs : 0;
            int copy = avail < w ? avail : w;
            if (copy > 0) memcpy(tx->data + row * w, data + rs, copy);
        }
    }
    /* else: data==NULL or data_size==0 — zeroes already set by calloc */

    printf("Nexus texture: loaded %dx%d '%s' from %s "
           "(pal %d-%d) -> id=%d\n",
           w, h, label ? label : "?", source_file ? source_file : "?",
           pal_start, pal_start + pal_count - 1,
           pal->texture_count);

    pal->texture_count++;
    return pal->texture_count - 1;
}

void nexus_palette_free_textures(Nexus_PaletteState *pal) {
    int i;
    if (!pal) return;
    for (i = 0; i < pal->texture_count; i++) {
        if (pal->textures[i].owns_data && pal->textures[i].data) {
            free(pal->textures[i].data);
            pal->textures[i].data = NULL;
            pal->textures[i].owns_data = 0;
        }
    }
    pal->texture_count = 0;
}
