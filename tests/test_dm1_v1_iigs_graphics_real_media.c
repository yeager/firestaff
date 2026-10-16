#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "firestaff_prodos.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr,"FAIL: %s\n",message); goto cleanup; } \
} while (0)
enum { DISK_BYTES=819200, BANK_BYTES=398713, AGGREGATE_BYTES=1692424 };
static unsigned le16(const uint8_t *p) { return (unsigned)p[0] | ((unsigned)p[1]<<8); }
static void put16(uint8_t *p, unsigned value) { p[0]=(uint8_t)value; p[1]=(uint8_t)(value>>8); }

static int check_bank(const uint8_t *bank, size_t size) {
    uint8_t *pixels=NULL, *raw=NULL, *aggregate=NULL;
    size_t offset=2302u, used=0u, rawSize;
    unsigned rasters=0u;
    int result=0;
    uint16_t width, height;
    REQUIRE(size==BANK_BYTES && asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112"),
        "unchanged original IIGS graphics bank identity");
    REQUIRE(dm1_v1_iigs_graphics_probe(bank,size) && !dm1_v1_iigs_graphics_probe(bank,size-1u),
        "complete original bank accepted and truncated original rejected");
    pixels=malloc(65538u); raw=malloc(65535u); aggregate=malloc(AGGREGATE_BYTES);
    REQUIRE(pixels && raw && aggregate,"bounded original bitmap test buffers");
    for (uint16_t index=0u; index<575u; ++index) {
        size_t length=le16(bank+2u+(size_t)index*2u);
        size_t count;
        int raster=index<=532u && index!=12u;
        REQUIRE(length<=size-offset && length==le16(bank+1152u+(size_t)index*2u),
            "source-owned little-endian equal-size record framing");
        REQUIRE(dm1_v1_iigs_graphics_read_raw(bank,size,index,raw,65535u,&rawSize) &&
            rawSize==length && memcmp(raw,bank+offset,length)==0,
            "all 575 raw records preserve original bytes");
        width=height=0u;
        if (!raster) {
            REQUIRE(!dm1_v1_iigs_graphics_query(bank,size,index,&width,&height) && !width && !height,
                "unused native font and non-raster records are not bitmap queries");
            offset+=length;
            continue;
        }
        REQUIRE(length>=4u && dm1_v1_iigs_graphics_query(bank,size,index,&width,&height) &&
            width==le16(bank+offset) && height==le16(bank+offset+2u),
            "every original raster has its source-owned dimensions including record 21");
        count=(size_t)width*height;
        REQUIRE(count && count<=65535u && used+6u+count<=AGGREGATE_BYTES,
            "all original raster dimensions fit bounded output and aggregate");
        memset(pixels,0xa5,count+2u);
        REQUIRE(dm1_v1_iigs_graphics_decode(bank,size,index,pixels+1u,count,&width,&height) &&
            pixels[0]==0xa5u && pixels[count+1u]==0xa5u,
            "bank decoder renders original raster without crossing capacity");
        put16(aggregate+used,index); put16(aggregate+used+2u,width); put16(aggregate+used+4u,height);
        memcpy(aggregate+used+6u,pixels+1u,count); used+=6u+count;
        REQUIRE(dm1_v1_iigs_graphics_decode_item(bank+offset,length,raw,65535u,&width,&height) &&
            memcmp(raw,pixels+1u,count)==0,"item and bank APIs expose the same complete native raster");
        memset(pixels,0xa5,count+2u); width=height=1u;
        REQUIRE(!dm1_v1_iigs_graphics_decode(bank,size,index,pixels+1u,count-1u,&width,&height) &&
            !width && !height && pixels[0]==0xa5u && pixels[count]==0xa5u,
            "insufficient raster capacity rejects with cleared dimensions and intact boundary");
        width=height=1u;
        REQUIRE(!dm1_v1_iigs_graphics_decode_item(bank+offset,length-1u,pixels+1u,count,&width,&height) &&
            !width && !height && pixels[0]==0xa5u && pixels[count+1u]==0xa5u,
            "every truncated original command stream rejects within its output boundary");
        ++rasters; offset+=length;
    }
    /* Independently derived from EXPAND.C F0466's GS assembly:324-845.
     * Developer Python oracle emits commands directly as indexed pixels and
     * hashes [LE16 index, width, height, pixels] for all 532 authentic rasters.
     * No production decoder output was used to select this expected hash.
     * Original streams exercise short/long fill, odd/even literal and both
     * previous-row copy lengths; A/E no-output commands do not occur here. */
    REQUIRE(offset==size && rasters==532u && used==AGGREGATE_BYTES &&
        asset_memory_matches_md5(aggregate,used,"cf735a173ec8294f393a07c540595d6a"),
        "all source-owned raster pixels and dimensions match independent original-assembly oracle");
    width=height=1u;
    REQUIRE(!dm1_v1_iigs_graphics_decode(bank,size,575u,pixels,65535u,&width,&height) && !width && !height,
        "out-of-range original record rejects");
    REQUIRE(asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112"),
        "decoder leaves original graphics bytes unchanged");
    result=1;
cleanup:
    free(aggregate); free(raw); free(pixels);
    return result;
}

typedef struct { int found, valid; } GraphicsVisit;
static int visit_graphics(const char *path, const uint8_t *bytes, size_t size, void *user) {
    GraphicsVisit *visit=user;
    if (strcmp(path,"GRAPHICS.GAME")!=0) return 0;
    ++visit->found; visit->valid=check_bank(bytes,size);
    return 1;
}
static int check_disk(const char *path, int edition) {
    static const char *const hashes[]={"15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    uint8_t *disk=NULL;
    FILE *file=NULL;
    GraphicsVisit visit={0,0};
    int result=0;
    file=fopen(path,"rb");
    REQUIRE(file && fseek(file,0,SEEK_END)==0 && ftell(file)==DISK_BYTES &&
        fseek(file,0,SEEK_SET)==0,"bounded authentic original IIGS disk size");
    disk=malloc(DISK_BYTES);
    REQUIRE(disk && fread(disk,1,DISK_BYTES,file)==DISK_BYTES,"read original disk into bounded memory");
    { int closed=fclose(file); file=NULL; REQUIRE(closed==0,"close original disk"); }
    REQUIRE(asset_memory_matches_md5(disk,DISK_BYTES,hashes[edition]),"original IIGS disk content identity");
    REQUIRE(firestaff_prodos_visit_files(disk,DISK_BYTES,visit_graphics,&visit)>0 &&
        visit.found==1 && visit.valid,"native original graphics member passes complete raster verification");
    REQUIRE(asset_memory_matches_md5(disk,DISK_BYTES,hashes[edition]),"original disk bytes remain unchanged");
    result=1;
cleanup:
    if (file) fclose(file);
    free(disk);
    return result;
}
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s <original-2.0.po> <original-2.1.po>\n",argv[0]); return 2; }
    if (!check_disk(argv[1],0) || !check_disk(argv[2],1)) return 1;
    puts("PASS: all 532 original IIGS rasters match independent assembly oracle; startup and palettes unverified");
    return 0;
}
