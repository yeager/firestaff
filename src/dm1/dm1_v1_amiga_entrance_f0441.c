#include "dm1_v1_amiga_entrance_f0441.h"

#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_amiga_title_f0437.h"
#include "asset_loader_m11.h"
#include "memory_graphics_dat_state_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_pc34_compat.h"
#include "asset_status_m12.h"
#include "firestaff_ftl_container.h"
#include "firestaff_ftl_hunk_data_zero_run.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static void md5_to_hex(const uint8_t md5[16], char out[33]) {
    static const char digits[] = "0123456789abcdef";
    unsigned int index;
    for (index = 0u; index < 16u; ++index) {
        out[index * 2u] = digits[md5[index] >> 4u];
        out[index * 2u + 1u] = digits[md5[index] & 0x0fu];
    }
    out[32] = '\0';
}

static unsigned int count_nonzero(const uint8_t *pixels, size_t count) {
    size_t index;
    unsigned int nonzero = 0u;
    for (index = 0u; index < count; ++index) {
        nonzero += pixels[index] != 0u;
    }
    return nonzero;
}

static int entrance_receipt_impl(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    const uint8_t *swsh_executable,
    size_t swsh_executable_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt,
    const DM1_V1_AmigaStartupMedia *startup) {
    static const uint16_t a20_entrance_palette_rgb4[
        DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES] = {
        0x000u, 0x666u, 0x888u, 0x840u, 0xca8u, 0x0c0u, 0x080u, 0x0a0u,
        0x864u, 0xf00u, 0xa86u, 0x642u, 0x444u, 0xaaau, 0x620u, 0xfffu
    };
    DM1_V1_AmigaTitleF0437Receipt title_receipt;
    DM1_V1_AmigaGraphicsReceipt graphics_receipt;
    DM1_V1_AmigaEntranceF0441Receipt receipt;
    uint8_t *screen = NULL;
    uint8_t *left = NULL;
    uint8_t *right = NULL;
    const size_t pixel_capacity = 320u * 200u;
    uint16_t screen_width = 0u;
    uint16_t screen_height = 0u;
    uint16_t left_width = 0u;
    uint16_t left_height = 0u;
    uint16_t right_width = 0u;
    uint16_t right_height = 0u;
    int valid = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!graphics_dat || !swsh_executable || !out_receipt ||
        !(startup ? dm1_v1_amiga_title_f0437_receipt_for_startup(
            startup, &title_receipt) : dm1_v1_amiga_title_f0437_receipt(
            graphics_dat, graphics_dat_bytes, swsh_executable,
            swsh_executable_bytes, &title_receipt)) ||
        dm1_v1_amiga_graphics_receipt(
            graphics_dat, graphics_dat_bytes, &graphics_receipt) != 0) {
        return 0;
    }

    screen = (uint8_t *)malloc(pixel_capacity);
    left = (uint8_t *)malloc(pixel_capacity);
    right = (uint8_t *)malloc(pixel_capacity);
    if (!screen || !left || !right ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_SCREEN,
            screen, pixel_capacity, &screen_width, &screen_height) ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_LEFT_DOOR,
            left, pixel_capacity, &left_width, &left_height) ||
        !dm1_v1_amiga_graphics_decode(
            graphics_dat, graphics_dat_bytes,
            DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_RIGHT_DOOR,
            right, pixel_capacity, &right_width, &right_height)) {
        goto cleanup;
    }

    if (screen_width != 320u || screen_height != 200u ||
        left_width != 128u || left_height != 161u ||
        right_width != 128u || right_height != 161u) {
        goto cleanup;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    md5_to_hex(graphics_receipt.md5, receipt.graphics_md5);
    (void)snprintf(receipt.executable_md5,
                   sizeof(receipt.executable_md5), "%s",
                   title_receipt.executable_md5);
    receipt.screen_width = screen_width;
    receipt.screen_height = screen_height;
    receipt.left_door_width = left_width;
    receipt.left_door_height = left_height;
    receipt.right_door_width = right_width;
    receipt.right_door_height = right_height;
    receipt.screen_nonzero_pixels = count_nonzero(
        screen, (size_t)screen_width * screen_height);
    receipt.left_door_nonzero_pixels = count_nonzero(
        left, (size_t)left_width * left_height);
    receipt.right_door_nonzero_pixels = count_nonzero(
        right, (size_t)right_width * right_height);
    receipt.door_frame_count = DM1_V1_AMIGA_ENTRANCE_F0441_DOOR_FRAMES;
    receipt.opening_steps = DM1_V1_AMIGA_ENTRANCE_F0441_OPENING_STEPS;
    receipt.switch_delay_ticks =
        DM1_V1_AMIGA_ENTRANCE_F0441_SWITCH_DELAY_TICKS;
    receipt.mouse_input_only = 1;
    memcpy(receipt.entrance_palette_rgb4, a20_entrance_palette_rgb4,
           sizeof(a20_entrance_palette_rgb4));
    receipt.source_evidence =
        "ReDMCSB ENTRANCE.C F0441 A20: C002/C003 128x161, C004 320x200, "
        "8 door frames, mouse-only input, C01 switch and Delay(20); "
        "DATA.C MEDIA424 G0020 RGB4 palette";

    valid = receipt.screen_nonzero_pixels > 0u &&
            receipt.left_door_nonzero_pixels > 0u &&
            receipt.right_door_nonzero_pixels > 0u;
    if (valid) *out_receipt = receipt;

cleanup:
    free(screen);
    free(left);
    free(right);
    return valid;
}

