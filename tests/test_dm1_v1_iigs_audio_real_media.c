#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_audio.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "firestaff_prodos.h"
#include "audio_sdl_m11.h"
#ifndef FIRESTAFF_NO_SDL_AUDIO
#include <SDL3/SDL.h>
#endif
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr,"FAIL: %s\n",message); goto cleanup; } \
} while (0)
static void put16(uint8_t *p, unsigned v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); }
typedef struct { unsigned count, indexes[34]; } PendingVisit;
static void visit_pending(unsigned index, DM1_V1_IigsSoundVolume volume, void *user) {
    PendingVisit *v=user;
    if (v->count<34u && volume.left && volume.right) v->indexes[v->count++]=index;
}
static int check_volume(const DM1_V1_IigsAudioMetadata *metadata) {
    DM1_V1_IigsSoundVolume volume, quantized, near, mirror, center, edge, one;
    DM1_V1_IigsPendingSounds pending;
    PendingVisit visited={0};
    int result=0;
    /* SOUND.C F0505:1886-1923: rotate coordinates into a fixed native
     * lookup frame, mirror columns for left. Inverse rotations below keep
     * each original table cell fixed across all four party directions. */
    for (int y=-12; y<=12; ++y) for (int x=-12; x<=12; ++x) for (unsigned dir=0u; dir<4u; ++dir) {
        int dx=x, dy=y;
        if (dir==1u) { dx=-y; dy=x; }
        else if (dir==2u) { dx=-x; dy=-y; }
        else if (dir==3u) { dx=y; dy=-x; }
        REQUIRE(dm1_v1_iigs_audio_volume(metadata,20,21,dir,20+dx,21+dy,&volume) &&
            volume.left==metadata->distance_volume[y+12][-x+12] &&
            volume.right==metadata->distance_volume[y+12][x+12],"all original distance cells retain source channel rotation");
        for (unsigned master=0u; master<=15u; ++master)
            REQUIRE(dm1_v1_iigs_audio_quantize_volume(volume,master,&quantized) &&
                quantized.left==(volume.left/15u)*master && quantized.right==(volume.right/15u)*master,
                "native volume divides before multiplying for every source cell and master level");
    }
    REQUIRE(!dm1_v1_iigs_audio_volume(metadata,0,0,4u,0,0,&volume) && !volume.left && !volume.right &&
        !dm1_v1_iigs_audio_volume(metadata,INT_MIN,INT_MIN,0u,INT_MAX,INT_MAX,&volume) &&
        !volume.left && !volume.right,"invalid direction and distant integer coordinates reject safely");
    REQUIRE(dm1_v1_iigs_audio_volume(metadata,0,0,0u,1,0,&near) &&
        dm1_v1_iigs_audio_volume(metadata,0,0,0u,-1,0,&mirror) &&
        dm1_v1_iigs_audio_volume(metadata,0,0,0u,0,0,&center) &&
        dm1_v1_iigs_audio_volume(metadata,0,0,0u,12,0,&edge) &&
        dm1_v1_iigs_audio_quantize_volume(edge,1u,&one) && !one.left && one.right,
        "derive pending-state cases solely from original distance and quantization tables");
    memset(&pending,0,sizeof(pending));
    REQUIRE(dm1_v1_iigs_audio_pending_merge(&pending,0u,near) &&
        !dm1_v1_iigs_audio_pending_merge(&pending,0u,mirror) &&
        pending.volumes[0].left==near.left && pending.volumes[0].right==near.right &&
        dm1_v1_iigs_audio_pending_merge(&pending,33u,center) &&
        dm1_v1_iigs_audio_pending_merge(&pending,17u,one),"pending ties retain original channels without a priority replacement");
    REQUIRE(dm1_v1_iigs_audio_pending_flush(&pending,visit_pending,&visited)==2 && visited.count==2u &&
        visited.indexes[0]==0u && visited.indexes[1]==33u && !pending.volumes[0].left &&
        !pending.volumes[33].right && pending.volumes[17].right==one.right &&
        dm1_v1_iigs_audio_pending_flush(&pending,visit_pending,&visited)==0,
        "native pending flush orders events and retains the source one-sided-channel quirk");
    result=1;
cleanup:
    return result;
}
/* Independent scalar DOC oracle: native F0060 programs Period as phase
 * increment; size==resolution selects shift9. Read the OLD accumulator at
 * each DOC scan, then advance. E1FF=32 gives34 oscillator scan slots. */
