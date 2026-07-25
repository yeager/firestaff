#include "redmcsb_f0540_input_crawcin_pc34_compat.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

typedef struct KeyboardFeed {
    const uint16_t *keys;
    size_t key_count;
    size_t read_count;
} KeyboardFeed;

static uint16_t read_next_key(void *context)
{
    KeyboardFeed *feed = context;

    assert(feed->read_count < feed->key_count);
    return feed->keys[feed->read_count++];
}

static void shifted_extended_arrows_map_to_pc34_command_keys(void)
{
    const uint16_t keys[] = { 0x1248U, 0x1250U, 0x124BU, 0x124DU };
    KeyboardFeed feed = { keys, sizeof(keys) / sizeof(keys[0]), 0U };
    ReDMCSBF0540InputCrawcinPc34Compat input = {
        read_next_key, &feed, false
    };
    (void)input;

    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == (uint16_t)'L');
    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == (uint16_t)'P');
    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == (uint16_t)'K');
    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == (uint16_t)'M');
    assert(feed.read_count == 4U);
    assert(!input.exit_game_immediately);
}

static void exit_shortcuts_set_the_persistent_exit_flag_without_rewriting_key(void)
{
    const uint16_t keys[] = { 0x0C53U, 0x0410U, 0x001CU };
    KeyboardFeed feed = { keys, sizeof(keys) / sizeof(keys[0]), 0U };
    ReDMCSBF0540InputCrawcinPc34Compat input = {
        read_next_key, &feed, false
    };
    (void)input;

    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == 0x0C53U);
    assert(input.exit_game_immediately);
    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == 0x0410U);
    assert(input.exit_game_immediately);
    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == 0x001CU);
    assert(feed.read_count == 3U);
}

static void absent_input_driver_returns_zero_without_side_effect(void)
{
    ReDMCSBF0540InputCrawcinPc34Compat input = { NULL, NULL, false };
    (void)input;

    assert(redmcsb_f0540_input_crawcin_pc34_compat(NULL) == 0U);
    assert(redmcsb_f0540_input_crawcin_pc34_compat(&input) == 0U);
    assert(!input.exit_game_immediately);
}

int main(void)
{
    shifted_extended_arrows_map_to_pc34_command_keys();
    exit_shortcuts_set_the_persistent_exit_flag_without_rewriting_key();
    absent_input_driver_returns_zero_without_side_effect();
    assert(strstr(redmcsb_f0540_input_crawcin_source_evidence_pc34(),
                  "F0540_INPUT_Crawcin") != NULL);
    return 0;
}