int dm1_v1_amiga_entrance_f0441_receipt(
    const uint8_t *graphics_dat, size_t graphics_dat_bytes,
    const uint8_t *swsh_executable, size_t swsh_executable_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt) {
    return entrance_receipt_impl(graphics_dat, graphics_dat_bytes,
        swsh_executable, swsh_executable_bytes, out_receipt, NULL);
}

int dm1_v1_amiga_entrance_f0441_receipt_for_startup(
    const DM1_V1_AmigaStartupMedia *media,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt) {
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm1_v1_amiga_startup_media_is_a22g(media)) return 0;
    /* STARTUP1.C:249ff and TITLE.C MEDIA413 retain the A22G shared path. */
    return entrance_receipt_impl(media->graphics_dat, media->graphics_dat_bytes,
        media->swsh_executable, media->swsh_executable_bytes, out_receipt, media);
}

typedef struct { int type, parent, x, y; } A36LayoutRecord;

static unsigned int a36_be16(const uint8_t *p) {
    return ((unsigned int)p[0] << 8) | p[1];
}

/* COORD.C F0639: ranges precede contiguous eight-byte records. Only the
 * selected original C696 is accepted; never substitute PC layout tables. */
static int a36_layout_record(const uint8_t *data, size_t bytes, int zone,
                             A36LayoutRecord *out) {
    size_t count, cursor;
    int found = 0;
    if (!data || bytes < 4 || a36_be16(data) != 0xfc0d) return 0;
    count = a36_be16(data + 2);
    if (count == 0 || count > 256 || count > (bytes - 4) / 4) return 0;
    cursor = 4 + count * 4;
    for (size_t i = 0; i < count; ++i) {
        unsigned int first = a36_be16(data + 4 + i * 4);
        unsigned int last = a36_be16(data + 6 + i * 4);
        size_t length;
        if (last < first) return 0;
        length = ((size_t)last - first + 1) * 8;
        if (length > bytes - cursor) return 0;
        if (zone >= (int)first && zone <= (int)last) {
            const uint8_t *p = data + cursor + (size_t)(zone - (int)first) * 8;
            if (found) return 0;
            out->type = (int)a36_be16(p);
            out->parent = (int)(int16_t)a36_be16(p + 2);
            out->x = (int)(int16_t)a36_be16(p + 4);
            out->y = (int)(int16_t)a36_be16(p + 6);
            found = 1;
        }
        cursor += length;
    }
    return found && cursor == bytes;
}

/* The authenticated entrance chains contain top-left offsets, right anchors
 * and explicit parent dimensions only. Reject other F0635 record types
 * rather than approximating the rest of the game's layout language. */
