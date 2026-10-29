#include "csb_v1_atari_f0128_cell_schedule.h"

/* ReDMCSB DUNVIEW.C F0128 calls F0115 for D4L/D4R/D4C before the twelve
 * square functions. DUNVIEW.C:8468-8477 uses order 0x0001 for these
 * object-only passes; F0115 exits before creature drawing for D4. The
 * F0116/F0117/F0118/F0119/F0120/F0121/F0124 scripts also conditionally call
 * F0115 with order 0x0000 for alcove objects (DUNVIEW.C:6434,6570,6717,
 * 6970,7121,7309,7843). F0116-F0127 traversal is at :8490-8542. */
static const CSB_V1_AtariF0128CellScheduleEntry k_schedule[] = {
    { 115, 4, -1, -2, 0, 1, { 0x0001, 0x0000 }, 0x0000, 0x0000 },
    { 115, 4,  1, -1, 0, 1, { 0x0001, 0x0000 }, 0x0000, 0x0000 },
    { 115, 4,  0, -3, 0, 1, { 0x0001, 0x0000 }, 0x0000, 0x0000 },
    { 116, 3, -1, 1, 1, 2, { 0x3421, 0x0321 }, 0x0218, 0x0349 },
    { 117, 3,  1, 2, 1, 2, { 0x4312, 0x0412 }, 0x0128, 0x0439 },
    { 118, 3,  0, 0, 1, 1, { 0x3421, 0x0000 }, 0x0218, 0x0349 },
    { 119, 2, -1, 4, 1, 2, { 0x3421, 0x0342 }, 0x0218, 0x0349 },
    { 120, 2,  1, 5, 1, 2, { 0x4312, 0x0431 }, 0x0128, 0x0439 },
    { 121, 2,  0, 3, 1, 1, { 0x3421, 0x0000 }, 0x0218, 0x0349 },
    { 122, 1, -1, 7, 0, 1, { 0x0032, 0x0000 }, 0x0028, 0x0039 },
    { 123, 1,  1, 8, 0, 1, { 0x0041, 0x0000 }, 0x0018, 0x0049 },
    { 124, 1,  0, 6, 1, 1, { 0x3421, 0x0000 }, 0x0218, 0x0349 },
    { 125, 0, -1, 10, 0, 1, { 0x0002, 0x0000 }, 0x0000, 0x0000 },
    { 126, 0,  1, 11, 0, 1, { 0x0001, 0x0000 }, 0x0000, 0x0000 },
    { 127, 0,  0, 9, 0, 1, { 0x0021, 0x0000 }, 0x0000, 0x0000 }
};

/* DUNVIEW.C F0128's conditional D3L2 and D3R2 wall drawing is static
 * geometry; neither edge invokes F0115. */
static const CSB_V1_AtariF0128StaticEdge k_static_edges[] = {
    { 3, -2 },
    { 3,  2 }
};

const CSB_V1_AtariF0128CellScheduleEntry *
csb_v1_atari_f0128_cell_schedule(size_t *count)
{
    if (count) *count = sizeof(k_schedule) / sizeof(k_schedule[0]);
    return k_schedule;
}

const CSB_V1_AtariF0128StaticEdge *
csb_v1_atari_f0128_static_edges(size_t *count)
{
    if (count) *count = sizeof(k_static_edges) / sizeof(k_static_edges[0]);
    return k_static_edges;
}