typedef struct { uint8_t *bytes; size_t count; unsigned period; } OriginalDoc;
typedef struct { int active, sound; unsigned phase, left, right, base, mask; } ReferencePair;
typedef struct {
    OriginalDoc sounds[34]; ReferencePair pairs[15]; unsigned next_pair;
    uint64_t clock_num, threshold, remainder, frames, scans, triggers, evictions;
    uint8_t ram[65536], owner[256]; int resident[34], latest[34];
    unsigned pages[34]; uint64_t used[34]; int held_left,held_right,primed,asymmetric;
} ReferenceDoc;
static void reference_reset(ReferenceDoc *r, uint64_t num, uint64_t den, unsigned rate) {
    OriginalDoc sounds[34]; memcpy(sounds,r->sounds,sizeof(sounds)); memset(r,0,sizeof(*r));
    memcpy(r->sounds,sounds,sizeof(sounds)); r->clock_num=num; r->threshold=den*272u*rate;
    memset(r->owner,255,sizeof(r->owner));
    for (unsigned i=0u; i<34u; ++i) r->resident[i]=r->latest[i]=-1;
    for (unsigned i=0u; i<15u; ++i) r->pairs[i].sound=-1;
}
static void reference_evict(ReferenceDoc *r, unsigned sound) {
    if (r->resident[sound]<0) return;
    for (unsigned i=0u; i<r->pages[sound]; ++i) r->owner[(unsigned)r->resident[sound]+i]=255u;
    if (r->latest[sound]>=0) r->pairs[r->latest[sound]].active=0;
    r->resident[sound]=-1; ++r->evictions;
}
static void reference_trigger(ReferenceDoc *r, unsigned sound, DM1_V1_IigsSoundVolume volume, unsigned master) {
    unsigned align=1u, pages=(unsigned)((r->sounds[sound].count+256u)/256u);
    while (align*256u<r->sounds[sound].count+1u) align*=2u;
    if (r->resident[sound]<0) {
        unsigned chosen=256u;
        for (unsigned start=0u; start<256u; start+=align) {
            unsigned i=0u; while (i<pages && r->owner[start+i]==255u) ++i;
            if (i==pages) { chosen=start; break; }
        }
        if (chosen==256u) {
            chosen=0u;
            for (unsigned start=0u; start<256u; start+=align) {
                if (r->owner[start]==255u) { chosen=start; break; }
                if (r->used[r->owner[start]]<r->used[r->owner[chosen]]) chosen=start;
            }
            for (unsigned i=0u; i<pages; ++i)
                if (r->owner[chosen+i]!=255u) reference_evict(r,r->owner[chosen+i]);
        }
        r->resident[sound]=(int)chosen; r->pages[sound]=pages;
        for (unsigned i=0u; i<pages; ++i) r->owner[chosen+i]=(uint8_t)sound;
        memcpy(r->ram+chosen*256u,r->sounds[sound].bytes,r->sounds[sound].count+1u);
    }
    { ReferencePair *pair=&r->pairs[r->next_pair];
      for (unsigned i=0u; i<34u; ++i) if (r->latest[i]==(int)r->next_pair) r->latest[i]=-1;
      r->latest[sound]=(int)r->next_pair;
      memset(pair,0,sizeof(*pair)); pair->active=1; pair->sound=(int)sound;
      pair->base=(unsigned)r->resident[sound]*256u; pair->mask=align*256u-1u;
      pair->left=(volume.left/15u)*master; pair->right=(volume.right/15u)*master;
      r->next_pair=(r->next_pair+1u)%15u;
    }
    r->used[sound]=++r->triggers;
}
static void reference_scan(ReferenceDoc *r) {
    int left=0,right=0;
    for (unsigned i=0u; i<15u; ++i) {
        ReferencePair *pair=&r->pairs[i]; unsigned sample;
        if (!pair->active) continue;
        unsigned address=(pair->phase>>9)&pair->mask;
        sample=r->ram[pair->base+address];
        pair->phase+=r->sounds[pair->sound].period;
        if (!sample) pair->active=0;
        else { left+=((int)sample-128)*(int)pair->left; right+=((int)sample-128)*(int)pair->right;
            if (address==pair->mask) pair->active=0; }
    }
    r->held_left=left; r->held_right=right; ++r->scans;
}
static void reference_render(ReferenceDoc *r, float *out, size_t frames) {
    if (!r->primed) { reference_scan(r); r->primed=1; }
    for (size_t frame=0u; frame<frames; ++frame) {
        int left=r->held_left,right=r->held_right;
        out[2u*frame]=(float)((double)left/(128.0*255.0*15.0));
        out[2u*frame+1u]=(float)((double)right/(128.0*255.0*15.0));
        if (left!=right) r->asymmetric=1;
        r->remainder+=r->clock_num;
        while (r->remainder>=r->threshold) { r->remainder-=r->threshold; reference_scan(r); }
        ++r->frames;
    }
}

