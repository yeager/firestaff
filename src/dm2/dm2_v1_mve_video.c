#include "dm2_v1_mve_video.h"

#include <string.h>

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t at;
} DM2_MveReader;

static int mve_get8(DM2_MveReader *r, uint8_t *out)
{
    if (!r || !out || r->at >= r->size) return 0;
    *out = r->data[r->at++];
    return 1;
}

static int mve_get16(DM2_MveReader *r, uint16_t *out)
{
    uint8_t a, b;
    if (!mve_get8(r, &a) || !mve_get8(r, &b)) return 0;
    *out = (uint16_t)(a | ((uint16_t)b << 8u));
    return 1;
}

static int mve_get32(DM2_MveReader *r, uint32_t *out)
{
    uint16_t a, b;
    if (!mve_get16(r, &a) || !mve_get16(r, &b)) return 0;
    *out = (uint32_t)a | ((uint32_t)b << 16u);
    return 1;
}

static int mve_get64(DM2_MveReader *r, uint64_t *out)
{
    uint32_t a, b;
    if (!mve_get32(r, &a) || !mve_get32(r, &b)) return 0;
    *out = (uint64_t)a | ((uint64_t)b << 32u);
    return 1;
}

static void mve_put(DM2_V1_MveVideo *v, unsigned x, unsigned y, uint8_t p)
{
    v->current[y * DM2_V1_MVE_VIDEO_WIDTH + x] = p;
}

static void mve_fill(DM2_V1_MveVideo *v, unsigned bx, unsigned by, uint8_t p)
{
    unsigned y;
    for (y = 0u; y < 8u; ++y)
        memset(v->current + (by + y) * DM2_V1_MVE_VIDEO_WIDTH + bx, p, 8u);
}

static int mve_copy(DM2_V1_MveVideo *v, const uint8_t *source,
                    unsigned bx, unsigned by, int dx, int dy)
{
    int x = (int)bx + dx;
    int y = (int)by + dy;
    unsigned row;
    /* Interplay's 8-bit copy helper wraps X once and carries into Y. */
    if (x < 0) { x += (int)DM2_V1_MVE_VIDEO_WIDTH; --y; }
    else if (x >= (int)DM2_V1_MVE_VIDEO_WIDTH) {
        x -= (int)DM2_V1_MVE_VIDEO_WIDTH; ++y;
    }
    if (!source || x < 0 || x + 8 > (int)DM2_V1_MVE_VIDEO_WIDTH || y < 0 ||
        y + 8 > (int)DM2_V1_MVE_VIDEO_HEIGHT) return 0;
    for (row = 0u; row < 8u; ++row)
        memmove(v->current + (by + row) * DM2_V1_MVE_VIDEO_WIDTH + bx,
                source + ((unsigned)y + row) * DM2_V1_MVE_VIDEO_WIDTH + (unsigned)x,
                8u);
    return 1;
}

static int mve_opcode_7(DM2_V1_MveVideo *v, DM2_MveReader *r, unsigned bx, unsigned by)
{
    uint8_t p0, p1, bit; uint16_t flags; unsigned x, y;
    if (!mve_get8(r, &p0) || !mve_get8(r, &p1)) return 0;
    if (p0 <= p1) {
        for (y = 0u; y < 8u; ++y) {
            if (!mve_get8(r, &bit)) return 0;
            for (x = 0u; x < 8u; ++x) { mve_put(v, bx+x, by+y, (bit & 1u) ? p1 : p0); bit >>= 1u; }
        }
    } else {
        if (!mve_get16(r, &flags)) return 0;
        for (y = 0u; y < 8u; y += 2u) for (x = 0u; x < 8u; x += 2u) {
            uint8_t p = (flags & 1u) ? p1 : p0; flags >>= 1u;
            mve_put(v,bx+x,by+y,p); mve_put(v,bx+x+1u,by+y,p);
            mve_put(v,bx+x,by+y+1u,p); mve_put(v,bx+x+1u,by+y+1u,p);
        }
    }
    return 1;
}

