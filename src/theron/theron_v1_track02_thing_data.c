/* theron_v1_track02_thing_data.c — TQ ground references and item data loader.
 *
 * Loads the "thing list" data from Track 02 quest blocks:
 * ground references (per-tile item chains), doors, teleporters, texts,
 * actuators, monsters, weapons, clothing, scrolls, potions, chests, misc,
 * missiles, and clouds.
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

static uint16_t read_le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int theron_v1_track02_item_record_decode(
    unsigned int category,
    const uint8_t *raw,
    size_t raw_size,
    Theron_Track02ItemRecord *out)
{
    if (!raw || !out ||
        (category < THERON_CAT_MONSTER ||
         (category > THERON_CAT_MISC && category != THERON_CAT_MISSILE &&
          category != THERON_CAT_CLOUD)) ||
        raw_size < theron_item_bytes[category])
        return 0;

    memset(out, 0, sizeof(*out));
    out->category = category;
    out->next_ref = read_le16(raw);

    switch (category) {
    case THERON_CAT_MONSTER: {
        uint16_t flags;
        uint16_t unknown;
        out->value.monster.next_ref = out->next_ref;
        out->value.monster.type = raw[2];
        out->value.monster.position = raw[3];
        for (unsigned int i = 0; i < 4u; ++i)
            out->value.monster.health[i] = read_le16(raw + 4u + i * 2u);
        flags = read_le16(raw + 12u);
        unknown = read_le16(raw + 14u);
        out->value.monster.number = (uint8_t)((flags >> 5) & 0x03u);
        out->value.monster.direction_flags = (uint8_t)(unknown >> 8);
        break;
    }
    case THERON_CAT_WEAPON: {
        uint16_t w = read_le16(raw + 2u);
        out->value.weapon.type = (uint8_t)(w & 0x7Fu);
        out->value.weapon.keep = (uint8_t)((w >> 7) & 1u);
        out->value.weapon.cursed = (uint8_t)((w >> 8) & 1u);
        out->value.weapon.poisoned = (uint8_t)((w >> 9) & 1u);
        out->value.weapon.charges = (uint8_t)((w >> 10) & 0x0Fu);
        out->value.weapon.broken = (uint8_t)((w >> 14) & 1u);
        out->value.weapon.unknown = (uint8_t)((w >> 15) & 1u);
        break;
    }
    case THERON_CAT_CLOTHING: {
        uint16_t w = read_le16(raw + 2u);
        out->value.clothing.type = (uint8_t)(w & 0x7Fu);
        out->value.clothing.keep = (uint8_t)((w >> 7) & 1u);
        out->value.clothing.cursed = (uint8_t)((w >> 8) & 1u);
        out->value.clothing.dump = (uint8_t)((w >> 9) & 0x1Fu);
        out->value.clothing.broken = (uint8_t)((w >> 14) & 1u);
        out->value.clothing.unknown = (uint8_t)((w >> 15) & 1u);
        break;
    }
    case THERON_CAT_SCROLL: {
        uint16_t w = read_le16(raw + 2u);
        out->value.scroll.reftxt = (uint16_t)(w & 0x03FFu);
        out->value.scroll.closed = (uint8_t)((w >> 10) & 1u);
        out->value.scroll.type = (uint8_t)((w >> 11) & 0x1Fu);
        break;
    }
    case THERON_CAT_POTION: {
        uint16_t w = read_le16(raw + 2u);
        out->value.potion.power = (uint8_t)(w & 0xFFu);
        out->value.potion.type = (uint8_t)((w >> 8) & 0x1Fu);
        out->value.potion.unknown = (uint8_t)((w >> 13) & 0x03u);
        out->value.potion.keep = (uint8_t)((w >> 15) & 1u);
        break;
    }
    case THERON_CAT_CHEST:
        out->value.chest.chested = (int16_t)read_le16(raw + 2u);
        out->value.chest.data1 = read_le16(raw + 4u);
        out->value.chest.unknown = read_le16(raw + 6u);
        break;
    case THERON_CAT_MISC: {
        uint16_t w = read_le16(raw + 2u);
        out->value.misc.type = (uint8_t)(w & 0x7Fu);
        out->value.misc.keep = (uint8_t)((w >> 7) & 1u);
        out->value.misc.unknown = (uint8_t)((w >> 8) & 0x3Fu);
        out->value.misc.capacity = (uint8_t)((w >> 14) & 0x03u);
        break;
    }
    case THERON_CAT_MISSILE:
        out->value.missile.unknown1 = raw[2];
        out->value.missile.spell = raw[3];
        out->value.missile.power = raw[4];
        out->value.missile.unknown2 = raw[5];
        out->value.missile.zero = raw[6];
        out->value.missile.e = raw[7];
        break;
    case THERON_CAT_CLOUD:
        out->value.cloud.power = raw[2];
        out->value.cloud.spell = raw[3];
        break;
    default:
        return 0;
    }
    return 1;
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
    return theron_v1_track02_thing_data_load_for_variant(
        ud_data, ud_size, THERON_TRACK02_VARIANT_US_BIN, dungeon_index,
        object_counts, ground_ref_count, out);
}

int theron_v1_track02_thing_data_load_for_variant(
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_Track02Variant variant,
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
    if (!theron_v1_track02_dungeon_map_quest_block_offsets_for_variant(
            variant, dungeon_index, &qb))
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
