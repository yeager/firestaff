#include "dm2_v1_skproject_core.h"

#include <stdlib.h>
#include <string.h>

#define DM2_V1_SKPROJECT_RANDOM_MAGIC 0xbb40e62du

static uint32_t dm2_v1_skproject_rect_hash(
    uint8_t slot,
    uint8_t next_slot,
    const DM2_V1_SkprojectRect *rect)
{
    uint32_t hash = 2166136261u;

    hash = (hash ^ slot) * 16777619u;
    hash = (hash ^ next_slot) * 16777619u;
    hash = (hash ^ (uint16_t)rect->x) * 16777619u;
    hash = (hash ^ (uint16_t)rect->y) * 16777619u;
    hash = (hash ^ (uint16_t)rect->w) * 16777619u;
    hash = (hash ^ (uint16_t)rect->h) * 16777619u;
    return hash ? hash : 1u;
}

int16_t dm2_v1_skproject_between_value(int16_t minv,
                                       int16_t newv,
                                       int16_t maxv)
{
    if (newv < minv) return minv;
    if (newv > maxv) return maxv;
    return newv;
}

int16_t dm2_v1_skproject_dm2_between_value(int16_t minv,
                                           int16_t maxv,
                                           int16_t value)
{
    return dm2_v1_skproject_between_value(minv, value, maxv);
}

void dm2_v1_skproject_temp_rect_ring_init(
    DM2_V1_SkprojectTempRectRing *ring)
{
    if (!ring) return;
    memset(ring, 0, sizeof(*ring));
}

int dm2_v1_skproject_alloc_temp_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt)
{
    uint8_t slot;
    DM2_V1_SkprojectRect *rect;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!ring || !out_receipt || ring->next_index >= 4u) return 0;

    slot = ring->next_index;
    rect = &ring->rects[slot];
    ring->next_index = (uint8_t)((slot + 1u) & 3u);

    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;

    out_receipt->valid = 1;
    out_receipt->slot = slot;
    out_receipt->next_slot = ring->next_index;
    out_receipt->rect = *rect;
    out_receipt->receipt_hash =
        dm2_v1_skproject_rect_hash(slot, ring->next_index, rect);
    return 1;
}

int dm2_v1_skproject_alloc_temp_origin_rect(
    DM2_V1_SkprojectTempRectRing *ring,
    int16_t w,
    int16_t h,
    DM2_V1_SkprojectTempRectReceipt *out_receipt)
{
    return dm2_v1_skproject_alloc_temp_rect(ring, 0, 0, w, h, out_receipt);
}

void dm2_v1_skproject_random_init(DM2_V1_SkprojectRandomData *randdat)
{
    if (!randdat) return;
    randdat->random = 0u;
}

uint32_t dm2_v1_skproject_rand(DM2_V1_SkprojectRandomData *randdat)
{
    uint32_t value;

    if (!randdat) return 0u;
    value = randdat->random * DM2_V1_SKPROJECT_RANDOM_MAGIC + 11u;
    randdat->random = value;
    return value >> 8;
}

uint16_t dm2_v1_skproject_rand16(DM2_V1_SkprojectRandomData *randdat,
                                 uint16_t max_value)
{
    if (!randdat || max_value == 0u) return 0u;
    return (uint16_t)(dm2_v1_skproject_rand(randdat) % (uint32_t)max_value);
}

int dm2_v1_skproject_randbit(DM2_V1_SkprojectRandomData *randdat)
{
    return (int)(dm2_v1_skproject_rand(randdat) & 1u);
}

uint8_t dm2_v1_skproject_randdir(DM2_V1_SkprojectRandomData *randdat)
{
    return (uint8_t)(dm2_v1_skproject_rand(randdat) & 3u);
}