static int compare_doc_state(DM1_V1_IigsDocMixer *mixer, const ReferenceDoc *r) {
    DM1_V1_IigsDocSnapshot actual; int result=0; unsigned active=0u;
    REQUIRE(dm1_v1_iigs_doc_mixer_snapshot(mixer,&actual) && actual.rendered_frames==r->frames &&
        actual.doc_scans==r->scans && actual.triggers==r->triggers && actual.next_pair==r->next_pair &&
        actual.evictions==r->evictions && memcmp(actual.page_owner,r->owner,256u)==0,
        "native DOC counters and complete 64K page ownership match independent source allocator");
    for (unsigned sound=0u; sound<34u; ++sound)
        REQUIRE(actual.resident_page[sound]==r->resident[sound] && actual.latest_pair[sound]==r->latest[sound] &&
            actual.last_use[sound]==r->used[sound] && (r->resident[sound]<0 || actual.resident_pages[sound]==r->pages[sound]),
            "native source aligned residency, latest pair and allocation-before-use LRU identity match oracle");
    for (unsigned pair=0u; pair<15u; ++pair) {
        active+=(unsigned)r->pairs[pair].active;
        REQUIRE(actual.sound_index[pair]==r->pairs[pair].sound && actual.active[pair]==r->pairs[pair].active &&
            actual.left[pair]==r->pairs[pair].left && actual.right[pair]==r->pairs[pair].right &&
            (!r->pairs[pair].active || actual.accumulator[pair]==r->pairs[pair].phase),
            "all fifteen native pair identities, active phases and asymmetric channels match old-accumulator oracle");
    }
    REQUIRE(actual.active_pairs==active,"native active-pair count matches distinct source voices");
    result=1;
cleanup:
    return result;
}
static int compare_doc_frames(DM1_V1_IigsDocMixer *mixer, ReferenceDoc *r, size_t frames) {
    float actual[514], expected[514]; int result=0;
    while (frames) {
        size_t block=frames>257u ? 257u : frames;
        REQUIRE(dm1_v1_iigs_doc_mixer_render_f32_stereo(mixer,actual,block),"bounded original DOC stereo render");
        reference_render(r,expected,block);
        for (size_t i=0u; i<block*2u; ++i) {
            double difference=(double)actual[i]-expected[i];
            REQUIRE(difference>=-0.000001 && difference<=0.000001,
                "every host stereo sample matches independent rational DOC scan and source signed-byte oracle");
        }
        frames-=block;
    }
    REQUIRE(compare_doc_state(mixer,r),"complete native DOC state after chunked original sound stream"); result=1;
cleanup:
    return result;
}
static int check_doc_mixer(const uint8_t *bank, size_t bank_size, const uint8_t *program, size_t program_size,
    const DM1_V1_IigsAudioMetadata *metadata) {
    ReferenceDoc *reference=calloc(1,sizeof(*reference));
    DM1_V1_IigsDocMixer *mixer=NULL,*invalid=NULL;
    uint8_t *raw=malloc(65535u), *bank_copy=malloc(bank_size), *program_copy=malloc(program_size);
    DM1_V1_IigsSoundVolume volume={metadata->distance_volume[12][11],metadata->distance_volume[12][13]};
    DM1_V1_IigsDocSnapshot before,after; int result=0;
    REQUIRE(reference && raw && bank_copy && program_copy && volume.left!=volume.right && volume.left && volume.right,
        "bounded native DOC fixtures and authentic asymmetric source-table channel volumes");
    for (unsigned sound=0u; sound<34u; ++sound) {
        size_t size=0u; OriginalDoc *original=&reference->sounds[sound];
        REQUIRE(dm1_v1_iigs_graphics_read_raw(bank,bank_size,(uint16_t)metadata->sounds[sound].graphic_index,raw,65535u,&size),
            "independent DOC oracle reads all34 genuine event-owned original PCM records");
        original->count=((size_t)raw[0]<<8)|raw[1]; original->period=metadata->sounds[sound].period;
        original->bytes=malloc(original->count+1u); REQUIRE(original->bytes,"bounded authentic DOC oracle sample copy");
        for (size_t i=0u; i<original->count; ++i) {
            unsigned value=((unsigned)raw[i+2u]+128u)&255u; original->bytes[i]=(uint8_t)(value ? value : 1u);
        }
        original->bytes[original->count]=0u;
    }
    memcpy(bank_copy,bank,bank_size); memcpy(program_copy,program,program_size);
    mixer=dm1_v1_iigs_doc_mixer_create(program_copy,program_size,bank_copy,bank_size,7159090u,1u,48000u);
    REQUIRE(mixer,"native DOC mixer authenticates original complete bank and program");
    memset(bank_copy,0,bank_size); memset(program_copy,0,program_size);
    reference_reset(reference,7159090u,1u,48000u);
    for (unsigned sound=0u; sound<34u; ++sound) {
        uint64_t stop_scan=(reference->sounds[sound].count*512u+reference->sounds[sound].period-1u)/reference->sounds[sound].period;
        size_t frames=(size_t)((stop_scan*reference->threshold+reference->clock_num-1u)/reference->clock_num)+3u;
        REQUIRE(dm1_v1_iigs_doc_mixer_trigger(mixer,sound,volume,11u),"all original native events trigger with independent channel quantization");
        reference_trigger(reference,sound,volume,11u);
        REQUIRE(compare_doc_state(mixer,reference) && compare_doc_frames(mixer,reference,frames),
            "all34 complete original sound streams reach exact zero-stop under rational DOC clock");
        REQUIRE(dm1_v1_iigs_doc_mixer_snapshot(mixer,&after) && !after.active_pairs,
            "original terminal DOC zero halts each complete native event without table-padding playback");
    }
    REQUIRE(reference->asymmetric && reference->evictions>0u,
        "genuine bank exercises left/right asymmetry and original aggregate exceeding 64K RAM");
    dm1_v1_iigs_doc_mixer_reset(mixer); reference_reset(reference,7159090u,1u,48000u);
    for (unsigned sound=0u; sound<16u; ++sound) {
        REQUIRE(dm1_v1_iigs_doc_mixer_trigger(mixer,sound,volume,15u),"concurrent native original event trigger");
        reference_trigger(reference,sound,volume,15u);
        REQUIRE(compare_doc_frames(mixer,reference,7u),"overlapping source sounds retain fixed headroom across pair wrap");
    }
    REQUIRE(dm1_v1_iigs_doc_mixer_snapshot(mixer,&after) && after.replacements>0u && after.next_pair==1u,
        "sixteenth actual native event replaces first of fifteen oscillator pairs");
    for (unsigned repeated=0u; repeated<20u; ++repeated) {
        unsigned sound=repeated%3u ? 33u : 0u;
        REQUIRE(dm1_v1_iigs_doc_mixer_trigger(mixer,sound,volume,9u),"native same-event retrigger preserves earlier pair semantics");
        reference_trigger(reference,sound,volume,9u);
        REQUIRE(compare_doc_frames(mixer,reference,19u),"same-sound overlap and source latest-mapping eviction match independent RAM oracle");
    }
    REQUIRE(dm1_v1_iigs_doc_mixer_snapshot(mixer,&before) &&
        !dm1_v1_iigs_doc_mixer_trigger(mixer,34u,volume,15u) &&
        !dm1_v1_iigs_doc_mixer_trigger(mixer,0u,volume,16u) &&
        dm1_v1_iigs_doc_mixer_snapshot(mixer,&after) && before.triggers==after.triggers && before.next_pair==after.next_pair,
        "invalid event/master cannot mutate native playback pair allocation");
    dm1_v1_iigs_doc_mixer_destroy(mixer); mixer=NULL;
    memcpy(bank_copy,bank,bank_size); memcpy(program_copy,program,program_size); bank_copy[0]^=1u;
    invalid=dm1_v1_iigs_doc_mixer_create(program,program_size,bank_copy,bank_size,7159090u,1u,48000u);
    REQUIRE(!invalid,"original-derived wrong bank identity rejects native mixer admission");
    program_copy[program_size-1u]^=1u;
    invalid=dm1_v1_iigs_doc_mixer_create(program_copy,program_size,bank,bank_size,7159090u,1u,48000u);
    REQUIRE(!invalid,"original-derived wrong executable identity rejects native mixer admission");
    mixer=dm1_v1_iigs_doc_mixer_create(program,program_size,bank,bank_size,21477272u,3u,44100u);
    REQUIRE(mixer,"native mixer rebinds genuine sources to explicitly configured rational host clock");
    reference_reset(reference,21477272u,3u,44100u);
    REQUIRE(dm1_v1_iigs_doc_mixer_trigger(mixer,20u,volume,15u),"native remapped sound20 retains source period and record537");
    reference_trigger(reference,20u,volume,15u);
    REQUIRE(compare_doc_frames(mixer,reference,8193u),"rational-clock rebind preserves old-accumulator order at alternate host rate");
    dm1_v1_iigs_doc_mixer_reset(mixer); reference_reset(reference,21477272u,3u,44100u);
    REQUIRE(compare_doc_frames(mixer,reference,31u),"native reset clears voices, 64K residency and phase before silent rendering");
    result=1;
cleanup:
    dm1_v1_iigs_doc_mixer_destroy(invalid); dm1_v1_iigs_doc_mixer_destroy(mixer);
    if (reference) for (unsigned i=0u; i<34u; ++i) free(reference->sounds[i].bytes);
    free(reference); free(raw); free(bank_copy); free(program_copy); return result;
}