static int a36_layout_rect(const uint8_t *data, size_t bytes, int zone,
                           int width, int height, DM1_V1_AmigaEntranceRect *out) {
    A36LayoutRecord leaf, parent;
    int x, y, right_anchor;
    if (!a36_layout_record(data, bytes, zone, &leaf) ||
        (leaf.type != 1 && leaf.type != 11 && leaf.type != 12) ||
        width <= 0 || height <= 0) return 0;
    x = leaf.x; y = leaf.y; right_anchor = leaf.type == 12;
    for (unsigned int depth = 0; depth < 16; ++depth) {
        if (!leaf.parent ||
            !a36_layout_record(data, bytes, leaf.parent, &parent)) return 0;
        if (parent.type == 9) {
            if (right_anchor) x += parent.x - width;
            if (x < 0 || y < 0 || x + width > parent.x ||
                y + height > parent.y) return 0;
            *out = (DM1_V1_AmigaEntranceRect){x, y, width, height};
            return 1;
        }
        if (parent.type != 1) return 0;
        x += parent.x; y += parent.y; leaf = parent;
    }
    return 0;
}

static int a36_bind_layout(M11_AssetLoader *loader,
                           DM1_V1_AmigaEntranceF0441Receipt *receipt) {
    struct MemoryGraphicsDatRuntimeState_Compat *rt = loader->runtimeState;
    struct MemoryGraphicsDatHeader_Compat header = {0};
    struct MemoryGraphicsDatSelection_Compat item;
    A36LayoutRecord working;
    uint8_t *data;
    int ok;
    if (!rt) return 0;
    header.format = rt->format; header.graphicCount = rt->graphicCount;
    header.compressedByteCounts = rt->compressedByteCounts;
    header.decompressedByteCounts = rt->decompressedByteCounts;
    header.widthHeight = rt->widthHeight;
    if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(&header, 696, &item) ||
        item.compressedByteCount != item.decompressedByteCount ||
        item.compressedByteCount < 4) return 0;
    data = malloc(item.compressedByteCount);
    if (!data) return 0;
    ok = F0474_MEMORY_LoadGraphic_CPSDF_Compat(item.offset,
        item.compressedByteCount, loader->fileState, data) &&
        a36_layout_record(data, item.compressedByteCount, 428, &working) &&
        working.type == 9 && working.parent == 0 &&
        working.x > 0 && working.x <= 320 && working.y > 0 && working.y <= 200 &&
        a36_layout_rect(data, item.compressedByteCount, 429,
            working.x, working.y, &receipt->a36_working) &&
        a36_layout_rect(data, item.compressedByteCount, 427,
            working.x, working.y, &receipt->a36_transfer) &&
        a36_layout_rect(data, item.compressedByteCount, 430,
            (int)receipt->left_door_width, (int)receipt->left_door_height,
            &receipt->a36_left) &&
        a36_layout_rect(data, item.compressedByteCount, 431,
            (int)receipt->right_door_width, (int)receipt->right_door_height,
            &receipt->a36_right) &&
        a36_layout_rect(data, item.compressedByteCount, 432,
            224, 136, &receipt->a36_viewport) &&
        a36_layout_rect(data, item.compressedByteCount, 435,
            14, 13, &receipt->a36_enter_button) &&
        a36_layout_rect(data, item.compressedByteCount, 436,
            14, 13, &receipt->a36_resume_button);
    free(data);
    if (!ok) return 0;
    receipt->a36_step_pixels = 2; /* COORD.C G2349_, MEDIA746 A36M. */
    receipt->opening_steps = (unsigned int)
        ((receipt->a36_left.w > receipt->a36_right.w
            ? receipt->a36_left.w : receipt->a36_right.w) + 1) / 2;
    receipt->door_frame_count = 2; /* Original native left/right bitmaps. */
    receipt->a36_layout_valid = 1;
    return 1;
}