int dm2_v1_skproject_calc_vector_w_dir(
    int16_t dir,
    int16_t xx,
    int16_t yy,
    int16_t *x,
    int16_t *y,
    DM2_V1_SkprojectVectorWDirReceipt *out_receipt)
{
    static const int16_t x_delta[4] = { 0, 1, 0, -1 };
    static const int16_t y_delta[4] = { -1, 0, 1, 0 };
    DM2_V1_SkprojectVectorWDirReceipt receipt;
    uint8_t idx;
    uint8_t side_idx;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.dir = (uint8_t)((uint16_t)dir & 3u);
    receipt.input_xx = xx;
    receipt.input_yy = yy;
    if (!x || !y) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    idx = receipt.dir;
    side_idx = (uint8_t)((idx + 1u) & 3u);
    receipt.initial_x = *x;
    receipt.initial_y = *y;
    receipt.forward_dx = (int16_t)(xx * x_delta[idx]);
    receipt.forward_dy = (int16_t)(xx * y_delta[idx]);
    receipt.side_dx = (int16_t)(yy * x_delta[side_idx]);
    receipt.side_dy = (int16_t)(yy * y_delta[side_idx]);
    *x = (int16_t)(*x + receipt.forward_dx + receipt.side_dx);
    *y = (int16_t)(*y + receipt.forward_dy + receipt.side_dy);
    receipt.final_x = *x;
    receipt.final_y = *y;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

void dm2_v1_skproject_cache_state_init(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_capacity,
    uint16_t raw_count,
    uint16_t mement_count)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (cache_capacity > DM2_V1_SKPROJECT_CACHE_LIMIT)
        cache_capacity = DM2_V1_SKPROJECT_CACHE_LIMIT;
    if (raw_count > DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT)
        raw_count = DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT;
    if (mement_count > DM2_V1_SKPROJECT_CACHE_LIMIT)
        mement_count = DM2_V1_SKPROJECT_CACHE_LIMIT;
    state->cache_capacity = cache_capacity;
    state->raw_count = raw_count;
    state->mement_count = mement_count;
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_CACHE_LIMIT; ++i)
        state->cache_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_RAW_MEMENT_LIMIT; ++i)
        state->raw_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
}

int dm2_v1_skproject_find_ici_from_cache_hash(
    const DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t *out_ici)
{
    int di = -1;
    int cx;

    if (!state || !out_ici || state->cache_count > state->cache_capacity)
        return 0;
    cx = (int)state->cache_count;
    while (1) {
        int si = (di + cx) >> 1;
        uint16_t cache_index;
        uint32_t stored_hash;

        if (si == di) {
            *out_ici = (uint16_t)(si + 1);
            return 0;
        }
        if (si < 0 || si >= (int)state->cache_count)
            return 0;
        cache_index = state->sorted_cache_indices[si];
        if (cache_index >= state->cache_capacity)
            return 0;
        stored_hash = state->hashes[cache_index];
        if (cache_hash < stored_hash) {
            cx = si;
        } else if (cache_hash > stored_hash) {
            di = si;
        } else {
            *out_ici = (uint16_t)si;
            return 1;
        }
    }
}

static uint16_t dm2_v1_skproject_first_free_cache_index(
    const DM2_V1_SkprojectCacheState *state)
{
    for (uint16_t i = 0; i < state->cache_capacity; ++i) {
        int used = 0;
        for (uint16_t j = 0; j < state->cache_count; ++j) {
            if (state->sorted_cache_indices[j] == i) {
                used = 1;
                break;
            }
        }
        if (!used) return i;
    }
    return DM2_V1_SKPROJECT_MEMENT_NONE;
}

uint16_t dm2_v1_skproject_insert_cache_hash_at(
    DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t ici)
{
    uint16_t cache_index;

    if (!state || state->cache_count >= state->cache_capacity ||
        state->cache_count >= DM2_V1_SKPROJECT_CACHE_LIMIT ||
        ici > state->cache_count) {
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    }
    cache_index = dm2_v1_skproject_first_free_cache_index(state);
    if (cache_index == DM2_V1_SKPROJECT_MEMENT_NONE)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    for (uint16_t i = state->cache_count; i > ici; --i)
        state->sorted_cache_indices[i] = state->sorted_cache_indices[i - 1u];
    state->sorted_cache_indices[ici] = cache_index;
    state->hashes[cache_index] = cache_hash;
    state->cache_to_mement[cache_index] =
        cache_index < state->mement_count ? cache_index :
        DM2_V1_SKPROJECT_MEMENT_NONE;
    state->cache_count++;
    return cache_index;
}

