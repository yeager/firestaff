#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_dungeon.h"
#include "dm1_v1_iigs_metadata.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
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
static int check_native_profile(const uint8_t *body, size_t size,
    const DM1_V1_IigsMetadata *metadata, const struct GameWorld_Compat *control) {
    struct GameWorld_Compat *native=calloc(1,sizeof(*native)), *clone=calloc(1,sizeof(*clone));
    DM1_V1_IigsCreatureInfo copied[27];
    int loaded=0, cloned=0, result=0;
    REQUIRE(native && clone,"bounded native profile worlds");
    memcpy(copied,metadata->creature_info,sizeof(copied));
    REQUIRE(!F0882G_WORLD_InitFromIigsDungeonDatBuffer_Compat(body,(int)size,0xF1A5u,copied,26u,native) &&
        !native->iigsCreatureProfileBound,"native constructor requires all 27 original creature rows");
    loaded=F0882G_WORLD_InitFromIigsDungeonDatBuffer_Compat(body,(int)size,0xF1A5u,copied,27u,native);
    memset(copied,0,sizeof(copied));
    REQUIRE(loaded && native->iigsCreatureProfileBound && !control->iigsCreatureProfileBound &&
        native->party.mapX==control->party.mapX && native->party.mapY==control->party.mapY &&
        memcmp(native->iigsCreatureInfo,metadata->creature_info,sizeof(copied))==0,
        "actual native world owns authentic creature rows independently of caller lifetime");
    for (int i=0; i<27; ++i) {
        const DM1_V1_IigsCreatureInfo *r=&metadata->creature_info[i];
        struct DM1CreatureInfo_Compat actual, pc, baseline;
        const struct CreatureBehaviorProfile_Compat *profile=F0890e_ORCH_GetCreatureProfileForWorld_Compat(native,i);
        REQUIRE(F0890d_ORCH_GetCreatureInfoForWorld_Compat(native,i,&actual),"native creature accessor");
#define SAME(host,field) REQUIRE(actual.host==r->field,"native typed creature accessor retains original " #field)
        SAME(creatureAspectIndex,creature_aspect_index); SAME(attackSoundOrdinal,attack_sound_ordinal);
        SAME(attributes,attributes); SAME(graphicInfo,graphic_info); SAME(movementTicks,movement_ticks);
        SAME(attackTicks,attack_ticks); SAME(defense,defense); SAME(baseHealth,base_health);
        SAME(attack,attack); SAME(poisonAttack,poison_attack); SAME(dexterity,dexterity);
        SAME(ranges,ranges); SAME(properties,properties); SAME(resistances,resistances);
        SAME(animationTicks,animation_ticks); SAME(woundProbabilities,wound_probabilities); SAME(attackType,attack_type);
#undef SAME
        REQUIRE(profile && profile->sightRange==(r->ranges&15) && profile->smellRange==((r->ranges>>8)&15) &&
            profile->movementTicks==r->movement_ticks && profile->attackTicks==r->attack_ticks &&
            profile->baseAttack==r->attack && profile->baseDefense==r->defense && profile->baseHealth==r->base_health &&
            profile->dexterity==r->dexterity && profile->poisonAttack==r->poison_attack &&
            profile->attackType==r->attack_type && profile->woundProbabilities==r->wound_probabilities &&
            profile->attributes==r->attributes && profile->properties==r->properties,
            "all authentic native behavior fields derive from scoped source rows");
        memset(&pc,0,sizeof(pc)); memset(&baseline,0,sizeof(baseline));
        REQUIRE(F0890d_ORCH_GetCreatureInfoForWorld_Compat(control,i,&pc) &&
            F0890d_ORCH_GetCreatureInfoPc34Compat(i,&baseline) && memcmp(&pc,&baseline,sizeof(pc))==0 &&
            F0890e_ORCH_GetCreatureProfileForWorld_Compat(control,i)==CREATURE_GetProfile_Compat(i),
            "interleaved default world keeps original PC behavior without global profile mutation");
    }
    cloned=F0880b_WORLD_Clone_Compat(native,clone);
    REQUIRE(cloned && clone->iigsCreatureProfileBound &&
        memcmp(clone->iigsCreatureInfo,native->iigsCreatureInfo,sizeof(copied))==0 &&
        memcmp(clone->iigsCreatureProfiles,native->iigsCreatureProfiles,sizeof(native->iigsCreatureProfiles))==0,
        "world clone retains copied native rows and derived profiles");
    F0883_WORLD_Free_Compat(native); loaded=0;
    REQUIRE(!native->iigsCreatureProfileBound && clone->iigsCreatureProfileBound &&
        F0890e_ORCH_GetCreatureProfileForWorld_Compat(clone,0)->baseHealth==metadata->creature_info[0].base_health,
        "free resets original profile while independent clone retains native behavior");
    result=1;
cleanup:
    if (cloned) F0883_WORLD_Free_Compat(clone);
    if (loaded) F0883_WORLD_Free_Compat(native);
    free(clone); free(native); return result;
}

