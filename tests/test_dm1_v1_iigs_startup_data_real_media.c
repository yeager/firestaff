#include "asset_find_by_hash.h"
#include "dm1_v1_iigs_graphics_dat.h"
#include "dm1_v1_iigs_metadata.h"
#include "firestaff_prodos.h"
#include "font_m11.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define REQUIRE(condition, message) do { \
    if (!(condition)) { fprintf(stderr,"FAIL: %s\n",message); goto cleanup; } \
} while (0)
static void words_le(uint8_t *out, size_t *used, const void *source, size_t bytes) {
    for (size_t i=0u; i<bytes; i+=2u) {
        uint16_t word; memcpy(&word,(const uint8_t *)source+i,2u);
        out[(*used)++]=(uint8_t)word; out[(*used)++]=(uint8_t)(word>>8);
    }
}
static int check_metadata(const uint8_t *program, size_t size, unsigned edition) {
    static const char *const hashes[]={"51b9d22c7418651ee49ad77f37c80a3c","eccc49e7b29ee233cfb10615e8ec86f3"};
    DM1_V1_IigsMetadata metadata, rejected;
    uint8_t aggregate[2493], zero[sizeof(rejected)]={0};
    uint8_t *copy=NULL;
    size_t used=0u;
    int result=0;
    REQUIRE(size==(edition ? 231865u : 232158u) && asset_memory_matches_md5(program,size,hashes[edition]),
        "exact original G20/G21 program identity");
    copy=malloc(size); REQUIRE(copy,"bounded original program copy"); memcpy(copy,program,size);
    REQUIRE(dm1_v1_iigs_metadata_read(copy,size,&metadata) &&
        metadata.program_version==(edition ? DM1_V1_IIGS_PROGRAM_G21 : DM1_V1_IIGS_PROGRAM_G20),
        "native executable metadata binds its authenticated original edition");
    memset(copy,0,size); free(copy); copy=NULL;
#define WORDS(field) do { REQUIRE(used+sizeof(metadata.field)<=sizeof(aggregate),"bounded metadata words");     words_le(aggregate,&used,metadata.field,sizeof(metadata.field)); } while (0)
#define BYTES(field) do { REQUIRE(used+sizeof(metadata.field)<=sizeof(aggregate),"bounded metadata bytes");     memcpy(aggregate+used,metadata.field,sizeof(metadata.field)); used+=sizeof(metadata.field); } while (0)
    WORDS(entrance_palette); WORDS(interface_palette); WORDS(dungeon_palettes); WORDS(ui_boxes);
    BYTES(object_aspects); BYTES(object_coordinates); BYTES(object_palette_changes);
    BYTES(wall_frames); BYTES(floor_frames); BYTES(door_frames); BYTES(door_button_frames);
    BYTES(wall_palette_changes); BYTES(door_palette_changes);
    BYTES(wall_coordinate_sets); BYTES(floor_coordinate_sets); BYTES(door_coordinate_sets);
    BYTES(wall_bitmap_increments); BYTES(floor_bitmap_increments);
    BYTES(unreadable_inscription_y2); BYTES(inscription_line_y); BYTES(wall_inscription_patch);
    WORDS(credits_palette); BYTES(palette_no_changes); WORDS(mandatory_graphics);
#undef WORDS
#undef BYTES
    /* Fixed from 24 independently located original executable sections;
     * LE words and raw arrays are serialized in the explicit order above.
     * No production metadata-reader output selected this expected digest. */
    REQUIRE(used==sizeof(aggregate) && asset_memory_matches_md5(aggregate,used,
        "307939a44cf5fc0e1b59203aacf95329"),
        "all native startup/dungeon metadata fields retain independently source-matched original bytes after caller lifetime ends");
    for (unsigned i=0u; i<16u; ++i) {
        REQUIRE(metadata.palette_no_changes[i]==i && metadata.credits_palette[i]<=0xfffu &&
            metadata.entrance_palette[i]<=0xfffu && metadata.interface_palette[i]<=0xfffu,
            "native RGB4 startup palettes and identity remap preserve original field semantics");
        for (unsigned light=0u; light<6u; ++light)
            REQUIRE(metadata.dungeon_palettes[light][i]<=0xfffu,"all six native dungeon light palettes remain RGB4");
    }
    REQUIRE(sizeof(metadata.mandatory_graphics)/sizeof(metadata.mandatory_graphics[0])==70u,
        "GS startup has its own complete seventy-record mandatory table");
    for (unsigned i=0u; i<70u; ++i)
        REQUIRE(metadata.mandatory_graphics[i]<575u,"every native mandatory index belongs to the original bank");
    memset(&rejected,0xa5,sizeof(rejected));
    REQUIRE(!dm1_v1_iigs_metadata_read(program,size-1u,&rejected) &&
        memcmp(&rejected,zero,sizeof(rejected))==0,"truncated original program clears rejected metadata receipt");
    copy=malloc(size); REQUIRE(copy,"bounded original-derived identity negative");
    memcpy(copy,program,size); copy[size-1u]^=1u; memset(&rejected,0xa5,sizeof(rejected));
    REQUIRE(!dm1_v1_iigs_metadata_read(copy,size,&rejected) && memcmp(&rejected,zero,sizeof(rejected))==0,
        "changed original program cannot borrow native table identity or retain stale metadata");
    REQUIRE(asset_memory_matches_md5(program,size,hashes[edition]),"original executable bytes remain unchanged");
    result=1;
cleanup:
    free(copy); return result;
}

