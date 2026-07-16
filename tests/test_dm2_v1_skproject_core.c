#include "dm2_v1_skproject_core.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        ++failed; \
        printf("FAIL: %s\n", msg); \
    } else { \
        printf("PASS: %s\n", msg); \
    } \
} while (0)

static void test_between_value(void)
{
    CHECK(dm2_v1_skproject_between_value(10, 9, 20) == 10,
          "BETWEEN_VALUE clamps below min");
    CHECK(dm2_v1_skproject_between_value(10, 10, 20) == 10,
          "BETWEEN_VALUE admits min");
    CHECK(dm2_v1_skproject_between_value(10, 17, 20) == 17,
          "BETWEEN_VALUE admits middle");
    CHECK(dm2_v1_skproject_between_value(10, 20, 20) == 20,
          "BETWEEN_VALUE admits max");
    CHECK(dm2_v1_skproject_between_value(10, 21, 20) == 20,
          "BETWEEN_VALUE clamps above max");
    CHECK(dm2_v1_skproject_dm2_between_value(-1, 127, 200) == 127,
          "DM2_BETWEEN_VALUE wrapper uses v5 argument order");
}

static void test_temp_rect_ring(void)
{
    DM2_V1_SkprojectTempRectRing ring;
    DM2_V1_SkprojectTempRectReceipt receipt[5];

    dm2_v1_skproject_temp_rect_ring_init(&ring);
    memset(receipt, 0, sizeof(receipt));

    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4,
                                           &receipt[0]) == 1,
          "ALLOC_TEMP_RECT accepts first rect");
    CHECK(receipt[0].slot == 0 && receipt[0].next_slot == 1 &&
              receipt[0].rect.x == 1 && receipt[0].rect.y == 2 &&
              receipt[0].rect.w == 3 && receipt[0].rect.h == 4,
          "ALLOC_TEMP_RECT writes slot 0 and advances ring");
    CHECK(dm2_v1_skproject_alloc_temp_origin_rect(&ring, 5, 6,
                                                  &receipt[1]) == 1,
          "ALLOC_TEMP_ORIGIN_RECT delegates to temp rect");
    CHECK(receipt[1].slot == 1 && receipt[1].next_slot == 2 &&
              receipt[1].rect.x == 0 && receipt[1].rect.y == 0 &&
              receipt[1].rect.w == 5 && receipt[1].rect.h == 6,
          "ALLOC_TEMP_ORIGIN_RECT writes origin rectangle");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 7, 8, 9, 10,
                                           &receipt[2]) == 1 &&
              receipt[2].slot == 2 && receipt[2].next_slot == 3,
          "third temp rect uses slot 2");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 11, 12, 13, 14,
                                           &receipt[3]) == 1 &&
              receipt[3].slot == 3 && receipt[3].next_slot == 0,
          "fourth temp rect wraps next index to zero");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 15, 16, 17, 18,
                                           &receipt[4]) == 1 &&
              receipt[4].slot == 0 && receipt[4].next_slot == 1 &&
              ring.rects[0].x == 15 && ring.rects[0].h == 18,
          "fifth temp rect overwrites slot 0 like skproject ringbuffer");
    CHECK(receipt[0].receipt_hash != 0u && receipt[4].receipt_hash != 0u &&
              receipt[0].receipt_hash != receipt[4].receipt_hash,
          "temp rect receipts carry nonzero slot/value hash");
    CHECK(dm2_v1_skproject_alloc_temp_rect(0, 1, 2, 3, 4,
                                           &receipt[0]) == 0,
          "ALLOC_TEMP_RECT rejects missing ring");
    CHECK(dm2_v1_skproject_alloc_temp_rect(&ring, 1, 2, 3, 4, 0) == 0,
          "ALLOC_TEMP_RECT rejects missing receipt");
}