int dm1_v1_amiga36_entrance_f0441_receipt(
    const uint8_t *graphics_dat,
    size_t graphics_dat_bytes,
    DM1_V1_AmigaEntranceF0441Receipt *out_receipt) {
    static const uint16_t a36_entrance_palette_rgb4[
        DM1_V1_AMIGA_ENTRANCE_F0441_PALETTE_ENTRIES] = {
        0x000u, 0x666u, 0x888u, 0x840u, 0xca8u, 0x0c0u, 0x080u, 0x0a0u,
        0x864u, 0xf00u, 0xa86u, 0x642u, 0x444u, 0xaaau, 0x620u, 0xfffu
    };
    DM1_V1_AmigaGraphicsReceipt graphics_receipt;
    DM1_V1_AmigaEntranceF0441Receipt receipt;
    M11_AssetLoader loader;
    const M11_AssetSlot *screen;
    const M11_AssetSlot *left;
    const M11_AssetSlot *right;
    const M11_AssetSlot *credits;
    const M11_AssetSlot *buttons;
    int valid = 0;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&loader, 0, sizeof(loader));
    if (!graphics_dat || graphics_dat_bytes > (size_t)LONG_MAX ||
        !out_receipt ||
        dm1_v1_amiga_graphics_receipt(
            graphics_dat, graphics_dat_bytes, &graphics_receipt) != 0 ||
        graphics_receipt.version != DM1_AMIGA_VER_3_6 ||
        !M11_AssetLoader_InitFromBuffer(
            &loader, graphics_dat, (long)graphics_dat_bytes)) {
        return 0;
    }

    /* A36M's 0x8001 bank is IMG3, so use the exact decoder that will back
     * runtime drawing. This proves the five selected source entries really
     * decode from the authenticated bytes instead of admitting by pathname. */
    screen = M11_AssetLoader_Load(
        &loader, DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_SCREEN);
    left = M11_AssetLoader_Load(
        &loader, DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_LEFT_DOOR);
    right = M11_AssetLoader_Load(
        &loader, DM1_V1_AMIGA_ENTRANCE_F0441_GRAPHIC_RIGHT_DOOR);
    credits = M11_AssetLoader_Load(&loader, 5u);
    buttons = M11_AssetLoader_Load(&loader, 11u);
    if (!screen || !left || !right || !credits || !buttons ||
        !screen->pixels || !left->pixels || !right->pixels ||
        !credits->pixels || !buttons->pixels) {
        goto cleanup;
    }

    /* ENTRANCE.C F0441/F0442 loads these five original IMG1s before the
     * interaction loop. C004 and both door halves have fixed source sizes;
     * C005/C011 vary with language/layout but must decode to visible pixels. */
    if (screen->width != 320u || screen->height != 200u ||
        left->width != 105u || left->height != 161u ||
        right->width != 128u || right->height != 161u ||
        credits->width == 0u || credits->height == 0u ||
        buttons->width == 0u || buttons->height == 0u) {
        goto cleanup;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    md5_to_hex(graphics_receipt.md5, receipt.graphics_md5);
    receipt.screen_width = screen->width;
    receipt.screen_height = screen->height;
    receipt.left_door_width = left->width;
    receipt.left_door_height = left->height;
    receipt.right_door_width = right->width;
    receipt.right_door_height = right->height;
    receipt.screen_nonzero_pixels = count_nonzero(
        screen->pixels, (size_t)screen->width * screen->height);
    receipt.left_door_nonzero_pixels = count_nonzero(
        left->pixels, (size_t)left->width * left->height);
    receipt.right_door_nonzero_pixels = count_nonzero(
        right->pixels, (size_t)right->width * right->height);
    receipt.door_frame_count = DM1_V1_AMIGA_ENTRANCE_F0441_DOOR_FRAMES;
    receipt.opening_steps = DM1_V1_AMIGA_ENTRANCE_F0441_OPENING_STEPS;
    receipt.switch_delay_ticks = DM1_V1_AMIGA_ENTRANCE_F0441_SWITCH_DELAY_TICKS;
    receipt.mouse_input_only = 0;
    receipt.has_credits_graphic = count_nonzero(
        credits->pixels, (size_t)credits->width * credits->height) > 0u;
    receipt.has_entrance_buttons_graphic = count_nonzero(
        buttons->pixels, (size_t)buttons->width * buttons->height) > 0u;
    memcpy(receipt.entrance_palette_rgb4, a36_entrance_palette_rgb4,
           sizeof(a36_entrance_palette_rgb4));
    if (!a36_bind_layout(&loader, &receipt)) goto cleanup;
    receipt.source_evidence =
        "ReDMCSB STARTUP2.C F0441 A36M: STARTUP1.C skips F0437; "
        "ENTRANCE.C loads C004/C005, C002/C003/C011, waits for C200; "
        "COMMAND.C G2195 maps C200 to Amiga key 0x4400; DATA.C G0020 RGB4";

    valid = receipt.screen_nonzero_pixels > 0u &&
            receipt.left_door_nonzero_pixels > 0u &&
            receipt.right_door_nonzero_pixels > 0u &&
            receipt.has_credits_graphic &&
            receipt.has_entrance_buttons_graphic;
    if (valid) *out_receipt = receipt;

cleanup:
    M11_AssetLoader_Shutdown(&loader);
    return valid;
}

