#include "asset_find_by_hash.h"
#include "firestaff_prodos.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* All positive bytes come from the two unchanged original IIGS images.
 * Negative cases alter only a private copy of their filesystem metadata. */
enum { ORIGINAL_BYTES = 819200, BLOCK_BYTES = 512 };
#define CHECK(condition, message) do { \
    if (!(condition)) { fprintf(stderr,"FAIL: %s\n",message); goto cleanup; } \
} while (0)

typedef struct {
    int edition;
    int calls;
    unsigned found;
    int invalid;
} OriginalVisit;

static const char *const names[]={
    "GRAPHICS.GAME","DUNGEON.GAME","SYSTEM/START","DUNGEON.MASTER"};
static const char *const hashes[]={
    "1045a36952e64eb2bc7b4c7b2965b112","e10959de5ce74e48a17e2cc84d1c0b92",
    "3bcae225346ac24fb1975aa6d93f0fa6","51b9d22c7418651ee49ad77f37c80a3c"};
static const size_t sizes[]={398713u,25006u,14603u,232158u};
static const char *original_hash(size_t file, int edition) {
    return file==3u && edition ? "eccc49e7b29ee233cfb10615e8ec86f3" : hashes[file];
}
static size_t original_size(size_t file, int edition) {
    return file==3u && edition ? 231865u : sizes[file];
}

static int check_original_file(const char *path, const uint8_t *bytes,
                                size_t size, void *user) {
    OriginalVisit *visit=user;
    ++visit->calls;
    if (!path || !path[0] || path[0]=='/' || !bytes) { visit->invalid=1; return 1; }
    for (size_t i=0; i<4u; ++i) {
        if (strcmp(path,names[i])==0) {
            const char *hash=original_hash(i,visit->edition);
            size_t expected=original_size(i,visit->edition);
            if ((visit->found & (1u<<i)) || size!=expected ||
                !asset_memory_matches_md5(bytes,size,hash)) visit->invalid=1;
            visit->found |= 1u<<i;
        }
    }
    return visit->invalid;
}

static int count_file(const char *path, const uint8_t *bytes,
                       size_t size, void *user) {
    int *calls=user;
    (void)path; (void)bytes; (void)size;
    ++*calls;
    return 0;
}

static int stop_file(const char *path, const uint8_t *bytes,
                      size_t size, void *user) {
    (void)count_file(path,bytes,size,user);
    return 1;
}

/* Locate an entry in the authenticated original's first root directory block.
 * This is a mutation locator, not an alternative filesystem decoder. */
static uint8_t *original_root_entry(uint8_t *image, const char *name) {
    for (size_t i=1u; i<13u; ++i) {
        uint8_t *entry=image+2u*BLOCK_BYTES+4u+i*39u;
        size_t length=entry[0]&15u;
        if (length==strlen(name) && memcmp(entry+1,name,length)==0) return entry;
    }
    return NULL;
}