static void test_random_helpers(void)
{
    DM2_V1_SkprojectRandomData randdat;

    dm2_v1_skproject_random_init(&randdat);
    CHECK(randdat.random == 0u, "c_random init clears random seed");
    CHECK(dm2_v1_skproject_rand(&randdat) == 0u &&
              randdat.random == 11u,
          "DM2_RAND returns seed*magic+11 shifted by 8");
    CHECK(dm2_v1_skproject_rand16(&randdat, 10u) == 9u,
          "DM2_RAND16 modulo uses the 24-bit DM2_RAND value");
    CHECK(dm2_v1_skproject_randbit(&randdat) == 0,
          "DM2_RANDBIT masks one random bit");
    CHECK(dm2_v1_skproject_randdir(&randdat) == 0u,
          "DM2_RANDDIR masks two random direction bits");
    CHECK(dm2_v1_skproject_rand(&randdat) == 13344383u,
          "c_random sequence advances through every helper");
    CHECK(dm2_v1_skproject_rand16(&randdat, 0u) == 0u,
          "DM2_RAND16 zero range returns zero");
}

static void test_calc_vector_w_dir(void)
{
    DM2_V1_SkprojectVectorWDirReceipt receipt;
    int16_t x;
    int16_t y;

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              0, 3, 2, &x, &y, &receipt) == 1 &&
              x == 12 && y == 17 &&
              receipt.valid && receipt.dir == 0u &&
              receipt.forward_dx == 0 && receipt.forward_dy == -3 &&
              receipt.side_dx == 2 && receipt.side_dy == 0 &&
              receipt.initial_x == 10 && receipt.initial_y == 20,
          "CALC_VECTOR_W_DIR dir north adds forward and right-hand side deltas");

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              1, -1, 4, &x, &y, &receipt) == 1 &&
              x == 9 && y == 24 &&
              receipt.forward_dx == -1 && receipt.forward_dy == 0 &&
              receipt.side_dx == 0 && receipt.side_dy == 4,
          "CALC_VECTOR_W_DIR dir east preserves signed source operands");

    x = 10;
    y = 20;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              3, 5, -2, &x, &y, &receipt) == 1 &&
              x == 5 && y == 22 &&
              receipt.forward_dx == -5 && receipt.forward_dy == 0 &&
              receipt.side_dx == 0 && receipt.side_dy == 2,
          "CALC_VECTOR_W_DIR dir west wraps side direction to north");

    x = -7;
    y = 8;
    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              5, 2, 3, &x, &y, &receipt) == 1 &&
              x == -5 && y == 11 && receipt.dir == 1u,
          "CALC_VECTOR_W_DIR masks direction like the source table index");

    CHECK(dm2_v1_skproject_calc_vector_w_dir(
              0, 1, 1, 0, &y, &receipt) == 0 &&
              receipt.blocked_missing_output,
          "CALC_VECTOR_W_DIR rejects missing output accumulator");
}

