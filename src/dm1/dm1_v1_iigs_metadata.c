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
static uint16_t iigs_meta_u16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0]|((uint16_t)p[1]<<8));
}
static int iigs_meta_signed_words(const uint8_t *program, size_t size,
                                  size_t offset, void *out, size_t count) {
    size_t i;
    uint8_t *destination=(uint8_t *)out;
    if (offset>size || count>(size-offset)/2U) return 0;
    for (i=0; i<count; ++i) {
        uint16_t bits=iigs_meta_u16(program+offset+2U*i);
        int16_t word=(int16_t)(bits<0x8000U ? (int32_t)bits : (int32_t)bits-65536);
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
    /* DATA.C G0026/G0030/G0038 and COMMAND.C G0449, exact LE16
     * spans independently matched in authenticated G20/G21 programs. */
    if (!iigs_meta_words(program,size,57947U,value.icon_first,7U) ||
        !iigs_meta_signed_words(program,size,57989U,value.slot_boxes,138U) ||
        !iigs_meta_signed_words(program,size,46961U,value.inventory_mouse,228U) ||
        !iigs_meta_words(program,size,58313U,value.slot_masks,38U)) return 0;
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
    /* DUNGEON.C:439 onward, G0243 native GS CREATURE_INFO initializer;
     * DEFS.H:1575–1594 defines its 26-byte layout. The complete 702-byte
     * table independently matches BOTH original programs at offset 45084
     * (MD5 a158c8dd19f97c8cd4a8b905ff5d0e6a). Decode each field instead of
     * borrowing PC gameplay defaults or depending on host struct padding. */
    if (size<45084U+702U) return 0;
    for (i=0; i<27U; ++i) {
        const uint8_t *p=program+45084U+26U*i;
        DM1_V1_IigsCreatureInfo *c=&value.creature_info[i];
        c->creature_aspect_index=p[0]; c->attack_sound_ordinal=p[1];
        c->attributes=iigs_meta_u16(p+2U); c->graphic_info=iigs_meta_u16(p+4U);
        c->movement_ticks=p[6]; c->attack_ticks=p[7]; c->defense=p[8];
        c->base_health=p[9]; c->attack=p[10]; c->poison_attack=p[11];
        c->dexterity=p[12]; c->a_unreferenced=p[13];
        c->ranges=iigs_meta_u16(p+14U); c->properties=iigs_meta_u16(p+16U);
        c->resistances=iigs_meta_u16(p+18U); c->animation_ticks=iigs_meta_u16(p+20U);
        c->wound_probabilities=iigs_meta_u16(p+22U);
        c->attack_type=p[24]; c->b_unreferenced=p[25];
        if (c->creature_aspect_index>=27U || c->attack_sound_ordinal>34U ||
            !c->movement_ticks || !c->attack_ticks) return 0;
    }

    /* Additional complete GS source initializers independently matched both
     * programs: DUNVIEW.C G0210/11:1303–1324, G0212:1533,
     * G0215..17:1541–1591, G0219:1625–1653, G0220:1705–1720,
     * G0221..24:1821–1869, G0225..28:1871–1923. Native field widths are
     * DEFS.H CREATURE_ASPECT:2001–2014, PROJECTIL_ASPECT:2034–2044,
     * CREATURE_REPLACEMENT_COLOR_SET:1979–1984. */
    if (size<55127U+182U || size<54803U+324U || size<54319U+84U) return 0;
    for (i=0; i<27U; ++i) {
        const uint8_t *p=program+54803U+12U*i;
        DM1_V1_IigsCreatureAspect *a=&value.creature_aspects[i];
        a->first_native_bitmap_relative_index=iigs_meta_u16(p);
        a->first_derived_bitmap_index=iigs_meta_u16(p+2U);
        a->byte_width_front=p[4]; a->height_front=p[5];
        a->byte_width_side=p[6]; a->height_side=p[7];
        a->byte_width_attack=p[8]; a->height_attack=p[9];
        a->coordinate_set_transparent_color=p[10];
        a->replacement_color_set_ordinals=p[11];
        if (a->first_native_bitmap_relative_index>86U ||
            !a->byte_width_front || !a->height_front ||
            a->byte_width_front>112U || a->height_front>136U ||
            a->byte_width_side>112U || a->height_side>136U ||
            a->byte_width_attack>112U || a->height_attack>136U ||
            (!!a->byte_width_side != !!a->height_side) ||
            (!!a->byte_width_attack != !!a->height_attack) ||
            (a->coordinate_set_transparent_color>>4)>=3U ||
            (a->replacement_color_set_ordinals&15U)>13U ||
            (a->replacement_color_set_ordinals>>4)>13U) return 0;
    }
    for (i=0; i<14U; ++i) {
        const uint8_t *p=program+54319U+6U*i;
        DM1_V1_IigsProjectileAspect *a=&value.projectile_aspects[i];
        a->first_native_bitmap_relative_index=p[0];
        a->first_derived_bitmap_relative_index=p[1];
        a->byte_width=p[2]; a->height=p[3]; a->graphic_info=iigs_meta_u16(p+4U);
        if (a->first_native_bitmap_relative_index>31U || !a->byte_width ||
            a->byte_width>112U || !a->height || a->height>136U ||
            (a->graphic_info&~0x0113U)) return 0;
    }
    for (i=0; i<13U; ++i) {
        const uint8_t *p=program+55127U+14U*i;
        DM1_V1_IigsCreatureReplacementColorSet *r=&value.creature_replacement_colors[i];
        for (j=0; j<6U; ++j) {
            r->rgb[j]=iigs_meta_u16(p+2U*j);
            if (r->rgb[j]>0x0fffU) return 0;
        }
        r->d2_replacement_color=p[12]; r->d3_replacement_color=p[13];
        if (p[12]>15U || p[13]>15U) return 0;
    }
#define IIGS_COPY(field, offset) \
    if (!iigs_meta_bytes(program,size,(offset),value.field,sizeof(value.field))) return 0
    IIGS_COPY(creature_palette_changes,55309U);
    IIGS_COPY(creature_coordinates,55389U);
    IIGS_COPY(explosion_aspects,54403U);
    IIGS_COPY(smoke_palette_changes,54411U);
    IIGS_COPY(projectile_scales,54459U);
    IIGS_COPY(explosion_base_scales,54466U);
    IIGS_COPY(object_pile_shift_sets,54471U);
#undef IIGS_COPY
    if (!iigs_meta_signed_words(program,size,55341U,value.shift_sets,24U) ||
        !iigs_meta_signed_words(program,size,55719U,value.centered_explosion_coordinates,30U) ||
        !iigs_meta_signed_words(program,size,55779U,value.explosion_coordinates,60U) ||
        !iigs_meta_signed_words(program,size,55899U,value.rebirth_step2_coordinates,21U) ||
        !iigs_meta_signed_words(program,size,55941U,value.rebirth_step1_coordinates,21U)) return 0;
    for (i=0; i<16U; ++i)
        if (value.creature_palette_changes[0][i]>15U ||
            value.creature_palette_changes[1][i]>15U || value.smoke_palette_changes[i]>15U)
            return 0;
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

static int iigs_meta_apply_replacement(const DM1_V1_IigsMetadata *metadata,
    DM1_V1_IigsCreaturePalette *palette, unsigned color, unsigned index) {
    const DM1_V1_IigsCreatureReplacementColorSet *set;
    size_t i;
    if (index>=13U || color>=16U) return 0;
    set=&metadata->creature_replacement_colors[index];
    if (set->d2_replacement_color>15U || set->d3_replacement_color>15U) return 0;
    for (i=0; i<6U; ++i) {
        if (set->rgb[i]>0x0fffU) return 0;
        palette->dungeon_palettes[i][color]=set->rgb[i];
    }
    palette->creature_palette_changes[0][color]=set->d3_replacement_color;
    palette->creature_palette_changes[1][color]=set->d2_replacement_color;
    return 1;
}

int dm1_v1_iigs_metadata_creature_palette(const DM1_V1_IigsMetadata *metadata,
    const uint8_t *allowed_creatures, size_t count,
    DM1_V1_IigsCreaturePalette *out) {
    DM1_V1_IigsCreaturePalette value;
    size_t i;
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    if (!metadata || count>15U || (count && !allowed_creatures) ||
        (metadata->program_version!=DM1_V1_IIGS_PROGRAM_G20 &&
         metadata->program_version!=DM1_V1_IIGS_PROGRAM_G21)) return 0;
    memcpy(value.dungeon_palettes,metadata->dungeon_palettes,sizeof(value.dungeon_palettes));
    memcpy(value.creature_palette_changes,metadata->creature_palette_changes,
           sizeof(value.creature_palette_changes));
    /* DUNVIEW.C F0095:2805–2815 resets defaults, then processes native
     * allowed-creature order. DEFS.H M073/M074:2018–2019 extracts ORDINALS;
     * zero leaves the current color, nonzero uses ordinal-1, so later map
     * entries win independently for colors 9 and 10. F0093:2008–2014 updates
     * six RGB4 palettes plus native D2 and D3 remaps. */
    if (!iigs_meta_apply_replacement(metadata,&value,9U,8U) ||
        !iigs_meta_apply_replacement(metadata,&value,10U,12U)) return 0;
    for (i=0; i<count; ++i) {
        unsigned ordinals,color9,color10;
        if (allowed_creatures[i]>=27U) return 0;
        ordinals=metadata->creature_aspects[allowed_creatures[i]].replacement_color_set_ordinals;
        color9=ordinals&15U; color10=ordinals>>4;
        if ((color9 && !iigs_meta_apply_replacement(metadata,&value,9U,color9-1U)) ||
            (color10 && !iigs_meta_apply_replacement(metadata,&value,10U,color10-1U))) return 0;
    }
    *out=value;
    return 1;
}
