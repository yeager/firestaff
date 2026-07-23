#include "dm1_v1_viewport_planar_fill_material_pc34_compat.h"

#include "dm1_f0135_video_fillbox_planar_20260714_pc34_compat.h"
#include "video_f0134_fill_bitmap_bounded_pc34_compat.h"

static int dm1_v1_viewport_planar_material_is_admitted_pc34(
    const DM1_V1_ViewportPlanarFillMaterialPc34 *material)
{
    size_t required_size;

    if (!material || !material->original_material_verified ||
        !material->bitmap || material->row_bytes == 0 ||
        (material->row_bytes % 8u) != 0 || material->pixel_height == 0 ||
        material->pixel_height > SIZE_MAX / material->row_bytes) {
        return 0;
    }
    required_size = material->row_bytes * material->pixel_height;
    return material->bitmap_size >= required_size;
}

int dm1_v1_viewport_fill_material_f0134_pc34(
    DM1_V1_ViewportPlanarFillMaterialPc34 *material,
    uint8_t color)
{
    size_t unit_count;

    if (!dm1_v1_viewport_planar_material_is_admitted_pc34(material)) return 0;
    unit_count = (material->row_bytes * material->pixel_height) / 8u;
    return video_f0134_fill_bitmap_bounded_pc34_compat(
        material->bitmap, material->bitmap_size, color, unit_count);
}

int dm1_v1_viewport_fill_material_box_f0135_pc34(
    DM1_V1_ViewportPlanarFillMaterialPc34 *material,
    const int16_t box[4],
    uint16_t color)
{
    if (!dm1_v1_viewport_planar_material_is_admitted_pc34(material)) return 0;
    return dm1_f0135_video_fillbox_planar_20260714_pc34_compat(
        material->bitmap, material->bitmap_size, material->row_bytes,
        material->pixel_height, box, color);
}

const char *dm1_v1_viewport_planar_fill_material_source_evidence_pc34(void)
{
    return "ReDMCSB ACTIDRAW.C F0134_VIDEO_FillBitmap and FILLBOX.C "
           "F0135_VIDEO_FillBox operate only on their caller-owned planar "
           "viewport bitmap. Firestaff requires a verified original material "
           "owner and rejects absent material without a fallback fill.";
}