static void test_cache_hash_helpers(void)
{
    DM2_V1_SkprojectCacheState state;
    uint16_t ici = 0xffffu;
    uint16_t cache_index = 0xffffu;
    uint8_t *buff;

    dm2_v1_skproject_cache_state_init(&state, 4, 3, 4);
    state.raw_to_mement[2] = 3u;

    CHECK(state.cache_capacity == 4 && state.raw_count == 3 &&
              state.mement_count == 4,
          "cache state clamps source table sizes");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x2000u, &ici) == 0 && ici == 0u,
          "FIND_ICI_FROM_CACHE_HASH returns insertion slot for empty table");
    CHECK(dm2_v1_skproject_insert_cache_hash_at(
              &state, 0x2000u, ici) == 0u,
          "INSERT_CACHE_HASH_AT inserts the first cache hash");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x1000u, &ici) == 0 && ici == 0u,
          "FIND_ICI_FROM_CACHE_HASH finds lower insertion point");
    CHECK(dm2_v1_skproject_insert_cache_hash_at(
              &state, 0x1000u, ici) == 1u,
          "INSERT_CACHE_HASH_AT preserves sorted hash order");
    CHECK(state.cache_count == 2 &&
              state.sorted_cache_indices[0] == 1u &&
              state.sorted_cache_indices[1] == 0u,
          "sorted cache index table mirrors skproject indirect order");
    CHECK(dm2_v1_skproject_find_ici_from_cache_hash(
              &state, 0x2000u, &ici) == 1 && ici == 1u,
          "FIND_ICI_FROM_CACHE_HASH returns existing sorted index");
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x2000u, &cache_index) == 1 &&
              cache_index == 0u,
          "ADD_CACHE_HASH returns existing cache index");
    CHECK(dm2_v1_skproject_add_cache_hash(
              &state, 0x3000u, &cache_index) == 0 &&
              cache_index == 2u,
          "ADD_CACHE_HASH inserts a new cache index");
    CHECK(dm2_v1_skproject_query_mementi_from(&state, 0x8000u) == 0u &&
              dm2_v1_skproject_query_mementi_from(&state, 0x8001u) == 1u &&
              dm2_v1_skproject_query_mementi_from(&state, 2u) == 3u,
          "QUERY_MEMENTI_FROM handles cache-index and raw-data routes");
    CHECK(dm2_v1_skproject_query_mementi_from(&state, 0x8008u) ==
              DM2_V1_SKPROJECT_MEMENT_NONE,
          "QUERY_MEMENTI_FROM rejects out-of-range cache index");
    buff = dm2_v1_skproject_query_mement_buff_from_cache_index(&state, 1u);
    CHECK(buff == state.mement_buffers[1],
          "QUERY_MEMENT_BUFF_FROM_CACHE_INDEX returns mement payload bytes");
    CHECK(dm2_v1_skproject_get_temp_cache_hash(&state) == 0xffff0000u,
          "GET_TEMP_CACHE_HASH starts in the source temp hash range");
    CHECK(dm2_v1_skproject_alloc_temp_cache_index(&state) == 3u &&
              state.hashes[3] == 0xffff0000u &&
              state.temp_hash_counter == 1u,
          "ALLOC_TEMP_CACHE_INDEX allocates a temp hash through ADD_CACHE_HASH");
    CHECK(dm2_v1_skproject_alloc_temp_cache_index(&state) ==
              DM2_V1_SKPROJECT_MEMENT_NONE,
          "ALLOC_TEMP_CACHE_INDEX fails closed when cache table is full");
}