static int check_disk(const char *path, int edition) {
    static const char *const diskHashes[]={
        "15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    uint8_t *image=NULL, *corrupt=NULL, *entry;
    FILE *file=NULL;
    OriginalVisit visit;
    int calls, delivered, result=0;
    size_t key;
    char actual[33];
    CHECK(asset_file_md5_hex(path,actual) && strcmp(actual,diskHashes[edition])==0,
        "original IIGS disk MD5 identity");
    file=fopen(path,"rb");
    CHECK(file && fseek(file,0,SEEK_END)==0 && ftell(file)==ORIGINAL_BYTES &&
        fseek(file,0,SEEK_SET)==0,"bounded original IIGS disk size");
    image=malloc(ORIGINAL_BYTES); corrupt=malloc(ORIGINAL_BYTES);
    CHECK(image && corrupt && fread(image,1,ORIGINAL_BYTES,file)==ORIGINAL_BYTES,
        "read unchanged original IIGS disk into bounded memory");
    {
        int closed=fclose(file); file=NULL;
        CHECK(closed==0,"close original disk");
    }
    CHECK(asset_memory_matches_md5(image,ORIGINAL_BYTES,diskHashes[edition]),
        "retained original disk bytes match authenticated source");
    memset(&visit,0,sizeof(visit)); visit.edition=edition;
    delivered=firestaff_prodos_visit_files(image,ORIGINAL_BYTES,check_original_file,&visit);
    CHECK(delivered==visit.calls && delivered>=4 && !visit.invalid && visit.found==15u,
        "root and subdirectory files retain exact original sizes and content hashes");
    /* Deleted slots may retain names/pointers. Reframe only an unused
     * directory slot with an inactive copy of original metadata; all game
     * bytes and the declared active count remain unchanged. */
    memcpy(corrupt,image,ORIGINAL_BYTES);
    entry=original_root_entry(corrupt,"BAD.BLOCKS");
    CHECK(entry && corrupt[2u*BLOCK_BYTES+4u+12u*39u]==0,
        "locate an original directory entry and unused slot");
    memcpy(corrupt+2u*BLOCK_BYTES+4u+12u*39u,entry,39u);
    corrupt[2u*BLOCK_BYTES+4u+12u*39u]&=15u;
    memset(&visit,0,sizeof(visit)); visit.edition=edition;
    CHECK(firestaff_prodos_visit_files(corrupt,ORIGINAL_BYTES,check_original_file,&visit)==delivered &&
        !visit.invalid && visit.found==15u,
        "inactive directory metadata cannot hide authenticated original game files");
    calls=0;
    CHECK(firestaff_prodos_visit_files(image,ORIGINAL_BYTES,stop_file,&calls)==1 && calls==1,
        "visitor stop delivers exactly one transient original file");
    calls=0;
    CHECK(firestaff_prodos_visit_files(image,ORIGINAL_BYTES-1u,count_file,&calls)==-1 && calls==0,
        "partial final original block rejects before callbacks");
    CHECK(firestaff_prodos_visit_files(image,2u*BLOCK_BYTES,count_file,&calls)==-1 && calls==0,
        "original truncated before root directory rejects before callbacks");
    memcpy(corrupt,image,ORIGINAL_BYTES);
    corrupt[2u*BLOCK_BYTES+2u]=2u; corrupt[2u*BLOCK_BYTES+3u]=0u;
    CHECK(firestaff_prodos_visit_files(corrupt,ORIGINAL_BYTES,count_file,&calls)==-1 && calls==0,
        "original-derived directory cycle rejects before callbacks");
    memcpy(corrupt,image,ORIGINAL_BYTES);
    entry=original_root_entry(corrupt,"GRAPHICS.GAME");
    CHECK(entry && (entry[0]>>4)==3u,"locate authentic graphics tree entry");
    key=(size_t)entry[17]+((size_t)entry[18]<<8);
    CHECK(key<ORIGINAL_BYTES/BLOCK_BYTES,"original graphics master index is in range");
    /* A block pointer exactly at the end of the original disk is invalid. */
    entry[17]=0x40u; entry[18]=0x06u;
    CHECK(firestaff_prodos_visit_files(corrupt,ORIGINAL_BYTES,count_file,&calls)==-1 && calls==0,
        "later original file with out-of-range key rejects the entire visit before callbacks");
    memcpy(corrupt,image,ORIGINAL_BYTES);
    corrupt[key*BLOCK_BYTES]=0x40u; corrupt[key*BLOCK_BYTES+256u]=0x06u;
    CHECK(firestaff_prodos_visit_files(corrupt,ORIGINAL_BYTES,count_file,&calls)==-1 && calls==0,
        "original-derived invalid tree index rejects before callbacks");
    CHECK(asset_memory_matches_md5(image,ORIGINAL_BYTES,diskHashes[edition]) &&
        asset_file_md5_hex(path,actual) && strcmp(actual,diskHashes[edition])==0,
        "reader and malformed probes leave original source bytes unchanged");
    result=1;
cleanup:
    if (file) fclose(file);
    free(corrupt); free(image);
    return result;
}

/* prefix names the exact physical source and all transport members preceding
 * the native file. Every public lookup must return that same ownership chain. */
static int check_public_intake(const char *owner, const char *prefix, int edition) {
    const char *wanted[5];
    char expected[4][ASSET_PATH_MAX], paths[4][ASSET_PATH_MAX], single[ASSET_PATH_MAX];
    char collected[1][ASSET_PATH_MAX], before[33], after[33];
    int matched[4]={0}, result=0;
    unsigned char *bytes=NULL;
    size_t size=0;
    CHECK(asset_file_md5_hex(owner,before),"hash original transport before public intake");
    for (size_t i=0; i<4u; ++i) {
        wanted[i]=original_hash(i,edition);
        CHECK(snprintf(expected[i],sizeof(expected[i]),"%s::%s",prefix,names[i])<
            (int)sizeof(expected[i]),"construct exact original virtual source");
        CHECK(asset_find_by_md5(owner,wanted[i],single,sizeof(single),0) &&
            strcmp(single,expected[i])==0,"single hash lookup preserves original native-file ownership");
        CHECK(asset_collect_paths_by_md5(owner,wanted[i],collected,1u,0)==1 &&
            strcmp(collected[0],expected[i])==0,"distinct path collection preserves original native-file ownership");
        CHECK(asset_read_path_alloc(expected[i],&bytes,&size) &&
            size==original_size(i,edition) && asset_memory_matches_md5(bytes,size,wanted[i]),
            "public native member read preserves complete original content including subdirectories");
        free(bytes); bytes=NULL;
    }
    wanted[4]=NULL;
    CHECK(asset_find_all_by_md5_list(owner,wanted,paths,matched,4,0)==4,
        "batched lookup discovers all authenticated original native files");
    for (size_t i=0; i<4u; ++i)
        CHECK(matched[i] && strcmp(paths[i],expected[i])==0,
            "batched native member paths retain their exact original transport owner");
    CHECK(asset_file_md5_hex(owner,after) && strcmp(before,after)==0,
        "public intake leaves raw disk or ZIP transport unchanged");
    result=1;
cleanup:
    free(bytes);
    return result;
}

static int check_malformed_intake(const char *owner, const char *prefix) {
    char before[33], after[33];
    char path[ASSET_PATH_MAX], found[ASSET_PATH_MAX], paths[4][ASSET_PATH_MAX];
    const char *wanted[]={hashes[0],hashes[1],hashes[2],hashes[3],NULL};
    int matched[4]={0}, result=0;
    unsigned char *bytes=NULL;
    size_t size=0;
    CHECK(asset_file_md5_hex(owner,before),"hash malformed private transport before rejected intake");
    CHECK(asset_find_all_by_md5_list(owner,wanted,paths,matched,4,0)==0,
        "malformed original-derived disk cannot publish batched native-file matches");
    for (size_t i=0; i<4u; ++i) {
        CHECK(!matched[i] && !paths[i][0] &&
            !asset_find_by_md5(owner,wanted[i],found,sizeof(found),0) &&
            asset_collect_paths_by_md5(owner,wanted[i],paths,1u,0)==0,
            "malformed original-derived disk cannot publish individual or collected native files");
        CHECK(snprintf(path,sizeof(path),"%s::%s",prefix,names[i])<(int)sizeof(path) &&
            !asset_read_path_alloc(path,&bytes,&size),
            "malformed original-derived native member read rejects before returning payload");
        free(bytes); bytes=NULL;
    }
    CHECK(asset_file_md5_hex(owner,after) && strcmp(before,after)==0,
        "rejected intake leaves malformed private transport unchanged");
    result=1;
cleanup:
    free(bytes);
    return result;
}

static int check_native_archive(const char *archive) {
    char prefix[2][ASSET_PATH_MAX], expected[2][ASSET_PATH_MAX];
    char collected[16][ASSET_PATH_MAX], found[ASSET_PATH_MAX], paths[2][ASSET_PATH_MAX];
    static const char *const members[]={
        "Dungeon Master for Apple IIGS 2.0 (English) Cracked (ACS), Cheat CDA, HD install.po",
        "Dungeon Master for Apple IIGS 2.0 (English) Cracked (ACS).po",
        "Dungeon Master for Apple IIGS 2.0 (English) Cracked (Computist), Cheat CDA.po",
        "Dungeon Master for Apple IIGS 2.0 (English) Original (Not working).po",
        "Dungeon Master for Apple IIGS 2.1 (English) Cracked (Computist), Cheat CDA.po",
        "Dungeon Master for Apple IIGS 2.1 (English) Original (Not working).po"};
    char before[33], after[33];
    const char *programs[]={original_hash(3u,0),original_hash(3u,1),NULL};
    int matched[2]={0}, result=0;
    unsigned char *bytes=NULL;
    size_t size=0;
    CHECK(asset_file_md5_hex(archive,before),"hash unchanged original 7z before native intake");
    for (int edition=0; edition<2; ++edition)
        CHECK(snprintf(prefix[edition],sizeof(prefix[edition]),
            "%s::Dungeon Master for Apple IIGS %s (English) Original (Not working).po",
            archive,edition ? "2.1" : "2.0")<(int)sizeof(prefix[edition]),
            "construct exact original 7z disk ownership chain");
    /* Hash intake reports byte identities, including identical files inside
     * historical modified disks. Those matches do not establish game readiness. */
    for (size_t file=0; file<3u; ++file) {
        int count=file==2u ? 2 : 6;
        CHECK(asset_collect_paths_by_md5(archive,hashes[file],collected,16u,0)==count,
            "native 7z collection retains every observed matching member owner");
        for (int i=0; i<count; ++i) {
            size_t member=file==2u ? (i==0 ? 3u : 5u) : (size_t)i;
            CHECK(snprintf(found,sizeof(found),"%s::%s::%s%s",archive,members[member],
                member==0u ? "DM20/" : "",names[file])<(int)sizeof(found) &&
                strcmp(collected[i],found)==0 && (i==0 || strcmp(collected[i-1],collected[i])<0),
                "native 7z sorted matches preserve all exact source paths including both originals");
            CHECK(asset_read_path_alloc(collected[i],&bytes,&size) && size==original_size(file,0) &&
                asset_memory_matches_md5(bytes,size,hashes[file]),
                "native 7z matching shared file reads preserve authenticated bytes from every owner");
            free(bytes); bytes=NULL;
        }
        if (count>2) {
            char limited[2][ASSET_PATH_MAX];
            memset(limited,0xa5,sizeof(limited));
            CHECK(asset_collect_paths_by_md5(archive,hashes[file],limited,2u,0)==-2 &&
                !limited[0][0] && !limited[1][0],
                "native 7z overflow does not publish a partial set of matching owners");
        }
    }
    CHECK(asset_find_all_by_md5_list(archive,programs,paths,matched,2,0)==2,
        "native 7z batched lookup discovers both distinct original programs");
    for (int edition=0; edition<2; ++edition) {
        CHECK(snprintf(expected[edition],sizeof(expected[edition]),"%s::%s",prefix[edition],names[3])<
            (int)sizeof(expected[edition]),"construct exact original native 7z program source");
        CHECK(matched[edition] && strcmp(paths[edition],expected[edition])==0 &&
            asset_find_by_md5(archive,programs[edition],found,sizeof(found),0) &&
            strcmp(found,expected[edition])==0,
            "native 7z single/batched program lookup preserves edition-specific original ownership");
        CHECK(asset_read_path_alloc(found,&bytes,&size) && size==original_size(3u,edition) &&
            asset_memory_matches_md5(bytes,size,programs[edition]),
            "native 7z original program content retains its authenticated edition identity");
        free(bytes); bytes=NULL;
    }
    CHECK(asset_file_md5_hex(archive,after) && strcmp(before,after)==0,
        "native 7z intake leaves original source archive unchanged");
    result=1;
cleanup:
    free(bytes);
    return result;
}

int main(int argc, char **argv) {
    char prefix[ASSET_PATH_MAX];
    if (argc!=11) {
        fprintf(stderr,"usage: %s <2.0.po> <2.1.po> <2.0.zip> <2.1.zip> <outer-2.0.zip> <outer-2.1.zip> <truncated.po> <malformed.zip> <zip-alias.Po> <original-software.7z>\n",argv[0]);
        return 2;
    }
    for (int edition=0; edition<2; ++edition) {
        const char *version=edition ? "2.1" : "2.0";
        if (!check_disk(argv[1+edition],edition) ||
            !check_public_intake(argv[1+edition],argv[1+edition],edition)) return 1;
        if (snprintf(prefix,sizeof(prefix),"%s::original-%s.po",argv[3+edition],version)>=
            (int)sizeof(prefix) || !check_public_intake(argv[3+edition],prefix,edition)) return 1;
        if (snprintf(prefix,sizeof(prefix),"%s::disk-%s.zip::original-%s.po",argv[5+edition],version,version)>=
            (int)sizeof(prefix) || !check_public_intake(argv[5+edition],prefix,edition)) return 1;
    }
    if (snprintf(prefix,sizeof(prefix),"%s::original-2.0.po",argv[9])>=
        (int)sizeof(prefix) || !check_public_intake(argv[9],prefix,0)) return 1;
    if (!check_malformed_intake(argv[7],argv[7]) ||
        snprintf(prefix,sizeof(prefix),"%s::truncated.po",argv[8])>=(int)sizeof(prefix) ||
        !check_malformed_intake(argv[8],prefix)) return 1;
    if (!check_native_archive(argv[10])) return 1;
    puts("PASS: original IIGS native/raw/ZIP/nested-ZIP/7z identities, public intake and malformed-media rejection");
    return 0;
}
