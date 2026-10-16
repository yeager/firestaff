#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_dungeon.h"
#include "firestaff_prodos.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr,"FAIL: %s\n",message); goto cleanup; } \
} while (0)
typedef struct {
    const char *name, *source_hash, *body_hash;
    size_t source_size, body_size, checksum_bit;
    unsigned id, maps, x, y, width, height, raw_bytes, text_words, heads, checksum;
} OriginalWorld;
/* Independent bit-at-a-time DECOMPDU.C GS:257-395 oracle; no production
 * decompressor output was used to choose these body identities. */
static const OriginalWorld worlds[]={
    {"DUNGEON.GAME","e10959de5ce74e48a17e2cc84d1c0b92","3dd93c3b0270d086812a533e9da0b630",
     25006u,33444u,200043u,10u,14u,1u,3u,19u,19u,12366u,1749u,1682u,10323u},
    {"DUNGEON.BONUS","550efd94ecb941d44e65def1b758587c","6cb06136f5d2710f342bea054bed3f52",
     4958u,6562u,39652u,50u,3u,4u,1u,16u,21u,1180u,390u,381u,14537u}
};
static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1]<<8); }

static int rejected(const uint8_t *source, size_t size, uint8_t *guarded, size_t capacity) {
    DM1_V1_IigsDungeonReceipt receipt;
    uint8_t zero[sizeof(receipt)]={0};
    size_t decoded=123u;
    memset(guarded,0xa5,capacity+2u); memset(&receipt,0xa5,sizeof(receipt));
    return !dm1_v1_iigs_dungeon_decode(source,size,guarded+1u,capacity,&decoded,&receipt) &&
        !decoded && memcmp(&receipt,zero,sizeof(receipt))==0 &&
        guarded[0]==0xa5u && guarded[capacity+1u]==0xa5u;
}
static int check_world(const uint8_t *source, size_t size, const OriginalWorld *expected) {
    uint8_t *out=NULL, *badout=NULL, *changed=NULL;
    DM1_V1_IigsDungeonReceipt receipt;
    struct DungeonDatState_Compat state;
    struct DungeonThings_Compat things;
    struct GameWorld_Compat *world=NULL;
    size_t decoded=0u, required=0u;
    unsigned checksum=0u;
    int result=0, world_loaded=0;
    memset(&state,0,sizeof(state)); memset(&things,0,sizeof(things));
    REQUIRE(size==expected->source_size && asset_memory_matches_md5(source,size,expected->source_hash),
        "native original compressed world identity");
    REQUIRE(dm1_v1_iigs_dungeon_query(source,size,&required) && required==expected->body_size,
        "original native LE wrapper reports independently verified output length");
    out=malloc(required+2u); badout=malloc(required+2u); changed=malloc(size);
    REQUIRE(out && badout && changed,"bounded original dungeon test buffers");
    memset(out,0xa5,required+2u);
    REQUIRE(dm1_v1_iigs_dungeon_decode(source,size,out+1u,required,&decoded,&receipt) &&
        decoded==required && out[0]==0xa5u && out[required+1u]==0xa5u &&
        asset_memory_matches_md5(out+1u,decoded,expected->body_hash),
        "native dungeon body matches complete independent original-bitstream oracle");
    /* READWRIT.C F0421:265-289 sums unsigned source bytes modulo 65536;
     * LOADSAVE.C F0434 reads the native LE trailer after all world sections. */
    for (size_t i=0; i<decoded-2u; ++i) checksum=(checksum+out[i+1u])&65535u;
    REQUIRE(checksum==expected->checksum && le16(out+1u+decoded-2u)==checksum,
        "complete original dungeon carries its source byte-sum checksum trailer");
    REQUIRE(receipt.decoded_size==decoded && receipt.dungeon_id==expected->id &&
        receipt.map_count==expected->maps && receipt.start_map_index==0u &&
        receipt.start_x==expected->x && receipt.start_y==expected->y && receipt.start_direction==2u &&
        receipt.first_map_width==expected->width && receipt.first_map_height==expected->height &&
        receipt.raw_map_data_byte_count==expected->raw_bytes &&
        receipt.text_data_word_count==expected->text_words && receipt.square_first_thing_count==expected->heads,
        "native world receipt preserves original maps, location and section counts");
    REQUIRE(F0504_DUNGEON_LoadTailBuffer_Compat(out+1u,(int)decoded,&state,&things) &&
        state.loaded && state.tilesLoaded && things.loaded && state.maps && state.tiles &&
        state.header.mapCount==expected->maps && state.maps[0].width==expected->width &&
        state.maps[0].height==expected->height && state.dungeonColumnCount==receipt.column_count &&
        state.fileSize==(long)decoded,
        "existing complete LE world parser loads the native map layout and complete body extent");
    for (unsigned i=0u; i<expected->maps; ++i)
        REQUIRE(state.tiles[i].squareData && state.tiles[i].squareCount==
            (int)state.maps[i].width*state.maps[i].height,"all native map tile arrays are complete");
    for (unsigned i=0u; i<16u; ++i)
        REQUIRE(things.thingCounts[i]==state.header.thingCounts[i],"all original Thing section counts survive LE parsing");
    /* Exercise the actual world constructor, beyond the structural parser.
     * Native GS STARTUP2.C:1381-1393 decodes the original party location;
     * this does not establish M11/CLI admission or platform gameplay rules. */
    world=calloc(1,sizeof(*world));
    REQUIRE(world,"allocate actual native world test state");
    world_loaded=F0882_WORLD_InitFromDungeonDatBuffer_Compat(
        out+1u,(int)decoded,0xF1A5U,world);
    REQUIRE(world_loaded==1 && world->ownsDungeon &&
        world->dungeon && world->things && world->partyMapIndex==0 &&
        world->party.mapIndex==0 && world->party.mapX==(int)expected->x &&
        world->party.mapY==(int)expected->y && world->party.direction==2 &&
        world->party.championCount==0,
        "actual world constructor preserves original GS first map, location and empty party");
    REQUIRE(rejected(source,size,badout,required-1u),"insufficient output capacity rejects without stale receipt or overflow");
    REQUIRE(rejected(source,size-1u,badout,required) && rejected(source,27u,badout,required),
        "truncated original payload and frequency table reject safely");
    memcpy(changed,source,size);
    /* Flip the final literal bit of the first checksum byte, independently
     * located in each original compressed stream. Every body byte before the
     * trailer remains identical, so this specifically tests checksum failure. */
    changed[expected->checksum_bit/8u]^=(uint8_t)(1u<<(7u-expected->checksum_bit%8u));
    REQUIRE(rejected(changed,size,badout,required) &&
        memcmp(badout+1u,out+1u,required-2u)==0 &&
        memcmp(badout+1u+required-2u,out+1u+required-2u,2u)!=0,
        "checksum-only mutation rejects an otherwise unchanged original world");
    memcpy(changed,source,size);
    changed[2]=(uint8_t)(required-2u); changed[3]=(uint8_t)((required-2u)>>8);
    changed[4]=changed[5]=0u;
    REQUIRE(rejected(changed,size,badout,required),"original-derived declared trailer omission remains unsupported");
    memcpy(changed,source,size); changed[0]=source[1]; changed[1]=source[0];
    REQUIRE(!dm1_v1_iigs_dungeon_query(changed,size,&decoded) && !decoded,
        "foreign byte-order wrapper cannot masquerade as native IIGS input");
    memcpy(changed,source,size); memset(changed+2u,0xff,4u);
    REQUIRE(!dm1_v1_iigs_dungeon_query(changed,size,&decoded) && !decoded,
        "original-derived excessive declared allocation rejects");
    REQUIRE(asset_memory_matches_md5(source,size,expected->source_hash),"original compressed source remains unchanged");
    result=1;
cleanup:
    if (world_loaded==1) F0883_WORLD_Free_Compat(world);
    free(world);
    F0504_DUNGEON_FreeThingData_Compat(&things); F0500_DUNGEON_FreeDatHeader_Compat(&state);
    free(changed); free(badout); free(out);
    return result;
}