static void test_picture_mement_helpers(void)
{
    DM2_V1_SkprojectCacheState state;
    DM2_V1_SkprojectNewPictReceipt new_pict;
    DM2_V1_SkprojectExtendedPictureRef ext;
    DM2_V1_SkprojectImageMementRequest image;
    DM2_V1_SkprojectPictureRef pict;
    DM2_V1_SkprojectImageMementReceipt image_receipt;
    DM2_V1_SkprojectPictMementReceipt pict_receipt;
    DM2_V1_SkprojectFreeImageMementReceipt free_receipt;
    DM2_V1_SkprojectRecycleMementReceipt recycle_receipt;
    uint16_t pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;

    dm2_v1_skproject_cache_state_init(&state, 4, 8, 4);
    CHECK(dm2_v1_skproject_test_mement(-20, -20) == 1,
          "TEST_MEMENT accepts matching stored length");
    CHECK(dm2_v1_skproject_test_mement(-20, -24) == 0,
          "TEST_MEMENT rejects mismatched stored length");
    CHECK(dm2_v1_skproject_alloc_new_pict(
              7u, 13u, 5u, 4u, &new_pict) == 1 &&
              new_pict.payload_bytes == 35u &&
              new_pict.header_width == 13u &&
              new_pict.header_height == 5u &&
              new_pict.header_bpp == 4u,
          "ALLOC_NEW_PICT stores headers and 4bpp rounded row bytes");
    CHECK(dm2_v1_skproject_alloc_new_pict(
              8u, 13u, 5u, 8u, &new_pict) == 1 &&
              new_pict.payload_bytes == 65u,
          "ALLOC_NEW_PICT keeps 8bpp row bytes unrounded");

    ext.w6 = 0x3fffu;
    ext.w52 = 0x00ffu;
    ext.w54 = 0x003fu;
    CHECK(dm2_v1_skproject_calc_pict_ent_hash(&ext) ==
              (((uint32_t)0x1fffu << 12) | ((uint32_t)0x7fu << 5) | 0x1fu),
          "CALC_PICT_ENT_HASH masks and packs w6/w52/w54");

    memset(&image, 0, sizeof(image));
    image.cls1 = 1u;
    image.cls2 = 2u;
    image.cls4 = 3u;
    image.data_index = 5u;
    image.fallback_data_index = 9u;
    image.y_offset = -32;
    image.bits_pixel = 8u;
    image.existing_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY &&
              pinned_entry == 5u,
          "ALLOC_IMAGE_MEMENT pins 8bpp Y=-32 real image entry");
    image.y_offset = -31;
    pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_Y_OFFSET &&
              pinned_entry == DM2_V1_SKPROJECT_MEMENT_NONE,
          "ALLOC_IMAGE_MEMENT rejects non-source Y offset");
    image.y_offset = -32;
    image.bits_pixel = 4u;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_REJECT_BPP,
          "ALLOC_IMAGE_MEMENT rejects non-8bpp image mement");
    image.bits_pixel = 8u;
    image.data_absent = 1;
    image.fallback_absent = 0;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_PINNED_ENTRY &&
              image_receipt.selected_data_index == 9u,
          "ALLOC_IMAGE_MEMENT falls back to default when primary is absent");
    image.data_index = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status == DM2_V1_SKPROJECT_IMAGE_MEMENT_NO_ENTRY,
          "ALLOC_IMAGE_MEMENT does not fabricate fallback for missing primary");
    image.data_index = 5u;
    image.data_absent = 0;
    image.fallback_absent = 1;
    image.existing_mementi = 2u;
    CHECK(dm2_v1_skproject_alloc_image_mement(
              &state, &image, &pinned_entry, &image_receipt) == 1 &&
              image_receipt.status ==
                  DM2_V1_SKPROJECT_IMAGE_MEMENT_TOUCHED_EXISTING &&
              image_receipt.touched_mementi == 2u,
          "ALLOC_IMAGE_MEMENT touches existing mement instead of pinning");

    CHECK(dm2_v1_skproject_recycle_mementi(
              &state, 2u, DM2_V1_SKPROJECT_MEMENT_NONE, 0u,
              &recycle_receipt) == 1 &&
              recycle_receipt.valid &&
              recycle_receipt.recycled_to_free_list,
          "RECYCLE_MEMENTI records free-list recycle for w4=0xffff");

    image.existing_mementi = 1u;
    state.raw_to_mement[5] = 1u;
    pinned_entry = 5u;
    CHECK(dm2_v1_skproject_free_image_mement(
              &state, &image, &pinned_entry, &free_receipt) == 1 &&
              free_receipt.cleared_pinned_entry &&
              free_receipt.recycled_existing &&
              state.raw_to_mement[5] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_IMAGE_MEMENT clears pinned entry and recycles existing mement");

    memset(&pict, 0, sizeof(pict));
    pict.w4 = 0x0004u;
    image.existing_mementi = DM2_V1_SKPROJECT_MEMENT_NONE;
    pinned_entry = DM2_V1_SKPROJECT_MEMENT_NONE;
    CHECK(dm2_v1_skproject_alloc_pict_mement(
              &state, &pict, &image, &pinned_entry, &pict_receipt) == 1 &&
              pict_receipt.route == DM2_V1_SKPROJECT_PICT_MEMENT_IMAGE,
          "ALLOC_PICT_MEMENT routes image-backed pictures to image mement");
    pict.w4 = 0x0008u;
    pict.w12 = 2u;
    CHECK(dm2_v1_skproject_alloc_pict_mement(
              &state, &pict, &image, &pinned_entry, &pict_receipt) == 1 &&
              pict_receipt.route == DM2_V1_SKPROJECT_PICT_MEMENT_CACHE &&
              pict_receipt.cache_index == 2u && state.cache_count == 0u,
          "ALLOC_PICT_MEMENT routes cache-backed pictures by w12 index");
    state.cache_to_mement[2] = 2u;
    CHECK(dm2_v1_skproject_free_pict_mement(
              &state, &pict, &image, &pinned_entry, &free_receipt) == 1 &&
              state.cache_to_mement[2] == DM2_V1_SKPROJECT_MEMENT_NONE,
          "FREE_PICT_MEMENT frees cache-backed pictures by w12 index");
}

