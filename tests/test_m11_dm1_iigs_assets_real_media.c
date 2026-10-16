#include "asset_find_by_hash.h"
#include "asset_loader_m11.h"
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
static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1]<<8); }
static void put16(uint8_t *p, unsigned v) { p[0]=(uint8_t)v; p[1]=(uint8_t)(v>>8); }

static int check_cache(M11_AssetLoader *loader, const uint8_t *original) {
    uint8_t *aggregate=malloc(1692424u);
    size_t used=0u, offset=2302u;
    const M11_AssetSlot *first=NULL;
    const unsigned char *firstPixels=NULL;
    int result=0;
    REQUIRE(aggregate && M11_AssetLoader_IsReady(loader) && loader->iigsDm1 &&
        !loader->legacyDm1 && !loader->atariStDm1 && !loader->dm1Amiga36 &&
        !loader->csbAmiga && !loader->csbFmtowns && loader->graphicCount==575u &&
        loader->cacheUsed==0,"explicit native profile starts with isolated source and empty cache");
    for (unsigned index=0u; index<575u; ++index) {
        unsigned short width=0u, height=0u;
        size_t length=le16(original+2u+index*2u), count;
        const M11_AssetSlot *slot;
        int before=loader->cacheUsed;
        if (index==12u || index>532u) {
            REQUIRE(!M11_AssetLoader_QuerySize(loader,index,&width,&height) &&
                !M11_AssetLoader_Load(loader,index) && loader->cacheUsed==before,
                "native font and non-raster records do not enter M11 bitmap cache");
            offset+=length; continue;
        }
        REQUIRE(M11_AssetLoader_QuerySize(loader,index,&width,&height) &&
            width==le16(original+offset) && height==le16(original+offset+2u),
            "actual M11 query preserves each original native raster dimension");
        count=(size_t)width*height;
        slot=M11_AssetLoader_Load(loader,index);
        REQUIRE(slot && slot->loaded && slot->pixels && slot->graphicIndex==index &&
            slot->width==width && slot->height==height && loader->cacheUsed==before+1 &&
            M11_AssetLoader_Load(loader,index)==slot && loader->cacheUsed==before+1,
            "actual M11 native decoder caches one stable slot per original raster");
        if (index==0u) { first=slot; firstPixels=slot->pixels; }
        if (index==21u)
            REQUIRE(width==16u && height==17u && asset_memory_matches_md5(slot->pixels,count,
                "382405ffee479fc53b2bf939b9145eac"),"native IIGS record 21 uses its own raster decoder");
        REQUIRE(used+6u+count<=1692424u,"bounded complete M11 raster aggregate");
        put16(aggregate+used,index); put16(aggregate+used+2u,width); put16(aggregate+used+4u,height);
        memcpy(aggregate+used+6u,slot->pixels,count); used+=6u+count; offset+=length;
    }
    /* Independent EXPAND.C GS assembly oracle, shared with the primitive
     * original-media test; these hashes were not derived from M11 output. */
    REQUIRE(used==1692424u && loader->cacheUsed==532 &&
        asset_memory_matches_md5(aggregate,used,"cf735a173ec8294f393a07c540595d6a") &&
        M11_AssetLoader_Load(loader,0u)==first && first->pixels==firstPixels &&
        asset_memory_matches_md5(firstPixels,224u*136u,"6e2891475cf2f408675255c5ed6e54e9"),
        "all cached native rasters match independent original-assembly oracle without evicting early slots");
    {
        unsigned short width=0u,height=0u;
        REQUIRE(!M11_AssetLoader_QuerySize(loader,65536u,&width,&height) &&
            !M11_AssetLoader_Load(loader,65536u) &&
            !M11_AssetLoader_QuerySize(loader,UINT_MAX,&width,&height) &&
            !M11_AssetLoader_Load(loader,UINT_MAX) && loader->cacheUsed==532,
            "wide invalid record indexes cannot alias cached native record zero");
    }
    result=1;
cleanup:
    free(aggregate);
    return result;
}
static int check_bank(const uint8_t *bank, size_t size, const char *diskPath) {
    M11_AssetLoader loader;
    uint8_t *caller=NULL;
    char member[ASSET_PATH_MAX];
    int result=0;
    memset(&loader,0,sizeof(loader));
    REQUIRE(size==398713u && asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112"),
        "authentic original IIGS graphics source identity");
    caller=malloc(size); REQUIRE(caller,"allocate private original source copy");
    memcpy(caller,bank,size); caller[size-1u]^=1u;
    REQUIRE(dm1_v1_iigs_graphics_probe(caller,size) &&
        !M11_AssetLoader_InitDm1IigsFromBuffer(&loader,caller,(long)size) &&
        !M11_AssetLoader_IsReady(&loader),"structurally valid original-derived altered bank cannot acquire native identity");
    /* The legacy LE envelope remains structural. Opening the same original
     * bytes explicitly through that transport must not select the GS decoder;
     * this makes no claim that the bank belongs to an FM Towns edition. */
    REQUIRE(M11_AssetLoader_InitDm1LegacyFromBuffer(&loader,bank,(long)size,0) &&
        loader.legacyDm1 && !loader.iigsDm1 && !loader.iigsData &&
        !M11_AssetLoader_Load(&loader,21u),"legacy structural profile cannot silently become the native IIGS decoder");
    M11_AssetLoader_Shutdown(&loader);
    REQUIRE(snprintf(member,sizeof(member),"%s::GRAPHICS.GAME",diskPath)<(int)sizeof(member),
        "construct exact native original file source");
    for (unsigned cycle=0u; cycle<3u; ++cycle) {
        if (cycle==1u) {
            REQUIRE(M11_AssetLoader_InitDm1IigsFromFile(&loader,member) &&
                strcmp(loader.graphicsDatPath,member)==0,"native file profile reads original PO member directly in memory");
        } else {
            memcpy(caller,bank,size);
            REQUIRE(M11_AssetLoader_InitDm1IigsFromBuffer(&loader,caller,(long)size) &&
                loader.iigsData!=caller,"native buffer profile owns an independent source copy");
            memset(caller,0,size); free(caller); caller=NULL;
        }
        REQUIRE(check_cache(&loader,bank),"actual M11 query/load/cache consumes complete original native bitmap bank");
        M11_AssetLoader_Shutdown(&loader);
        REQUIRE(!M11_AssetLoader_IsReady(&loader) && !loader.iigsDm1 && !loader.iigsData &&
            !loader.legacyData && !loader.ownedBuffer && loader.cacheUsed==0,
            "shutdown clears native source and every cached bitmap before reload");
        M11_AssetLoader_Shutdown(&loader);
        if (!caller && cycle<2u) { caller=malloc(size); REQUIRE(caller,"allocate next original source lifetime check"); }
    }
    REQUIRE(asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112"),
        "M11 asset profile leaves original graphics bytes unchanged");
    result=1;
cleanup:
    M11_AssetLoader_Shutdown(&loader); free(caller);
    return result;
}
typedef struct { const char *diskPath; int found, valid; } GraphicsVisit;
static int visit_graphics(const char *path, const uint8_t *bytes, size_t size, void *user) {
    GraphicsVisit *visit=user;
    if (strcmp(path,"GRAPHICS.GAME")!=0) return 0;
    ++visit->found; visit->valid=check_bank(bytes,size,visit->diskPath); return 1;
}
static int check_disk(const char *path, unsigned edition) {
    static const char *const hashes[]={"15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    FILE *file=NULL;
    uint8_t *disk=NULL;
    GraphicsVisit visit={path,0,0};
    int result=0;
    file=fopen(path,"rb");
    REQUIRE(file && fseek(file,0,SEEK_END)==0 && ftell(file)==819200 && fseek(file,0,SEEK_SET)==0,
        "bounded original IIGS disk size");
    disk=malloc(819200u);
    REQUIRE(disk && fread(disk,1,819200u,file)==819200u,"read bounded original disk bytes");
    { int closed=fclose(file); file=NULL; REQUIRE(closed==0,"close original disk"); }
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]) &&
        firestaff_prodos_visit_files(disk,819200u,visit_graphics,&visit)>0 && visit.found==1 && visit.valid,
        "both original native editions use the actual M11 source profile and cache");
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"original disk source remains unchanged");
    result=1;
cleanup:
    if (file) fclose(file);
    free(disk); return result;
}
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s <original-2.0.po> <original-2.1.po>\n",argv[0]); return 2; }
    if (!check_disk(argv[1],0u) || !check_disk(argv[2],1u)) return 1;
    puts("PASS: original IIGS M11 bitmap profile, source lifetime and cache; game admission remains unverified");
    return 0;
}
