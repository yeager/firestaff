#include "firestaff/csb/v1/viewport/d1c_f0115_thing_pass_pc34_compat.h"

enum {
    CSB_D1C_VIEW_SQUARE = 3,          /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C. */
    CSB_DOOR_MARKER_MASK = 0x0008,    /* ReDMCSB DUNVIEW.C F0115:4795-4800. */
    CSB_ORDER_DOORPASS1 = 0x0218,     /* ReDMCSB DEFS.H:2669 C0x0218. */
    CSB_ORDER_DOORPASS2 = 0x0349,     /* ReDMCSB DEFS.H:2672 C0x0349. */
    CSB_ORDER_DOORPASS1_STRIPPED = 0x0021,
    CSB_ORDER_DOORPASS2_STRIPPED = 0x0034,
    CSB_C10_COLOR_FLESH = 10,
    CSB_PRESENT = 1,
    CSB_ABSENT = 0
};

static const char s_source_evidence[] =
    "CSB V1 D1C F0115 thing-pass contract-only source-lock gate; no "
    "GRAPHICS.DAT, no DUNGEON.DAT, no real-asset pixels. ReDMCSB "
    "DUNVIEW.C F0124:7873-7911 makes the D1C door-front BACK pass call "
    "to F0115 before the door frame/door with M606_VIEW_SQUARE_D1C and "
    "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, then lines "
    "7910-7937 make the FRONT pass call through T0124018 with "
    "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT after the "
    "door/frame work. F0115 anchors: DUNVIEW.C:4547-4581 defines the "
    "objects, creatures, projectiles, explosions thing-pass order; "
    "DUNVIEW.C:4795-4800 decodes MASK 0x0008 door-marker nibbles into "
    "pass 1/pass 2 and strips the marker nibble; DUNVIEW.C:4923 guards "
    "item visibility by view depth/cell; DUNVIEW.C:5180-5188 blits object "
    "pixels with C10 transparency and resumes projectile-as-object; "
    "DUNVIEW.C:5211-5214 guards creature rows; DUNVIEW.C:5668-5671 guards "
    "projectile rows. DEFS.H:2669/2672 anchor C0x0218 and C0x0349 "
    "DOORPASS1/2 cell-order constants. This gate is disjoint from "
    "CustomBackgrounds first/backdrop and mask passes, D1C F0108 "
    "floor+ceiling+ornament, partly-open-door F0111 gates, DM1 D0C "
    "F0115, and D1L/D1R/D2L2/D2R2 thing-pass or door slices.";

static const CSB_V1_D1CF0115ThingPassContractPc34 s_contract = {
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_D1C_VIEW_SQUARE,
    2,
    CSB_ORDER_DOORPASS1,
    CSB_ORDER_DOORPASS2,
    CSB_DOOR_MARKER_MASK,
    0x0008,
    0x0009,
    "ReDMCSB DUNVIEW.C F0124:7873-7911,7910-7937 D1C door-front F0115 calls",
    "ReDMCSB DUNVIEW.C F0115:4547-4581,4795-4800,4923,5180-5188,5211-5214,5668-5671",
    "ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C; 2669/2672 C0x0218/C0x0349 DOORPASS1/2",
    "D1C door-front thing-pass source contract: BACK before door/frame, FRONT after door/frame"
};