static void test_item_charge_helpers(void)
{
    DM2_V1_SkprojectItemChargeReceipt receipt;
    uint16_t w2;

    CHECK(dm2_v1_skproject_get_max_charge(0x1400u) == 15u,
          "GET_MAX_CHARGE returns 15 for DB5 weapon");
    CHECK(dm2_v1_skproject_get_max_charge(0x1800u) == 15u,
          "GET_MAX_CHARGE returns 15 for DB6 cloth");
    CHECK(dm2_v1_skproject_get_max_charge(0x2800u) == 3u,
          "GET_MAX_CHARGE returns 3 for DB10 miscellaneous item");
    CHECK(dm2_v1_skproject_get_max_charge(0xffffu) == 0u,
          "GET_MAX_CHARGE returns zero for OBJECT_NULL");
    CHECK(dm2_v1_skproject_get_max_charge(0x0800u) == 0u,
          "GET_MAX_CHARGE returns zero for unsupported DB type");

    w2 = (uint16_t)(7u << 10);
    CHECK(dm2_v1_skproject_add_item_charge(0x1400u, &w2, 3, &receipt) ==
              10u &&
              receipt.valid && receipt.db_type == 5 &&
              receipt.previous_charge == 7u && receipt.new_charge == 10u &&
              receipt.max_charge == 15u && ((w2 >> 10) & 0x0fu) == 10u,
          "ADD_ITEM_CHARGE updates DB5 weapon charges in bits 10..13");

    w2 = (uint16_t)(14u << 10);
    CHECK(dm2_v1_skproject_add_item_charge(0x1400u, &w2, 9, &receipt) ==
              15u &&
              ((w2 >> 10) & 0x0fu) == 15u,
          "ADD_ITEM_CHARGE clamps DB5 weapon charges to 15");

    w2 = (uint16_t)(2u << 9);
    CHECK(dm2_v1_skproject_add_item_charge(0x1800u, &w2, -5, &receipt) ==
              0u &&
              receipt.valid && receipt.db_type == 6 &&
              ((w2 >> 9) & 0x0fu) == 0u,
          "ADD_ITEM_CHARGE clamps DB6 cloth charges to zero");

    w2 = (uint16_t)(1u << 14);
    CHECK(dm2_v1_skproject_add_item_charge(0x2800u, &w2, 5, &receipt) ==
              3u &&
              receipt.valid && receipt.db_type == 10 &&
              receipt.max_charge == 3u && ((w2 >> 14) & 0x03u) == 3u,
          "ADD_ITEM_CHARGE clamps DB10 miscellaneous charges to 3");

    w2 = 0xaaaau;
    CHECK(dm2_v1_skproject_add_item_charge(0xffffu, &w2, 1, &receipt) ==
              0u &&
              receipt.blocked_null_object && w2 == 0xaaaau,
          "ADD_ITEM_CHARGE rejects OBJECT_NULL without mutation");

    w2 = 0x5555u;
    CHECK(dm2_v1_skproject_add_item_charge(0x0800u, &w2, 1, &receipt) ==
              0u &&
              receipt.blocked_unsupported_db_type && w2 == 0x5555u,
          "ADD_ITEM_CHARGE rejects unsupported DB type without mutation");
}

