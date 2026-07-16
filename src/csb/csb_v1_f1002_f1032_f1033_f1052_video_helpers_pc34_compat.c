#include "redmcsb_f1002_call_f0132_video_blit.h"
#include "redmcsb_f1032_hatch_box_pc34_compat.h"
#include "redmcsb_f1033_hatch_box.h"
#include "redmcsb_f1052_wait_for_scan_line_pc34_compat.h"

void redmcsb_f1002_call_f0132_video_blit(
    uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t *xyz,
    int16_t x,
    int16_t y,
    int16_t transparent_color,
    int16_t flip,
    redmcsb_f1002_video_blit video_blit)
{
    int16_t source_pixel_width;
    int16_t destination_pixel_width;

    if (bitmap_source == 0 || bitmap_destination == 0 || video_blit == 0) {
        return;
    }

    source_pixel_width = ((int16_t *)bitmap_source)[-2];
    destination_pixel_width = ((int16_t *)bitmap_destination)[-2];
    video_blit(
        bitmap_source,
        bitmap_destination,
        xyz,
        x,
        y,
        source_pixel_width,
        destination_pixel_width,
        transparent_color,
        flip);
}

void F1002_Call_F0132_VIDEO_Blit(
    uint8_t *bitmap_source,
    uint8_t *bitmap_destination,
    int16_t *xyz,
    int16_t x,
    int16_t y,
    int16_t transparent_color,
    int16_t flip,
    redmcsb_f1002_video_blit video_blit)
{
    redmcsb_f1002_call_f0132_video_blit(
        bitmap_source,
        bitmap_destination,
        xyz,
        x,
        y,
        transparent_color,
        flip,
        video_blit);
}

const char *redmcsb_f1002_call_f0132_video_blit_source_evidence(void)
{
    return "ReDMCSB BASE.C:1202-1212 F1002_Call_F0132_VIDEO_Blit";
}

bool redmcsb_f1032_hatch_box_pc34_compat(void)
{
    return false;
}

bool F1032_GRF1_12_HatchBox(void)
{
    return redmcsb_f1032_hatch_box_pc34_compat();
}

const char *redmcsb_f1032_hatch_box_source_evidence_pc34(void)
{
    return "ReDMCSB BLITFILL.C:287 F1032_GRF1_12_HatchBox; "
           "PC34 has no source-defined hatch hardware backend";
}

void redmcsb_f1033_hatch_box(
    redmcsb_f1033_hatch_box_primitive_fn hatch_box_primitive,
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width)
{
    if (hatch_box_primitive != 0) {
        hatch_box_primitive(screen_bitmap, xyz, color, screen_pixel_width);
    }
}

void F1033_HatchBox_Unreferenced(
    redmcsb_f1033_hatch_box_primitive_fn hatch_box_primitive,
    uint8_t *screen_bitmap,
    int16_t *xyz,
    int16_t color,
    int16_t screen_pixel_width)
{
    redmcsb_f1033_hatch_box(
        hatch_box_primitive,
        screen_bitmap,
        xyz,
        color,
        screen_pixel_width);
}

const char *redmcsb_f1033_hatch_box_source_evidence(void)
{
    return "ReDMCSB BLITFILL.C:428 F1033_HatchBox_Unreferenced";
}

void redmcsb_f1052_wait_for_scan_line_pc34_compat(int16_t scan_line)
{
    (void)scan_line;
}

void F1052_WaitForScanLine(int16_t scan_line)
{
    redmcsb_f1052_wait_for_scan_line_pc34_compat(scan_line);
}

const char *redmcsb_f1052_wait_for_scan_line_source_evidence_pc34(void)
{
    return "ReDMCSB FILLBOX.C:17 F1052_WaitForScanLine; "
           "PC34 compatibility boundary performs no Amiga hardware wait";
}
