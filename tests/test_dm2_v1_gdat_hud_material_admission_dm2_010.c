#include "dm2_v1_gdat_hud_material_admission_dm2_010.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static DM2_V1_GdatHudAdmissionSource source_for(int category, int type,
                                                  const uint8_t *bytes,
                                                  size_t byte_count)
{
    DM2_V1_GdatHudAdmissionSource source;
    source.category = category;
    source.index = 0;
    source.entry_type = type;
    source.field = 0;
    source.bytes = bytes;
    source.byte_count = byte_count;
    source.content_hash = 0x13579bdfu;
    return source;
}

static DM2_V1_GdatHudAdmissionRequest valid_request(void)
{
    static const uint8_t layout[] = { 1 };
    static const uint8_t palette[16] = { 0 };
    static const uint8_t font[] = { 1 };
    static const uint8_t portrait[] = { 1, 2, 3, 4, 5, 6 };
    DM2_V1_GdatHudAdmissionRequest request;

    memset(&request, 0, sizeof(request));
    request.layout = source_for(DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY,
                                DM2_V1_GDAT_HUD_ADMISSION_RAW7_TYPE,
                                layout, sizeof(layout));
    request.palette = source_for(DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY,
                                 DM2_V1_GDAT_HUD_ADMISSION_PALETTE16_TYPE,
                                 palette, sizeof(palette));
    request.font = source_for(DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY,
                              DM2_V1_GDAT_HUD_ADMISSION_RAW7_TYPE,
                              font, sizeof(font));
    request.champion = source_for(DM2_V1_GDAT_HUD_ADMISSION_CHAMPION_CATEGORY,
                                  DM2_V1_GDAT_HUD_ADMISSION_IMAGE_TYPE,
                                  portrait, sizeof(portrait));
    request.champion_width = 2;
    request.champion_height = 2;
    request.champion_stride = 3;
    return request;
}

int main(void)
{
    DM2_V1_GdatHudAdmissionRequest request = valid_request();
    DM2_V1_GdatHudAdmissionReceipt receipt;

    check(dm2_v1_gdat_hud_material_admit_dm2_010(&request, &receipt) == 1 &&
              receipt.admitted && receipt.rejection_mask == 0 &&
              receipt.champion_pixels == request.champion.bytes &&
              receipt.champion_pixel_count == request.champion.byte_count,
          "complete real GDAT evidence admits original champion pixels");

    request.palette.content_hash = 0;
    check(dm2_v1_gdat_hud_material_admit_dm2_010(&request, &receipt) == 0 &&
              (receipt.rejection_mask & DM2_V1_GDAT_HUD_ADMISSION_REJECT_PALETTE) &&
              receipt.champion_pixels == NULL && receipt.champion_pixel_count == 0U,
          "missing palette rejects without fallback pixels");

    request = valid_request();
    request.font.entry_type = DM2_V1_GDAT_HUD_ADMISSION_IMAGE_TYPE;
    request.champion.category = DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY;
    check(dm2_v1_gdat_hud_material_admit_dm2_010(&request, &receipt) == 0 &&
              (receipt.rejection_mask & DM2_V1_GDAT_HUD_ADMISSION_REJECT_FONT) &&
              (receipt.rejection_mask & DM2_V1_GDAT_HUD_ADMISSION_REJECT_CHAMPION) &&
              !receipt.admitted,
          "wrong GDAT provenance rejects font and champion material");

    request = valid_request();
    request.champion.byte_count = 5;
    check(dm2_v1_gdat_hud_material_admit_dm2_010(&request, &receipt) == 0 &&
              (receipt.rejection_mask & DM2_V1_GDAT_HUD_ADMISSION_REJECT_PIXELS) &&
              receipt.champion_pixels == NULL,
          "short pixel payload is never admitted");

    check(dm2_v1_gdat_hud_material_admit_dm2_010(NULL, &receipt) == 0 &&
              receipt.rejection_mask ==
                  (DM2_V1_GDAT_HUD_ADMISSION_REJECT_LAYOUT |
                   DM2_V1_GDAT_HUD_ADMISSION_REJECT_PALETTE |
                   DM2_V1_GDAT_HUD_ADMISSION_REJECT_FONT |
                   DM2_V1_GDAT_HUD_ADMISSION_REJECT_CHAMPION |
                   DM2_V1_GDAT_HUD_ADMISSION_REJECT_PIXELS),
          "null request rejects every required provenance component");

    return failures != 0;
}