static void test_item_value_weight_helpers(void)
{
    DM2_V1_SkprojectItemValueRecord records[8];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectItemValueReceipt receipt;

    memset(records, 0, sizeof(records));
    world.records = records;
    world.record_count = 8u;

    records[0].object_id = 0x1400u;
    records[0].w2 = (uint16_t)(3u << 10);
    records[0].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[0].gdat_word_values[1] = 4u;
    records[0].gdat_word_values[2] = 20u;
    records[0].gdat_word_values[0x34] = 2u;
    records[0].gdat_word_values[0x35] = 5u;

    records[1].object_id = 0x2001u;
    records[1].w2 = 128u;
    records[1].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[1].gdat_word_values[2] = 100u;

    records[2].object_id = 0x2402u;
    records[2].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[2].contained_object_id = 0x1400u;
    records[2].container_type = 0u;
    records[2].gdat_word_values[1] = 10u;

    records[3].object_id = 0x2403u;
    records[3].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[3].contained_object_id = 0x2804u;
    records[3].container_type = 0u;
    records[3].is_moneybox = 1u;
    records[3].gdat_word_values[1] = 1u;

    records[4].object_id = 0x2804u;
    records[4].w2 = (uint16_t)(2u << 14);
    records[4].next_object_id = 0x2805u;
    records[4].gdat_word_values[1] = 5u;
    records[4].gdat_word_values[2] = 7u;

    records[5].object_id = 0x2805u;
    records[5].w2 = 0u;
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[5].gdat_word_values[1] = 6u;
    records[5].gdat_word_values[2] = 11u;

    records[6].object_id = 0x2406u;
    records[6].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[6].container_type = 1u;
    records[6].gdat_word_values[1] = 9u;

    records[7].object_id = 0x1807u;
    records[7].w2 = (uint16_t)(1u << 9);
    records[7].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[7].gdat_word_values[1] = 3u;
    records[7].gdat_word_values[0x34] = 4u;

    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1400u, 1u, &receipt) == 10 &&
              receipt.valid && receipt.base_value == 4 &&
              receipt.charge == 3u && receipt.charge_value_added == 6,
          "QUERY_ITEM_VALUE adds cls4=0x34 weight per charge");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1400u, 2u, &receipt) == 35 &&
              receipt.valid && receipt.base_value == 20 &&
              receipt.charge_value_added == 15,
          "QUERY_ITEM_VALUE adds cls4=0x35 money per charge");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2001u, 2u, &receipt) == 75 &&
              receipt.potion_value_before_scale == 100 &&
              receipt.potion_value_after_scale == 75,
          "QUERY_ITEM_VALUE scales potion money by low-byte power");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2402u, 1u, &receipt) == 20 &&
              receipt.contained_recursive_value == 10,
          "QUERY_ITEM_VALUE recurses normal container contents");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2403u, 1u, &receipt) == 6 &&
              receipt.moneybox_contained_value == 21 &&
              receipt.moneybox_rounding_value == 5,
          "QUERY_ITEM_VALUE rounds moneybox weight as (sum+4)/5");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x2403u, 2u, &receipt) == 32 &&
              receipt.moneybox_contained_value == 32 &&
              receipt.moneybox_rounding_value == 32,
          "QUERY_ITEM_VALUE adds moneybox non-weight value directly");
    CHECK(dm2_v1_skproject_query_item_weight(
              &world, 0x1807u, &receipt) == 7 &&
              receipt.charge_multiplier_cls4 == 0x34u,
          "QUERY_ITEM_WEIGHT delegates to QUERY_ITEM_VALUE cls4=1");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, DM2_V1_SKPROJECT_MEMENT_NONE, 1u, &receipt) == 0 &&
              receipt.blocked_null_object,
          "QUERY_ITEM_VALUE rejects OBJECT_NULL");
    CHECK(dm2_v1_skproject_query_item_value(
              &world, 0x1410u, 1u, &receipt) == 0 &&
              receipt.blocked_missing_record,
          "QUERY_ITEM_VALUE rejects missing source record");
}

