#include "dm1_v1_iigs_audio.h"

#include <string.h>

int dm1_v1_iigs_audio_metadata_read(const uint8_t *program, size_t size,
                                   DM1_V1_IigsAudioMetadata *out) {
    DM1_V1_IigsMetadata identity;
    DM1_V1_IigsAudioMetadata value;
    unsigned i;
    if (!out) return 0;
    memset(out,0,sizeof(*out)); memset(&value,0,sizeof(value));
    if (!dm1_v1_iigs_metadata_read(program,size,&identity) ||
        size<59048U+34U*8U || size<56021U+625U) return 0;
    value.program_version=identity.program_version;
    /* ReDMCSB DEFS.H:31–56 SOUND_VOLUME/SOUND_DATA and native count 34;
     * DATA.C:478–533 GS event table. All 272 source initializer bytes match
     * both original programs at 59048, without relying on host struct size.
     * SOUND.C:84–109 GS G1028 table likewise matches 625 bytes at 56021. */
    for (i=0; i<DM1_V1_IIGS_SOUND_COUNT; ++i) {
        const uint8_t *p=program+59048U+8U*i;
        DM1_V1_IigsSoundSpec *s=&value.sounds[i];
        s->graphic_index=(int16_t)((uint16_t)p[0]|((uint16_t)p[1]<<8));
        s->a_unreferenced=p[2]; s->period=p[3]; s->priority=p[4];
        s->b_unreferenced=p[5]; s->loud_distance=p[6]; s->soft_distance=p[7];
        if (s->graphic_index<533 || s->graphic_index>=575 ||
            s->graphic_index==538 || s->graphic_index==548 ||
            !s->period || s->loud_distance>s->soft_distance) return 0;
    }
    memcpy(value.distance_volume,program+56021U,sizeof(value.distance_volume));
    *out=value;
    return 1;
}

int dm1_v1_iigs_audio_pcm_query(const uint8_t *record, size_t size,
                               DM1_V1_IigsPcmReceipt *out) {
    size_t count,table=256U;
    uint8_t code=0;
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    if (!record || size<3U || size>32771U) return 0;
    count=((size_t)record[0]<<8)|record[1];
    if (!count || count>32767U || count>size-2U || size-2U-count>2U) return 0;
    while (table<count+1U) { table*=2U; ++code; }
    out->sample_count=count; out->doc_byte_count=count+1U;
    out->table_size_code=code; out->resolution_code=code;
    return 1;
}

int dm1_v1_iigs_audio_pcm_decode(const uint8_t *record, size_t size,
    uint8_t *doc_bytes, size_t capacity, DM1_V1_IigsPcmReceipt *out) {
    DM1_V1_IigsPcmReceipt value;
    uintptr_t source=(uintptr_t)record,destination=(uintptr_t)doc_bytes;
    size_t i;
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    if (!doc_bytes || !dm1_v1_iigs_audio_pcm_query(record,size,&value) ||
        capacity<value.doc_byte_count) return 0;
    if ((destination>=source && destination-source<size) ||
        (source>destination && source-destination<value.doc_byte_count)) return 0;
    /* SOUND.C F0062:778–791 byte-swaps the count and reserves N+1 bytes.
     * F2189:819–854 converts N source samples, remaps unsigned zero to one,
     * and appends the DOC stop marker. F2188:684–737 selects power-of-two
     * table size and the identical resolution code. Padding is never audio. */
    for (i=0; i<value.sample_count; ++i) {
        uint8_t sample=(uint8_t)(record[2U+i]+0x80U);
        doc_bytes[i]=sample ? sample : 1U;
    }
    doc_bytes[value.sample_count]=0;
    *out=value;
    return 1;
}

int dm1_v1_iigs_audio_volume(const DM1_V1_IigsAudioMetadata *metadata,
    int party_x, int party_y, unsigned direction, int source_x, int source_y,
    DM1_V1_IigsSoundVolume *out) {
    int64_t dx=(int64_t)source_x-party_x,dy=(int64_t)source_y-party_y;
    int64_t column,line;
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    if (!metadata || direction>3U ||
        (metadata->program_version!=DM1_V1_IIGS_PROGRAM_G20 &&
         metadata->program_version!=DM1_V1_IIGS_PROGRAM_G21)) return 0;
    /* SOUND.C F0505:1886–1923 rotates relative location and mirrors the
     * native GS G1028 column for left/right. Outside +/-12 is inaudible. */
    switch (direction) {
    case 0:column=dx;line=dy;break;
    case 1:column=dy;line=-dx;break;
    case 2:column=-dx;line=-dy;break;
    default:column=-dy;line=dx;break;
    }
    if (column < -12 || column > 12 || line < -12 || line > 12) return 0;
    out->right=metadata->distance_volume[line+12][column+12];
    out->left=metadata->distance_volume[line+12][-column+12];
    return 1;
}

int dm1_v1_iigs_audio_quantize_volume(DM1_V1_IigsSoundVolume requested,
    unsigned master_level, DM1_V1_IigsSoundVolume *out) {
    if (!out) return 0;
    memset(out,0,sizeof(*out));
    if (master_level>15U) return 0;
    /* SOUND.C F0060:957–958 divides before multiplying; COMMAND.C:
     * 1227–1243 limits the GS master volume level to 0..15. */
    out->left=(uint8_t)((requested.left/15U)*master_level);
    out->right=(uint8_t)((requested.right/15U)*master_level);
    return 1;
}

int dm1_v1_iigs_audio_pending_merge(DM1_V1_IigsPendingSounds *pending,
    unsigned sound_index, DM1_V1_IigsSoundVolume volume) {
    DM1_V1_IigsSoundVolume *old;
    if (!pending || sound_index>=DM1_V1_IIGS_SOUND_COUNT) return 0;
    old=&pending->volumes[sound_index];
    /* SOUND.C F0064:1648–1654, G20/G21 branch: strict greater L+R. */
    if ((unsigned)volume.left+volume.right<=(unsigned)old->left+old->right) return 0;
    *old=volume;
    return 1;
}

int dm1_v1_iigs_audio_pending_flush(DM1_V1_IigsPendingSounds *pending,
    DM1_V1_IigsPendingVisitor visitor, void *user_data) {
    unsigned i;
    int delivered=0;
    if (!pending || !visitor) return 0;
    /* SOUND.C F0065:1792–1802: one-sided entries remain pending. */
    for (i=0; i<DM1_V1_IIGS_SOUND_COUNT; ++i) {
        DM1_V1_IigsSoundVolume *v=&pending->volumes[i];
        if (!v->left || !v->right) continue;
        visitor(i,*v,user_data);
        v->left=0; v->right=0;
        ++delivered;
    }
    return delivered;
}