/* Independent TEXT.C F0040 GS:1047-1220 interpretation: five source bits,
 * sixth background column, top=baseline-4, six rows. No production glyph helper. */
static void reference_glyph(const uint8_t *raw, uint8_t *out, int width, int height,
    int x, int baseline, unsigned code, int columns) {
    for (int row=0; row<6; ++row) for (int col=0; col<columns; ++col) {
        int64_t px=(int64_t)x+col, py=(int64_t)baseline-4+row;
        if (px>=0 && px<width && py>=0 && py<height)
            out[(size_t)py*(size_t)width+(size_t)px]=
                col<5 && (raw[row*128+code]&(16u>>col)) ? 12u : 3u;
    }
}
static int check_font(const uint8_t *bank, size_t size) {
    M11_FontState native, generic;
    uint8_t raw[768], caller[768], aggregate[4608], pixels[36], expected[36];
    uint8_t guarded[145], clipped[143];
    static const int positions[][2]={{-3,2},{1,4},{8,10},{INT_MIN,4},{INT_MAX,4},{0,INT_MIN},{0,INT_MAX}};
    size_t length=0u;
    int result=0;
    REQUIRE(size==398713u && asset_memory_matches_md5(bank,size,"1045a36952e64eb2bc7b4c7b2965b112") &&
        dm1_v1_iigs_graphics_read_raw(bank,size,557u,raw,sizeof(raw),&length) && length==768u &&
        asset_memory_matches_md5(raw,length,"c2e4a7671e51524d85e25430be5ba818"),
        "native font 557 is the complete authenticated raw source bitmap");
    M11_Font_Init(&native); M11_Font_Init(&generic);
    memcpy(caller,raw,sizeof(raw));
    REQUIRE(M11_Font_LoadIigsFromRawBitmap(&native,caller,sizeof(caller)) && native.iigsNative,
        "explicit native font binding copies source-owned raw bytes");
    memset(caller,0,sizeof(caller));
    for (unsigned code=0u; code<128u; ++code) {
        memset(pixels,0xee,sizeof(pixels)); memset(expected,0xee,sizeof(expected));
        reference_glyph(raw,expected,6,6,0,4,code,6);
        REQUIRE(M11_Font_DrawIigsChar(&native,pixels,6,6,0,4,(unsigned char)code,12u,3)==6 &&
            memcmp(pixels,expected,sizeof(pixels))==0,"all 128 native glyphs preserve source rows, background and six-pixel advance");
        memcpy(aggregate+code*36u,pixels,sizeof(pixels));
        for (size_t pos=0u; pos<sizeof(positions)/sizeof(positions[0]); ++pos) {
            memset(guarded,0xee,sizeof(guarded)); memset(clipped,0xee,sizeof(clipped));
            reference_glyph(raw,clipped,13,11,positions[pos][0],positions[pos][1],code,6);
            REQUIRE(M11_Font_DrawIigsChar(&native,guarded+1u,13,11,positions[pos][0],positions[pos][1],
                (unsigned char)code,12u,3)==6 && guarded[0]==0xeeu && guarded[144]==0xeeu &&
                memcmp(guarded+1u,clipped,sizeof(clipped))==0,
                "native glyph baseline/odd-X/clipping and extreme coordinates preserve framebuffer bounds");
        }
    }
    REQUIRE(asset_memory_matches_md5(aggregate,sizeof(aggregate),"cbf80c5749da2df1ee669360763e15df"),
        "complete native glyph raster matches independent TEXT.C oracle");
    memset(guarded,0xee,sizeof(guarded)); memset(clipped,0xee,sizeof(clipped));
    reference_glyph(raw,clipped,13,11,1,4,'A',6); reference_glyph(raw,clipped,13,11,7,4,10u,6);
    reference_glyph(raw,clipped,13,11,13,4,'Z',6);
    REQUIRE(M11_Font_DrawIigsString(&native,guarded+1u,13,11,1,4,"A\nZ",12u,3)==18 &&
        memcmp(guarded+1u,clipped,sizeof(clipped))==0,"native string treats LF as glyph ten and preserves source advance under clipping");
    memset(pixels,0xee,sizeof(pixels)); memset(expected,0xee,sizeof(expected));
    REQUIRE(!M11_Font_DrawIigsString(&native,pixels,6,6,0,4,"A\200",12u,3) &&
        !M11_Font_DrawIigsChar(&native,pixels,6,6,0,4,128u,12u,3) &&
        !M11_Font_DrawIigsChar(&native,pixels,6,6,0,4,'A',16u,3) &&
        !M11_Font_DrawIigsChar(&native,pixels,6,6,0,4,'A',12u,-1) &&
        memcmp(pixels,expected,sizeof(pixels))==0,"invalid native text/color inputs do not paint a partial string or fallback glyph");
    REQUIRE(M11_Font_LoadFromRawBitmap(&generic,557,raw,sizeof(raw)) && !generic.iigsNative &&
        !M11_Font_DrawIigsChar(&generic,pixels,6,6,0,4,'A',12u,3),
        "generic raw font binding does not infer native GS text policy");
    for (int row=0; row<6; ++row) for (int col=0; col<6; ++col) {
        unsigned x=(unsigned)'A'*8u+3u+(unsigned)col;
        expected[row*6+col]=(raw[row*128+x/8u]&(128u>>(x%8u))) ? 12u : 3u;
    }
    REQUIRE(M11_Font_DrawChar(&generic,pixels,6,6,0,0,'A',12u,3,1)==8 &&
        memcmp(pixels,expected,sizeof(pixels))==0,"shared generic helper keeps eight-pixel advance/top-Y math on authenticated GS font bytes");
    memset(pixels,0xee,sizeof(pixels)); memset(expected,0xee,sizeof(expected));
    reference_glyph(raw,expected,6,6,0,4,(unsigned)'A'-32u,5);
    REQUIRE(M11_Font_DrawF31CEDTAsciiString(&generic,pixels,6,6,0,4,"A",12u,3)==6 &&
        memcmp(pixels,expected,sizeof(pixels))==0,"shared CEDT helper keeps printable-ASCII offset/five-column math on GS font bytes");
    REQUIRE(M11_Font_LoadFromRawBitmap(&native,557,raw,sizeof(raw)) && !native.iigsNative &&
        !M11_Font_DrawIigsChar(&native,pixels,6,6,0,4,'A',12u,3),"generic rebind clears native text policy");
    M11_Font_Init(&native);
    REQUIRE(!M11_Font_LoadIigsFromRawBitmap(&native,raw,sizeof(raw)-1u) && !native.iigsNative && !native.loaded,
        "truncated original font cannot create a native binding");
    result=1;
cleanup:
    return result;
}