static const CSB_V1_D1CF0115ThingPassPc34 s_passes[] = {
    {
        CSB_V1_D1C_F0115_PASS_BACK_PC34,
        "D1C BACK thing pass",
        CSB_D1C_VIEW_SQUARE,
        CSB_ORDER_DOORPASS1,
        CSB_ORDER_DOORPASS1_STRIPPED,
        0x0008,
        1,
        2,
        { CSB_V1_D1C_F0115_CELL_BACK_LEFT_PC34,
          CSB_V1_D1C_F0115_CELL_BACK_RIGHT_PC34, 0, 0 },
        2,
        0,
        CSB_PRESENT,
        CSB_ABSENT,
        1,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        "ReDMCSB DUNVIEW.C F0124:7874-7875 first F0115 call before door frame",
        "ReDMCSB DEFS.H:2669 C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
        "BACK cells are processed by F0115 pass 1 before F0111 draws the D1C door"
    },
    {
        CSB_V1_D1C_F0115_PASS_FRONT_PC34,
        "D1C FRONT thing pass",
        CSB_D1C_VIEW_SQUARE,
        CSB_ORDER_DOORPASS2,
        CSB_ORDER_DOORPASS2_STRIPPED,
        0x0009,
        2,
        2,
        { CSB_V1_D1C_F0115_CELL_FRONT_LEFT_PC34,
          CSB_V1_D1C_F0115_CELL_FRONT_RIGHT_PC34, 0, 0 },
        0,
        2,
        CSB_ABSENT,
        CSB_PRESENT,
        2,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        "ReDMCSB DUNVIEW.C F0124:7910-7937 second F0115 call after door frame",
        "ReDMCSB DEFS.H:2672 C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
        "FRONT cells are processed by F0115 pass 2 after F0111 draws the D1C door"
    }
};

const CSB_V1_D1CF0115ThingPassContractPc34 *
csb_v1_viewport_d1c_f0115_thing_pass_contract_pc34(void)
{
    return &s_contract;
}

size_t csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(void)
{
    return sizeof(s_passes) / sizeof(s_passes[0]);
}

const CSB_V1_D1CF0115ThingPassPc34 *
csb_v1_viewport_d1c_f0115_thing_pass_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1c_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_passes[index];
}

const CSB_V1_D1CF0115ThingPassPc34 *
csb_v1_viewport_d1c_f0115_thing_pass_for_pass_pc34(int door_pass)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d1c_f0115_thing_pass_count_pc34(); ++i) {
        if (s_passes[i].door_pass == door_pass) return &s_passes[i];
    }
    return 0;
}

int csb_v1_viewport_d1c_f0115_decode_order_pc34(
    uint16_t ordered_view_cell_ordinals,
    CSB_V1_D1CF0115DecodedOrderPc34 *out_decoded)
{
    CSB_V1_D1CF0115DecodedOrderPc34 decoded = { 0 };
    uint16_t remaining;
    unsigned int nibble;

    if (!out_decoded) return 0;

    decoded.input_order = ordered_view_cell_ordinals;

    /* ReDMCSB: DUNVIEW.C F0115 lines 4795-4800 test MASK 0x0008 in the
     * first nibble, compute pass as (word & 1) + 1, then shift the marker
     * nibble away before processing the remaining cell ordinals. */
    if ((ordered_view_cell_ordinals & CSB_DOOR_MARKER_MASK) != 0u) {
        decoded.has_door_marker = CSB_PRESENT;
        decoded.door_pass = (ordered_view_cell_ordinals & 0x0001u) + 1;
        remaining = (uint16_t)(ordered_view_cell_ordinals >> 4);
    } else {
        decoded.has_door_marker = CSB_ABSENT;
        decoded.door_pass = 0;
        remaining = ordered_view_cell_ordinals;
    }
    decoded.stripped_order = remaining;

    while (decoded.cell_count < 4) {
        nibble = remaining & 0x000fu;
        if (nibble == 0u) {
            decoded.stops_at_zero_nibble = CSB_PRESENT;
            break;
        }
        if (nibble > CSB_V1_D1C_F0115_CELL_FRONT_LEFT_PC34) {
            decoded.invalid_cell_seen = CSB_PRESENT;
            break;
        }
        decoded.cells[decoded.cell_count] = (int)nibble;
        if (nibble == CSB_V1_D1C_F0115_CELL_BACK_LEFT_PC34 ||
            nibble == CSB_V1_D1C_F0115_CELL_BACK_RIGHT_PC34) {
            ++decoded.back_cell_count;
        } else {
            ++decoded.front_cell_count;
        }
        ++decoded.cell_count;
        remaining = (uint16_t)(remaining >> 4);
    }

    *out_decoded = decoded;
    return 1;
}

const char *csb_v1_viewport_d1c_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