static int mve_opcode_8(DM2_V1_MveVideo *v, DM2_MveReader *r, unsigned bx, unsigned by)
{
    uint8_t p[4], b; uint16_t f16; uint32_t f32; unsigned x,y;
    if (!mve_get8(r,&p[0]) || !mve_get8(r,&p[1])) return 0;
    if (p[0] <= p[1]) {
        for (y=0u; y<16u; ++y) {
            if (!(y&3u)) { if (y && (!mve_get8(r,&p[0]) || !mve_get8(r,&p[1]))) return 0; if (!mve_get16(r,&f16)) return 0; }
            for (x=0u;x<4u;++x) { mve_put(v,bx + (y/8u)*4u+x,by+(y%8u),(f16&1u)?p[1]:p[0]); f16>>=1u; }
        }
    } else {
        if (!mve_get32(r,&f32) || !mve_get8(r,&p[2]) || !mve_get8(r,&p[3])) return 0;
        if (p[2] <= p[3]) {
            for (y=0u;y<16u;++y) { for(x=0u;x<4u;++x) { mve_put(v,bx+(y/8u)*4u+x,by+(y%8u),(f32&1u)?p[1]:p[0]);f32>>=1u; } if(y==7u) {p[0]=p[2];p[1]=p[3];if(!mve_get32(r,&f32))return 0;} }
        } else {
            for(y=0u;y<8u;++y) { if(y==4u){p[0]=p[2];p[1]=p[3];if(!mve_get32(r,&f32))return 0;} for(x=0u;x<8u;++x){mve_put(v,bx+x,by+y,(f32&1u)?p[1]:p[0]);f32>>=1u;} }
        }
    }
    (void)b; return 1;
}

static int mve_opcode_9(DM2_V1_MveVideo *v, DM2_MveReader *r, unsigned bx, unsigned by)
{
    uint8_t p[4]; uint16_t f16; uint32_t f32; uint64_t f64; unsigned x,y;
    for(x=0u;x<4u;++x) if(!mve_get8(r,&p[x])) return 0;
    if(p[0]<=p[1]) {
        if(p[2]<=p[3]) { for(y=0u;y<8u;++y){if(!mve_get16(r,&f16))return 0;for(x=0u;x<8u;++x){mve_put(v,bx+x,by+y,p[f16&3u]);f16>>=2u;}} }
        else { if(!mve_get32(r,&f32))return 0;for(y=0u;y<8u;y+=2u)for(x=0u;x<8u;x+=2u){uint8_t q=p[f32&3u];f32>>=2u;mve_put(v,bx+x,by+y,q);mve_put(v,bx+x+1u,by+y,q);mve_put(v,bx+x,by+y+1u,q);mve_put(v,bx+x+1u,by+y+1u,q);} }
    } else {
        if(!mve_get64(r,&f64)) return 0;
        if(p[2]<=p[3]) for(y=0u;y<8u;++y)for(x=0u;x<8u;x+=2u){uint8_t q=p[f64&3u];f64>>=2u;mve_put(v,bx+x,by+y,q);mve_put(v,bx+x+1u,by+y,q);}
        else for(y=0u;y<8u;y+=2u)for(x=0u;x<8u;++x){uint8_t q=p[f64&3u];f64>>=2u;mve_put(v,bx+x,by+y,q);mve_put(v,bx+x,by+y+1u,q);}
    }
    return 1;
}

static int mve_opcode_10(DM2_V1_MveVideo *v, DM2_MveReader *r, unsigned bx, unsigned by)
{
    uint8_t p[8]; uint32_t f32; uint64_t f64; unsigned x,y;
    for(x=0u;x<4u;++x) if(!mve_get8(r,&p[x])) return 0;
    if(p[0]<=p[1]) {
        for(y=0u;y<16u;++y) { if(!(y&3u)){if(y)for(x=0u;x<4u;++x)if(!mve_get8(r,&p[x]))return 0;if(!mve_get32(r,&f32))return 0;}for(x=0u;x<4u;++x){mve_put(v,bx+(y/8u)*4u+x,by+(y%8u),p[f32&3u]);f32>>=2u;} }
    } else {
        if(!mve_get64(r,&f64))return 0;for(x=0u;x<4u;++x)if(!mve_get8(r,&p[4u+x]))return 0;
        for(y=0u;y<16u;++y){int vert=p[4]<=p[5];for(x=0u;x<4u;++x){mve_put(v,bx+(vert?(y/8u)*4u:((y&1u)*4u))+x,by+(vert?(y%8u):(y/2u)),p[f64&3u]);f64>>=2u;}if(y==7u){memcpy(p,p+4u,4u);if(!mve_get64(r,&f64))return 0;}}
    }
    return 1;
}

