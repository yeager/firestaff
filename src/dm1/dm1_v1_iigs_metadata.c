#include "dm1_v1_iigs_metadata.h"
#include "asset_find_by_hash.h"

#include <string.h>

static int iigs_meta_bytes(const uint8_t *program, size_t size, size_t offset,
                           void *out, size_t count) {
    if (offset>size || count>size-offset) return 0;
    memcpy(out,program+offset,count);
    return 1;
}
static int iigs_meta_words(const uint8_t *program, size_t size, size_t offset,
                           void *out, size_t count) {
    size_t i;
    uint8_t *destination=(uint8_t *)out;
    if (offset>size || count>(size-offset)/2U) return 0;
    for (i=0; i<count; ++i) {
        uint16_t word=(uint16_t)((uint16_t)program[offset+2U*i] |
                                ((uint16_t)program[offset+2U*i+1U]<<8));
        memcpy(destination+2U*i,&word,sizeof(word));
    }
    return 1;
}
static int iigs_meta_frames_valid(const uint8_t *frames, size_t count) {
    size_t i;
    for (i=0; i<count; ++i) {
        const uint8_t *f=frames+6U*i;
        if (f[0]>f[1] || f[1]>=224U || f[2]>f[3] || f[3]>=136U ||
            !f[4] || f[4]>112U || !f[5] || f[5]>136U) return 0;
    }
    return 1;
}

int dm1_v1_iigs_metadata_read(const uint8_t *program, size_t size,
                             DM1_V1_IigsMetadata *out) {
    DM1_V1_IigsMetadata value;
    size_t i,j;
    if (!out) return 0;
    memset(out,0,sizeof(*out)); memset(&value,0,sizeof(value));
    if (!program) return 0;
    if (size==232158U && asset_memory_matches_md5(program,size,
            "51b9d22c7418651ee49ad77f37c80a3c"))
        value.program_version=DM1_V1_IIGS_PROGRAM_G20;
    else if (size==231865U && asset_memory_matches_md5(program,size,
            "eccc49e7b29ee233cfb10615e8ec86f3"))
        value.program_version=DM1_V1_IIGS_PROGRAM_G21;
    else return 0;

    /* Native GS compiled DATA.C:119–244, G0000..G0021/G0347. Each complete
     * initializer was compared with BOTH original executables at these
     * offsets, including all 70 GS mandatory graphics and six RGB4 dungeon
     * palettes. Read words explicitly in the original little-endian order. */
    if (!iigs_meta_words(program,size,57615U,value.credits_palette,16U) ||
        !iigs_meta_words(program,size,57647U,value.entrance_palette,16U) ||
        !iigs_meta_words(program,size,57679U,value.interface_palette,16U) ||
        !iigs_meta_words(program,size,57711U,&value.dungeon_palettes[0][0],96U) ||
        !iigs_meta_words(program,size,57323U,&value.ui_boxes[0][0],68U) ||
        !iigs_meta_bytes(program,size,57459U,value.palette_no_changes,16U) ||
        !iigs_meta_words(program,size,57475U,value.mandatory_graphics,70U)) return 0;

    /* Native GS compiled DUNVIEW.C:780–930 (G0190..G0196),1042–1302
     * (G0198..G0209),1534–1535 (G0213/14),1593–1623 (G0218).
     * All byte-table spans below match those exact GS source initializers
     * in both G20E and G21E programs. Their layout differs from Amiga M558;
     * no peer platform's bank, offsets or scaled palette values are used. */
#define IIGS_COPY(field, offset) \
    if (!iigs_meta_bytes(program,size,(offset),value.field,sizeof(value.field))) return 0
    IIGS_COPY(wall_bitmap_increments,52732U);
    IIGS_COPY(floor_bitmap_increments,52744U);
    IIGS_COPY(wall_coordinate_sets,52758U);
    IIGS_COPY(floor_coordinate_sets,52818U);
    IIGS_COPY(door_coordinate_sets,52827U);
    IIGS_COPY(wall_palette_changes,52840U);
    IIGS_COPY(door_palette_changes,52872U);
    IIGS_COPY(wall_inscription_patch,52904U);
    IIGS_COPY(inscription_line_y,52908U);
    IIGS_COPY(unreadable_inscription_y2,52912U);
    IIGS_COPY(wall_frames,52927U);
    IIGS_COPY(floor_frames,53551U);
    IIGS_COPY(door_frames,53713U);
    IIGS_COPY(door_button_frames,53785U);
    IIGS_COPY(object_aspects,53809U);
    IIGS_COPY(object_palette_changes,54427U);
    IIGS_COPY(object_coordinates,54503U);
#undef IIGS_COPY
    for (i=0; i<16U; ++i) {
        if (value.credits_palette[i]>0x0fffU || value.entrance_palette[i]>0x0fffU ||
            value.interface_palette[i]>0x0fffU || value.palette_no_changes[i]!=i) return 0;
        for (j=0; j<6U; ++j) if (value.dungeon_palettes[j][i]>0x0fffU) return 0;
        for (j=0; j<2U; ++j)
            if (value.object_palette_changes[j][i]>15U ||
                value.wall_palette_changes[j][i]>15U ||
                value.door_palette_changes[j][i]>15U) return 0;
    }
    for (i=0; i<17U; ++i)
        if (value.ui_boxes[i][0]>value.ui_boxes[i][1] || value.ui_boxes[i][1]>=320U ||
            value.ui_boxes[i][2]>value.ui_boxes[i][3] || value.ui_boxes[i][3]>=200U) return 0;
    for (i=0; i<70U; ++i) if (value.mandatory_graphics[i]>=575U) return 0;
    for (i=0; i<85U; ++i) {
        const uint8_t *a=value.object_aspects[i];
        if (a[0]>85U || !a[2] || a[2]>112U || !a[3] || a[3]>136U || a[5]>=3U) return 0;
    }
    for (i=0; i<60U; ++i) if (value.wall_coordinate_sets[i]>=8U) return 0;
    for (i=0; i<9U; ++i)
        if (value.floor_coordinate_sets[i]>=3U || value.floor_bitmap_increments[i]>=6U) return 0;
    for (i=0; i<12U; ++i)
        if (value.door_coordinate_sets[i]>=4U || value.wall_bitmap_increments[i]>=5U) return 0;
    if (!iigs_meta_frames_valid(&value.wall_frames[0][0][0],104U) ||
        !iigs_meta_frames_valid(&value.floor_frames[0][0][0],27U) ||
        !iigs_meta_frames_valid(&value.door_frames[0][0][0],12U) ||
        !iigs_meta_frames_valid(&value.door_button_frames[0][0],4U)) return 0;
    *out=value;
    return 1;
}
