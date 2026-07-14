#ifndef FIRESTAFF_DM2_V1_GDAT_HUD_MATERIAL_ADMISSION_DM2_010_H
#define FIRESTAFF_DM2_V1_GDAT_HUD_MATERIAL_ADMISSION_DM2_010_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DM2-010: admission only. This module never manufactures HUD pixels. */
enum {
    DM2_V1_GDAT_HUD_ADMISSION_INTERFACE_CATEGORY = 0x01,
    DM2_V1_GDAT_HUD_ADMISSION_CHAMPION_CATEGORY = 0x16,
    DM2_V1_GDAT_HUD_ADMISSION_IMAGE_TYPE = 0x01,
    DM2_V1_GDAT_HUD_ADMISSION_RAW7_TYPE = 0x07,
    DM2_V1_GDAT_HUD_ADMISSION_PALETTE16_TYPE = 0x0d
};

typedef enum {
    DM2_V1_GDAT_HUD_ADMISSION_REJECT_NONE = 0,
    DM2_V1_GDAT_HUD_ADMISSION_REJECT_LAYOUT = 1 << 0,
    DM2_V1_GDAT_HUD_ADMISSION_REJECT_PALETTE = 1 << 1,
    DM2_V1_GDAT_HUD_ADMISSION_REJECT_FONT = 1 << 2,
    DM2_V1_GDAT_HUD_ADMISSION_REJECT_CHAMPION = 1 << 3,
    DM2_V1_GDAT_HUD_ADMISSION_REJECT_PIXELS = 1 << 4
} DM2_V1_GdatHudAdmissionReject;

typedef struct {
    int category;
    int index;
    int entry_type;
    int field;
    const uint8_t *bytes;
    size_t byte_count;
    uint32_t content_hash;
} DM2_V1_GdatHudAdmissionSource;

typedef struct {
    DM2_V1_GdatHudAdmissionSource layout;
    DM2_V1_GdatHudAdmissionSource palette;
    DM2_V1_GdatHudAdmissionSource font;
    DM2_V1_GdatHudAdmissionSource champion;
    int champion_width;
    int champion_height;
    int champion_stride;
} DM2_V1_GdatHudAdmissionRequest;

typedef struct {
    int admitted;
    unsigned int rejection_mask;
    int champion_width;
    int champion_height;
    int champion_stride;
    const uint8_t *champion_pixels;
    size_t champion_pixel_count;
} DM2_V1_GdatHudAdmissionReceipt;

/* Validates the real GDAT provenance required before HUD pixels may be used.
 * On rejection, out_receipt contains no pixel pointer or pixel count. */
int dm2_v1_gdat_hud_material_admit_dm2_010(
    const DM2_V1_GdatHudAdmissionRequest *request,
    DM2_V1_GdatHudAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
