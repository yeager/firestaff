/* theron_v1_track02_thing_data.c — TQ ground references and item data loader.
 *
 * Loads the "thing list" data from Track 02 quest blocks:
 * ground references (per-tile item chains), doors, teleporters, texts,
 * actuators, weapons, clothing, scrolls, potions, containers, misc, missiles,
 * creatures, and champions.
 *
 * Format from gbsphenx/dmbuilder (item.c itemBytes[], loaddungeon.c).
 * Item data is split across two regions per quest block:
 *   offset[3] = items part 1 (categories 0..N-1)
 *   offset[4] = items part 2 (categories N..10), always at xB800 alignment
 * Split point N varies per dungeon (iItemDataPart2StartIndex).
 */

#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_dungeon_map.h"
#include <string.h>

#define PREGAP_SECTORS 225
#define BYTES_PER_SECTOR 2048
#define UD_BASE (PREGAP_SECTORS * BYTES_PER_SECTOR)

static int theron_range_fits(size_t offset, size_t length, size_t size) {
    return offset <= size && length <= size - offset;
}

static unsigned int get_items_split_index(unsigned int dungeon_index) {
    switch (dungeon_index) {
        case 0: return 4;  /* AKUTUBA */
        case 4: return 1;  /* SHADODAN */
        case 5: return 4;  /* THIEVES */
        default: return 3;
    }
}

unsigned int theron_v1_track02_compute_ground_ref_count(
    const uint8_t *tiles_flat,
    unsigned int total_tiles)
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < total_tiles; i++) {
        if ((tiles_flat[i] >> 4) & 1)
            count++;
    }
    return count;
}

int theron_v1_track02_thing_data_load(
    const uint8_t *ud_data,
    size_t ud_size,
    unsigned int dungeon_index,
    const uint16_t *object_counts,
    unsigned int ground_ref_count,
    Theron_ThingData *out)
{
    if (!ud_data || !out || !object_counts || dungeon_index >= 7 ||
        ground_ref_count > THERON_MAX_GROUND_REFS)
        return 0;

    memset(out, 0, sizeof(*out));
    memcpy(out->object_counts, object_counts,
           sizeof(uint16_t) * THERON_ITEM_CATEGORY_COUNT);
    out->ground_ref_count = (uint16_t)ground_ref_count;

    Theron_QuestBlockOffsets qb;
    if (!theron_v1_track02_dungeon_map_quest_block_offsets(dungeon_index, &qb))
        return 0;

    size_t gref_abs = UD_BASE + qb.ground_refs_offset;
    size_t gref_bytes = (size_t)ground_ref_count * 2;
    if (!theron_range_fits(gref_abs, gref_bytes, ud_size)) return 0;
    memcpy(out->ground_refs, ud_data + gref_abs, gref_bytes);

    unsigned int split = get_items_split_index(dungeon_index);

    size_t pos = UD_BASE + qb.items_part1_offset;
    for (unsigned int cat = 0; cat < split; cat++) {
        size_t item_size = theron_item_bytes[cat];
        size_t n = object_counts[cat];
        size_t total = item_size * n;
        if (item_size == 0 || n == 0) continue;
        if (!theron_range_fits(pos, total, ud_size)) return 0;
        if (total > sizeof(out->items[cat])) return 0;
        memcpy(out->items[cat], ud_data + pos, total);
        pos += total;
    }

    pos = UD_BASE + qb.items_part2_offset;
    for (unsigned int cat = split; cat < 11; cat++) {
        size_t item_size = theron_item_bytes[cat];
        size_t n = object_counts[cat];
        size_t total = item_size * n;
        if (item_size == 0 || n == 0) continue;
        if (!theron_range_fits(pos, total, ud_size)) return 0;
        if (total > sizeof(out->items[cat])) return 0;
        memcpy(out->items[cat], ud_data + pos, total);
        pos += total;
    }

    uint16_t text_size = theron_v1_track02_dungeon_text_data_size(dungeon_index);
    if (text_size > 0) {
        size_t text_abs = UD_BASE + qb.text_data_offset;
        size_t text_bytes = (size_t)text_size * 2;
        if (!theron_range_fits(text_abs, text_bytes, ud_size)) return 0;
        if (text_size > 1024) return 0;
        out->text_data_count = text_size;
        memcpy(out->text_data, ud_data + text_abs, text_bytes);
    }

    return 1;
}