static void test_player_weight_helper(void)
{
    DM2_V1_SkprojectItemValueRecord records[4];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectPlayerWeightRequest request;
    DM2_V1_SkprojectPlayerWeightReceipt receipt;

    memset(records, 0, sizeof(records));
    memset(&request, 0xff, sizeof(request));
    world.records = records;
    world.record_count = 4u;

    records[0].object_id = 0x1400u;
    records[0].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[0].gdat_word_values[1] = 4u;
    records[1].object_id = 0x1801u;
    records[1].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[1].gdat_word_values[1] = 5u;
    records[2].object_id = 0x2402u;
    records[2].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[2].container_type = 0u;
    records[3].object_id = 0x2803u;
    records[3].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[3].gdat_word_values[1] = 6u;

    request.inventory[0] = 0x1400u;
    request.inventory[1] = 0x1801u;
    for (uint16_t i = 2u; i < DM2_V1_SKPROJECT_PLAYER_INVENTORY_SLOTS; ++i)
        request.inventory[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.current_container_items[0] = 0x2803u;
    for (uint16_t i = 1u; i < DM2_V1_SKPROJECT_CURRENT_CONTAINER_SLOTS; ++i)
        request.current_container_items[i] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.selected_hand_items[0] = 0x2402u;
    request.selected_hand_items[1] = DM2_V1_SKPROJECT_MEMENT_NONE;
    request.selected_hand_action = 0u;
    request.selected_player_plus_one = 1u;

    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.valid && receipt.inventory_weight == 9u &&
              receipt.open_chest_weight == 6u &&
              receipt.final_weight == 15u &&
              receipt.included_open_chest_overlay &&
              receipt.hero_flag_or == 0x1000u,
          "CALC_PLAYER_WEIGHT sums inventory and selected open chest overlay");

    request.selected_hand_action = 2u;
    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.final_weight == 9u &&
              receipt.blocked_selected_hand_action &&
              !receipt.included_open_chest_overlay,
          "CALC_PLAYER_WEIGHT skips overlay when selected hand action is not 0/1");

    request.selected_hand_action = 0u;
    request.selected_player_plus_one = 2u;
    CHECK(dm2_v1_skproject_calc_player_weight(
              &world, 0u, &request, &receipt) == 1 &&
              receipt.final_weight == 9u &&
              receipt.blocked_player_not_selected,
          "CALC_PLAYER_WEIGHT skips overlay for non-selected player");
}

