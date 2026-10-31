#include "redmcsb_f1131_invert_box_amiga_compat.h"

bool redmcsb_f1131_invert_box_amiga_compat(
    const RedmcsbF1131AmigaRasterCompat *raster,
    const RedmcsbF1131AmigaBoxCompat *box)
{
    int32_t right;
    int32_t bottom;

    if (!box) return true;
    if (!raster || !raster->set_draw_mode || !raster->set_pen ||
        !raster->set_mask || !raster->wait_for_scan_line ||
        !raster->rect_fill || !raster->wait_blit) {
        return false;
    }

    right = (int32_t)box->x + (int32_t)box->width - 1;
    bottom = (int32_t)box->y + (int32_t)box->height - 1;
    raster->set_draw_mode(raster->context, 2); /* Amiga COMPLEMENT */
    raster->set_pen(raster->context, -1);
    raster->set_mask(raster->context, UINT8_C(0x0f));
    raster->wait_for_scan_line(raster->context, bottom);
    raster->rect_fill(raster->context, box->x, box->y, right, bottom);
    raster->wait_blit(raster->context);
    return true;
}

const char *redmcsb_f1131_invert_box_amiga_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/FILLBOX.C:736-751, "
           "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M; DEFS.H:175-177 "
           "defines the Amiga box as x,y,width,height. The source sets "
           "COMPLEMENT, pen -1 and mask 0x0f, waits for bottom scan line, "
           "fills through x+width-1/y+height-1, then calls WaitBlit.";
}
