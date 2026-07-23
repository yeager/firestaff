#include "csb_v1_f0217_group_lookup_pc34_compat.h"

int csb_v1_f0217_find_group_thing_pc34_compat(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int map_x,
    int map_y,
    uint16_t *out_thing)
{
    int thing;
    int guard;

    if (out_thing) *out_thing = 0xffffu;
    if (!dungeon || !out_thing) return 0;
    thing = csb_v1_dungeon_get_first_thing(
        dungeon, map_index, map_x, map_y);
    for (guard = 0;
         guard < 128 && thing != 0xfffe && thing != 0xffff;
         ++guard) {
        const uint8_t *record;
        int thing_type = -1;
        int thing_size = 0;

        record = csb_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)thing, &thing_type, NULL, &thing_size);
        if (!record || thing_size < 2) return 0;
        if (thing_type == CSB_V1_THING_TYPE_GROUP) {
            *out_thing = (uint16_t)thing;
            return 1;
        }
        thing = (int)((uint16_t)record[0] |
                      ((uint16_t)record[1] << 8));
    }
    return 0;
}