uint16_t dm2_v1_skproject_query_mementi_from(
    const DM2_V1_SkprojectCacheState *state,
    uint16_t index)
{
    uint16_t plain_index;

    if (!state) return DM2_V1_SKPROJECT_MEMENT_NONE;
    plain_index = (uint16_t)(index & 0x7fffu);
    if ((index & 0x8000u) != 0u) {
        if (plain_index >= state->cache_capacity)
            return DM2_V1_SKPROJECT_MEMENT_NONE;
        return state->cache_to_mement[plain_index];
    }
    if (plain_index >= state->raw_count)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    return state->raw_to_mement[plain_index];
}

int dm2_v1_skproject_add_cache_hash(
    DM2_V1_SkprojectCacheState *state,
    uint32_t cache_hash,
    uint16_t *out_cache_index)
{
    uint16_t ici = 0u;
    uint16_t cache_index;

    if (!state || !out_cache_index) return -1;
    if (dm2_v1_skproject_find_ici_from_cache_hash(
            state, cache_hash, &ici) != 0) {
        cache_index = state->sorted_cache_indices[ici];
        if (cache_index >= state->cache_capacity)
            return -1;
        *out_cache_index = cache_index;
        return 1;
    }
    cache_index =
        dm2_v1_skproject_insert_cache_hash_at(state, cache_hash, ici);
    if (cache_index == DM2_V1_SKPROJECT_MEMENT_NONE)
        return -1;
    *out_cache_index = cache_index;
    return 0;
}

uint8_t *dm2_v1_skproject_query_mement_buff_from_cache_index(
    DM2_V1_SkprojectCacheState *state,
    uint16_t cache_index)
{
    uint16_t mementi;

    if (!state) return 0;
    mementi = dm2_v1_skproject_query_mementi_from(
        state, (uint16_t)(cache_index | 0x8000u));
    if (mementi == DM2_V1_SKPROJECT_MEMENT_NONE ||
        mementi >= state->mement_count) {
        return 0;
    }
    return state->mement_buffers[mementi];
}

uint32_t dm2_v1_skproject_get_temp_cache_hash(
    const DM2_V1_SkprojectCacheState *state)
{
    uint16_t ici = 0u;
    uint32_t hash;

    if (!state) return 0u;
    for (uint32_t guard = 0; guard < 0x10000u; ++guard) {
        hash = 0xffff0000u |
               (uint32_t)((state->temp_hash_counter + guard) & 0xffffu);
        if (dm2_v1_skproject_find_ici_from_cache_hash(
                state, hash, &ici) == 0) {
            return hash;
        }
    }
    return 0u;
}

uint16_t dm2_v1_skproject_alloc_temp_cache_index(
    DM2_V1_SkprojectCacheState *state)
{
    uint16_t cache_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    uint32_t hash;

    if (!state) return DM2_V1_SKPROJECT_MEMENT_NONE;
    hash = dm2_v1_skproject_get_temp_cache_hash(state);
    if (hash == 0u) return DM2_V1_SKPROJECT_MEMENT_NONE;
    if (dm2_v1_skproject_add_cache_hash(state, hash, &cache_index) < 0)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    state->temp_hash_counter++;
    return cache_index;
}

int dm2_v1_skproject_test_mement(int32_t dw0, int32_t stored_len)
{
    int32_t probe_offset;

    probe_offset = abs((int)-dw0) - 4;
    if (probe_offset < 0 || probe_offset >= 65536)
        return 0;
    return dw0 == stored_len;
}

int dm2_v1_skproject_recycle_mementi(
    DM2_V1_SkprojectCacheState *state,
    uint16_t mementi,
    uint16_t previous_w4,
    uint16_t yy,
    DM2_V1_SkprojectRecycleMementReceipt *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !out_receipt || mementi >= state->mement_count)
        return 0;
    out_receipt->valid = 1;
    out_receipt->mementi = mementi;
    out_receipt->previous_w4 = previous_w4;
    out_receipt->yy = yy;
    out_receipt->recycled_to_free_list =
        previous_w4 == DM2_V1_SKPROJECT_MEMENT_NONE;
    if (out_receipt->recycled_to_free_list) {
        for (uint16_t i = 0; i < state->cache_capacity; ++i) {
            if (state->cache_to_mement[i] == mementi)
                state->cache_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
        }
        for (uint16_t i = 0; i < state->raw_count; ++i) {
            if (state->raw_to_mement[i] == mementi)
                state->raw_to_mement[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
        }
    }
    return 1;
}