/* F0797 (ENTRANCE.C:58-81) authors this column-major C255 map. It
 * has no Things, ornaments, stairs or loaded dungeon state. F0128's A36
 * order includes both D3/D2 outer walls and terminal D0L/D0R (DUNVIEW.C:
 * 8478-8538). Do not delegate its visibility to the PC diagnostic walker. */
static int a36_c255_blit(M11_AssetLoader *loader, const uint8_t *layout,
                         size_t layout_bytes, uint8_t *staged,
                         unsigned int graphic, unsigned int zone,
                         int transparent, uint32_t *mask) {
    A36LayoutRecord record;
    const M11_AssetSlot *slot = M11_AssetLoader_Load(loader, graphic);
    int x, y;
    if (!slot || !slot->loaded || !slot->pixels ||
        graphic < 78u || graphic > 107u ||
        !a36_layout_record(layout, layout_bytes, (int)zone, &record) ||
        record.parent != 4 || (record.type != 11 && record.type != 12 &&
                              record.type != 14 && record.type != 15)) return 0;
    x = record.x;
    y = record.y;
    if (record.type == 12) x += 224 - slot->width;
    if (record.type == 15) x += (224 + 1) / 2 - (slot->width + 1) / 2;
    if (record.type == 14) y += 136 - slot->height;
    for (int sy = 0; sy < slot->height; ++sy) {
        for (int sx = 0; sx < slot->width; ++sx) {
            int dx = x + sx, dy = y + sy;
            uint8_t pixel = slot->pixels[sy * slot->width + sx];
            /* F0104:3141-3155 passes C10 to the native A36 blitter.
             * F0098/F0674's complete floor/ceiling copies are opaque. */
            if (dx >= 0 && dx < 224 && dy >= 0 && dy < 136 &&
                (!transparent || pixel != 10u)) staged[dy * 224 + dx] = pixel;
        }
    }
    *mask |= UINT32_C(1) << (graphic - 78u);
    return 1;
}

int dm1_v1_amiga36_entrance_render_c255(M11_AssetLoader *loader,
                                        uint8_t *viewport, size_t bytes) {
    static const struct { int depth, lateral; unsigned int graphic, zone; } walls[] = {
        {3,-2,104,702}, {3,2,103,703},
        {3,-1,106,705}, {3,1,105,706}, {3,0,107,704},
        {2,-2,99,707}, {2,2,98,708},
        {2,-1,101,710}, {2,1,100,711}, {2,0,102,709},
        {1,-1,96,713}, {1,1,95,714}, {1,0,97,712},
        {0,-1,94,716}, {0,1,93,717}
    };
    struct MemoryGraphicsDatRuntimeState_Compat *rt;
    struct MemoryGraphicsDatHeader_Compat header = {0};
    struct MemoryGraphicsDatSelection_Compat item;
    uint8_t grid[25] = {0}, staged[224 * 136];
    uint8_t *layout;
    uint32_t mask = 0u;
    int valid;
    if (!loader || !loader->dm1Amiga36 || !viewport || bytes < sizeof(staged) ||
        !(rt = loader->runtimeState)) return 0;
    header.format = rt->format; header.graphicCount = rt->graphicCount;
    header.compressedByteCounts = rt->compressedByteCounts;
    header.decompressedByteCounts = rt->decompressedByteCounts;
    header.widthHeight = rt->widthHeight;
    if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(&header, 696, &item) ||
        item.compressedByteCount != item.decompressedByteCount) return 0;
    layout = malloc(item.compressedByteCount);
    if (!layout) return 0;
    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(item.offset,
            item.compressedByteCount, loader->fileState, layout)) {
        free(layout); return 0;
    }
    for (int y = 0; y < 5; ++y) grid[10 + y] = 1u << 5;
    grid[7] = 1u << 5;
    memset(staged, 0, sizeof(staged));
    valid = a36_c255_blit(loader, layout, item.compressedByteCount, staged,
                         79u, 700u, 0, &mask) &&
            a36_c255_blit(loader, layout, item.compressedByteCount, staged,
                         78u, 701u, 0, &mask);
    for (size_t i = 0; valid && i < sizeof(walls) / sizeof(walls[0]); ++i) {
        /* South from (2,0): left increases map X, depth increases map Y. */
        int x = 2 - walls[i].lateral, y = walls[i].depth;
        if (x < 0 || x >= 5 || y < 0 || y >= 5) { valid = 0; break; }
        if (grid[x * 5 + y] == 0u)
            valid = a36_c255_blit(loader, layout, item.compressedByteCount,
                staged, walls[i].graphic, walls[i].zone, 1, &mask);
    }
    free(layout);
    /* Bit zero is record 78. All eleven source-visible wall records plus
     * floor/ceiling must be admitted before publishing the private raster. */
    if (!valid || mask != UINT32_C(0x1eb78003)) return 0;
    memcpy(viewport, staged, sizeof(staged));
    return 1;
}

