#include "redmcsb_f0695_set_creature_replacement_colors_pc34_compat.h"

#include <assert.h>
#include <string.h>

enum {
    COLOR_COUNT = 16,
    SET_COUNT = 3
};

static void initialize_palette(RedmcsbF0695ColorPc34Compat *palette,
                               uint8_t seed)
{
    size_t index;

    for (index = 0U; index < COLOR_COUNT; ++index) {
        palette[index].red = (uint8_t)(seed + index);
        palette[index].green = (uint8_t)(seed + index + 1U);
        palette[index].blue = (uint8_t)(seed + index + 2U);
    }
}

static void test_applies_one_set_to_all_six_palettes_and_invalidates_cache(void)
{
    RedmcsbF0695ColorPc34Compat palettes[
        REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT][COLOR_COUNT];
    RedmcsbF0695ColorPc34Compat sets[
        SET_COUNT][REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT];
    RedmcsbF0695StatePc34Compat state = { 0 };
    int16_t cache_index = 4;
    size_t palette_index;

    for (palette_index = 0U;
         palette_index < REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT;
         ++palette_index) {
        initialize_palette(palettes[palette_index], (uint8_t)(palette_index * 20U));
        state.palette_tables[palette_index] = palettes[palette_index];
        sets[1][palette_index].red = (uint8_t)(50U + palette_index);
        sets[1][palette_index].green = (uint8_t)(70U + palette_index);
        sets[1][palette_index].blue = (uint8_t)(90U + palette_index);
    }
    state.color_count = COLOR_COUNT;
    state.replacement_sets = &sets[0][0];
    state.replacement_set_count = SET_COUNT;
    state.palette_cache_index = &cache_index;

    assert(redmcsb_f0695_set_creature_replacement_colors_pc34_compat(
        &state, 9, 1));
    assert(cache_index == -1);
    for (palette_index = 0U;
         palette_index < REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT;
         ++palette_index) {
        assert(memcmp(&palettes[palette_index][9], &sets[1][palette_index],
                      sizeof(sets[1][palette_index])) == 0);
        assert(palettes[palette_index][8].red ==
               (uint8_t)(palette_index * 20U + 8U));
    }
}

static void test_rejection_is_atomic(void)
{
    RedmcsbF0695ColorPc34Compat palettes[
        REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT][COLOR_COUNT];
    RedmcsbF0695ColorPc34Compat original[
        REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT][COLOR_COUNT];
    RedmcsbF0695ColorPc34Compat set[
        REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT] = { { 1U, 2U, 3U } };
    RedmcsbF0695StatePc34Compat state = { 0 };
    int16_t cache_index = 2;
    size_t palette_index;

    for (palette_index = 0U;
         palette_index < REDMCSB_F0695_PC34_DUNGEON_PALETTE_COUNT;
         ++palette_index) {
        initialize_palette(palettes[palette_index], (uint8_t)(palette_index * 10U));
        state.palette_tables[palette_index] = palettes[palette_index];
    }
    memcpy(original, palettes, sizeof(original));
    state.color_count = COLOR_COUNT;
    state.replacement_sets = set;
    state.replacement_set_count = 1U;
    state.palette_cache_index = &cache_index;

    assert(!redmcsb_f0695_set_creature_replacement_colors_pc34_compat(
        &state, COLOR_COUNT, 0));
    assert(cache_index == 2);
    assert(memcmp(palettes, original, sizeof(palettes)) == 0);

    state.palette_tables[4] = NULL;
    assert(!redmcsb_f0695_set_creature_replacement_colors_pc34_compat(
        &state, 9, 0));
    assert(cache_index == 2);
    assert(memcmp(palettes, original, sizeof(palettes)) == 0);
}

int main(void)
{
    test_applies_one_set_to_all_six_palettes_and_invalidates_cache();
    test_rejection_is_atomic();
    assert(strstr(redmcsb_f0695_set_creature_replacement_colors_source_evidence_pc34(),
                  "VIDEODRV.C:3543-3564") != NULL);
    return 0;
}
