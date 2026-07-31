#include "nexus_v1_item_ibs.h"
#include <string.h>

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint32_t bgr555_to_rgba(uint16_t c) {
    int r = (c & 0x1F) << 3;
    int g = ((c >> 5) & 0x1F) << 3;
    int b = ((c >> 10) & 0x1F) << 3;
    return 0xFF000000U | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static uint32_t fnv1a_pixels(const uint32_t *px, int count) {
    uint32_t h = 0x811C9DC5U;
    int i;
    for (i = 0; i < count; ++i) {
        uint32_t v = px[i];
        h = (h ^ (v & 0xFF)) * 0x01000193U;
        h = (h ^ ((v >> 8) & 0xFF)) * 0x01000193U;
        h = (h ^ ((v >> 16) & 0xFF)) * 0x01000193U;
        h = (h ^ ((v >> 24) & 0xFF)) * 0x01000193U;
    }
    return h;
}

static uint32_t align2k(uint32_t v) {
    return (v + 2047U) & ~2047U;
}

int nexus_v1_item_ibs_parse_header(const uint8_t *data, int data_size,
                                    Nexus_V1_ItemIbsHeader *out) {
    uint32_t decl_data_size, img_data_size, floor_data_size;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!data || data_size < NEXUS_ITEM_IBS_HEADER_SIZE) return 0;
    if (read_be16(data) != NEXUS_ITEM_IBS_SIGNATURE) return 0;

    out->item_count = read_be16(data + 2);
    out->palette_count = read_be16(data + 4);
    out->assoc_count = read_be16(data + 6);
    out->image_count = read_be16(data + 8);
    out->floor_image_count = read_be16(data + 14);

    out->decl_section_offset = NEXUS_ITEM_IBS_HEADER_SIZE;

    decl_data_size = (uint32_t)out->item_count * NEXUS_ITEM_IBS_DECL_SIZE;
    out->images_section_offset = out->decl_section_offset + align2k(decl_data_size);

    img_data_size = (uint32_t)out->palette_count * NEXUS_ITEM_IBS_PALETTE_COLORS * 2 +
                    (uint32_t)out->assoc_count * 2 +
                    (uint32_t)out->image_count * NEXUS_ITEM_IBS_IMG_SIZE;
    out->floor_section_offset = out->images_section_offset + align2k(img_data_size);

    floor_data_size = read_be32(data + 0x30);
    out->anim_section_offset = out->floor_section_offset + align2k(floor_data_size);

    if ((int)out->images_section_offset > data_size) return 0;
    if ((int)out->floor_section_offset > data_size) return 0;

    out->valid = 1;
    return 1;
}

static void parse_item_decl(const uint8_t *p, Nexus_V1_ItemIbsDecl *d) {
    d->item_index = p[0];
    d->category = p[1];
    d->carry_locations = p[2];
    d->flags = p[3];
    d->byte4 = p[4];
    d->byte5 = p[5];
    d->skill1 = p[6];
    d->skill2 = p[7];
    d->weight = p[8];
    d->byte9 = p[9];
    d->byte10 = p[10];
    d->byte11 = p[11];
    d->byte12 = p[12];
    d->byte13 = p[13];
    d->byte14 = p[14];
    d->byte15 = p[15];
    d->action1 = p[16];
    d->action2 = p[17];
    d->action3 = p[18];
    d->byte19 = p[19];
    d->inventory_image_index = read_be16(p + 20);
    d->floor_image_index = read_be16(p + 22);
    d->name_string = read_be16(p + 24);
    d->desc_string = read_be16(p + 26);
    d->action1_string = read_be16(p + 28);
    d->action2_string = read_be16(p + 30);
    d->action3_string = read_be16(p + 32);
    d->word34 = read_be16(p + 34);
    d->attribute = read_be16(p + 36);
    d->byte38 = p[38];
    d->byte39 = p[39];
    d->has_floor_image = (d->flags & 0x80) != 0;
}