static int check_native_transport(const uint8_t *bank, size_t bank_size, const uint8_t *program,
    size_t program_size, const DM1_V1_IigsAudioMetadata *metadata) {
    M11_AudioState state; DM1_V1_IigsDocSnapshot before,after;
    const char *required=getenv("FIRESTAFF_IIGS_REQUIRE_AUDIO_BACKEND");
    int require_backend=required && strcmp(required,"1")==0, result=0;
    unsigned left=metadata->distance_volume[12][11], right=metadata->distance_volume[12][13];
    memset(&state,0,sizeof(state));
    REQUIRE(M11_Audio_InitDm1Iigs(&state,program,program_size,bank,bank_size) && state.initialized &&
        state.iigsMixer && !state.sdlStream && !state.originalSnd3Available && !state.soundPackAvailable &&
        !state.playedMarkerCount,"actual M11 native transport binds original DOC source without PC or procedural audio");
    REQUIRE(!require_backend || (M11_Audio_IsAvailable(&state) && state.iigsStream),
        "requested native SDL backend must actually open and become available");
    REQUIRE(M11_Audio_PlayDm1IigsSound(&state,0u,(uint8_t)left,(uint8_t)right,15u) &&
        M11_Audio_PlayDm1IigsSound(&state,8u,(uint8_t)right,(uint8_t)left,11u) &&
        M11_Audio_GetDm1IigsSnapshot(&state,&before) && before.triggers==2u &&
        before.left[0]==(left/15u)*15u && before.right[0]==(right/15u)*15u &&
        before.left[1]==(right/15u)*11u && before.right[1]==(left/15u)*11u,
        "actual native M11 triggers original sound zero/eight with separate source left/right channel volumes");
    REQUIRE(M11_Audio_SetVolumes(&state,60,40,90,80),"native transport accepts normal master and SFX volume settings");
#ifndef FIRESTAFF_NO_SDL_AUDIO
    if (state.iigsStream) {
        SDL_AudioStream *stream=(SDL_AudioStream *)state.iigsStream; SDL_AudioSpec input,output;
        float gain=SDL_GetAudioStreamGain(stream);
        REQUIRE(SDL_GetAudioStreamFormat(stream,&input,&output) && input.format==SDL_AUDIO_F32 &&
            input.channels==2 && input.freq==48000,"actual SDL native transport input is 48000Hz interleaved stereo float");
        REQUIRE(gain>0.146483f && gain<0.146486f,"native SDL stream applies master times SFX gain exactly once");
        for (unsigned waits=0u; waits<10u; ++waits) {
            SDL_Delay(20u);
            REQUIRE(M11_Audio_GetDm1IigsSnapshot(&state,&after),"locked native transport snapshot during callback playback");
            if (after.rendered_frames>before.rendered_frames) break;
        }
        REQUIRE(after.rendered_frames>before.rendered_frames && after.doc_scans>before.doc_scans,
            "actual SDL callback advances native source mixer within bounded dummy-device wait");
        REQUIRE(M11_Audio_SetHostPaused(&state,1) && SDL_AudioStreamDevicePaused(stream) &&
            state.hostResumeIigsStream && M11_Audio_GetDm1IigsSnapshot(&state,&before),
            "host pause owns native SDL device pause and captures locked playback state");
        SDL_Delay(40u);
        REQUIRE(M11_Audio_GetDm1IigsSnapshot(&state,&after) && after.rendered_frames==before.rendered_frames,
            "paused native device does not advance authentic DOC playback");
        REQUIRE(M11_Audio_SetHostPaused(&state,0) && !SDL_AudioStreamDevicePaused(stream) && !state.hostResumeIigsStream,
            "host resume restores native SDL playback ownership");
        for (unsigned waits=0u; waits<10u; ++waits) {
            SDL_Delay(20u);
            REQUIRE(M11_Audio_GetDm1IigsSnapshot(&state,&after),"locked native resume snapshot");
            if (after.rendered_frames>before.rendered_frames) break;
        }
        REQUIRE(after.rendered_frames>before.rendered_frames,"resumed SDL callback continues native source playback");
    }
#else
    REQUIRE(!require_backend,"native SDL backend was required but this test was built without SDL audio");
#endif
    REQUIRE(!state.playedMarkerCount && !state.originalSnd3LoadedCount && !state.soundPackLoadedCount,
        "native transport never substitutes procedural markers or PC sound banks");
    M11_Audio_Shutdown(&state); M11_Audio_Shutdown(&state);
    REQUIRE(!state.initialized && !state.iigsStream && !state.iigsMixer,"repeated shutdown releases native SDL stream and DOC owner");
    REQUIRE(M11_Audio_InitDm1Iigs(&state,program,program_size,bank,bank_size) &&
        M11_Audio_GetDm1IigsSnapshot(&state,&after) && !after.triggers,
        "actual native M11 transport can reinitialize authentic sources after shutdown");
    result=1;
cleanup:
    M11_Audio_Shutdown(&state); return result;
}

