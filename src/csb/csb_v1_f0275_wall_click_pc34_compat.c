#include "csb_v1_f0275_wall_click_pc34_compat.h"

#include <string.h>

static uint16_t read_u16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }

static int object_type(const CSB_V1_DungeonData *d, uint16_t thing)
{
    const uint8_t *r;
    int t, size;
    r = csb_v1_dungeon_get_thing_record(d, thing, &t, NULL, &size);
    if (!r || t <= CSB_V1_THING_TYPE_GROUP || t >= 14 || size < 4) return -1;
    return (int)(read_u16(r + 2) & 0x7fu);
}

int csb_v1_f0275_wall_click_receipt_pc34(
    const CSB_V1_RuntimeProfile *p, int x, int y, int cell,
    CSB_V1_F0275WallClickReceiptPc34 *out)
{
    CSB_V1_F0275WallClickReceiptPc34 r;
    const CSB_V1_DungeonData *d;
    int thing, guard, count[4] = {0, 0, 0, 0};
    int leader, hand_type = -1;

    if (!out) return 0;
    memset(&r, 0, sizeof(r)); r.sensor_thing = 0xffffu; *out = r;
    if (!p || !p->dungeon_handle || !p->dungeon_handle->raw_data ||
        p->dungeon_handle->square_bytes != 1 || p->current_level < 0 ||
        cell < 0 || cell > 3) return 0;
    d = p->dungeon_handle;
    if (csb_v1_dungeon_get_raw_square(d, p->current_level, x, y) < 0 ||
        ((csb_v1_dungeon_get_raw_square(d, p->current_level, x, y) >> 5) & 7) != 0) return 0;
    leader = p->leader_index >= 0;
    if (p->party_state_valid && p->party_state.LeaderHandThing != 0xffffu &&
        p->party_state.LeaderHandThing != 0xfffeu) {
        hand_type = object_type(d, p->party_state.LeaderHandThing);
        if (hand_type < 0) return 0;
    }
    thing = csb_v1_dungeon_get_first_thing(d, p->current_level, x, y);
    if (thing < 0) return 0;
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff; ++guard) {
        const uint8_t *q; int type, size;
        q = csb_v1_dungeon_get_thing_record(d, (uint16_t)thing, &type, NULL, &size);
        if (!q || size < 2) return 0;
        if (type == CSB_V1_THING_TYPE_ACTUATOR) ++count[((uint16_t)thing >> 14) & 3];
        else if (type >= CSB_V1_THING_TYPE_GROUP) break;
        thing = (int)read_u16(q);
    }
    if (guard >= 128) return 0;
    thing = csb_v1_dungeon_get_first_thing(d, p->current_level, x, y);
    for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff; ++guard) {
        const uint8_t *q; int type, size, c, st, sd, accepted = 0; uint16_t flags, td;
        q = csb_v1_dungeon_get_thing_record(d, (uint16_t)thing, &type, NULL, &size);
        if (!q || size < 2) return 0;
        if (type >= CSB_V1_THING_TYPE_GROUP) break;
        if (type != CSB_V1_THING_TYPE_ACTUATOR || size < 8) { thing = (int)read_u16(q); continue; }
        c = ((uint16_t)thing >> 14) & 3; --count[c]; td = read_u16(q + 2); flags = read_u16(q + 4);
        st = td & 0x7f; sd = td >> 7;
        if (st != 0 && c == cell && (leader || st == 127)) {
            switch (st) {
            case 1: accepted = ((flags >> 3) & 3) != 3; break;
            case 2: accepted = ((hand_type < 0) != ((flags >> 5) & 1)); break;
            case 3: case 4: accepted = ((sd == hand_type) != ((flags >> 5) & 1)); break;
            case 11: case 17: accepted = count[c] == 0 && ((sd == hand_type) != ((flags >> 5) & 1)); break;
            case 12: accepted = count[c] == 0 && hand_type < 0; break;
            case 127: accepted = 1; break;
            default: break;
            }
        }
        if (accepted) {
            r.valid=1; r.map_index=p->current_level; r.map_x=x; r.map_y=y; r.cell=cell;
            r.sensor_thing=(uint16_t)thing; r.sensor_type=st; r.sensor_data=sd;
            r.effect=(flags >> 3)&3; r.click_accepted=1;
            r.requires_mutation = st == 4 || st == 11 || st == 12 || st == 17 || st == 127;
            r.source_evidence="ReDMCSB SENSOR.C F0275 raw C03 admission; CSBWin Mouse.cpp TouchWall";
            *out=r; return 1;
        }
        thing = (int)read_u16(q);
    }
    if (guard >= 128) return 0;
    r.valid=1; r.map_index=p->current_level; r.map_x=x; r.map_y=y; r.cell=cell;
    r.source_evidence="ReDMCSB SENSOR.C F0275 raw C03 admission; no source candidate"; *out=r; return 1;
}
