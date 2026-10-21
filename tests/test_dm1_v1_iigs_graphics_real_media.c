#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "dm1_v1_iigs_title_f0437.h"
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
    uint8_t *pixels=NULL, *raw=NULL, *aggregate=NULL, *screen=NULL;
    size_t offset=2302u, used=0u, rawSize;
    unsigned rasters=0u;
    int result=0;
    uint16_t width, height;
    REQUIRE(size==BANK_BYTES && asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112"),
        "unchanged original IIGS graphics bank identity");
    REQUIRE(dm1_v1_iigs_graphics_probe(bank,size) && !dm1_v1_iigs_graphics_probe(bank,size-1u),
        "complete original bank accepted and truncated original rejected");
    pixels=malloc(65538u); raw=malloc(65535u); aggregate=malloc(AGGREGATE_BYTES);
    screen=malloc(320u*200u);
    REQUIRE(pixels && raw && aggregate && screen,"bounded original bitmap test buffers");
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
    {
        uint16_t title_width=0,title_height=0,palette[16];
        unsigned frame,x,y;
        REQUIRE(dm1_v1_iigs_graphics_decode(bank,size,1u,pixels,65535u,
            &title_width,&title_height) && title_width==320u && title_height==200u,
            "authentic C001_GRAPHIC_TITLE has source-owned 320x200 IIGS dimensions");
        for (frame=0; frame<DM1_V1_IIGS_TITLE_ZOOM_FRAMES; ++frame) {
            unsigned w=48u+16u*frame,h=12u+4u*frame,left=(320u-w)/2u,top=(160u-h)/2u;
            size_t colored=0;
            REQUIRE(dm1_v1_iigs_title_f0437_compose(pixels,frame,screen,palette),
                "F0437 composes each native-media zoom step");
            REQUIRE(palette[0]==0x0009u && palette[3]==0x0a82u &&
                palette[4]==0x0842u && palette[5]==0x0ca2u && palette[6]==0x0a62u &&
                palette[8]==0x0ff4u && palette[15]==0x0f00u,
                "F0437 IIGS RGB4 title palette matches source");
            for (y=0; y<200u; ++y) for (x=0; x<320u; ++x) {
                uint8_t value=screen[y*320u+x];
                if (x<left || x>=left+w || y<top || y>=top+h)
                    REQUIRE(value==0u,"F0437 zoom stays in centered source-owned frame bounds");
                else if (value!=0u) ++colored;
            }
            REQUIRE(colored>0u,"authentic F0437 title frame contains title pixels");
        }
        REQUIRE(!dm1_v1_iigs_title_f0437_compose(pixels,18u,screen,palette),
            "F0437 rejects a frame after its 18 source-owned steps");
    }
    width=height=1u;
    REQUIRE(!dm1_v1_iigs_graphics_decode(bank,size,575u,pixels,65535u,&width,&height) && !width && !height,
        "out-of-range original record rejects");
    REQUIRE(asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112"),
        "decoder leaves original graphics bytes unchanged");
    result=1;
cleanup:
    free(screen); free(aggregate); free(raw); free(pixels);
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
    puts("PASS: original G20/G21 rasters and C001 F0437 zoom compositor verified; GS F0129 visual parity and F0441 entrance unverified");
    return 0;
}
