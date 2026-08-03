#include "dm2_v1_gdat_stairs_front_receipt.h"
#include <string.h>

int dm2_v1_gdat_stairs_front_source_receipt(
    uint8_t cell, uint8_t graphicsset, uint16_t light,
    const DM2_V1_StairsFrontCallbacks *cb, void *ctx,
    DM2_V1_GdatStairsFrontSourceReceipt *out)
{
    DM2_V1_GdatStairsFrontSourceReceipt r;
    uint32_t h = 2166136261u;
    if (!out) return 0;
    memset(&r, 0, sizeof(r));

    r.view_cell = cell;
    r.graphicsset = graphicsset;
    r.light_parameter = light;

    /* SKProject: offset = (8*index + index) * 2 = 18*index
     * has_detail = word_at(ptr1e1044 + offset + 8) != 0 */
    if (cb && cb->detail_ptr) {
        int32_t offset = (int32_t)cell * 18;
        uint16_t w = (uint16_t)(cb->detail_ptr[offset + 8] |
                                (cb->detail_ptr[offset + 9] << 8));
        r.has_detail = (w != 0) ? 1 : 0;
    }

    /* rect = table1d6f9c[has_detail + 2*index] */
    int table_idx = r.has_detail + 2 * (int)cell;
    if (cb && cb->table_1d6f9c) {
        r.rect_number = cb->table_1d6f9c[table_idx];
    } else {
        r.rect_number = -1;
    }

    if (r.rect_number < 0) {
        r.no_draw = 1;
        r.valid = 1;
        h ^= 0xDEAD; h *= 16777619u;
        r.identity_hash = h;
        *out = r;
        return 1;
    }

    /* field = table1d6f5c[has_detail + 2*index] */
    if (cb && cb->table_1d6f5c) {
        r.field = cb->table_1d6f5c[table_idx];
    }

    /* alt_field = table1d6f7c[has_detail + 2*index] */
    if (cb && cb->table_1d6f7c) {
        r.alt_field = cb->table_1d6f7c[table_idx];
    }

    /* Check if GDAT entry is loadable: cls=0x08, sub=graphicsset, idx=1, field */
    if (cb && cb->query_gdat_entry_if_loadable) {
        r.loadable = cb->query_gdat_entry_if_loadable(ctx, 0x08, graphicsset, 1, r.field);
    }

    r.valid = 1;
    h ^= (uint32_t)cell; h *= 16777619u;
    h ^= (uint32_t)graphicsset; h *= 16777619u;
    h ^= (uint32_t)r.rect_number; h *= 16777619u;
    h ^= (uint32_t)r.field; h *= 16777619u;
    h ^= (uint32_t)r.loadable; h *= 16777619u;
    r.identity_hash = h;
    *out = r;
    return 1;
}
