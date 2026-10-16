#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_installation.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr,"FAIL: %s\n",message); goto cleanup; } \
} while (0)
static int check_owner(const DM1_V1_IigsInstallation *installation, const char *prefix, int version) {
    static const char *const names[]={"GRAPHICS.GAME","DUNGEON.MASTER","SYSTEM/START","DUNGEON.GAME"};
    const char *paths[]={installation->graphics_path,installation->program_path,installation->start_path,installation->dungeon_path};
    const char *hashes[]={"1045a36952e64eb2bc7b4c7b2965b112",version==20 ?
        "51b9d22c7418651ee49ad77f37c80a3c" : "eccc49e7b29ee233cfb10615e8ec86f3",
        "3bcae225346ac24fb1975aa6d93f0fa6","e10959de5ce74e48a17e2cc84d1c0b92"};
    char expected[ASSET_PATH_MAX]; int result=0;
    for (unsigned i=0u; i<4u; ++i)
        REQUIRE(snprintf(expected,sizeof(expected),"%s::%s",prefix,names[i])<(int)sizeof(expected) &&
            strcmp(paths[i],expected)==0 && asset_file_matches_md5(paths[i],hashes[i]),
            "installation binds every authenticated component to the exact same native disk owner");
    REQUIRE((int)installation->metadata.program_version==version && installation->world.dungeon_id==10u &&
        installation->world.decoded_size==33444u && installation->world.map_count==14u &&
        installation->world.start_map_index==0u && installation->world.start_x==1u &&
        installation->world.start_y==3u && installation->world.start_direction==2u,
        "same-owner receipt preserves authentic executable edition and original main-world location");
    result=1;
cleanup:
    return result;
}
static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1]<<8); }
static void put16(uint8_t *p, unsigned value) { p[0]=(uint8_t)value; p[1]=(uint8_t)(value>>8); }
static uint8_t *entry_named(uint8_t *disk, unsigned block, const char *name) {
    if (block>=1600u) return NULL;
    for (unsigned slot=1u; slot<13u; ++slot) {
        uint8_t *entry=disk+(size_t)block*512u+4u+slot*39u;
        if ((entry[0]&15u)==strlen(name) && memcmp(entry+1u,name,strlen(name))==0) return entry;
    }
    return NULL;
}
static int write_private(const char *path, const uint8_t *disk) {
    FILE *file=fopen(path,"wb"); int ok;
    if (!file) return 0;
    ok=fwrite(disk,1,819200u,file)==819200u;
    if (fclose(file)!=0) ok=0;
    return ok;
}
static int check_negative_owners(const char *original, const char *scratch) {
    static const char *const labels[]={"a-missing-program.po","b-missing-start.po","c-bonus-world.po"};
    DM1_V1_IigsInstallation installation;
    uint8_t zero[sizeof(installation)]={0};
    uint8_t *disk=NULL; size_t size=0u;
    char path[ASSET_PATH_MAX], graphics[ASSET_PATH_MAX], expected[ASSET_PATH_MAX];
    int result=0;
    for (unsigned mode=0u; mode<3u; ++mode) {
        uint8_t *entry, *header;
        REQUIRE(asset_read_path_alloc(original,&disk,&size) && size==819200u &&
            asset_memory_matches_md5(disk,size,"15e3f954f408612fd2f15a474a182f9f"),
            "negative framing starts from unchanged original G20 disk");
        header=disk+2u*512u+4u;
        if (mode==0u) {
            entry=entry_named(disk,2u,"DUNGEON.MASTER");
            REQUIRE(entry,"locate authentic original program entry");
            entry[0]&=15u; put16(header+33u,le16(header+33u)-1u);
        } else if (mode==1u) {
            uint8_t *system=entry_named(disk,2u,"SYSTEM"); unsigned block;
            REQUIRE(system,"locate original native SYSTEM directory"); block=le16(system+17u);
            entry=entry_named(disk,block,"START"); REQUIRE(entry,"locate authentic original START entry");
            entry[0]&=15u; header=disk+(size_t)block*512u+4u;
            put16(header+33u,le16(header+33u)-1u);
        } else {
            uint8_t *bonus=entry_named(disk,2u,"DUNGEON.BONUS");
            entry=entry_named(disk,2u,"DUNGEON.GAME");
            REQUIRE(entry && bonus,"locate both authentic native dungeon entries");
            /* Reframe unchanged original BONUS bytes as the selected main
             * filename, deactivating its former entry to avoid block aliases.
             * No payload bytes, bitmap bits or blocks are synthesized. */
            memcpy(entry,bonus,39u); entry[0]=(uint8_t)((entry[0]&0xf0u)|12u);
            memset(entry+1u,0,15u); memcpy(entry+1u,"DUNGEON.GAME",12u); bonus[0]&=15u;
            put16(header+33u,le16(header+33u)-1u);
        }
        REQUIRE(snprintf(path,sizeof(path),"%s/%s",scratch,labels[mode])<(int)sizeof(path) && write_private(path,disk),
            "write only private original-derived filesystem metadata negative");
        free(disk); disk=NULL;
        REQUIRE(snprintf(expected,sizeof(expected),"%s::GRAPHICS.GAME",path)<(int)sizeof(expected) &&
            asset_find_by_md5(path,"1045a36952e64eb2bc7b4c7b2965b112",graphics,sizeof(graphics),0) &&
            strcmp(graphics,expected)==0,"negative bundle still exposes its valid original graphics through native filesystem intake");
        if (mode==2u) {
            REQUIRE(snprintf(expected,sizeof(expected),"%s::DUNGEON.GAME",path)<(int)sizeof(expected) &&
                asset_file_matches_md5(expected,"550efd94ecb941d44e65def1b758587c"),
                "foreign default world contains unchanged authentic BONUS bytes rather than malformed media");
        }
        memset(&installation,0xa5,sizeof(installation));
        REQUIRE(dm1_v1_iigs_installation_find(path,0,&installation)==0 &&
            memcmp(&installation,zero,sizeof(installation))==0,
            "explicit incomplete or foreign-world disk cannot borrow components from neighboring valid editions");
    }
    result=1;
cleanup:
    free(disk); return result;
}
int main(int argc, char **argv) {
    DM1_V1_IigsInstallation installation;
    uint8_t zero[sizeof(installation)]={0};
    char prefix[ASSET_PATH_MAX], paths[16][ASSET_PATH_MAX];
    int result=1;
    if (argc!=6) {
        fprintf(stderr,"usage: %s <2.0.po> <2.1.po> <software.7z> <unlabelled.zip> <private-directory>\n",argv[0]); return 2;
    }
    REQUIRE(asset_collect_paths_by_md5(argv[3],"1045a36952e64eb2bc7b4c7b2965b112",paths,16u,0)==6,
        "original software preservation archive exposes all six byte-identical graphics owners");
    for (int edition=0; edition<2; ++edition) {
        int version=edition ? 21 : 20;
        REQUIRE(dm1_v1_iigs_installation_find(argv[1+edition],version,&installation)==1 &&
            check_owner(&installation,argv[1+edition],version),"explicit original raw disk owns every installation component");
        memset(&installation,0xa5,sizeof(installation));
        REQUIRE(dm1_v1_iigs_installation_find(argv[1+edition],edition ? 20 : 21,&installation)==0 &&
            memcmp(&installation,zero,sizeof(installation))==0,"explicit requested edition cannot be supplied by a neighboring original disk");
        REQUIRE(snprintf(prefix,sizeof(prefix),"%s::Dungeon Master for Apple IIGS %s (English) Original (Not working).po",
            argv[3],edition ? "2.1" : "2.0")<(int)sizeof(prefix) &&
            dm1_v1_iigs_installation_find(argv[3],version,&installation)==1 &&
            check_owner(&installation,prefix,version),"archive collection skips incomplete shared-graphics owners and admits each exact original edition");
    }
    for (int edition=0; edition<2; ++edition) {
        int version=edition ? 21 : 20;
        char graphics[ASSET_PATH_MAX];
        REQUIRE(snprintf(prefix,sizeof(prefix),"%s::Dungeon Master for Apple IIGS %s (English) Original (Not working).po",
            argv[3],edition ? "2.1" : "2.0")<(int)sizeof(prefix),"bounded explicit native image locator");
        REQUIRE(dm1_v1_iigs_installation_find(prefix,version,&installation)==1 &&
            check_owner(&installation,prefix,version),"selected virtual PO keeps its exact complete native owner");
        REQUIRE(snprintf(graphics,sizeof(graphics),"%s::GRAPHICS.GAME",prefix)<(int)sizeof(graphics) &&
            dm1_v1_iigs_installation_find(graphics,version,&installation)==1 &&
            check_owner(&installation,prefix,version),"selected virtual graphics locator owns every native companion");
        memset(&installation,0xa5,sizeof(installation));
        REQUIRE(dm1_v1_iigs_installation_find(graphics,edition ? 20 : 21,&installation)==0 &&
            memcmp(&installation,zero,sizeof(installation))==0,
            "selected virtual owner cannot borrow the other executable edition from its outer archive");
    }
    REQUIRE(snprintf(prefix,sizeof(prefix),"%s::media.po",argv[4])<(int)sizeof(prefix) &&
        dm1_v1_iigs_installation_find(argv[4],0,&installation)==1 && check_owner(&installation,prefix,20),
        "authentic installation admission does not depend on Original or canonical archive/member filenames");
    REQUIRE(check_negative_owners(argv[1],argv[5]),"valid native filesystem negatives enforce complete same-owner installation");
    REQUIRE(dm1_v1_iigs_installation_find(argv[5],0,&installation)==1 &&
        installation.metadata.program_version==DM1_V1_IIGS_PROGRAM_G20 &&
        !strstr(installation.graphics_path,"a-missing-program.po::") &&
        !strstr(installation.graphics_path,"b-missing-start.po::") &&
        !strstr(installation.graphics_path,"c-bonus-world.po::"),
        "directory discovery continues beyond earlier incomplete owners to a complete original bundle");
    REQUIRE(asset_file_matches_md5(argv[1],"15e3f954f408612fd2f15a474a182f9f") &&
        asset_file_matches_md5(argv[2],"d38f00e5ad8516553c5cba3214b29bef"),
        "installation inspection and private negative framing leave both original disks unchanged");
    puts("PASS: exact native installation ownership for both originals; gameplay/runtime admission unverified"); result=0;
cleanup:
    return result;
}