static int mve_decode_block(DM2_V1_MveVideo *v, DM2_MveReader *r, uint8_t op, unsigned bx, unsigned by)
{
    uint8_t b,a; unsigned x,y;
    switch(op) {
    case 0u:return mve_copy(v,v->last,bx,by,0,0);
    case 1u:return mve_copy(v,v->second_last,bx,by,0,0);
    case 2u:if(!mve_get8(r,&b))return 0;return b<56u?mve_copy(v,v->second_last,bx,by,8+(b%7u),b/7u):mve_copy(v,v->second_last,bx,by,-14+((b-56u)%29u),8+((b-56u)/29u));
    case 3u:if(!mve_get8(r,&b))return 0;return b<56u?mve_copy(v,v->current,bx,by,-(8+(b%7u)),-(int)(b/7u)):mve_copy(v,v->current,bx,by,-(-14+(int)((b-56u)%29u)),-(8+(int)((b-56u)/29u)));
    case 4u:if(!mve_get8(r,&b))return 0;return mve_copy(v,v->last,bx,by,-8+(b&15u),-8+(b>>4u));
    case 5u:if(!mve_get8(r,&a)||!mve_get8(r,&b))return 0;return mve_copy(v,v->last,bx,by,(int)(int8_t)a,(int)(int8_t)b);
    case 6u:return 0;
    case 7u:return mve_opcode_7(v,r,bx,by);
    case 8u:return mve_opcode_8(v,r,bx,by);
    case 9u:return mve_opcode_9(v,r,bx,by);
    case 10u:return mve_opcode_10(v,r,bx,by);
    case 11u:for(y=0u;y<8u;++y)for(x=0u;x<8u;++x){if(!mve_get8(r,&b))return 0;mve_put(v,bx+x,by+y,b);}return 1;
    case 12u:for(y=0u;y<8u;y+=2u)for(x=0u;x<8u;x+=2u){if(!mve_get8(r,&b))return 0;mve_put(v,bx+x,by+y,b);mve_put(v,bx+x+1u,by+y,b);mve_put(v,bx+x,by+y+1u,b);mve_put(v,bx+x+1u,by+y+1u,b);}return 1;
    case 13u:for(y=0u;y<8u;++y){if(!(y&3u)&&(!mve_get8(r,&a)||!mve_get8(r,&b)))return 0;for(x=0u;x<8u;++x)mve_put(v,bx+x,by+y,x<4u?a:b);}return 1;
    case 14u:if(!mve_get8(r,&b))return 0;mve_fill(v,bx,by,b);return 1;
    case 15u:if(!mve_get8(r,&a)||!mve_get8(r,&b))return 0;for(y=0u;y<8u;++y)for(x=0u;x<8u;++x)mve_put(v,bx+x,by+y,((x+y)&1u)?b:a);return 1;
    default:return 0;
    }
}

void dm2_v1_mve_video_init(DM2_V1_MveVideo *video)
{
    if (!video) return;
    memset(video,0,sizeof(*video)); video->initialized=1;
}

int dm2_v1_mve_video_decode_presentation(DM2_V1_MveVideo *v,const DM2_V1_MvePresentation *p,const uint8_t *bytes,size_t n)
{
    DM2_MveReader r; uint16_t gridx,gridy; unsigned block, x, y; uint8_t opcode;
    if(!v||!p||!bytes||!v->initialized||!p->valid||p->code_map_size!=500u||p->video_version!=3u||p->video_data_size<14u||p->code_map_offset>n-p->code_map_size||p->video_data_offset>n-p->video_data_size)return 0;
    if(p->palette_size){uint16_t start,count;size_t i;if(p->palette_offset>n-p->palette_size||p->palette_size<4u)return 0;start=(uint16_t)(bytes[p->palette_offset]|((uint16_t)bytes[p->palette_offset+1u]<<8u));count=(uint16_t)(bytes[p->palette_offset+2u]|((uint16_t)bytes[p->palette_offset+3u]<<8u));if((size_t)count*3u+4u!=p->palette_size||(unsigned)start+(unsigned)count>256u)return 0;for(i=0u;i<count*3u;++i){uint8_t q=bytes[p->palette_offset+4u+i];v->palette_rgb[(size_t)start*3u+i]=(uint8_t)((q<<2u)|(q>>4u));}}
    gridx=(uint16_t)(bytes[p->video_data_offset+8u]|((uint16_t)bytes[p->video_data_offset+9u]<<8u));gridy=(uint16_t)(bytes[p->video_data_offset+10u]|((uint16_t)bytes[p->video_data_offset+11u]<<8u));
    if(gridx!=40u||gridy!=25u)return 0;
    r.data=bytes+p->video_data_offset+14u;r.size=p->video_data_size-14u;r.at=0u;
    for(block=0u;block<1000u;++block){uint8_t m=bytes[p->code_map_offset+block/2u];opcode=(block&1u)?(m>>4u):(m&15u);x=(block%40u)*8u;y=(block/40u)*8u;if(!mve_decode_block(v,&r,opcode,x,y))return 0;}
    while(r.at!=r.size) if(r.data[r.at++]!=0u)return 0;
    memcpy(v->second_last,v->last,DM2_V1_MVE_VIDEO_PIXELS);memcpy(v->last,v->current,DM2_V1_MVE_VIDEO_PIXELS);++v->decoded_presentations;return 1;
}

const uint8_t *dm2_v1_mve_video_pixels(const DM2_V1_MveVideo *video){return video&&video->initialized?video->current:NULL;}
const uint8_t *dm2_v1_mve_video_palette_rgb(const DM2_V1_MveVideo *video){return video&&video->initialized?video->palette_rgb:NULL;}