int dm2_v1_skproject_alloc_new_pict(
    uint16_t index,
    uint16_t width,
    uint16_t height,
    uint16_t bpp,
    DM2_V1_SkprojectNewPictReceipt *out_receipt)
{
    uint32_t row_bytes;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || height == 0u)
        return 0;
    row_bytes = bpp == 4u ? (uint32_t)(((width + 1u) & 0xfffeu) >> 1) :
                            (uint32_t)width;
    out_receipt->index = index;
    out_receipt->width = width;
    out_receipt->height = height;
    out_receipt->bpp = bpp;
    out_receipt->payload_bytes = row_bytes * (uint32_t)height;
    out_receipt->header_width = width;
    out_receipt->header_height = height;
    out_receipt->header_bpp = bpp;
    return 1;
}

uint32_t dm2_v1_skproject_calc_pict_ent_hash(
    const DM2_V1_SkprojectExtendedPictureRef *ref)
{
    if (!ref) return 0u;
    return ((uint32_t)(ref->w6 & 0x1fffu) << 12) |
           ((uint32_t)(ref->w52 & 0x007fu) << 5) |
           (uint32_t)(ref->w54 & 0x001fu);
}

static uint16_t dm2_v1_skproject_select_image_mement_entry(
    const DM2_V1_SkprojectImageMementRequest *request,
    int *absent)
{
    if (absent) *absent = 1;
    if (!request) return DM2_V1_SKPROJECT_MEMENT_NONE;
    if (request->data_index != DM2_V1_SKPROJECT_MEMENT_NONE &&
        !request->data_absent) {
        if (absent) *absent = 0;
        return request->data_index;
    }
    if (request->data_index == DM2_V1_SKPROJECT_MEMENT_NONE)
        return DM2_V1_SKPROJECT_MEMENT_NONE;
    if (request->fallback_data_index != DM2_V1_SKPROJECT_MEMENT_NONE &&
        !request->fallback_absent) {
        if (absent) *absent = 0;
        return request->fallback_data_index;
    }
    if (request->data_index != DM2_V1_SKPROJECT_MEMENT_NONE)
        return request->data_index;
    return request->fallback_data_index;
}

int dm2_v1_skproject_alloc_image_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectImageMementRequest *request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectImageMementReceipt *out_receipt)
{
    int absent = 1;
    uint16_t selected;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !request || !out_receipt || !pinned_entry)
        return 0;
    out_receipt->selected_data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    out_receipt->touched_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    out_receipt->pinned_entry_index = DM2_V1_SKPROJECT_MEMENT_NONE;

    selected = dm2_v1_skproject_select_image_mement_entry(request, &absent);
    out_receipt->selected_data_index = selected;
    if (selected == DM2_V1_SKPROJECT_MEMENT_NONE) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_NO_ENTRY;
        return 1;
    }
    if (request->existing_mementi != DM2_V1_SKPROJECT_MEMENT_NONE) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_TOUCHED_EXISTING;
        out_receipt->touched_mementi = request->existing_mementi;
        return request->existing_mementi < state->mement_count;
    }
    if (absent) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_ABSENT;
        return 1;
    }
    if (request->y_offset != -32) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_Y_OFFSET;
        return 1;
    }
    if (request->bits_pixel != 8u) {
        out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_BPP;
        return 1;
    }
    *pinned_entry = selected;
    out_receipt->status = DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY;
    out_receipt->pinned_entry_index = selected;
    return 1;
}

