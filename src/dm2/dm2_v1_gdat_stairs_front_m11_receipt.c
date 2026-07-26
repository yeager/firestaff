#include "dm2_v1_gdat_stairs_front_m11_receipt.h"

#include <string.h>

static const uint16_t s_rect[32] = {
    0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0x336,0x329,
    0x335,0x328,0x337,0x32a,0x333,0x326,0x332,0x325,
    0x334,0x327,0xffff,0xffff,0xffff,0xffff,0x330,0x323,
    0x32f,0x322,0x331,0x324,0x320,0x320,0x321,0x321 };
static const uint8_t s_field[32] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0x4f,0x3b,
    0x50,0x3c,0x51,0x3d,0x52,0x3e,0x53,0x3f,
    0x54,0x40,0xff,0xff,0xff,0xff,0x55,0x41,
    0x56,0x42,0x57,0x43,0x58,0x44,0x59,0x45 };
static const uint8_t s_fallback_field[32] = {
    0xff,0xff,0xff,0xff,0xff,0xff,0x4f,0x3b,
    0x50,0x3c,0x50,0x3c,0x52,0x3e,0x53,0x3f,
    0x53,0x3f,0xff,0xff,0xff,0xff,0x55,0x41,
    0x56,0x42,0x56,0x42,0x58,0x44,0x58,0x44 };

static uint32_t hash_bytes(uint32_t hash, const uint8_t *bytes, size_t size)
{ size_t i; for (i = 0; i < size; ++i) { hash ^= bytes[i]; hash *= 16777619u; } return hash; }
static uint16_t le16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

static const uint8_t *raw4_row(const uint8_t *table, size_t size, uint16_t wanted)
{
    uint16_t groups, group; size_t offset;
    if (!table || size < 4u || le16(table) != 0xfc0du) return NULL;
    groups = le16(table + 2u); if (!groups || (size_t)groups > (size - 4u) / 4u) return NULL;
    offset = 4u + (size_t)groups * 4u;
    for (group = 0; group < groups; ++group) {
        uint16_t first = le16(table + 4u + (size_t)group * 4u);
        uint16_t last = le16(table + 6u + (size_t)group * 4u);
        size_t count = last >= first ? (size_t)(last - first + 1u) : 0u;
        if (!count || count > (size - offset) / 8u) return NULL;
        if (wanted >= first && wanted <= last) return table + offset + (size_t)(wanted - first) * 8u;
        offset += count * 8u;
    }
    return NULL;
}

int dm2_v1_gdat_stairs_front_source_receipt(uint8_t cell, uint16_t state_word,
    uint8_t graphicsset, uint16_t light, int loadable, DM2_V1_GdatStairsFrontSourceReceipt *out)
{
    uint8_t variant; uint16_t i; uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* c_gui_vp.cpp:480-511; dm2data.cpp:289-310.  ptr1e1044 state selects
     * exactly one table lane; only the successful IF_LOADABLE lane is owned. */
    if (cell >= 16u || light > 640u || !loadable) return 0;
    variant = state_word != 0u; i = (uint16_t)cell * 2u + variant;
    if (s_rect[i] == 0xffffu || s_field[i] == 0xffu) return 0;
    out->valid = 1; out->no_draw = 1; out->view_cell = cell;
    out->state_variant = variant; out->graphicsset = graphicsset; out->field = s_field[i];
    out->rect_number = s_rect[i]; out->light_parameter = light;
    h = hash_bytes(h, (const uint8_t *)&out->view_cell, 1u);
    h = hash_bytes(h, &out->state_variant, 1u); h = hash_bytes(h, &out->graphicsset, 1u);
    h = hash_bytes(h, &out->field, 1u); h = hash_bytes(h, (const uint8_t *)&out->rect_number, 2u);
    h = hash_bytes(h, (const uint8_t *)&out->light_parameter, 2u); out->identity_hash = h ? h : 1u;
    return 1;
}

