#ifndef NEXUS_V1_ITEM_IBS_H
#define NEXUS_V1_ITEM_IBS_H

#include <stdint.h>

#define NEXUS_ITEM_IBS_SIGNATURE        0x6931
#define NEXUS_ITEM_IBS_DECL_SIZE        40
#define NEXUS_ITEM_IBS_IMG_SIZE         128
#define NEXUS_ITEM_IBS_IMG_WIDTH        16
#define NEXUS_ITEM_IBS_IMG_HEIGHT       16
#define NEXUS_ITEM_IBS_PALETTE_COLORS   16
#define NEXUS_ITEM_IBS_MAX_PALETTES     8
#define NEXUS_ITEM_IBS_MAX_ITEMS        256
#define NEXUS_ITEM_IBS_MAX_IMAGES       256
#define NEXUS_ITEM_IBS_MAX_FLOOR_IMAGES 128
#define NEXUS_ITEM_IBS_SECTION_ALIGN    2048
#define NEXUS_ITEM_IBS_HEADER_SIZE      2048
#define NEXUS_ITEM_IBS_FLOOR_DESC_SIZE  20

typedef struct {
    uint8_t  item_index;
    uint8_t  category;
    uint8_t  carry_locations;
    uint8_t  flags;
    uint8_t  byte4;
    uint8_t  byte5;
    uint8_t  skill1;
    uint8_t  skill2;
    uint8_t  weight;
    uint8_t  byte9;
    uint8_t  byte10;
    uint8_t  byte11;
    uint8_t  byte12;
    uint8_t  byte13;
    uint8_t  byte14;
    uint8_t  byte15;
    uint8_t  action1;
    uint8_t  action2;
    uint8_t  action3;
    uint8_t  byte19;
    uint16_t inventory_image_index;
    uint16_t floor_image_index;
    uint16_t name_string;
    uint16_t desc_string;
    uint16_t action1_string;
    uint16_t action2_string;
    uint16_t action3_string;
    uint16_t word34;
    uint16_t attribute;
    uint8_t  byte38;
    uint8_t  byte39;
    int      has_floor_image;
} Nexus_V1_ItemIbsDecl;

typedef struct {
    uint16_t image_id;
    uint16_t encoding;
    uint16_t palette_id;
    uint16_t width;
    uint16_t height;
    uint32_t image_offset;
    uint32_t palette_offset;
} Nexus_V1_ItemIbsFloorDesc;

typedef struct {
    int valid;
    uint16_t item_count;
    uint16_t palette_count;
    uint16_t assoc_count;
    uint16_t image_count;
    uint16_t floor_image_count;
    uint32_t decl_section_offset;
    uint32_t images_section_offset;
    uint32_t floor_section_offset;
    uint32_t anim_section_offset;
} Nexus_V1_ItemIbsHeader;

typedef struct {
    int valid;
    int items_decoded;
    int images_decoded;
    int floor_images_decoded;
    Nexus_V1_ItemIbsHeader header;
    Nexus_V1_ItemIbsDecl items[NEXUS_ITEM_IBS_MAX_ITEMS];
    uint32_t palettes[NEXUS_ITEM_IBS_MAX_PALETTES][NEXUS_ITEM_IBS_PALETTE_COLORS];
    uint32_t image_hashes[NEXUS_ITEM_IBS_MAX_IMAGES];
    uint32_t floor_image_hashes[NEXUS_ITEM_IBS_MAX_FLOOR_IMAGES];
    Nexus_V1_ItemIbsFloorDesc floor_descs[NEXUS_ITEM_IBS_MAX_FLOOR_IMAGES];
} Nexus_V1_ItemIbsDecodeResult;

int nexus_v1_item_ibs_parse_header(const uint8_t *data, int data_size,
                                    Nexus_V1_ItemIbsHeader *out);

int nexus_v1_item_ibs_decode(const uint8_t *data, int data_size,
                              Nexus_V1_ItemIbsDecodeResult *out);

int nexus_v1_item_ibs_render_image(const uint8_t *data, int data_size,
                                    int image_index,
                                    const uint32_t palette[16],
                                    uint32_t *rgba_out);

int nexus_v1_item_ibs_render_floor_image(const uint8_t *data, int data_size,
                                          int floor_index,
                                          const Nexus_V1_ItemIbsDecodeResult *result,
                                          uint32_t *rgba_out,
                                          int rgba_capacity);

#endif
