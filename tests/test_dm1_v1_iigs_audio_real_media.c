#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_audio.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "firestaff_prodos.h"
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
    puts("PASS: original native sound data and channel rules; audible output and game startup unverified"); return 0;
}