typedef struct { unsigned found; int valid; } WorldVisit;
static int visit_world(const char *path, const uint8_t *bytes, size_t size, void *user) {
    WorldVisit *visit=user;
    for (unsigned i=0u; i<2u; ++i) if (strcmp(path,worlds[i].name)==0) {
        if ((visit->found&(1u<<i)) || !check_world(bytes,size,&worlds[i])) { visit->valid=0; return 1; }
        visit->found|=1u<<i;
    }
    return 0;
}
static int check_disk(const char *path, unsigned edition) {
    static const char *const hashes[]={"15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    FILE *file=NULL;
    uint8_t *disk=NULL;
    WorldVisit visit={0u,1};
    int result=0;
    file=fopen(path,"rb");
    REQUIRE(file && fseek(file,0,SEEK_END)==0 && ftell(file)==819200 && fseek(file,0,SEEK_SET)==0,
        "bounded authentic original IIGS disk size");
    disk=malloc(819200u);
    REQUIRE(disk && fread(disk,1,819200u,file)==819200u,"read bounded original disk bytes");
    { int closed=fclose(file); file=NULL; REQUIRE(closed==0,"close original disk"); }
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"unchanged original IIGS disk identity");
    REQUIRE(firestaff_prodos_visit_files(disk,819200u,visit_world,&visit)>0 && visit.valid && visit.found==3u,
        "both native original main and bonus worlds pass bounded decoding and complete LE parsing");
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"original disk bytes remain unchanged");
    result=1;
cleanup:
    if (file) fclose(file);
    free(disk);
    return result;
}
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s <original-2.0.po> <original-2.1.po>\n",argv[0]); return 2; }
    if (!check_disk(argv[1],0u) || !check_disk(argv[2],1u)) return 1;
    puts("PASS: original IIGS main/bonus dungeon bodies, checksums and LE world parsing; runtime admission unverified");
    return 0;
}