int dm2_v1_gdat_stairs_front_fallback_receipt(uint8_t cell, uint16_t state_word,
    uint8_t graphicsset, uint16_t light, int primary_loadable,
    DM2_V1_GdatStairsFrontFallbackReceipt *out)
{
    uint8_t variant; uint16_t i; uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* c_gui_vp.cpp:514-527, dm2data.cpp:289-302: this is reachable only
     * after the table1d6f5c IF_LOADABLE query failed. */
    if (cell >= 16u || light > 640u || primary_loadable) return 0;
    variant = state_word != 0u; i = (uint16_t)cell * 2u + variant;
    if (s_rect[i] == 0xffffu || s_fallback_field[i] == 0xffu) return 0;
    out->valid=1; out->no_draw=1; out->view_cell=cell; out->state_variant=variant;
    out->graphicsset=graphicsset; out->field=s_fallback_field[i]; out->rect_number=s_rect[i];
    out->light_parameter=light; out->blit_mode=1u; out->normal_scale=1u;
    out->palette_transaction_unproven=1u;
    h=hash_bytes(h,(const uint8_t *)&out->view_cell,1u); h=hash_bytes(h,&out->state_variant,1u);
    h=hash_bytes(h,&out->graphicsset,1u); h=hash_bytes(h,&out->field,1u);
    h=hash_bytes(h,(const uint8_t *)&out->rect_number,2u); h=hash_bytes(h,(const uint8_t *)&out->light_parameter,2u);
    h=hash_bytes(h,&out->blit_mode,1u); out->identity_hash=h?h:1u; return 1;
}

int dm2_v1_gdat_stairs_front_material_receipt_build(const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatStairsFrontSourceReceipt *source, DM2_V1_GdatStairsFrontMaterialReceipt *out)
{
    DM2_V1_GdatStairsFrontMaterialReceipt c; uint8_t *pixels; int width = 0, height = 0; uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !source || !source->valid || !source->no_draw || !source->identity_hash) return 0;
    memset(&c, 0, sizeof(c)); c.source = *source;
    if (!dm2_v1_query_gdat_summary_image_receipt(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
        source->graphicsset, source->field, &c.summary) || !c.summary.accepted ||
        c.summary.gdat_bypassed_for_ff || c.summary.colors != 16u || !c.summary.palette_hash ||
        !dm2_v1_gdat_image_raw_material_receipt(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
        source->graphicsset, source->field, &c.raw_material) || !c.raw_material.receipt_hash) return 0;
    pixels = dm2_v1_asset_load_image_field(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
        source->graphicsset, source->field, &width, &height, &c.format);
    if (!pixels || width <= 0 || height <= 0 || c.format != DM2_IMG_FMT_U4 || (size_t)width > SIZE_MAX / (size_t)height) { dm2_v1_asset_free_pixels(pixels); return 0; }
    c.valid = 1; c.no_draw = 1; c.indexed_pixels = pixels; c.width = (uint16_t)width; c.height = (uint16_t)height;
    c.pixel_stride = (uint16_t)width; c.indexed_pixel_count = (uint32_t)((size_t)width * (size_t)height);
    memcpy(c.palette16, c.summary.palette16, sizeof(c.palette16));
    c.decoded_hash = hash_bytes(2166136261u, pixels, c.indexed_pixel_count);
    c.palette_hash = hash_bytes(2166136261u, c.palette16, sizeof(c.palette16));
    if (!c.decoded_hash || !c.palette_hash) { dm2_v1_asset_free_pixels(pixels); return 0; }
    h = hash_bytes(h, (const uint8_t *)&source->identity_hash, 4u); h = hash_bytes(h, (const uint8_t *)&c.summary.receipt_hash, 4u);
    h = hash_bytes(h, (const uint8_t *)&c.raw_material.receipt_hash, 4u); h = hash_bytes(h, (const uint8_t *)&c.decoded_hash, 4u); h = hash_bytes(h, (const uint8_t *)&c.palette_hash, 4u);
    c.identity_hash = h ? h : 1u; *out = c; return 1;
}