int dm2_v1_skproject_alloc_pict_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectPictureRef *ref,
    const DM2_V1_SkprojectImageMementRequest *image_request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectPictMementReceipt *out_receipt)
{
    uint16_t cache_index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !ref || !out_receipt)
        return 0;
    if ((ref->w4 & 0x0004u) != 0u) {
        out_receipt->route = DM2_V1_SKPROJECT_PICT_MEMENT_IMAGE;
        return dm2_v1_skproject_alloc_image_mement(
            state, image_request, pinned_entry, &out_receipt->image);
    }
    if ((ref->w4 & 0x0008u) != 0u) {
        out_receipt->route = DM2_V1_SKPROJECT_PICT_MEMENT_CACHE;
        cache_index = ref->w12;
        if (cache_index >= state->cache_capacity)
            return 0;
        out_receipt->cache_index = cache_index;
        return 1;
    }
    out_receipt->route = DM2_V1_SKPROJECT_PICT_MEMENT_NONE;
    return 1;
}

int dm2_v1_skproject_free_image_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectImageMementRequest *request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectFreeImageMementReceipt *out_receipt)
{
    int absent = 1;
    uint16_t selected;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !request || !pinned_entry || !out_receipt)
        return 0;
    out_receipt->selected_data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    out_receipt->recycled_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    selected = dm2_v1_skproject_select_image_mement_entry(request, &absent);
    out_receipt->selected_data_index = selected;
    if (selected == DM2_V1_SKPROJECT_MEMENT_NONE)
        return 1;
    if (*pinned_entry == selected) {
        *pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
        out_receipt->cleared_pinned_entry = 1;
    }
    if (request->existing_mementi != DM2_V1_SKPROJECT_MEMENT_NONE &&
        request->existing_mementi < state->mement_count) {
        DM2_V1_SkprojectRecycleMementReceipt recycle;

        dm2_v1_skproject_recycle_mementi(
            state, request->existing_mementi, DM2_V1_SKPROJECT_MEMENT_NONE,
            0u, &recycle);
        out_receipt->recycled_existing = recycle.valid;
        out_receipt->recycled_mementi = request->existing_mementi;
    }
    return 1;
}

int dm2_v1_skproject_free_pict_mement(
    DM2_V1_SkprojectCacheState *state,
    const DM2_V1_SkprojectPictureRef *ref,
    const DM2_V1_SkprojectImageMementRequest *image_request,
    uint16_t *pinned_entry,
    DM2_V1_SkprojectFreeImageMementReceipt *out_receipt)
{
    uint16_t cache_index;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!state || !ref || !out_receipt)
        return 0;
    if ((ref->w4 & 0x0004u) != 0u)
        return dm2_v1_skproject_free_image_mement(
            state, image_request, pinned_entry, out_receipt);
    if ((ref->w4 & 0x0008u) != 0u && ref->w12 < state->cache_capacity) {
        cache_index = ref->w12;
        state->cache_to_mement[cache_index] =
            DM2_V1_SKPROJECT_MEMENT_NONE;
    }
    return 1;
}

uint16_t dm2_v1_skproject_add_item_charge(
    uint16_t object_id,
    uint16_t *record_w2,
    int16_t delta,
    DM2_V1_SkprojectItemChargeReceipt *out_receipt)
{
    DM2_V1_SkprojectItemChargeReceipt receipt;
    uint16_t charge = 0u;
    uint16_t max_charge = 0u;
    uint16_t mask = 0u;
    unsigned shift = 0u;
    int db_type;
    int adjusted;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.object_id = object_id;
    receipt.delta = delta;
    db_type = (int)((object_id >> 10) & 0x0fu);
    receipt.db_type = db_type;
    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE) {
        receipt.blocked_null_object = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0u;
    }
    if (!record_w2) {
        receipt.blocked_unsupported_db_type = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0u;
    }
    receipt.previous_w2 = *record_w2;
    switch (db_type) {
    case 5:
        shift = 10u;
        max_charge = 0x000fu;
        mask = (uint16_t)(0x000fu << 10);
        break;
    case 6:
        shift = 9u;
        max_charge = 0x000fu;
        mask = (uint16_t)(0x000fu << 9);
        break;
    case 10:
        shift = 14u;
        max_charge = 0x0003u;
        mask = (uint16_t)(0x0003u << 14);
        break;
    default:
        receipt.blocked_unsupported_db_type = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0u;
    }
    charge = (uint16_t)((*record_w2 & mask) >> shift);
    adjusted = (int)charge + (int)delta;
    charge = (uint16_t)dm2_v1_skproject_between_value(
        0, (int16_t)adjusted, (int16_t)max_charge);
    *record_w2 = (uint16_t)((*record_w2 & (uint16_t)~mask) |
                            (uint16_t)(charge << shift));
    receipt.valid = 1;
    receipt.previous_charge = (uint16_t)((receipt.previous_w2 & mask) >> shift);
    receipt.new_charge = charge;
    receipt.max_charge = max_charge;
    receipt.new_w2 = *record_w2;
    if (out_receipt) *out_receipt = receipt;
    return charge;
}