static int check_audio(const uint8_t *bank, size_t bankSize, const uint8_t *program, size_t programSize, unsigned edition) {
    DM1_V1_IigsAudioMetadata metadata, bad;
    DM1_V1_IigsPcmReceipt receipt, rejected;
    uint8_t metadataBytes[897], *aggregate=NULL, *raw=NULL, *doc=NULL;
    uint8_t zero[sizeof(rejected)]={0}, metadataZero[sizeof(bad)]={0};
    size_t used=0u, rawSize=0u;
    unsigned padding=0u, remapped=0u;
    int result=0;
    REQUIRE(bankSize==398713u && asset_memory_matches_md5(bank,bankSize,"1045a36952e64eb2bc7b4c7b2965b112") &&
        dm1_v1_iigs_audio_metadata_read(program,programSize,&metadata) &&
        metadata.program_version==(edition ? DM1_V1_IIGS_PROGRAM_G21 : DM1_V1_IIGS_PROGRAM_G20),
        "native sound tables bind authenticated original graphics and program edition");
    for (unsigned i=0u; i<34u; ++i) {
        const DM1_V1_IigsSoundSpec *s=&metadata.sounds[i]; uint8_t *p=metadataBytes+i*8u;
        put16(p,(unsigned)s->graphic_index); p[2]=s->a_unreferenced; p[3]=s->period; p[4]=s->priority;
        p[5]=s->b_unreferenced; p[6]=s->loud_distance; p[7]=s->soft_distance;
    }
    memcpy(metadataBytes+272u,metadata.distance_volume,625u);
    REQUIRE(asset_memory_matches_md5(metadataBytes,sizeof(metadataBytes),"920ac4a63f42b596fdb521c2de157287") &&
        memcmp(metadataBytes,program+59048u,272u)==0 && memcmp(metadataBytes+272u,program+56021u,625u)==0 &&
        metadata.sounds[20].graphic_index==537 && metadata.sounds[5].graphic_index==537,
        "all native program sound specs and distance bytes match independently source-audited original sections");
    aggregate=malloc(118433u); raw=malloc(65535u); doc=malloc(32770u);
    REQUIRE(aggregate && raw && doc,"bounded native audio test buffers");
    for (unsigned event=0u; event<34u; ++event) {
        unsigned graphic=(unsigned)metadata.sounds[event].graphic_index, code=0u;
        size_t count;
        REQUIRE(graphic!=538u && graphic!=548u && dm1_v1_iigs_graphics_read_raw(bank,bankSize,
            (uint16_t)graphic,raw,65535u,&rawSize),"all 34 native events select actual sound records and never placeholders");
        count=((size_t)raw[0]<<8)|raw[1];
        REQUIRE(count && count<=32767u && rawSize>=count+2u && rawSize-count-2u<=2u,
            "source record carries BE16 sample count and bounded alignment padding");
        padding|=1u<<(unsigned)(rawSize-count-2u);
        while ((256u<<code)<count+1u) ++code;
        REQUIRE(dm1_v1_iigs_audio_pcm_query(raw,rawSize,&receipt) && receipt.sample_count==count &&
            receipt.doc_byte_count==count+1u && receipt.table_size_code==code && receipt.resolution_code==code,
            "native DOC table size follows source power-of-two selection");
        memset(doc,0xa5,count+3u);
        REQUIRE(dm1_v1_iigs_audio_pcm_decode(raw,rawSize,doc+1u,count+1u,&receipt) &&
            doc[0]==0xa5u && doc[count+2u]==0xa5u && doc[count+1u]==0u,
            "DOC conversion appends one stop marker and respects output capacity");
        for (size_t i=0u; i<count; ++i) {
            unsigned value=((unsigned)raw[2u+i]+128u)&255u;
            if (!value) ++remapped;
            REQUIRE(doc[1u+i]==(value ? value : 1u),"every original signed sample retains source bias and reserved-zero remapping");
        }
        REQUIRE(used+8u+count+1u<=118433u,"bounded complete native sound aggregate");
        put16(aggregate+used,event); put16(aggregate+used+2u,graphic); put16(aggregate+used+4u,(unsigned)count);
        aggregate[used+6u]=(uint8_t)code; aggregate[used+7u]=(uint8_t)code;
        memcpy(aggregate+used+8u,doc+1u,count+1u); used+=8u+count+1u;
        REQUIRE(dm1_v1_iigs_audio_pcm_decode(raw,count+2u,doc+1u,count+1u,&receipt),
            "alignment bytes do not contribute audio or become mandatory payload");
        memset(&rejected,0xa5,sizeof(rejected)); memset(doc,0xa5,count+3u);
        REQUIRE(!dm1_v1_iigs_audio_pcm_decode(raw,rawSize,doc+1u,count,&rejected) &&
            memcmp(&rejected,zero,sizeof(rejected))==0 && doc[0]==0xa5u && doc[count+1u]==0xa5u &&
            !dm1_v1_iigs_audio_pcm_query(raw,count+1u,&rejected) && memcmp(&rejected,zero,sizeof(rejected))==0,
            "short output and truncated original audio reject without stale receipt");
    }
    /* Independent SOUND.C F2189 translation, preserving all 34 source
     * event identities (including reused 537), not production output. */
    REQUIRE(used==118433u && asset_memory_matches_md5(aggregate,used,"f9ba65fb99a68b7652ab5b47e50a6cda") &&
        padding==7u && remapped==22u,"complete native DOC output matches independent original-source oracle");
    REQUIRE(!dm1_v1_iigs_audio_pcm_query(bank,bankSize,&rejected),"excessive record size rejects before decoding");
    raw[0]=raw[1]=0u;
    REQUIRE(!dm1_v1_iigs_audio_pcm_query(raw,rawSize,&rejected),"original-derived zero sample count rejects");
    raw[0]=0x80u; raw[1]=0u;
    REQUIRE(!dm1_v1_iigs_audio_pcm_query(raw,rawSize,&rejected),"original-derived sample count above native signed range rejects");
    memset(&bad,0xa5,sizeof(bad));
    REQUIRE(!dm1_v1_iigs_audio_metadata_read(program,programSize-1u,&bad) &&
        memcmp(&bad,metadataZero,sizeof(bad))==0,"truncated original program cannot publish audio metadata");
    REQUIRE(check_volume(&metadata),"native distance, quantization and pending semantics");
    REQUIRE(check_doc_mixer(bank,bankSize,program,programSize,&metadata),"authentic native DOC stereo playback and residency");
    REQUIRE(check_native_transport(bank,bankSize,program,programSize,&metadata),"actual M11 native SDL transport lifecycle");
    result=1;
cleanup:
    free(doc); free(raw); free(aggregate); return result;
}
typedef struct { uint8_t *bank, *program; size_t bankSize, programSize; } Files;
static int collect(const char *path, const uint8_t *bytes, size_t size, void *user) {
    Files *f=user; uint8_t **out=NULL; size_t *length=NULL;
    if (!strcmp(path,"GRAPHICS.GAME")) { out=&f->bank; length=&f->bankSize; }
    if (!strcmp(path,"DUNGEON.MASTER")) { out=&f->program; length=&f->programSize; }
    if (!out) return 0;
    if (*out || size>400000u || !(*out=malloc(size))) return 1;
    memcpy(*out,bytes,size); *length=size; return 0;
}
static int check_disk(const char *path, unsigned edition) {
    static const char *const hashes[]={"15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    FILE *file=NULL; uint8_t *disk=NULL; Files f={0}; int result=0;
    file=fopen(path,"rb");
    REQUIRE(file && fseek(file,0,SEEK_END)==0 && ftell(file)==819200 && fseek(file,0,SEEK_SET)==0,"bounded original disk size");
    disk=malloc(819200u); REQUIRE(disk && fread(disk,1,819200u,file)==819200u,"read bounded original disk");
    { int closed=fclose(file); file=NULL; REQUIRE(closed==0,"close original disk"); }
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]) &&
        firestaff_prodos_visit_files(disk,819200u,collect,&f)>0 && f.bank && f.program &&
        check_audio(f.bank,f.bankSize,f.program,f.programSize,edition),"both authentic native editions pass complete source audio-data checks");
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"original disk bytes remain unchanged"); result=1;
cleanup:
    if (file) fclose(file);
    free(f.bank); free(f.program); free(disk); return result;
}
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s <original-2.0.po> <original-2.1.po>\n",argv[0]); return 2; }
    if (!check_disk(argv[1],0u) || !check_disk(argv[2],1u)) return 1;
    puts("PASS: original native DOC stereo oracle and transport; physical hardware audio parity unverified"); return 0;
}