int dm2_v1_gdat_stairs_front_fallback_material_receipt_build(
    const DM2_V1_AssetLoader *loader, const DM2_V1_GdatStairsFrontFallbackReceipt *f,
    DM2_V1_GdatStairsFrontMaterialReceipt *out)
{
    DM2_V1_GdatStairsFrontSourceReceipt source;
    if (!f || !f->valid || !f->no_draw || !f->identity_hash ||
        !f->palette_transaction_unproven) return 0;
    memset(&source, 0, sizeof(source)); source.valid=1; source.no_draw=1;
    source.view_cell=f->view_cell; source.state_variant=f->state_variant;
    source.graphicsset=f->graphicsset; source.field=f->field; source.rect_number=f->rect_number;
    source.light_parameter=f->light_parameter; source.identity_hash=f->identity_hash;
    return dm2_v1_gdat_stairs_front_material_receipt_build(loader, &source, out);
}

int dm2_v1_gdat_stairs_front_raw4_receipt_build(const DM2_V1_AssetLoader *loader,
    const DM2_V1_GdatStairsFrontMaterialReceipt *material, DM2_V1_GdatStairsFrontRaw4Receipt *out)
{
    const uint8_t *table, *row; size_t size = 0u; uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!loader || !material || !material->valid || !material->no_draw || !material->identity_hash) return 0;
    table = dm2_v1_asset_load_typed_sized(loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0, DM2_GDAT_ENTRY_TYPE_RAW4, 0, &size);
    row = raw4_row(table, size, material->source.rect_number);
    /* c_image.cpp:229-337/c_xrect.cpp:217-468: only root 1,0 rows here. */
    if (!row || le16(row) != 1u || le16(row + 2u) != 0u) return 0;
    out->valid = 1; out->no_draw = 1; out->rect_number = material->source.rect_number;
    out->destination_x = (int16_t)le16(row + 4u); out->destination_y = (int16_t)le16(row + 6u);
    out->width = material->width; out->height = material->height; out->material_identity_hash = material->identity_hash;
    out->raw4_table_hash = hash_bytes(2166136261u, table, size); out->raw4_row_hash = hash_bytes(2166136261u, row, 8u);
    h = hash_bytes(h, (const uint8_t *)&out->material_identity_hash, 4u); h = hash_bytes(h, (const uint8_t *)&out->raw4_table_hash, 4u); h = hash_bytes(h, (const uint8_t *)&out->raw4_row_hash, 4u); out->identity_hash = h ? h : 1u;
    return out->raw4_table_hash && out->raw4_row_hash;
}

int dm2_v1_gdat_stairs_front_m11_receipt_build(const DM2_V1_GdatStairsFrontMaterialReceipt *m,
    const DM2_V1_GdatStairsFrontRaw4Receipt *r, const DM2_V1_Dm2ViewportM11CompositionReceipt *c,
    const DM2_V1_ViewportState *owner, DM2_V1_GdatStairsFrontM11Receipt *out)
{
    DM2_V1_ViewportSurfaceSnapshot snap; uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!m || !r || !c || !owner || !m->valid || !m->no_draw || !r->valid || !r->no_draw ||
        r->material_identity_hash != m->identity_hash || r->width != m->width || r->height != m->height ||
        !c->valid || !c->no_draw || !c->session_identity || !c->data_epoch || !c->identity_hash ||
        !dm2_v1_viewport_surface_snapshot(owner, &snap) ||
        memcmp(&snap, &c->surface_before, sizeof(snap)) || memcmp(&snap, &c->surface_after, sizeof(snap))) return 0;
    out->valid=1; out->no_draw=1; out->indexed_pixels=m->indexed_pixels; out->width=m->width; out->height=m->height; out->pixel_stride=m->pixel_stride; out->indexed_pixel_count=m->indexed_pixel_count; out->palette_hash=m->palette_hash; out->material_identity_hash=m->identity_hash; out->raw4_identity_hash=r->identity_hash; out->session_identity=c->session_identity; out->data_epoch=c->data_epoch; out->composition_identity_hash=c->identity_hash; out->surface_before=snap; out->surface_after=snap;
    h=hash_bytes(h,(const uint8_t *)&out->material_identity_hash,4u); h=hash_bytes(h,(const uint8_t *)&out->raw4_identity_hash,4u); h=hash_bytes(h,(const uint8_t *)&out->palette_hash,4u); h=hash_bytes(h,(const uint8_t *)&out->session_identity,4u); h=hash_bytes(h,(const uint8_t *)&out->data_epoch,4u); h=hash_bytes(h,(const uint8_t *)&out->composition_identity_hash,4u); h=hash_bytes(h,(const uint8_t *)&snap.generation,4u); out->identity_hash=h?h:1u; return 1;
}

