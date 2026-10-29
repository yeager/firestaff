#include "csb_v1_atari_f0128_cell_schedule.h"

#include <stdio.h>

int main(void)
{
    static const int8_t forward[] = { 4,4,4,3,3,3,2,2,2,1,1,1,0,0,0 };
    static const int8_t right[] = { -1,1,0,-1,1,0,-1,1,0,-1,1,0,-1,1,0 };
    static const int8_t view[] = { -2,-1,-3,1,2,0,4,5,3,7,8,6,10,11,9 };
    static const uint8_t functions[] = { 115,115,115,116,117,118,119,120,121,122,123,124,125,126,127 };
    static const uint8_t alcove[] = { 0,0,0,1,1,1,1,1,1,0,0,1,0,0,0 };
    static const uint16_t ordinary[][2] = {
        { 0x3421,0x0321 }, { 0x4312,0x0412 }, { 0x3421,0 },
        { 0x3421,0x0342 }, { 0x4312,0x0431 }, { 0x3421,0 },
        { 0x0032,0 }, { 0x0041,0 }, { 0x3421,0 },
        { 0x0002,0 }, { 0x0001,0 }, { 0x0021,0 }
    };
    static const uint16_t door1[] = {
        0x0218,0x0128,0x0218,0x0218,0x0128,0x0218,
        0x0028,0x0018,0x0218,0,0,0
    };
    static const uint16_t door2[] = {
        0x0349,0x0439,0x0349,0x0349,0x0439,0x0349,
        0x0039,0x0049,0x0349,0,0,0
    };
    size_t count = 0;
    const CSB_V1_AtariF0128CellScheduleEntry *entries =
        csb_v1_atari_f0128_cell_schedule(&count);
    size_t edge_count = 0;
    const CSB_V1_AtariF0128StaticEdge *edges =
        csb_v1_atari_f0128_static_edges(&edge_count);

    if (!entries || count != 15 || !edges || edge_count != 2) return 1;
    for (size_t i = 0; i < count; ++i) {
        if (entries[i].source_function != functions[i] ||
            entries[i].forward != forward[i] || entries[i].right != right[i] ||
            entries[i].view_square != view[i] ||
            entries[i].has_conditional_alcove_pass != alcove[i] ||
            (i >= 3 && entries[i].normal_order_count !=
                (ordinary[i - 3][1] ? 2 : 1)) ||
            (i >= 3 && (entries[i].normal_orders[0] != ordinary[i - 3][0] ||
                entries[i].normal_orders[1] != ordinary[i - 3][1] ||
                entries[i].door_pass1_order != door1[i - 3] ||
                entries[i].door_pass2_order != door2[i - 3])) ||
            (i < 3 && (entries[i].normal_order_count != 1 ||
                entries[i].normal_orders[0] != 0x0001 ||
                entries[i].door_pass1_order != 0 ||
                entries[i].door_pass2_order != 0))) {
            fprintf(stderr, "F0128 schedule entry %zu mismatch\n", i);
            return 2;
        }
    }
    if (edges[0].forward != 3 || edges[0].right != -2 ||
        edges[1].forward != 3 || edges[1].right != 2) {
        fprintf(stderr, "F0128 static-only edge cells mismatch\n");
        return 3;
    }
    puts("ReDMCSB F0128 Atari per-square schedule verified");
    return 0;
}