static int check_world(const uint8_t *source, size_t size, const OriginalWorld *expected,
    const DM1_V1_IigsMetadata *metadata) {
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
    REQUIRE(check_native_profile(out+1u,decoded,metadata,world),"actual main/bonus native creature world profile");
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

typedef struct { uint8_t *bytes[3]; size_t sizes[3]; int valid; } WorldVisit;
static int visit_world(const char *path, const uint8_t *bytes, size_t size, void *user) {
    WorldVisit *visit=user;
    for (unsigned i=0u; i<3u; ++i) if (strcmp(path,i<2u ? worlds[i].name : "DUNGEON.MASTER")==0) {
        if (visit->bytes[i] || size>300000u || !(visit->bytes[i]=malloc(size))) { visit->valid=0; return 1; }
        memcpy(visit->bytes[i],bytes,size); visit->sizes[i]=size;
    }
    return 0;
}
static int check_disk(const char *path, unsigned edition) {
    static const char *const hashes[]={"15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    FILE *file=NULL;
    uint8_t *disk=NULL;
    WorldVisit visit={{NULL,NULL,NULL},{0u,0u,0u},1};
    DM1_V1_IigsMetadata metadata;
    int result=0;
    file=fopen(path,"rb");
    REQUIRE(file && fseek(file,0,SEEK_END)==0 && ftell(file)==819200 && fseek(file,0,SEEK_SET)==0,
        "bounded authentic original IIGS disk size");
    disk=malloc(819200u);
    REQUIRE(disk && fread(disk,1,819200u,file)==819200u,"read bounded original disk bytes");
    { int closed=fclose(file); file=NULL; REQUIRE(closed==0,"close original disk"); }
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"unchanged original IIGS disk identity");
    REQUIRE(firestaff_prodos_visit_files(disk,819200u,visit_world,&visit)>0 && visit.valid && visit.bytes[0] && visit.bytes[1] && visit.bytes[2],
        "both native original main and bonus worlds pass bounded decoding and complete LE parsing");
    REQUIRE(dm1_v1_iigs_metadata_read(visit.bytes[2],visit.sizes[2],&metadata) &&
        metadata.program_version==(edition ? DM1_V1_IIGS_PROGRAM_G21 : DM1_V1_IIGS_PROGRAM_G20),
        "same original disk supplies its authenticated native creature program rows");
    for (unsigned i=0u; i<2u; ++i)
        REQUIRE(check_world(visit.bytes[i],visit.sizes[i],&worlds[i],&metadata),
            "main and bonus actual worlds retain authentic native creature profiles");
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"original disk bytes remain unchanged");
    result=1;
cleanup:
    if (file) fclose(file);
    for (unsigned i=0u; i<3u; ++i) free(visit.bytes[i]);
    free(disk);
    return result;
}
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s <original-2.0.po> <original-2.1.po>\n",argv[0]); return 2; }
    if (!check_disk(argv[1],0u) || !check_disk(argv[2],1u)) return 1;
    puts("PASS: original IIGS main/bonus dungeon bodies, checksums and LE world parsing; runtime admission unverified");
    return 0;
}