int dm2_v1_gdat_stairs_front_m11_receipt_matches(const DM2_V1_GdatStairsFrontM11Receipt *receipt,
    const DM2_V1_GdatStairsFrontMaterialReceipt *m, const DM2_V1_GdatStairsFrontRaw4Receipt *r,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *c, const DM2_V1_ViewportState *owner)
{ DM2_V1_GdatStairsFrontM11Receipt candidate; return receipt && receipt->valid && receipt->no_draw && dm2_v1_gdat_stairs_front_m11_receipt_build(m,r,c,owner,&candidate) && candidate.identity_hash == receipt->identity_hash; }

int dm2_v1_gdat_stairs_front_fallback_temp_picst_receipt_build(
    const DM2_V1_GdatStairsFrontFallbackReceipt *f,
    const DM2_V1_GdatStairsFrontMaterialReceipt *m,
    const DM2_V1_GdatStairsFrontRaw4Receipt *r,
    const DM2_V1_GdatStairsFrontM11Receipt *m11,
    DM2_V1_GdatStairsFrontFallbackTempPicstReceipt *out)
{
    uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    /* c_gui_vp.cpp:522-527 -> c_querydb.cpp:2381-2415 -> c_image.cpp:
     * 98-226/229-337. QUERY_TEMP_PICST passes normal 0x40 scales, hflip
     * mode 1, zero offsets, query1=rect, alpha=light and -1 palette args.
     * query_32cb_0804's B073/field-7 transaction remains unavailable. */
    if (!f || !m || !r || !m11 || !f->valid || !f->no_draw ||
        !f->palette_transaction_unproven || f->blit_mode != 1u || !f->normal_scale ||
        !m->valid || !m->no_draw || !r->valid || !r->no_draw || !m11->valid || !m11->no_draw ||
        m->source.graphicsset != f->graphicsset || m->source.field != f->field ||
        m->source.rect_number != f->rect_number || m->source.light_parameter != f->light_parameter ||
        r->material_identity_hash != m->identity_hash || r->rect_number != f->rect_number ||
        m11->material_identity_hash != m->identity_hash || m11->raw4_identity_hash != r->identity_hash) return 0;
    out->valid=1; out->no_draw=1; out->blit_mode=1u; out->scale_x=0x40u; out->scale_y=0x40u;
    out->palette_mode=-1; out->palette_arg=-1; out->alpha_mask=f->light_parameter;
    out->palette_transaction_unproven=1u; out->fallback_identity_hash=f->identity_hash;
    out->material_identity_hash=m->identity_hash; out->raw4_identity_hash=r->identity_hash; out->m11_identity_hash=m11->identity_hash;
    h=hash_bytes(h,(const uint8_t *)&out->fallback_identity_hash,4u); h=hash_bytes(h,(const uint8_t *)&out->material_identity_hash,4u);
    h=hash_bytes(h,(const uint8_t *)&out->raw4_identity_hash,4u); h=hash_bytes(h,(const uint8_t *)&out->m11_identity_hash,4u);
    h=hash_bytes(h,(const uint8_t *)&out->alpha_mask,2u); h=hash_bytes(h,&out->blit_mode,1u); out->identity_hash=h?h:1u;
    return 1;
}