int dm1_v1_amiga36_entrance_compose(
    M11_AssetLoader *loader, const DM1_V1_AmigaEntranceF0441Receipt *r,
    const uint8_t *dungeon, unsigned int frame, int selected_button,
    uint8_t *out, size_t bytes) {
    const M11_AssetSlot *screen, *left, *right, *buttons;
    unsigned int shift;
    if (!loader || !loader->dm1Amiga36 || !r || !r->valid ||
        !r->a36_layout_valid || !dungeon || !out || bytes < 64000 ||
        frame > r->opening_steps || selected_button < -1 || selected_button > 1)
        return 0;
    screen = M11_AssetLoader_Load(loader, 4);
    left = M11_AssetLoader_Load(loader, 2);
    right = M11_AssetLoader_Load(loader, 3);
    buttons = M11_AssetLoader_Load(loader, 11);
    if (!screen || !left || !right || !buttons || !screen->pixels ||
        !left->pixels || !right->pixels || !buttons->pixels ||
        screen->width != 320 || screen->height != 200 ||
        left->width != r->left_door_width || left->height != r->left_door_height ||
        right->width != r->right_door_width || right->height != r->right_door_height ||
        buttons->width < 14 || buttons->height < 26) return 0;
    memcpy(out, screen->pixels, 64000);
    shift = frame * r->a36_step_pixels;
    for (int y = 0; y < r->a36_working.h; ++y) {
        for (int x = 0; x < r->a36_working.w; ++x) {
            uint8_t pixel = screen->pixels[(y + r->a36_transfer.y) * 320 +
                                           x + r->a36_transfer.x];
            if (frame && x >= r->a36_viewport.x &&
                x < r->a36_viewport.x + r->a36_viewport.w &&
                y >= r->a36_viewport.y &&
                y < r->a36_viewport.y + r->a36_viewport.h)
                pixel = dungeon[(y - r->a36_viewport.y) * 224 +
                                 x - r->a36_viewport.x];
            if (y >= r->a36_left.y && y < r->a36_left.y + r->a36_left.h &&
                x >= r->a36_left.x &&
                x < r->a36_left.x + r->a36_left.w - (int)shift)
                pixel = left->pixels[(y - r->a36_left.y) * left->width +
                                      x - r->a36_left.x + shift];
            if (y >= r->a36_right.y && y < r->a36_right.y + r->a36_right.h &&
                x >= r->a36_right.x + (int)shift &&
                x < r->a36_right.x + r->a36_right.w)
                pixel = right->pixels[(y - r->a36_right.y) * right->width +
                                       x - r->a36_right.x - shift];
            out[(y + r->a36_transfer.y) * 320 + x + r->a36_transfer.x] = pixel;
        }
    }
    if (selected_button >= 0) {
        const DM1_V1_AmigaEntranceRect *button = selected_button
            ? &r->a36_resume_button : &r->a36_enter_button;
        M11_AssetLoader_BlitRegion(buttons, 0, selected_button * 13, 14, 13,
            out, 320, 200, button->x, button->y, -1);
    }
    return 1;
}