uint16_t dm2_v1_skproject_get_max_charge(uint16_t object_id)
{
    int db_type;

    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE)
        return 0u;
    db_type = (int)((object_id >> 10) & 0x0fu);
    if (db_type == 5 || db_type == 6)
        return 15u;
    if (db_type == 10)
        return 3u;
    return 0u;
}

static int dm2_v1_skproject_item_db_type(uint16_t object_id)
{
    return (int)((object_id >> 10) & 0x0fu);
}

static const DM2_V1_SkprojectItemValueRecord *
dm2_v1_skproject_find_item_record(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id)
{
    if (!world || !world->records) return 0;
    for (uint16_t i = 0; i < world->record_count; ++i) {
        if (world->records[i].object_id == object_id)
            return &world->records[i];
    }
    return 0;
}

static int dm2_v1_skproject_is_container_chest_record(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id)
{
    const DM2_V1_SkprojectItemValueRecord *record;

    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE ||
        dm2_v1_skproject_item_db_type(object_id) != 9) {
        return 0;
    }
    record = dm2_v1_skproject_find_item_record(world, object_id);
    return record && record->container_type == 0u && !record->is_moneybox;
}

static uint16_t dm2_v1_skproject_gdat_word_value(
    const DM2_V1_SkprojectItemValueRecord *record,
    uint8_t cls4)
{
    if (!record || cls4 >= 0x36u) return 0u;
    return record->gdat_word_values[cls4];
}

