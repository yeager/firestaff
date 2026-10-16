#include "dm1_v1_iigs_audio.h"
#include "asset_find_by_hash.h"
#include "dm1_v1_legacy_graphics_dat.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

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

/* DEFS.H:75-127: shared runtime semantic IDs to the native GS bank.
 * PC entrance cue 3 has no native counterpart; GS entrance uses C02. */
int dm1_v1_iigs_audio_sound_index_from_pc34(int semantic)
{
    static const int8_t native[35] = {
        0,1,2,-1,4,5,20,6,8,9,10,11,12,16,17,18,13,28,25,
        14,15,19,21,3,29,7,30,31,22,23,24,26,27,32,33
    };
    return semantic >= 0 && semantic < 35 ? native[semantic] : -1;
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

/* Native SOUND.C F2184..F2189 residency and F0060's fifteen stereo pairs.
 * Hardware reference: Apple Ensoniq DOC ERS (1986), pp5/8/11; scan clock
 * CLK/(8*(32+2)), native equal size/resolution gives address shift9.
 * Fixed float headroom is host policy, independent of native DAC volume. */
typedef struct IigsDocSound {
    uint8_t *bytes;
    size_t count;
    uint8_t size_code;
} IigsDocSound;
typedef struct IigsDocPair {
    int sound;
    uint32_t accumulator;
    unsigned page, size_code;
    uint8_t active, left, right;
} IigsDocPair;
struct DM1_V1_IigsDocMixer {
    DM1_V1_IigsAudioMetadata metadata;
    IigsDocSound sounds[34];
    IigsDocPair pairs[15];
    uint8_t ram[65536], page_owner[256];
    int resident_page[34], latest_pair[34];
    uint8_t resident_pages[34];
    uint64_t last_use[34], use_serial;
    uint64_t clock_num, clock_den, threshold, phase;
    uint64_t frames, scans, triggers, replacements, stops, evictions;
    unsigned host_rate, next_pair;
    int first_scan;
    float held_left, held_right;
};

void dm1_v1_iigs_doc_mixer_reset(DM1_V1_IigsDocMixer *m) {
    unsigned i;
    if (!m) return;
    memset(m->pairs,0,sizeof(m->pairs));
    memset(m->ram,0,sizeof(m->ram));
    memset(m->page_owner,255,sizeof(m->page_owner));
    memset(m->resident_pages,0,sizeof(m->resident_pages));
    memset(m->last_use,0,sizeof(m->last_use));
    for (i=0;i<34;++i) { m->resident_page[i]=-1; m->latest_pair[i]=-1; }
    for (i=0;i<15;++i) m->pairs[i].sound=-1;
    m->phase=0; m->use_serial=0; m->frames=0; m->scans=0;
    m->triggers=0; m->replacements=0; m->stops=0; m->evictions=0;
    m->next_pair=0; m->first_scan=1; m->held_left=0; m->held_right=0;
}
void dm1_v1_iigs_doc_mixer_destroy(DM1_V1_IigsDocMixer *m) {
    unsigned i;
    if (!m) return;
    for (i=0;i<34;++i) free(m->sounds[i].bytes);
    memset(m,0,sizeof(*m));
    free(m);
}
DM1_V1_IigsDocMixer *dm1_v1_iigs_doc_mixer_create(
    const uint8_t *program, size_t program_size,
    const uint8_t *graphics, size_t graphics_size,
    uint64_t clock_num, uint64_t clock_den, unsigned host_rate) {
    DM1_V1_IigsDocMixer *m;
    uint8_t *record;
    unsigned i;
    uint64_t threshold;
    /* Bound scan work per output frame and avoid phase arithmetic overflow.
     * The explicit clock is a chosen hardware profile, not a PC timer. */
    if (!clock_den || host_rate<8000 || host_rate>192000 ||
        clock_num/clock_den<1000000 || clock_num/clock_den>20000000 ||
        clock_den>UINT64_MAX/(272U*(uint64_t)host_rate)) return NULL;
    threshold=clock_den*272U*host_rate;
    if (clock_num>UINT64_MAX-threshold || !graphics || graphics_size!=398713U ||
        !asset_memory_matches_md5(graphics,graphics_size,"1045a36952e64eb2bc7b4c7b2965b112")) return NULL;
    m=(DM1_V1_IigsDocMixer*)calloc(1,sizeof(*m));
    if (!m) return NULL;
    if (!dm1_v1_iigs_audio_metadata_read(program,program_size,&m->metadata)) {
        dm1_v1_iigs_doc_mixer_destroy(m); return NULL;
    }
    record=(uint8_t*)malloc(32771U);
    if (!record) { dm1_v1_iigs_doc_mixer_destroy(m); return NULL; }
    for (i=0;i<34;++i) {
        size_t length=0;
        DM1_V1_IigsPcmReceipt r, decoded;
        if (!m->metadata.sounds[i].period ||
            !dm1_v1_legacy_graphics_read_raw(graphics,graphics_size,0,
                (uint16_t)m->metadata.sounds[i].graphic_index,record,32771U,&length) ||
            !dm1_v1_iigs_audio_pcm_query(record,length,&r)) goto fail;
        m->sounds[i].bytes=(uint8_t*)malloc(r.doc_byte_count);
        if (!m->sounds[i].bytes ||
            !dm1_v1_iigs_audio_pcm_decode(record,length,m->sounds[i].bytes,r.doc_byte_count,&decoded) ||
            decoded.doc_byte_count!=r.doc_byte_count ||
            decoded.resolution_code!=decoded.table_size_code) goto fail;
        m->sounds[i].count=r.doc_byte_count;
        m->sounds[i].size_code=r.table_size_code;
    }
    free(record);
    m->clock_num=clock_num; m->clock_den=clock_den;
    m->threshold=threshold; m->host_rate=host_rate;
    dm1_v1_iigs_doc_mixer_reset(m);
    return m;
fail:
    free(record); dm1_v1_iigs_doc_mixer_destroy(m); return NULL;
}

static void iigs_doc_evict(DM1_V1_IigsDocMixer *m, unsigned sound) {
    int page=m->resident_page[sound], pair=m->latest_pair[sound];
    unsigned i;
    if (page<0) return;
    /* F2187 halts the latest G4034 pair only. Older same-sound pairs may
     * still read reused DOC RAM, so they must not be stopped by identity. */
    if (pair>=0 && pair<15) m->pairs[pair].active=0;
    for (i=(unsigned)page;i<(unsigned)page+m->resident_pages[sound];++i)
        m->page_owner[i]=255;
    m->resident_page[sound]=-1;
    ++m->evictions;
}
static int iigs_doc_reside(DM1_V1_IigsDocMixer *m, unsigned sound) {
    unsigned alignment=1U<<m->sounds[sound].size_code;
    unsigned pages=(unsigned)((m->sounds[sound].count+255U)/256U);
    unsigned start, i, attempts;
    if (m->resident_page[sound]>=0) return 1;
    /* F2188 tests aligned starts, reserves actual used pages (not entire
     * power-of-two wave table), and evicts overlapping owners as a unit. */
    for (attempts=0;attempts<=34;++attempts) {
        for (start=0;start+pages<=256;start+=alignment) {
            for (i=0;i<pages && m->page_owner[start+i]==255;++i) {}
            if (i==pages) {
                for (i=0;i<pages;++i) m->page_owner[start+i]=(uint8_t)sound;
                m->resident_page[sound]=(int)start;
                m->resident_pages[sound]=(uint8_t)pages;
                memcpy(m->ram+start*256U,m->sounds[sound].bytes,m->sounds[sound].count);
                return 1;
            }
        }
        /* Original candidate ordering: strict older-than, aligned starts,
         * first free aligned start wins even if its following pages conflict.
         * Treat an absent initial owner as no age, avoiding source's invalid
         * G4030[255] read without assigning a fabricated timestamp. */
        {
            unsigned victim=0, owner=m->page_owner[0];
            uint64_t oldest=owner<34 ? m->last_use[owner] : UINT64_MAX;
            for (start=0;start+pages<=256;start+=alignment) {
                owner=m->page_owner[start];
                if (owner==255) { victim=start; break; }
                if (owner>=34) return 0;
                if (m->last_use[owner]<oldest) { victim=start; oldest=m->last_use[owner]; }
            }
            for (i=victim;i<victim+pages;++i) {
                owner=m->page_owner[i];
                if (owner<34) iigs_doc_evict(m,owner);
                else if (owner!=255) return 0;
            }
        }
    }
    return 0;
}
int dm1_v1_iigs_doc_mixer_trigger(DM1_V1_IigsDocMixer *m,
    unsigned sound, DM1_V1_IigsSoundVolume requested, unsigned master_level) {
    DM1_V1_IigsSoundVolume volume;
    IigsDocPair *pair;
    unsigned i;
    if (!m || sound>=34 || m->use_serial==UINT64_MAX ||
        !dm1_v1_iigs_audio_quantize_volume(requested,master_level,&volume)) return 0;
    if (!iigs_doc_reside(m,sound)) return 0;
    m->last_use[sound]=++m->use_serial;
    pair=&m->pairs[m->next_pair];
    if (pair->active) ++m->replacements;
    /* F0060 clears every sound's stale mapping to this oscillator pair. */
    for (i=0;i<34;++i) if (m->latest_pair[i]==(int)m->next_pair) m->latest_pair[i]=-1;
    m->latest_pair[sound]=(int)m->next_pair;
    pair->sound=(int)sound; pair->accumulator=0;
    pair->page=(unsigned)m->resident_page[sound];
    pair->size_code=m->sounds[sound].size_code;
    pair->left=volume.left; pair->right=volume.right; pair->active=1;
    m->next_pair=(m->next_pair+1U)%15U;
    ++m->triggers;
    return 1;
}
static void iigs_doc_scan(DM1_V1_IigsDocMixer *m) {
    int64_t left=0,right=0;
    unsigned i;
    for (i=0;i<15;++i) {
        IigsDocPair *p=&m->pairs[i];
        unsigned address, size;
        uint8_t byte;
        int sample;
        if (!p->active) continue;
        size=256U<<p->size_code;
        address=p->accumulator>>9;
        if (address>=size) { p->active=0; ++m->stops; continue; }
        byte=m->ram[p->page*256U+address];
        p->accumulator=(p->accumulator+m->metadata.sounds[p->sound].period)&0xFFFFFFU;
        if (!byte) { p->active=0; ++m->stops; continue; }
        sample=(int)byte-128;
        left+=(int64_t)sample*p->left;
        right+=(int64_t)sample*p->right;
        if (address==size-1U) { p->active=0; ++m->stops; }
    }
    m->held_left=(float)((double)left/(128.0*255.0*15.0));
    m->held_right=(float)((double)right/(128.0*255.0*15.0));
    ++m->scans;
}
int dm1_v1_iigs_doc_mixer_render_f32_stereo(DM1_V1_IigsDocMixer *m,
    float *out, size_t frames) {
    size_t i;
    uint64_t scans_per_frame;
    if (!m || (!out && frames) || frames>SIZE_MAX/(2U*sizeof(float)) ||
        (uint64_t)frames>UINT64_MAX-m->frames || m->scans>=UINT64_MAX-1U) return 0;
    scans_per_frame=m->clock_num/m->threshold+1U;
    if ((uint64_t)frames>(UINT64_MAX-m->scans-1U)/scans_per_frame) return 0;
    if (!frames) return 1;
    if (m->first_scan) { iigs_doc_scan(m); m->first_scan=0; }
    for (i=0;i<frames;++i) {
        out[2U*i]=m->held_left; out[2U*i+1U]=m->held_right;
        m->phase+=m->clock_num;
        while (m->phase>=m->threshold) { m->phase-=m->threshold; iigs_doc_scan(m); }
    }
    m->frames+=(uint64_t)frames;
    return 1;
}
int dm1_v1_iigs_doc_mixer_snapshot(const DM1_V1_IigsDocMixer *m,
    DM1_V1_IigsDocSnapshot *out) {
    unsigned i;
    if (!m || !out) return 0;
    memset(out,0,sizeof(*out));
    out->rendered_frames=m->frames; out->doc_scans=m->scans;
    out->triggers=m->triggers; out->replacements=m->replacements;
    out->stops=m->stops; out->evictions=m->evictions;
    out->clock_numerator=m->clock_num; out->clock_denominator=m->clock_den;
    out->host_rate=m->host_rate; out->next_pair=m->next_pair;
    for (i=0;i<15;++i) {
        out->sound_index[i]=m->pairs[i].sound;
        out->accumulator[i]=m->pairs[i].accumulator;
        out->active[i]=m->pairs[i].active;
        out->left[i]=m->pairs[i].left; out->right[i]=m->pairs[i].right;
        out->active_pairs+=m->pairs[i].active ? 1U : 0U;
    }
    memcpy(out->resident_page,m->resident_page,sizeof(out->resident_page));
    memcpy(out->resident_pages,m->resident_pages,sizeof(out->resident_pages));
    memcpy(out->page_owner,m->page_owner,sizeof(out->page_owner));
    memcpy(out->latest_pair,m->latest_pair,sizeof(out->latest_pair));
    memcpy(out->last_use,m->last_use,sizeof(out->last_use));
    return 1;
}