int dm1_v1_amiga36_entrance_bind_driver(
    DM1_V1_AmigaEntranceF0441Receipt *receipt,
    const uint8_t *driver, size_t driver_bytes) {
    FirestaffFtlContainer container;
    const FirestaffFtlHunkHeader *data;
    uint8_t decoded[1110];
    char md5[33];
    size_t written = 0;
    if (!receipt) return 0;
    receipt->a36_fade_valid = 0;
    memset(receipt->a36_fade_increments, 0, sizeof(receipt->a36_fade_increments));
    receipt->a36_driver_md5[0] = '\0';
    if (!receipt->valid || !receipt->a36_layout_valid || !driver ||
        driver_bytes != 12554 || !m12_bytes_md5_hex(driver, driver_bytes, md5) ||
        strcmp(md5, "d86f045085481448f6ae3db8d3d640d2") ||
        FirestaffFtlContainer_Parse(driver, driver_bytes, &container) != 0 ||
        !container.has_bss_metadata || container.data_index < 0 ||
        container.bss.data_area1_memory_size != sizeof(decoded)) return 0;
    data = &container.hunks[container.data_index];
    /* PRIM.H DATASEGMENTHEADER is ten bytes; PRIM2B.C:466-487 expands
     * only DATACompressedSize, never the following relocation words. */
    if (data->size != 846 || data->payload[0] != 0 || data->payload[1] != 1 ||
        data->payload[2] != 0 || data->payload[3] != 0 ||
        a36_be16(data->payload + 4) != 798 ||
        data->payload[6] != 0 || data->payload[7] != 0 ||
        a36_be16(data->payload + 8) != 38 ||
        FirestaffFtlHunkData_DecompressZeroRun(data->payload + 10, 798,
            sizeof(decoded), decoded, &written) != FIRESTAFF_FTL_HUNK_DATA_OK ||
        written != sizeof(decoded)) return 0;
    memcpy(receipt->a36_fade_increments, decoded + 584,
           sizeof(receipt->a36_fade_increments));
    /* VDEOAMIG.C G3248: every delta reaches its target after fifteen
     * one-VBlank increments. Validate the native table before publication. */
    for (int delta = 0; delta < 16; ++delta) {
        unsigned int sum = 0;
        for (int frame = 0; frame < 15; ++frame) {
            unsigned int increment = receipt->a36_fade_increments[frame][delta];
            if (increment > 1) return 0;
            sum += increment;
        }
        if (sum != (unsigned int)delta) return 0;
    }
    memcpy(receipt->a36_driver_md5, md5, sizeof(md5));
    receipt->a36_fade_valid = 1;
    return 1;
}

int dm1_v1_amiga36_entrance_fade_step(
    const DM1_V1_AmigaEntranceF0441Receipt *receipt,
    const uint16_t source[16], const uint16_t target[16],
    unsigned int frame, uint16_t out[16]) {
    if (!receipt || !receipt->a36_fade_valid || !source || !target || !out ||
        frame == 0 || frame > 15) return 0;
    /* PALETTE.C F1127: both display banks receive the same destination
     * sixteen colours (index&15). Host 4-bit pixels need one bank here. */
    for (int color = 0; color < 16; ++color) {
        unsigned int packed = 0;
        if ((source[color] | target[color]) & 0xf000) return 0;
        for (unsigned int shift = 0; shift <= 8; shift += 4) {
            int start = (source[color] >> shift) & 15;
            int end = (target[color] >> shift) & 15;
            unsigned int delta = (unsigned int)abs(end - start), step = 0;
            for (unsigned int i = 0; i < frame; ++i)
                step += receipt->a36_fade_increments[i][delta];
            packed |= (unsigned int)(start + (end < start ? -(int)step : (int)step)) << shift;
        }
        out[color] = (uint16_t)packed;
    }
    return 1;
}
