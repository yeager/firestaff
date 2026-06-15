#include "dm1_v1_viewport_d1c_f0115_door_frame_pc34_compat.h"

enum {
    DM1_PRESENT = 1,
    DM1_ABSENT = 0,
    DM1_VIEW_SQUARE_D1C = 3,        /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C */
    DM1_VIEW_DEPTH_D1 = 1,          /* ReDMCSB DUNVIEW.C:372 G2027[3] */
    DM1_VIEW_LANE_CENTER = 0,       /* ReDMCSB DUNVIEW.C:371 G2026[3] */
    DM1_ELEMENT_DOOR_FRONT = 17,    /* ReDMCSB DUNVIEW.C:7873 */
    DM1_D1C_REAR_F0115_ORDER = 0x0218,  /* ReDMCSB DUNVIEW.C:7875; DEFS.H:2669 */
    DM1_D1C_FRONT_F0115_ORDER = 0x0349, /* ReDMCSB DUNVIEW.C:7910,7937; DEFS.H:2672 */
    DM1_D1C_FRAME_TOP_ZONE = 733,   /* ReDMCSB DUNVIEW.C:7886; DEFS.H:4092 */
    DM1_D1C_FRAME_LEFT_ZONE = 726,  /* ReDMCSB DUNVIEW.C:7887; DEFS.H:4084 */
    DM1_D1C_FRAME_RIGHT_ZONE = 727, /* ReDMCSB DUNVIEW.C:7893; DEFS.H:4085 */
    DM1_D1C_DOOR_ZONE = 3790,       /* ReDMCSB DUNVIEW.C:7908; DEFS.H:4259 */
    DM1_TRANSPARENT_COLOR = 10,     /* ReDMCSB DUNVIEW.C:3144-3148,3217-3219; DEFS.H:2088 */
    DM1_FLIP_HORIZONTAL_MASK = 1    /* ReDMCSB DUNVIEW.C:3217-3219 */
};

static const char s_source_evidence[] =
    "DM1 V1 source-locked contract gate only; contract_only=1 and no "
    "real-asset parity is claimed. ReDMCSB DUNVIEW.C:"
    "F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911 locks the D1C door-front "
    "route: F0108 floor ornament, F0115 rear object pass with C0x0218 at "
    "7875, PC34/I34 door-frame top/left draws through F0104 at 7886-7887, "
    "right frame draws by F0105 horizontal flip of G2117_DoorFrameLeftD1C "
    "at 7893, then the F0111 D1C door bitmap at 7908 and the front F0115 "
    "order C0x0349 at 7910/7937. ReDMCSB DUNVIEW.C:F0104:3113-3156 and "
    "F0105:3185-3238 lock C10 transparent frame blits and MASK0x0001 "
    "horizontal flip. DEFS.H:2669/2672 locks the rear/front F0115 orders; "
    "DEFS.H:4084-4093 locks C726/C727/C733 D1C frame zones; DEFS.H:4259 "
    "locks M631_ZONE_DOOR_D1C. DUNVIEW.C:F0128:8524-8533 dispatches D1L, "
    "D1R, then D1C, so this D1C door-frame/F0115 pairing does not cover "
    "the F0122 or F0123 side-door routes.";

static const DM1_V1_ViewportD1CF0115DoorFramePc34Contract s_contract = {
    DM1_PRESENT,
    DM1_VIEW_SQUARE_D1C,
    DM1_VIEW_DEPTH_D1,
    DM1_VIEW_LANE_CENTER,
    DM1_ELEMENT_DOOR_FRONT,
    DM1_D1C_REAR_F0115_ORDER,
    DM1_D1C_FRONT_F0115_ORDER,
    DM1_D1C_FRAME_TOP_ZONE,
    DM1_D1C_FRAME_LEFT_ZONE,
    DM1_D1C_FRAME_RIGHT_ZONE,
    DM1_D1C_DOOR_ZONE,
    DM1_TRANSPARENT_COLOR,
    DM1_FLIP_HORIZONTAL_MASK,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_PRESENT,
    DM1_ABSENT,
    DM1_ABSENT,
    "G2112_DoorFrameTopD1LCR",
    "G2117_DoorFrameLeftD1C",
    "G2117_DoorFrameLeftD1C",
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911",
    "ReDMCSB DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4582",
    "ReDMCSB DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156",
    "ReDMCSB DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3238",
    "ReDMCSB DEFS.H:C0x0218:2669; C0x0349:2672; "
    "C726/C727/C733:4084-4093; M631_ZONE_DOOR_D1C:4259",
    "ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8533",
    s_source_evidence
};

const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *
dm1_v1_viewport_d1c_f0115_door_frame_pc34_contract(void)
{
    return &s_contract;
}

const char *
dm1_v1_viewport_d1c_f0115_door_frame_pc34_source_evidence(void)
{
    return s_source_evidence;
}

int dm1_v1_viewport_d1c_f0115_door_frame_order_role_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int cell_order)
{
    if (!contract) return 0;
    if (cell_order == contract->f0115_rear_order) return 1;
    if (cell_order == contract->f0115_front_order) return 2;
    return 0;
}

int dm1_v1_viewport_d1c_f0115_door_frame_zone_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part)
{
    if (!contract) return -1;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_TOP) return contract->frame_top_zone;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_LEFT) return contract->frame_left_zone;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_RIGHT) return contract->frame_right_zone;
    return -1;
}

int dm1_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part)
{
    if (!contract) return -1;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_RIGHT) {
        return contract->flip_horizontal_mask;
    }
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_TOP ||
        part == DM1_V1_D1C_DOOR_FRAME_PART_LEFT) {
        return 0;
    }
    return -1;
}

const char *
dm1_v1_viewport_d1c_f0115_door_frame_bitmap_for_part_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part)
{
    if (!contract) return 0;
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_TOP) {
        return contract->frame_top_bitmap_symbol;
    }
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_LEFT) {
        return contract->frame_left_bitmap_symbol;
    }
    if (part == DM1_V1_D1C_DOOR_FRAME_PART_RIGHT) {
        return contract->frame_right_bitmap_symbol;
    }
    return 0;
}

int dm1_v1_viewport_d1c_f0115_door_frame_apply_blit_pc34(
    const DM1_V1_ViewportD1CF0115DoorFramePc34Contract *contract,
    int part,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;
    const int flip = dm1_v1_viewport_d1c_f0115_door_frame_flip_for_part_pc34(
        contract, part);

    if (!contract || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;
    if (flip < 0) return -1;

    /* ReDMCSB DUNVIEW.C:F0104:3141-3148 and F0105:3214-3219 route the
     * DM1 PC34/I34 D1C frame bitmaps through C10 transparent blits; F0105
     * adds MASK0x0001_FLIP_HORIZONTAL for the right frame. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int sx = flip ? (width - 1 - x) : x;
            const uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)contract->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}
