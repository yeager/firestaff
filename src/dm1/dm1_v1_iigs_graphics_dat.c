#include "dm1_v1_iigs_graphics_dat.h"
#include "dm1_v1_legacy_graphics_dat.h"

#include <string.h>

#define IIGS_RECORDS 575U

static uint16_t iigs_u16(const uint8_t *p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

int dm1_v1_iigs_graphics_probe(const uint8_t *data, size_t size) {
    /* ReDMCSB MEMORY.C F0479_MEMORY_ReadGraphicsDatHeader, lines 1330
     * and 1361–1381 (GS size tables). Only the
     * common bounded LE bank envelope is reused; GS bitmap commands below
     * are EXPAND.C F0466, not IMAGE2.C's other-platform decoder. */
    return dm1_v1_legacy_graphics_probe(data,size,0);
}

int dm1_v1_iigs_graphics_read_raw(const uint8_t *data, size_t size,
    uint16_t index, uint8_t *out, size_t capacity, size_t *length) {
    return dm1_v1_legacy_graphics_read_raw(data,size,0,index,out,capacity,length);
}

static int iigs_record(const uint8_t *data, size_t size, uint16_t index,
                       const uint8_t **item, size_t *length) {
    size_t offset=2U+IIGS_RECORDS*4U;
    uint16_t i;
    /* ReDMCSB DEFS.H C012 (unused font), M719 (first sound 533), M564
     * (object names 556), M653 (font 557). IIGS record 21 is a bitmap. */
    if (index>532U || index==12U || !dm1_v1_iigs_graphics_probe(data,size)) return 0;
    for (i=0; i<index; ++i) offset+=iigs_u16(data+2U+(size_t)i*2U);
    *length=iigs_u16(data+2U+(size_t)index*2U);
    if (*length<4U || offset>size || *length>size-offset) return 0;
    *item=data+offset;
    return 1;
}

static int iigs_dimensions(const uint8_t *item, size_t size,
                           uint16_t *width, uint16_t *height) {
    size_t total;
    if (!item || size<4U) return 0;
    *width=iigs_u16(item); *height=iigs_u16(item+2U);
    total=(size_t)*width * *height;
    /* F0466 GS uses width/2 packed row bytes and a 16-bit pixel counter.
     * Odd widths cannot represent its previous-row addressing faithfully. */
    return *width && *height && !(*width&1U) && *width<=640U &&
           *height<=400U && total<=65535U;
}

int dm1_v1_iigs_graphics_query(const uint8_t *data, size_t size,
    uint16_t index, uint16_t *out_width, uint16_t *out_height) {
    const uint8_t *item;
    size_t length;
    uint16_t width,height;
    if (out_width) *out_width=0;
    if (out_height) *out_height=0;
    if (!iigs_record(data,size,index,&item,&length) ||
        !iigs_dimensions(item,length,&width,&height)) return 0;
    if (out_width) *out_width=width;
    if (out_height) *out_height=height;
    return 1;
}

int dm1_v1_iigs_graphics_decode_item(const uint8_t *item, size_t item_size,
    uint8_t *pixels, size_t capacity,
    uint16_t *out_width, uint16_t *out_height) {
    uint16_t width,height;
    size_t source=4U, pixel=0U, total;
    if (out_width) *out_width=0;
    if (out_height) *out_height=0;
    if (!pixels || !iigs_dimensions(item,item_size,&width,&height)) return 0;
    total=(size_t)width*height;
    if (capacity<total) return 0;
    /* ReDMCSB EXPAND.C F0466 GS branch, lines 324–845:
     * L080074 short runs; L0800C4 extended counts; L080109 fill;
     * L0801AE literals; L0802D3 previous-row copy followed by one color.
     * GSTHINGS.C G3978 at line 5 and initialization 147–157 swaps nibbles,
     * not palette indices. Writing each logical pixel directly is equivalent
     * to GS packed high-nibble-first output followed by unpacking it. */
    while (pixel<total) {
        uint8_t command,color;
        unsigned operation;
        size_t count,i;
        if (source>=item_size) return 0;
        command=item[source++]; color=command&15U;
        if (!(command&0x80U)) {
            count=(size_t)(command>>4)+1U;
            if (count>total-pixel) return 0;
            memset(pixels+pixel,color,count); pixel+=count;
            continue;
        }
        if (source>=item_size) return 0;
        count=item[source++];
        if (command&0x40U) {
            if (source>=item_size) return 0;
            count=(count<<8)|item[source++];
        }
        ++count;
        operation=command&0x30U;
        /* L080105 consumes A/E count bytes and resumes without emission. */
        if (operation==0x20U) continue;
        if (operation==0U) {
            if (count>total-pixel) return 0;
            memset(pixels+pixel,color,count); pixel+=count;
        } else if (operation==0x10U) {
            if (count>total-pixel) return 0;
            if (count&1U) { pixels[pixel++]=color; --count; }
            if (count/2U>item_size-source) return 0;
            for (i=0; i<count/2U; ++i) {
                uint8_t packed=item[source++];
                pixels[pixel++]=packed>>4;
                pixels[pixel++]=packed&15U;
            }
        } else {
            if (count>=total-pixel || pixel<(size_t)width) return 0;
            for (i=0; i<count; ++i) {
                pixels[pixel]=pixels[pixel-width]; ++pixel;
            }
            pixels[pixel++]=color;
        }
    }
    if (out_width) *out_width=width;
    if (out_height) *out_height=height;
    return 1;
}

int dm1_v1_iigs_graphics_decode(const uint8_t *data, size_t size,
    uint16_t index, uint8_t *pixels, size_t capacity,
    uint16_t *out_width, uint16_t *out_height) {
    const uint8_t *item;
    size_t length;
    if (out_width) *out_width=0;
    if (out_height) *out_height=0;
    if (!iigs_record(data,size,index,&item,&length)) return 0;
    return dm1_v1_iigs_graphics_decode_item(item,length,pixels,capacity,out_width,out_height);
}