static void test_count_by_coin_types(void)
{
    DM2_V1_SkprojectItemValueRecord records[6];
    DM2_V1_SkprojectItemValueWorld world;
    DM2_V1_SkprojectCountByCoinTypesReceipt receipt;
    uint16_t money_ids[DM2_V1_SKPROJECT_MONEY_ITEM_MAX] = {
        0x10u, 0x20u, 0x20u, 0x30u, 0x40u,
        0x50u, 0x60u, 0x70u, 0x80u, 0x90u
    };
    int16_t counts[DM2_V1_SKPROJECT_MONEY_ITEM_MAX];

    memset(records, 0, sizeof(records));
    for (uint16_t i = 0; i < DM2_V1_SKPROJECT_MONEY_ITEM_MAX; ++i)
        counts[i] = -7;
    world.records = records;
    world.record_count = 6u;

    records[0].object_id = 0x2400u;
    records[0].contained_object_id = 0x2801u;
    records[0].container_type = 0u;
    records[0].is_moneybox = 1u;

    records[1].object_id = 0x2801u;
    records[1].w2 = (uint16_t)(2u << 14);
    records[1].next_object_id = 0x2802u;
    records[1].distinctive_item_type = 0x20u;
    records[1].is_currency = 1u;

    records[2].object_id = 0x2802u;
    records[2].w2 = 0u;
    records[2].next_object_id = 0x2803u;
    records[2].distinctive_item_type = 0x10u;
    records[2].is_currency = 1u;

    records[3].object_id = 0x2803u;
    records[3].w2 = (uint16_t)(3u << 14);
    records[3].next_object_id = 0x1404u;
    records[3].distinctive_item_type = 0x99u;
    records[3].is_currency = 1u;

    records[4].object_id = 0x1404u;
    records[4].w2 = (uint16_t)(1u << 10);
    records[4].next_object_id = 0x2805u;
    records[4].distinctive_item_type = 0x30u;
    records[4].is_currency = 1u;

    records[5].object_id = 0x2805u;
    records[5].w2 = (uint16_t)(1u << 14);
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;
    records[5].distinctive_item_type = 0x30u;
    records[5].is_currency = 0u;

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, counts, &receipt) == 1 &&
              receipt.valid && receipt.visited_records == 5u &&
              receipt.currency_records == 3u &&
              receipt.matched_currency_records == 3u &&
              counts[0] == 1 && counts[1] == 3 && counts[2] == 3 &&
              counts[3] == 0 && counts[9] == 0,
          "COUNT_BY_COIN_TYPES zeroes ten slots and adds charge+1 by money type");

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids, 2u, counts, &receipt) == 1 &&
              receipt.valid && receipt.money_item_count == 2u &&
              counts[0] == 1 && counts[1] == 3 && counts[2] == 0,
          "COUNT_BY_COIN_TYPES honors caller money table count after zeroing");

    records[5].next_object_id = 0x2810u;
    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, counts, &receipt) == 0 &&
              receipt.blocked_missing_record,
          "COUNT_BY_COIN_TYPES rejects missing source-shaped chain record");
    records[5].next_object_id = DM2_V1_SKPROJECT_MEMENT_NONE;

    CHECK(dm2_v1_skproject_count_by_coin_types(
              &world, 0x2400u, money_ids,
              DM2_V1_SKPROJECT_MONEY_ITEM_MAX, 0, &receipt) == 0 &&
              receipt.blocked_missing_output,
          "COUNT_BY_COIN_TYPES rejects missing output counter table");
}

int main(void)
{
    test_between_value();
    test_temp_rect_ring();
    test_random_helpers();
    test_calc_vector_w_dir();
    test_cache_hash_helpers();
    test_picture_mement_helpers();
    test_item_charge_helpers();
    test_item_value_weight_helpers();
    test_player_weight_helper();
    test_count_by_coin_types();
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_TEMP_RECT") != 0,
          "source evidence names ALLOC_TEMP_RECT");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_BETWEEN_VALUE") != 0,
          "source evidence names DM2_BETWEEN_VALUE");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "DM2_RAND16") != 0,
          "source evidence names c_random helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "CALC_VECTOR_W_DIR") != 0,
          "source evidence names vector direction helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADD_CACHE_HASH") != 0,
          "source evidence names cache hash helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ALLOC_IMAGE_MEMENT") != 0,
          "source evidence names picture mement helpers");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "ADD_ITEM_CHARGE") != 0,
          "source evidence names item charge helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "GET_MAX_CHARGE") != 0,
          "source evidence names max charge helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "QUERY_ITEM_VALUE") != 0,
          "source evidence names item value helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "CALC_PLAYER_WEIGHT") != 0,
          "source evidence names player weight helper");
    CHECK(strstr(dm2_v1_skproject_core_source_evidence(),
                 "COUNT_BY_COIN_TYPES") != 0,
          "source evidence names coin count helper");

    if (failed) {
        printf("%d failure(s)\n", failed);
        return 1;
    }
    puts("all DM2 skproject core helper checks passed");
    return 0;
}