int nexus_v1_item_ibs_decode(const uint8_t *data, int data_size,
                              Nexus_V1_ItemIbsDecodeResult *out) {
    Nexus_V1_ItemIbsHeader hdr;
    int i, pal, img;
    uint32_t pal_base, assoc_base, img_base, floor_base;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!nexus_v1_item_ibs_parse_header(data, data_size, &hdr)) return 0;
    out->header = hdr;

    for (i = 0; i < hdr.item_count && i < NEXUS_ITEM_IBS_MAX_ITEMS; ++i) {
        parse_item_decl(data + hdr.decl_section_offset + i * NEXUS_ITEM_IBS_DECL_SIZE,
                        &out->items[i]);
        out->items_decoded++;
    }

    pal_base = hdr.images_section_offset;
    for (pal = 0; pal < hdr.palette_count && pal < NEXUS_ITEM_IBS_MAX_PALETTES; ++pal) {
        for (i = 0; i < NEXUS_ITEM_IBS_PALETTE_COLORS; ++i) {
            uint16_t c = read_be16(data + pal_base + pal * 32 + i * 2);
            out->palettes[pal][i] = bgr555_to_rgba(c);
        }
    }

    assoc_base = pal_base + (uint32_t)hdr.palette_count * 32;
    img_base = assoc_base + (uint32_t)hdr.assoc_count * 2;

    for (img = 0; img < hdr.image_count && img < NEXUS_ITEM_IBS_MAX_IMAGES; ++img) {
        uint32_t rgba[256];
        const uint8_t *src = data + img_base + img * NEXUS_ITEM_IBS_IMG_SIZE;
        uint16_t assoc_word = read_be16(data + assoc_base + img * 2);
        int pal_idx = assoc_word >> 8;
        const uint32_t *palette;

        if (assoc_word == 0xFF00 || pal_idx >= hdr.palette_count) {
            pal_idx = 0;
        }
        palette = out->palettes[pal_idx];

        for (i = 0; i < NEXUS_ITEM_IBS_IMG_SIZE; ++i) {
            uint8_t b = src[i];
            rgba[i * 2] = palette[(b >> 4) & 0x0F];
            rgba[i * 2 + 1] = palette[b & 0x0F];
        }
        out->image_hashes[img] = fnv1a_pixels(rgba, 256);
        out->images_decoded++;
    }

    floor_base = hdr.floor_section_offset;
    {
        uint32_t floor_pals[64][16];
        int floor_pal_valid[64];
        memset(floor_pal_valid, 0, sizeof(floor_pal_valid));

        for (i = 0; i < hdr.floor_image_count && i < NEXUS_ITEM_IBS_MAX_FLOOR_IMAGES; ++i) {
            const uint8_t *desc = data + floor_base + i * NEXUS_ITEM_IBS_FLOOR_DESC_SIZE;
            Nexus_V1_ItemIbsFloorDesc *fd = &out->floor_descs[i];
            fd->image_id = read_be16(desc);
            fd->encoding = read_be16(desc + 2);
            fd->palette_id = read_be16(desc + 4);
            fd->width = read_be16(desc + 6);
            fd->height = read_be16(desc + 8);
            fd->image_offset = read_be32(desc + 12);
            fd->palette_offset = read_be32(desc + 16);

            if (fd->image_id == 0xFFFF) break;

            if (fd->encoding == 0x0008 && fd->width > 0 && fd->height > 0) {
                int px_count = fd->width * fd->height;
                int byte_count = (px_count + 1) / 2;
                uint32_t abs_img = floor_base + fd->image_offset;
                uint32_t pal_rgba[16];
                int pid = fd->palette_id < 64 ? fd->palette_id : 0;
                int j;

                if (fd->palette_offset) {
                    uint32_t abs_pal = floor_base + fd->palette_offset;
                    if ((int)(abs_pal + 32) <= data_size) {
                        for (j = 0; j < 16; ++j)
                            floor_pals[pid][j] = bgr555_to_rgba(read_be16(data + abs_pal + j * 2));
                        floor_pal_valid[pid] = 1;
                    }
                }

                if (!floor_pal_valid[pid]) {
                    out->floor_images_decoded++;
                    continue;
                }
                memcpy(pal_rgba, floor_pals[pid], sizeof(pal_rgba));

                if ((int)(abs_img + byte_count) <= data_size && px_count <= 16384) {
                    uint32_t rgba[16384];
                    const uint8_t *src = data + abs_img;
                    for (j = 0; j < byte_count; ++j) {
                        uint8_t b = src[j];
                        int idx0 = j * 2, idx1 = j * 2 + 1;
                        if (idx0 < px_count) rgba[idx0] = pal_rgba[(b >> 4) & 0x0F];
                        if (idx1 < px_count) rgba[idx1] = pal_rgba[b & 0x0F];
                    }
                    out->floor_image_hashes[i] = fnv1a_pixels(rgba, px_count);
                }
            }
            out->floor_images_decoded++;
        }
    }

    out->valid = 1;
    return 1;
}