static int32_t dm2_v1_skproject_query_item_value_depth(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    uint8_t cls4,
    unsigned depth,
    DM2_V1_SkprojectItemValueReceipt *out_receipt)
{
    DM2_V1_SkprojectItemValueReceipt receipt;
    const DM2_V1_SkprojectItemValueRecord *record;
    int32_t value;
    int db_type;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.object_id = object_id;
    receipt.cls4 = cls4;
    db_type = dm2_v1_skproject_item_db_type(object_id);
    receipt.db_type = db_type;
    if (object_id == DM2_V1_SKPROJECT_MEMENT_NONE) {
        receipt.blocked_null_object = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    if (depth > DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT) {
        receipt.blocked_recursion_limit = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    record = dm2_v1_skproject_find_item_record(world, object_id);
    if (!record) {
        receipt.blocked_missing_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    value = dm2_v1_skproject_gdat_word_value(record, cls4);
    receipt.base_value = value;

    if (cls4 == 1u || cls4 == 2u) {
        uint8_t multiplier_cls4 = cls4 == 1u ? 0x34u : 0x35u;
        uint16_t multiplier =
            dm2_v1_skproject_gdat_word_value(record, multiplier_cls4);

        receipt.charge_multiplier_cls4 = multiplier_cls4;
        if (multiplier > 0u) {
            uint16_t w2 = record->w2;
            DM2_V1_SkprojectItemChargeReceipt charge_receipt;

            receipt.charge = dm2_v1_skproject_add_item_charge(
                object_id, &w2, 0, &charge_receipt);
            receipt.charge_value_added =
                (int32_t)receipt.charge * (int32_t)multiplier;
            value += receipt.charge_value_added;
        }
    }
    if (cls4 == 2u && db_type == 8 && value > 1) {
        int32_t half = value >> 1;

        receipt.potion_value_before_scale = value;
        value = half + ((int32_t)(record->w2 & 0x00ffu) * half) / 255;
        receipt.potion_value_after_scale = value;
    }
    if (db_type == 9 && record->container_type == 0u) {
        uint16_t child = record->contained_object_id;
        int32_t moneybox_sum = 0;
        unsigned guard = 0;

        while (child != DM2_V1_SKPROJECT_MEMENT_NONE) {
            const DM2_V1_SkprojectItemValueRecord *child_record;
            int child_db_type;

            if (++guard > DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT) {
                receipt.blocked_recursion_limit = 1;
                if (out_receipt) *out_receipt = receipt;
                return value;
            }
            child_record = dm2_v1_skproject_find_item_record(world, child);
            if (!child_record) {
                receipt.blocked_missing_record = 1;
                if (out_receipt) *out_receipt = receipt;
                return value;
            }
            child_db_type = dm2_v1_skproject_item_db_type(child);
            if (record->is_moneybox && child_db_type == 10) {
                uint16_t w2 = child_record->w2;
                uint16_t charge = dm2_v1_skproject_add_item_charge(
                    child, &w2, 0, 0);

                moneybox_sum +=
                    (int32_t)dm2_v1_skproject_gdat_word_value(
                        child_record, cls4) *
                    (int32_t)(charge + 1u);
            } else {
                int32_t child_value =
                    dm2_v1_skproject_query_item_value_depth(
                        world, child, cls4, depth + 1u, 0);

                value += child_value;
                receipt.contained_recursive_value += child_value;
            }
            child = child_record->next_object_id;
        }
        if (record->is_moneybox) {
            receipt.moneybox_contained_value = moneybox_sum;
            receipt.moneybox_rounding_value =
                cls4 == 1u ? (moneybox_sum + 4) / 5 : moneybox_sum;
            value += receipt.moneybox_rounding_value;
        }
    }
    receipt.valid = 1;
    receipt.final_value = value;
    if (out_receipt) *out_receipt = receipt;
    return value;
}

int32_t dm2_v1_skproject_query_item_value(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    uint8_t cls4,
    DM2_V1_SkprojectItemValueReceipt *out_receipt)
{
    return dm2_v1_skproject_query_item_value_depth(
        world, object_id, cls4, 0u, out_receipt);
}

int32_t dm2_v1_skproject_query_item_weight(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t object_id,
    DM2_V1_SkprojectItemValueReceipt *out_receipt)
{
    return dm2_v1_skproject_query_item_value(world, object_id, 1u,
                                             out_receipt);
}

int dm2_v1_skproject_calc_player_weight(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t player,
    const DM2_V1_SkprojectPlayerWeightRequest *request,
    DM2_V1_SkprojectPlayerWeightReceipt *out_receipt)
{
    DM2_V1_SkprojectPlayerWeightReceipt receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.player = player;
    receipt.hero_flag_or = 0x1000u;
    if (!world || !request || !out_receipt) {
        receipt.blocked_missing_request = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS; ++i) {
        receipt.inventory_weight +=
            (uint32_t)dm2_v1_skproject_query_item_weight(
                world, request->inventory[i], 0);
    }
    if (request->selected_player_plus_one != (uint16_t)(player + 1u)) {
        receipt.blocked_player_not_selected = 1;
    } else if (request->selected_hand_action >= 2u) {
        receipt.blocked_selected_hand_action = 1;
    } else if (!dm2_v1_skproject_is_container_chest_record(
                   world,
                   request->selected_hand_items[
                       request->selected_hand_action])) {
        receipt.blocked_selected_hand_not_chest = 1;
    } else {
        receipt.included_open_chest_overlay = 1;
        for (uint16_t i = 0;
             i < DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS;
             ++i) {
            receipt.open_chest_weight +=
                (uint32_t)dm2_v1_skproject_query_item_weight(
                    world, request->current_container_items[i], 0);
        }
    }
    receipt.final_weight = receipt.inventory_weight + receipt.open_chest_weight;
    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_skproject_count_by_coin_types(
    const DM2_V1_SkprojectItemValueWorld *world,
    uint16_t moneybox_object_id,
    const uint16_t *money_item_ids,
    uint16_t money_item_count,
    int16_t *out_counts,
    DM2_V1_SkprojectCountByCoinTypesReceipt *out_receipt)
{
    DM2_V1_SkprojectCountByCoinTypesReceipt receipt;
    const DM2_V1_SkprojectItemValueRecord *moneybox;
    uint16_t child;
    uint16_t guard = 0u;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    receipt.moneybox_object_id = moneybox_object_id;
    if (money_item_count > DM2_V1_SKPROJECT_MONEY_ITEM_MAX)
        money_item_count = DM2_V1_SKPROJECT_MONEY_ITEM_MAX;
    receipt.money_item_count = money_item_count;
    if (!out_counts) {
        receipt.blocked_missing_output = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    memset(out_counts, 0,
           sizeof(out_counts[0]) * DM2_V1_SKPROJECT_MONEY_ITEM_MAX);
    if (!world || !money_item_ids) {
        receipt.blocked_missing_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    moneybox = dm2_v1_skproject_find_item_record(world, moneybox_object_id);
    if (!moneybox || dm2_v1_skproject_item_db_type(moneybox_object_id) != 9) {
        receipt.blocked_missing_record = 1;
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    child = moneybox->contained_object_id;
    while (child != DM2_V1_SKPROJECT_MEMENT_NONE) {
        const DM2_V1_SkprojectItemValueRecord *record;

        if (++guard > DM2_V1_SKPROJECT_ITEM_VALUE_RECORD_LIMIT) {
            receipt.blocked_recursion_limit = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        record = dm2_v1_skproject_find_item_record(world, child);
        if (!record) {
            receipt.blocked_missing_record = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.visited_records++;
        if (dm2_v1_skproject_item_db_type(child) == 10 &&
            record->is_currency) {
            uint16_t w2 = record->w2;
            uint16_t charge =
                dm2_v1_skproject_add_item_charge(child, &w2, 0, 0);

            receipt.currency_records++;
            for (uint16_t i = 0; i < money_item_count; ++i) {
                if (money_item_ids[i] == record->distinctive_item_type) {
                    out_counts[i] =
                        (int16_t)(out_counts[i] + (int16_t)(charge + 1u));
                    receipt.counts[i] = out_counts[i];
                    receipt.matched_currency_records++;
                }
            }
        }
        child = record->next_object_id;
    }
    receipt.valid = 1;
    for (uint16_t i = 0; i < money_item_count; ++i)
        receipt.counts[i] = out_counts[i];
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

const char *dm2_v1_skproject_core_source_evidence(void)
{
    return "skproject SKWINSPX/src/v4/skcore.cpp "
           "ALLOC_TEMP_RECT/ALLOC_TEMP_ORIGIN_RECT/BETWEEN_VALUE; "
           "SKWINSPX/src/v5/skrect.cpp alloc_tmprect/alloc_origin_tmprect; "
           "SKWINSPX/src/v5/util.cpp DM2_BETWEEN_VALUE; "
           "SKULLWIN/c_random.cpp DM2_RAND16/DM2_RANDBIT/DM2_RANDDIR; "
           "SKULLWIN/util.cpp DM2_CALC_VECTOR_W_DIR and "
           "SKWIN/SkWinCore.cpp CALC_VECTOR_W_DIR; "
           "SKWIN/SkWinCore.cpp FIND_ICI_FROM_CACHE_HASH/"
           "INSERT_CACHE_HASH_AT/QUERY_MEMENTI_FROM/ADD_CACHE_HASH/"
           "QUERY_MEMENT_BUFF_FROM_CACHE_INDEX/GET_TEMP_CACHE_HASH/"
           "ALLOC_TEMP_CACHE_INDEX/RECYCLE_MEMENTI/TEST_MEMENT/"
           "ALLOC_NEW_PICT/ALLOC_IMAGE_MEMENT/ALLOC_PICT_MEMENT/"
           "CALC_PICT_ENT_HASH/FREE_IMAGE_MEMENT/FREE_PICT_MEMENT; "
           "SKWIN/SkWinCore.cpp ADD_ITEM_CHARGE/GET_MAX_CHARGE/"
           "QUERY_ITEM_VALUE/QUERY_ITEM_WEIGHT/CALC_PLAYER_WEIGHT/"
           "COUNT_BY_COIN_TYPES and "
           "SKULLWIN/c_item.cpp DM2_ADD_ITEM_CHARGE/DM2_GET_MAX_CHARGE/"
           "DM2_QUERY_ITEM_VALUE/DM2_QUERY_ITEM_WEIGHT; "
           "SKULLWIN/c_querydb.cpp DM2_COUNT_BY_COIN_TYPES";
}