typedef struct { uint8_t *graphics, *program; size_t graphics_size, program_size; } OriginalFiles;
static int collect_file(const char *path, const uint8_t *bytes, size_t size, void *user) {
    OriginalFiles *files=user;
    uint8_t **target=NULL; size_t *length=NULL;
    if (strcmp(path,"GRAPHICS.GAME")==0) { target=&files->graphics; length=&files->graphics_size; }
    if (strcmp(path,"DUNGEON.MASTER")==0) { target=&files->program; length=&files->program_size; }
    if (!target) return 0;
    if (*target || size>400000u || !(*target=malloc(size))) return 1;
    memcpy(*target,bytes,size); *length=size; return 0;
}
static int check_disk(const char *path, unsigned edition) {
    static const char *const hashes[]={"15e3f954f408612fd2f15a474a182f9f","d38f00e5ad8516553c5cba3214b29bef"};
    FILE *file=NULL; uint8_t *disk=NULL; OriginalFiles files={0}; int result=0;
    file=fopen(path,"rb");
    REQUIRE(file && fseek(file,0,SEEK_END)==0 && ftell(file)==819200 && fseek(file,0,SEEK_SET)==0,"bounded original disk size");
    disk=malloc(819200u); REQUIRE(disk && fread(disk,1,819200u,file)==819200u,"read original disk bytes");
    { int closed=fclose(file); file=NULL; REQUIRE(closed==0,"close original disk"); }
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]) &&
        firestaff_prodos_visit_files(disk,819200u,collect_file,&files)>0 && files.graphics && files.program,
        "collect exact native graphics and original executable from authenticated disk");
    REQUIRE(check_metadata(files.program,files.program_size,edition) && check_font(files.graphics,files.graphics_size),
        "native startup metadata and source font pass independent original-data verification");
    REQUIRE(asset_memory_matches_md5(disk,819200u,hashes[edition]),"original disk remains unchanged"); result=1;
cleanup:
    if (file) fclose(file);
    free(files.graphics); free(files.program); free(disk); return result;
}
int main(int argc, char **argv) {
    if (argc!=3) { fprintf(stderr,"usage: %s <original-2.0.po> <original-2.1.po>\n",argv[0]); return 2; }
    if (!check_disk(argv[1],0u) || !check_disk(argv[2],1u)) return 1;
    puts("PASS: original IIGS startup metadata and all native glyphs; platform admission unverified"); return 0;
}