int nexus_v1_item_ibs_render_image(const uint8_t *data, int data_size,
                                    int image_index,
                                    const uint32_t palette[16],
                                    uint32_t *rgba_out) {
    Nexus_V1_ItemIbsHeader hdr;
    uint32_t pal_base, assoc_base, img_base;
    const uint8_t *src;
    int i;

    if (!rgba_out || !palette) return 0;
    if (!nexus_v1_item_ibs_parse_header(data, data_size, &hdr)) return 0;
    if (image_index < 0 || image_index >= hdr.image_count) return 0;

    pal_base = hdr.images_section_offset;
    assoc_base = pal_base + (uint32_t)hdr.palette_count * 32;
    img_base = assoc_base + (uint32_t)hdr.assoc_count * 2;
    src = data + img_base + image_index * NEXUS_ITEM_IBS_IMG_SIZE;

    for (i = 0; i < NEXUS_ITEM_IBS_IMG_SIZE; ++i) {
        uint8_t b = src[i];
        rgba_out[i * 2] = palette[(b >> 4) & 0x0F];
        rgba_out[i * 2 + 1] = palette[b & 0x0F];
    }
    return 1;
}

int nexus_v1_item_ibs_render_floor_image(const uint8_t *data, int data_size,
                                          int floor_index,
                                          const Nexus_V1_ItemIbsDecodeResult *result,
                                          uint32_t *rgba_out,
                                          int rgba_capacity) {
    const Nexus_V1_ItemIbsFloorDesc *fd;
    uint32_t floor_base, abs_img, abs_pal;
    uint32_t pal_rgba[16];
    int px_count, byte_count, j;
    const uint8_t *src;

    if (!data || data_size <= 0 || !rgba_out || !result || !result->valid)
        return 0;
    if (floor_index < 0 || floor_index >= result->floor_images_decoded) return 0;

    fd = &result->floor_descs[floor_index];
    if (fd->encoding != 0x0008 || fd->width == 0 || fd->height == 0) return 0;

    px_count = fd->width * fd->height;
    if (rgba_capacity < px_count) return 0;

    floor_base = result->header.floor_section_offset;
    if (floor_base > (uint32_t)data_size ||
        fd->image_offset > (uint32_t)data_size - floor_base ||
        (fd->palette_offset != 0U &&
         fd->palette_offset > (uint32_t)data_size - floor_base)) return 0;
    abs_img = floor_base + fd->image_offset;
    abs_pal = fd->palette_offset ? (floor_base + fd->palette_offset) : 0;

    if (!abs_pal) return 0;
    if (abs_pal > (uint32_t)data_size ||
        32U > (uint32_t)data_size - abs_pal) return 0;

    for (j = 0; j < 16; ++j)
        pal_rgba[j] = bgr555_to_rgba(read_be16(data + abs_pal + j * 2));

    byte_count = (px_count + 1) / 2;
    if (abs_img > (uint32_t)data_size ||
        (uint32_t)byte_count > (uint32_t)data_size - abs_img) return 0;

    src = data + abs_img;
    for (j = 0; j < byte_count; ++j) {
        uint8_t b = src[j];
        int idx0 = j * 2, idx1 = j * 2 + 1;
        if (idx0 < px_count) rgba_out[idx0] = pal_rgba[(b >> 4) & 0x0F];
        if (idx1 < px_count) rgba_out[idx1] = pal_rgba[b & 0x0F];
    }
    return 1;
}
