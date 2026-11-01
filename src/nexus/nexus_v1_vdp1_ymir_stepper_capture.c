#include "nexus_v1_vdp1_ymir_stepper_capture.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int32_t x, y; } Point;
typedef struct { int32_t value, inc, num, den, accum; } TextureStepper;
typedef struct {
    int32_t value, int_inc, frac_inc, num, den, accum;
} GouraudChannel;
typedef struct { GouraudChannel r, g, b; } GouraudStepper;
typedef struct {
    GouraudStepper gouraud;
    int gouraud_enabled;
    uint32_t dmaj;
    int32_t x, y, x_inc, y_inc;
    int32_t x_num, y_num, x_den, y_den, x_accum, y_accum;
    int32_t x_target, y_target;
    int32_t num, den, accum, target;
} Edge;
typedef struct {
    Edge left, right;
    uint32_t dmaj, step;
    int degenerate;
} Quad;
typedef struct {
    int32_t num, den, accum, target;
    int32_t x_major_inc, y_major_inc, x_minor_inc, y_minor_inc;
    int32_t x, y, x_end, y_end, aa_x_inc, aa_y_inc;
    uint32_t dmaj, step, length;
    int anti_alias;
} Line;

static uint16_t le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8U));
}

static int32_t sign13(int32_t value)
{
    uint32_t u = (uint32_t)value & UINT32_C(0x1fff);
    return (u & UINT32_C(0x1000)) ? (int32_t)(u | UINT32_C(0xffffe000)) : (int32_t)u;
}

static int32_t abs32(int32_t x) { return x < 0 ? -x : x; }

static uint8_t expand5(uint16_t value, unsigned shift)
{
    uint8_t c = (uint8_t)((value >> shift) & 31U);
    return (uint8_t)((c << 3U) | (c >> 2U));
}

static uint32_t to_rgba(uint16_t word)
{
    return UINT32_C(0xff000000) | ((uint32_t)expand5(word, 0U) << 16U) |
        ((uint32_t)expand5(word, 5U) << 8U) | expand5(word, 10U);
}

static uint32_t to_rgba_with_msb(uint16_t word)
{
    return ((word & UINT16_C(0x8000)) ? UINT32_C(0xff000000) : 0U) |
        ((uint32_t)expand5(word, 0U) << 16U) |
        ((uint32_t)expand5(word, 5U) << 8U) | expand5(word, 10U);
}

static uint32_t from_555(uint16_t word)
{
    return UINT32_C(0xff000000) | ((uint32_t)(((word & 31U) << 3U) | ((word & 31U) >> 2U)) << 16U) |
        ((uint32_t)((((word >> 5U) & 31U) << 3U) | (((word >> 5U) & 31U) >> 2U)) << 8U) |
        (uint32_t)((((word >> 10U) & 31U) << 3U) | (((word >> 10U) & 31U) >> 2U));
}

static uint16_t to_555(uint32_t rgba)
{
    return (uint16_t)((((rgba >> 16U) & 255U) >> 3U) |
        ((((rgba >> 8U) & 255U) >> 3U) << 5U) |
        (((rgba & 255U) >> 3U) << 10U) | UINT32_C(0x8000));
}

static uint16_t gouraud_blend(uint16_t base, int32_t shade_r,
                              int32_t shade_g, int32_t shade_b)
{
    int32_t r = (int32_t)(base & 31U) + shade_r - 16;
    int32_t g = (int32_t)((base >> 5U) & 31U) + shade_g - 16;
    int32_t b = (int32_t)((base >> 10U) & 31U) + shade_b - 16;
    if (r < 0) r = 0; else if (r > 31) r = 31;
    if (g < 0) g = 0; else if (g > 31) g = 31;
    if (b < 0) b = 0; else if (b > 31) b = 31;
    return (uint16_t)((unsigned)r | ((unsigned)g << 5U) | ((unsigned)b << 10U) | (base & 0x8000U));
}

static void texture_setup(TextureStepper *s, uint32_t length,
                          int32_t start, int32_t end, int hss, int select)
{
    int32_t delta;
    uint32_t adelta;
    if (hss) { start >>= 1; end >>= 1; }
    delta = end - start;
    adelta = (uint32_t)abs32(delta);
    s->value = start;
    s->inc = delta >= 0 ? 1 : -1;
    if (hss) { s->value = (s->value << 1) | (select & 1); s->inc <<= 1; }
    s->num = (int32_t)adelta;
    s->den = (int32_t)length;
    if (length <= adelta) {
        ++s->num;
        s->accum = (int32_t)adelta - (int32_t)(length << 1U);
        if (delta >= 0) ++s->accum;
    } else {
        --s->den;
        s->accum = (int32_t)length - (int32_t)(length << 1U);
        if (delta < 0) ++s->accum;
    }
    s->num <<= 1;
    s->den <<= 1;
}

static void texture_step_texel(TextureStepper *s)
{ s->value += s->inc; s->accum -= s->den; }
static void texture_step_pixel(TextureStepper *s) { s->accum += s->num; }
static int texture_should_step(const TextureStepper *s) { return s->accum >= 0; }

static void gouraud_channel_setup(GouraudChannel *s, uint32_t length,
                                  uint16_t start, uint16_t end)
{
    int32_t delta = (int32_t)end - (int32_t)start;
    uint32_t adelta = (uint32_t)abs32(delta);
    s->value = start; s->int_inc = 0; s->frac_inc = delta >= 0 ? 1 : -1;
    s->num = (int32_t)adelta; s->den = (int32_t)length;
    if (length <= adelta) {
        ++s->num;
        s->accum = (int32_t)adelta - (int32_t)(length << 1U);
        if (delta >= 0) ++s->accum;
    } else {
        --s->den;
        s->accum = -(int32_t)length;
        if (delta < 0) ++s->accum;
    }
    s->num <<= 1; s->den <<= 1;
    if (s->den != 0) {
        while (s->accum >= 0) { s->value += s->frac_inc; s->accum -= s->den; }
        while (s->num >= s->den) { s->int_inc += s->frac_inc; s->num -= s->den; }
    }
    s->accum = ~s->accum;
}

static void gouraud_setup(GouraudStepper *s, uint32_t length,
                          uint16_t a, uint16_t b)
{
    gouraud_channel_setup(&s->r, length, a & 31U, b & 31U);
    gouraud_channel_setup(&s->g, length, (a >> 5U) & 31U, (b >> 5U) & 31U);
    gouraud_channel_setup(&s->b, length, (a >> 10U) & 31U, (b >> 10U) & 31U);
}

static void gouraud_channel_step(GouraudChannel *s)
{
    s->value += s->int_inc; s->accum -= s->num;
    if (s->accum < 0) { s->value += s->frac_inc; s->accum += s->den; }
}
static void gouraud_step(GouraudStepper *s)
{ gouraud_channel_step(&s->r); gouraud_channel_step(&s->g); gouraud_channel_step(&s->b); }
static void gouraud_skip(GouraudStepper *s, uint32_t n)
{
    GouraudChannel *c[3] = {&s->r, &s->g, &s->b};
    unsigned i;
    if (!n) return;
    for (i = 0; i < 3; ++i) {
        c[i]->value += c[i]->int_inc * (int32_t)n;
        c[i]->accum -= c[i]->num * (int32_t)n;
        if (c[i]->den != 0)
            while (c[i]->accum < 0) { c[i]->value += c[i]->frac_inc; c[i]->accum += c[i]->den; }
    }
}

static void edge_setup(Edge *e, Point a, Point b, uint32_t delta)
{
    int32_t dx = sign13(b.x - a.x), dy = sign13(b.y - a.y);
    uint32_t adx = (uint32_t)abs32(dx), ady = (uint32_t)abs32(dy);
    e->dmaj = adx > ady ? adx : ady;
    e->x = a.x; e->y = a.y; e->x_inc = dx >= 0 ? 1 : -1; e->y_inc = dy >= 0 ? 1 : -1;
    e->x_num = (int32_t)(adx << 1U); e->y_num = (int32_t)(ady << 1U);
    e->x_den = e->y_den = (int32_t)(e->dmaj << 1U);
    e->x_accum = e->y_accum = ~(int32_t)e->dmaj;
    e->x_target = dy < 0 ? -1 : 0; e->y_target = dx < 0 ? -1 : 0;
    e->num = (int32_t)(e->dmaj << 1U); e->den = (int32_t)(delta << 1U);
    e->accum = ~(int32_t)delta;
    e->target = adx >= ady ? e->y_target : e->x_target;
    e->gouraud_enabled = 0;
}

static void edge_step(Edge *e)
{
    e->accum += e->num;
    if (e->accum >= e->target) {
        e->accum -= e->den;
        e->x_accum += e->x_num;
        if (e->x_accum >= e->x_target) { e->x_accum -= e->x_den; e->x += e->x_inc; }
        e->y_accum += e->y_num;
        if (e->y_accum >= e->y_target) { e->y_accum -= e->y_den; e->y += e->y_inc; }
        if (e->gouraud_enabled) gouraud_step(&e->gouraud);
    }
}

static int cross_vectors(Point a, Point b)
{
    int64_t v = (int64_t)a.x * b.y - (int64_t)a.y * b.x;
    return v < INT_MIN ? INT_MIN : (v > INT_MAX ? INT_MAX : (int)v);
}

static void quad_setup(Quad *q, Point a, Point b, Point c, Point d)
{
    uint32_t dlx=(uint32_t)abs32(sign13(d.x-a.x)), dly=(uint32_t)abs32(sign13(d.y-a.y));
    uint32_t drx=(uint32_t)abs32(sign13(c.x-b.x)), dry=(uint32_t)abs32(sign13(c.y-b.y));
    Point ab={b.x-a.x,b.y-a.y}, bc={c.x-b.x,c.y-b.y};
    Point cd={d.x-c.x,d.y-c.y}, da={a.x-d.x,a.y-d.y};
    int cs[4] = {cross_vectors(ab,bc), cross_vectors(bc,cd),
                 cross_vectors(cd,da), cross_vectors(da,ab)};
    int signs[4], sum=0; unsigned i;
    q->dmaj = dlx; if (dly>q->dmaj) q->dmaj=dly;
    if (drx>q->dmaj) q->dmaj=drx; if (dry>q->dmaj) q->dmaj=dry;
    q->dmaj &= 0xfffU; q->step=0;
    edge_setup(&q->left,a,d,q->dmaj); edge_setup(&q->right,b,c,q->dmaj);
    for(i=0;i<4;i++) { if(cs[i]==0) { q->degenerate=0; return; } signs[i]=cs[i]<0?-1:0; sum+=signs[i]; }
    /* Equivalent convexity test: a consistent turn sign is non-degenerate. */
    q->degenerate = !(sum==0 || sum==-4);
}

static void quad_step(Quad *q) { ++q->step; edge_step(&q->left); edge_step(&q->right); }

static void line_setup(Line *l, Point a, Point b, int aa)
{
    int32_t dx=b.x-a.x, dy=b.y-a.y, adx=abs32(dx), ady=abs32(dy);
    int xmajor=adx>=ady;
    l->x=a.x; l->y=a.y; l->x_end=b.x; l->y_end=b.y; l->anti_alias=aa;
    l->dmaj=(uint32_t)(adx>ady?adx:ady); l->length=l->dmaj+1U; l->step=0;
    if(xmajor) { l->x_major_inc=dx>=0?1:-1; l->y_major_inc=0; l->x_minor_inc=0; l->y_minor_inc=dy>=0?1:-1; }
    else { l->x_major_inc=0; l->y_major_inc=dy>=0?1:-1; l->x_minor_inc=dx>=0?1:-1; l->y_minor_inc=0; {int32_t t=dx;dx=dy;dy=t;t=adx;adx=ady;ady=t;} }
    l->num=ady<<1; l->den=adx<<1; l->accum=adx+1; l->target=0;
    if(!aa && dx<0) ++l->target;
    l->accum += l->num; l->x -= l->x_major_inc; l->y -= l->y_major_inc;
    if(aa) {
        int same=((a.x>b.x)==(a.y>b.y)); --l->accum; --l->target;
        if(xmajor) { l->aa_x_inc=same?0:-l->x_major_inc; l->aa_y_inc=same?-l->y_minor_inc:0; }
        else { l->aa_x_inc=same?0:-l->x_minor_inc; l->aa_y_inc=same?-l->y_major_inc:0; }
    } else { l->aa_x_inc=0; l->aa_y_inc=0; }
}

static uint32_t line_system_clip(Line *l, int32_t width, int32_t height)
{
    const int pad=1;
    uint32_t length=l->dmaj, start_clip=0, end_clip=0;
    int32_t xs=l->x, ys=l->y, xe=l->x_end, ye=l->y_end;
    width += pad+1; height += pad+1;
    if(length==0) return 0;
    if(xs>=-pad&&xs<=width&&ys>=-pad&&ys<=height&&xe>=-pad&&xe<=width&&ye>=-pad&&ye<=height) return 0;
    if((xs<-pad&&xe<-pad)||(xs>width&&xe>width)||(ys<-pad&&ye<-pad)||(ys>height&&ye>height)) {
        l->x=l->x_end; l->y=l->y_end; l->length=0; return 0;
    }
    {
        int xmajor=l->x_major_inc!=0;
        int32_t inc=l->x_major_inc+l->y_major_inc;
        int32_t start=xmajor?xs:ys, end=xmajor?xe:ye, maxlen=xmajor?width:height;
        if(inc>0&&start<-pad) start_clip=(uint32_t)(-start-1-pad);
        else if(inc<0&&start>maxlen) start_clip=(uint32_t)(start-maxlen);
        if(start_clip>length-1U) start_clip=length-1U;
        if(start_clip) {
            uint32_t i; l->x += l->x_major_inc*(int32_t)start_clip; l->y += l->y_major_inc*(int32_t)start_clip;
            for(i=0;i<start_clip;i++) { l->accum-=l->num; if(l->den) while(l->accum<=l->target) { l->accum+=l->den; l->x+=l->x_minor_inc; l->y+=l->y_minor_inc; } }
            length-=start_clip;
        }
        if(inc<0&&end<-pad) end_clip=(uint32_t)(-end-1-pad);
        else if(inc>0&&end>maxlen) end_clip=(uint32_t)(end-maxlen);
        if(end_clip>length) end_clip=length;
        if(end_clip) {
            uint32_t i; end_clip=length-end_clip; length=end_clip; l->x_end=l->x; l->y_end=l->y;
            { int32_t temp=l->accum; for(i=0;i<end_clip;i++) { temp-=l->num; if(l->den) while(temp<=l->target) { temp+=l->den; l->x_end+=l->x_minor_inc; l->y_end+=l->y_minor_inc; } } }
        }
    }
    l->length=length+1U;
    return start_clip;
}

static int line_can_step(const Line *l) { return l->step<=l->length; }
static int line_step(Line *l)
{
    ++l->step; l->x+=l->x_major_inc; l->y+=l->y_major_inc; l->accum-=l->num;
    if(l->accum<=l->target) { l->accum+=l->den; l->x+=l->x_minor_inc; l->y+=l->y_minor_inc; return l->anti_alias; }
    return 0;
}

static uint32_t color_calc(uint32_t src,uint32_t dst,int calc)
{
    if (calc == 0) return src;
    uint16_t s=to_555(src),d=to_555(dst),o=s;
    if(calc==1) { if(dst>>24U) { d=(uint16_t)(((d&31U)>>1U)|((((d>>5U)&31U)>>1U)<<5U)|((((d>>10U)&31U)>>1U)<<10U)|0x8000U); return from_555(d); } return dst; }
    if(calc==2) o=(uint16_t)(((s&31U)>>1U)|(((((s>>5U)&31U)>>1U)<<5U))|(((((s>>10U)&31U)>>1U)<<10U))|0x8000U);
    else if(calc==3 && (dst>>24U)) o=(uint16_t)(((((s&31U)+(d&31U))>>1U))|(((((s>>5U)&31U)+((d>>5U)&31U))>>1U)<<5U)|(((((s>>10U)&31U)+((d>>10U)&31U))>>1U)<<10U)|0x8000U);
    return from_555(o);
}

typedef struct { uint32_t u; uint8_t texel; uint16_t word; int transparent, end_code; } Texel;

static int read_texel(const TextureStepper *us,int32_t v,int width,int height,
                      const uint8_t *texture,size_t texture_size,
                      const uint8_t *palette,uint16_t draw_mode,
                      uint16_t colour_control, unsigned int colour_mode,
                      Texel *out)
{
    Texel t; uint32_t u=(uint32_t)us->value; uint32_t index;
    if (!out || us->value < 0 || v < 0 || us->value >= width || v >= height)
        return 0;
    index=u+(uint32_t)v*(uint32_t)width;
    t.u=u;
    if (colour_mode == 5U) {
        if ((size_t)index > (texture_size / 2U) ||
            (size_t)index * 2U > texture_size - 2U) return 0;
        t.texel = 1U;
        t.word = (uint16_t)(((uint16_t)texture[(size_t)index * 2U] << 8U) |
                            texture[(size_t)index * 2U + 1U]);
        t.end_code=t.word==UINT16_C(0x7fff) && !(draw_mode&0x80U);
        t.transparent=(t.word&UINT16_C(0x8000))==0U && !(draw_mode&0x40U);
    } else if (colour_mode == 4U) {
        if ((size_t)index >= texture_size) return 0;
        t.texel = texture[index];
        t.end_code = t.texel == 0xffU && !(draw_mode & 0x80U);
        t.transparent = t.texel == 0U && !(draw_mode & 0x40U);
        t.word = (uint16_t)((colour_control & UINT16_C(0xff00)) | t.texel);
    } else {
        if ((size_t)(index >> 1U) >= texture_size) return 0;
        {
            uint8_t packed=texture[index>>1U];
            t.texel=(uint8_t)((packed>>((~u&1U)*4U))&15U);
        }
        t.end_code=t.texel==15U && !(draw_mode&0x80U);
        t.transparent=t.texel==0U && !(draw_mode&0x40U);
        t.word=le16(palette+(size_t)t.texel*2U);
    }
    *out=t;
    return 1;
}

static int paint_pixel(Nexus_V1_Vdp1YmirCaptureSurface *working,
                       const Nexus_V1_Vdp1YmirCaptureInput *in,
                       int x,int y,int userclip_enabled,int clip_mode,int calc,
                       int mesh,int msb_only,uint32_t rgba,
                       int *sysclip,int *userclip)
{
    if(x<0||y<0||x>in->sys_clip_h||y>in->sys_clip_v||x>=NEXUS_FB_W||y>=NEXUS_FB_H) { ++*sysclip; return 0; }
    if(userclip_enabled) {
        int outside=x<in->user_clip_x0||x>in->user_clip_x1||y<in->user_clip_y0||y>in->user_clip_y1;
        if(outside!=clip_mode) { ++*userclip; return 0; }
    }
    /* Ymir vdp_renderer_sw.cpp::VDP1PlotPixel skips alternating mesh pixels
     * when transparent-mesh enhancement is disabled. */
    if (mesh && ((x ^ y) & 1)) return 0;
    {
        size_t p=(size_t)y*NEXUS_FB_W+(size_t)x;
        if (msb_only) {
            /* Ymir's MSB-on path preserves RGB and sets only the pixel MSB. */
            working->rgba[p] |= UINT32_C(0xff000000);
            working->command_owner[p]=in->command_index;
            return 1;
        }
        /* Shadow calculation leaves a transparent destination untouched. */
        if (calc == 1 && (working->rgba[p] >> 24U) == 0U) return 0;
        working->rgba[p]=color_calc(rgba,working->rgba[p],calc);
        working->command_owner[p]=in->command_index;
    }
    return 1;
}

static void gouraud_get(const Edge *e,int32_t *r,int32_t *g,int32_t *b)
{ *r=e->gouraud.r.value; *g=e->gouraud.g.value; *b=e->gouraud.b.value; }

static int rasterize_distorted(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *in,
    Nexus_V1_Vdp1YmirCaptureReceipt *out,
    unsigned int expected_color_mode)
{
    Nexus_V1_Vdp1YmirCaptureReceipt rec; Nexus_V1_Vdp1TextureCommand cmd;
    Nexus_V1_Vdp1YmirCaptureSurface *tmp=NULL; Point p[4]; Quad q;
    uint16_t shade[4]={0}; int gouraud,hss,calc,clipmode,userclip,mesh,flip_h,flip_v;
    uint32_t yline; int ok=0;
    memset(&rec,0,sizeof(rec)); rec.capture_only=1; if(!out) return 0; *out=rec;
    if(!surface||!in||!in->command||!in->texture_bytes||
       (expected_color_mode == 1U && !in->palette_bytes)||
       in->command_size!=NEXUS_V1_VDP1_COMMAND_BYTES||in->texture_size<=0||
       (expected_color_mode == 1U && in->palette_size!=32)||
       (expected_color_mode == 5U && in->texture_size % 2 != 0)||
       (expected_color_mode == 4U && !in->texture_capture_verified)||
       !in->command_capture_verified||
       (expected_color_mode == 1U && (!in->source_image_join_verified||!in->palette_join_verified))||
       (expected_color_mode == 5U && !in->direct_source_join_verified)||
       !in->local_origin_verified||!in->system_clip_verified||
       !in->single_density_verified||in->sys_clip_h<0||in->sys_clip_h>511||
       in->sys_clip_v<0||in->sys_clip_v>511||
       nexus_v1_vdp1_texture_command_parse(in->command,in->command_size,&cmd)!=0||
       !cmd.texture_command||cmd.command_type!=2U||cmd.colour_mode!=expected_color_mode||
       !cmd.texture_source_range_valid||cmd.texture_byte_count!=(uint32_t)in->texture_size||
       cmd.texture_width==0U||cmd.texture_height==0U) return 0;
    gouraud=(cmd.draw_mode&0x0004U)!=0U; hss=(cmd.draw_mode&0x1000U)!=0U;
    calc=cmd.draw_mode&3U; mesh=(cmd.draw_mode&0x0100U)!=0U;
    clipmode=(cmd.draw_mode&0x0200U)!=0U; userclip=(cmd.draw_mode&0x0400U)!=0U;
    flip_h=(cmd.control&0x0010U)!=0U; flip_v=(cmd.control&0x0020U)!=0U;
    if((userclip && (!in->user_clip_verified||
       in->user_clip_x0<0||in->user_clip_y0<0||
       in->user_clip_x1<in->user_clip_x0||in->user_clip_y1<in->user_clip_y0)) ||
       (hss&&!in->even_odd_coord_select_verified)||
       in->local_origin_x < -4096 || in->local_origin_x > 4095 ||
       in->local_origin_y < -4096 || in->local_origin_y > 4095) return 0;
    if(gouraud&&(!in->vdp1_vram_hash_verified||!in->vdp1_vram||
       in->vdp1_vram_size!=(int)NEXUS_V1_VDP1_VRAM_BYTES)) return 0;
    if(gouraud) {
        uint32_t off=(uint32_t)cmd.gouraud_table_word<<3U; unsigned i;
        if(off>=(uint32_t)in->vdp1_vram_size||8U>(uint32_t)in->vdp1_vram_size-off) return 0;
        for(i=0;i<4;i++) shade[i]=le16(in->vdp1_vram+off+i*2U);
    }
    p[0]=(Point){(int32_t)((int64_t)cmd.xa+in->local_origin_x),(int32_t)((int64_t)cmd.ya+in->local_origin_y)};
    p[1]=(Point){(int32_t)((int64_t)cmd.xb+in->local_origin_x),(int32_t)((int64_t)cmd.yb+in->local_origin_y)};
    p[2]=(Point){(int32_t)((int64_t)cmd.xc+in->local_origin_x),(int32_t)((int64_t)cmd.yc+in->local_origin_y)};
    p[3]=(Point){(int32_t)((int64_t)cmd.xd+in->local_origin_x),(int32_t)((int64_t)cmd.yd+in->local_origin_y)};
    quad_setup(&q,p[0],p[1],p[2],p[3]);
    if(gouraud) {
        gouraud_setup(&q.left.gouraud,q.dmaj+1U,shade[0],shade[3]);
        gouraud_setup(&q.right.gouraud,q.dmaj+1U,shade[1],shade[2]);
        q.left.gouraud_enabled=q.right.gouraud_enabled=1;
    }
    tmp=(Nexus_V1_Vdp1YmirCaptureSurface *)malloc(sizeof(*tmp));
    if(!tmp) return 0;
    *tmp=*surface;
    rec.command_verified=1;
    rec.texture_capture_verified = expected_color_mode == 4U;
    rec.source_image_join_verified = expected_color_mode == 1U || expected_color_mode == 5U;
    rec.palette_join_verified=expected_color_mode == 1U;
    rec.local_origin_verified=rec.system_clip_verified=1;
    rec.user_clip_verified=userclip?1:0; rec.single_density_verified=1;
    rec.gouraud_verified=gouraud?1:0; rec.gouraud_applied=gouraud?1:0;
    rec.high_speed_shrink_applied=hss?1:0;

    /* Ymir QuadStepper scans one row per step along the longer vertical quad
     * edge, interpolating the opposite edge with its signed integer DDA. */
    {
        TextureStepper vs; int32_t vstart=0,vend=(int32_t)cmd.texture_height-1;
        if(flip_v) { int32_t t=vstart;vstart=vend;vend=t; }
        texture_setup(&vs,q.dmaj+1U,vstart,vend,0,0);
        for(yline=0;yline<=q.dmaj;yline++) {
            Point left={q.left.x,q.left.y}, right={q.right.x,q.right.y};
            TextureStepper us; int32_t ustart=0,uend=(int32_t)cmd.texture_width-1;
            uint32_t line_clip; int aa=0, stop=0; int end_count;
            Texel tex; Line line; GouraudStepper gs;
            int use_hss;
            while(texture_should_step(&vs)) texture_step_texel(&vs);
            texture_step_pixel(&vs);
            if(flip_h) { int32_t t=ustart;ustart=uend;uend=t; }
            use_hss=hss && ((uint32_t)abs32(right.x-left.x)>=(uint32_t)abs32(right.y-left.y)?
                (uint32_t)abs32(right.x-left.x):(uint32_t)abs32(right.y-left.y)) < (uint32_t)cmd.texture_width-1U;
            end_count=use_hss?INT_MIN:0;
            texture_setup(&us,(uint32_t)((abs32(right.x-left.x)>abs32(right.y-left.y)?abs32(right.x-left.x):abs32(right.y-left.y))+1),ustart,uend,use_hss,in->even_odd_coord_select);
            line_setup(&line,left,right,1);
            line_clip=line_system_clip(&line,in->sys_clip_h,in->sys_clip_v);
            rec.system_clip_prefix_steps_skipped += (int)line_clip;
            us.accum += us.num * (int32_t)line_clip;
            if(gouraud) {
                int32_t lr,lg,lb,rr,rg,rb;
                gouraud_get(&q.left,&lr,&lg,&lb); gouraud_get(&q.right,&rr,&rg,&rb);
                /* Each channel uses the exact Ymir LineStepper DDA length. */
                gouraud_setup(&gs,line.dmaj+1U,(uint16_t)(lr|(lg<<5)|(lb<<10)),(uint16_t)(rr|(rg<<5)|(rb<<10)));
                gouraud_skip(&gs,line_clip);
            }
            /* Initial texel fetch precedes the first line.Step() in Ymir. */
            if(!read_texel(&us,vs.value,(int)cmd.texture_width,(int)cmd.texture_height,
                           in->texture_bytes,(size_t)in->texture_size,in->palette_bytes,
                           cmd.draw_mode,cmd.colour_control,expected_color_mode,&tex)) goto cleanup;
            if(tex.end_code) ++end_count;
            for((void)line_step(&line);line_can_step(&line);aa=line_step(&line)) {
                while(texture_should_step(&us)) {
                    texture_step_texel(&us);
                    if(!read_texel(&us,vs.value,(int)cmd.texture_width,(int)cmd.texture_height,
                                   in->texture_bytes,(size_t)in->texture_size,in->palette_bytes,
                                   cmd.draw_mode,cmd.colour_control,expected_color_mode,&tex)) goto cleanup;
                    if(tex.end_code) ++end_count;
                    if(end_count==2U) { stop=1; break; }
                }
                if(stop) { ++rec.end_code_stops; break; }
                texture_step_pixel(&us);
                if(tex.end_code || (tex.transparent&&!(cmd.draw_mode&0x0040U))) {
                    ++rec.transparent_texels;
                    if(gouraud) gouraud_step(&gs);
                    continue;
                }
                {
                    uint16_t word=tex.word; uint32_t rgba;
                    if(gouraud) word=gouraud_blend(word,gs.r.value,gs.g.value,gs.b.value);
                    rgba=expected_color_mode == 4U
                        ? to_rgba_with_msb(word) : to_rgba(word);
                    if(paint_pixel(tmp,in,line.x,line.y,userclip,clipmode,calc,
                                   mesh,(cmd.draw_mode&0x8000U)!=0U,rgba,
                                   &rec.system_or_host_clipped_pixels,&rec.user_clipped_pixels))
                        ++rec.pixels_written;
                    if(aa && paint_pixel(tmp,in,line.x+line.aa_x_inc,line.y+line.aa_y_inc,
                                   userclip,clipmode,calc,mesh,(cmd.draw_mode&0x8000U)!=0U,rgba,&rec.system_or_host_clipped_pixels,
                                   &rec.user_clipped_pixels)) ++rec.pixels_written;
                    if(gouraud) gouraud_step(&gs);
                }
            }
            quad_step(&q);
        }
    }
    rec.valid=1; rec.renderer_permitted=0; *surface=*tmp; ok=1;
cleanup:
    free(tmp);
    if(!ok) { rec.valid=0; rec.renderer_permitted=0; }
    *out=rec;
    return ok;
}

int nexus_v1_vdp1_ymir_capture_rasterize_mode1_distorted(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *input,
    Nexus_V1_Vdp1YmirCaptureReceipt *out_receipt)
{
    return rasterize_distorted(surface, input, out_receipt, 1U);
}

int nexus_v1_vdp1_ymir_capture_rasterize_mode5_distorted(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *input,
    Nexus_V1_Vdp1YmirCaptureReceipt *out_receipt)
{
    return rasterize_distorted(surface, input, out_receipt, 5U);
}

int nexus_v1_vdp1_ymir_capture_rasterize_polygon(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *in,
    Nexus_V1_Vdp1YmirCaptureReceipt *out)
{
    Nexus_V1_Vdp1YmirCaptureReceipt rec;
    Nexus_V1_Vdp1TextureCommand cmd;
    Nexus_V1_Vdp1YmirCaptureSurface *tmp = NULL;
    Point p[4];
    Quad q;
    uint16_t color;
    uint16_t draw_mode;
    int calc, clip_mode, user_clip_enabled;
    uint32_t row;
    int ok = 0;

    memset(&rec, 0, sizeof(rec));
    rec.capture_only = 1;
    if (!out) return 0;
    *out = rec;
    if (!surface || !in || !in->command ||
        in->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        !in->command_capture_verified || !in->local_origin_verified ||
        !in->system_clip_verified || !in->single_density_verified ||
        in->sys_clip_h < 0 || in->sys_clip_h > 511 ||
        in->sys_clip_v < 0 || in->sys_clip_v > 511 ||
        nexus_v1_vdp1_texture_command_parse(in->command, in->command_size,
            &cmd) != 0 || cmd.command_type != 4U || cmd.texture_command)
        return 0;

    draw_mode = le16(in->command + 4U);
    color = le16(in->command + 6U);
    calc = draw_mode & 3;
    clip_mode = (draw_mode & 0x0200U) != 0U;
    user_clip_enabled = (draw_mode & 0x0400U) != 0U;
    /* This path is deliberately limited to the flat polygons in the
     * authenticated capture. Gouraud and mesh use different pixel state. */
    if ((draw_mode & 0x0100U) != 0U ||
        (draw_mode & 0x0004U) != 0U ||
        (draw_mode & 0x8000U) != 0U || (draw_mode & 3U) != 0U ||
        (user_clip_enabled && (!in->user_clip_verified ||
         in->user_clip_x0 < 0 || in->user_clip_y0 < 0 ||
         in->user_clip_x1 < in->user_clip_x0 ||
         in->user_clip_y1 < in->user_clip_y0)) ||
        in->local_origin_x < -4096 || in->local_origin_x > 4095 ||
        in->local_origin_y < -4096 || in->local_origin_y > 4095)
        return 0;

    p[0] = (Point){(int32_t)((int64_t)cmd.xa + in->local_origin_x),
                   (int32_t)((int64_t)cmd.ya + in->local_origin_y)};
    p[1] = (Point){(int32_t)((int64_t)cmd.xb + in->local_origin_x),
                   (int32_t)((int64_t)cmd.yb + in->local_origin_y)};
    p[2] = (Point){(int32_t)((int64_t)cmd.xc + in->local_origin_x),
                   (int32_t)((int64_t)cmd.yc + in->local_origin_y)};
    p[3] = (Point){(int32_t)((int64_t)cmd.xd + in->local_origin_x),
                   (int32_t)((int64_t)cmd.yd + in->local_origin_y)};
    memset(&q, 0, sizeof(q));
    quad_setup(&q, p[0], p[1], p[2], p[3]);
    tmp = (Nexus_V1_Vdp1YmirCaptureSurface *)malloc(sizeof(*tmp));
    if (!tmp) return 0;
    *tmp = *surface;
    rec.command_verified = 1;
    rec.local_origin_verified = 1;
    rec.system_clip_verified = 1;
    rec.user_clip_verified = user_clip_enabled ? 1 : 0;
    rec.single_density_verified = 1;

    /* Ymir's DrawPolygon walks the same A-D/B-C quad edges as textured
     * quads and fills each scanline with an antialiased LineStepper. */
    for (row = 0U; row <= q.dmaj; ++row) {
        Point left = {q.left.x, q.left.y};
        Point right = {q.right.x, q.right.y};
        Line line;
        uint32_t clipped;
        int aa = 0;
        line_setup(&line, left, right, 1);
        clipped = line_system_clip(&line, in->sys_clip_h, in->sys_clip_v);
        rec.system_clip_prefix_steps_skipped += (int)clipped;
        for ((void)line_step(&line); line_can_step(&line); aa = line_step(&line)) {
            uint32_t rgba = to_rgba(color);
            if ((color & 0x8000U) == 0U) rgba &= UINT32_C(0x00ffffff);
            if (paint_pixel(tmp, in, line.x, line.y, user_clip_enabled,
                            clip_mode, calc, 0, 0, rgba,
                            &rec.system_or_host_clipped_pixels,
                            &rec.user_clipped_pixels))
                ++rec.pixels_written;
            if (aa && paint_pixel(tmp, in, line.x + line.aa_x_inc,
                                  line.y + line.aa_y_inc, user_clip_enabled,
                                  clip_mode, calc, 0, 0, rgba,
                                  &rec.system_or_host_clipped_pixels,
                                  &rec.user_clipped_pixels))
                ++rec.pixels_written;
        }
        quad_step(&q);
    }
    rec.valid = 1;
    rec.renderer_permitted = 0;
    *surface = *tmp;
    ok = 1;
    free(tmp);
    if (!ok) {
        rec.valid = 0;
        rec.renderer_permitted = 0;
    }
    *out = rec;
    return ok;
}

static void write_le16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

int nexus_v1_vdp1_ymir_capture_rasterize_mode5_normal(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *in,
    Nexus_V1_Vdp1YmirCaptureReceipt *out)
{
    Nexus_V1_Vdp1TextureCommand parsed;
    Nexus_V1_Vdp1YmirCaptureInput adapted;
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES];
    int32_t x0, y0, x1, y1;
    uint16_t control;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!surface || !in || !in->command ||
        in->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        !in->command_capture_verified || !in->direct_source_join_verified ||
        !in->local_origin_verified || !in->system_clip_verified ||
        !in->single_density_verified || in->sys_clip_h < 0 ||
        in->sys_clip_h > 511 || in->sys_clip_v < 0 || in->sys_clip_v > 511 ||
        nexus_v1_vdp1_texture_command_parse(in->command, in->command_size,
                                             &parsed) != 0 ||
        !parsed.texture_command || parsed.command_type != 0U ||
        parsed.colour_mode != 5U || !parsed.texture_source_range_valid ||
        parsed.texture_byte_count != (uint32_t)in->texture_size ||
        parsed.texture_width == 0U || parsed.texture_height == 0U ||
        !in->texture_bytes || in->texture_size <= 0 ||
        (in->texture_size & 1) != 0 ||
        (parsed.draw_mode & 0x0100U) != 0U ||
        in->local_origin_x < -4096 || in->local_origin_x > 4095 ||
        in->local_origin_y < -4096 || in->local_origin_y > 4095)
        return 0;

    /* Ymir's normal-sprite command expands its origin and size to the same
     * A-B-C-D quad consumed by the distorted-sprite rasterizer. */
    x0 = parsed.xa;
    y0 = parsed.ya;
    x1 = x0 + (int32_t)parsed.texture_width - 1;
    y1 = y0 + (int32_t)parsed.texture_height - 1;
    if (x1 < -4096 || x1 > 4095 || y1 < -4096 || y1 > 4095)
        return 0;
    memcpy(command, in->command, sizeof(command));
    control = (uint16_t)((le16(command) & (uint16_t)~0x000fU) | 2U);
    write_le16(command, control);
    write_le16(command + 12U, (uint16_t)x0);
    write_le16(command + 14U, (uint16_t)y0);
    write_le16(command + 16U, (uint16_t)x1);
    write_le16(command + 18U, (uint16_t)y0);
    write_le16(command + 20U, (uint16_t)x1);
    write_le16(command + 22U, (uint16_t)y1);
    write_le16(command + 24U, (uint16_t)x0);
    write_le16(command + 26U, (uint16_t)y1);
    adapted = *in;
    adapted.command = command;
    return rasterize_distorted(surface, &adapted, out, 5U);
}

int nexus_v1_vdp1_ymir_capture_rasterize_mode4_normal_capture(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *in,
    Nexus_V1_Vdp1YmirCaptureReceipt *out)
{
    Nexus_V1_Vdp1TextureCommand parsed;
    Nexus_V1_Vdp1YmirCaptureInput adapted;
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES];
    int32_t x0, y0, x1, y1;
    uint16_t control;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!surface || !in || !in->command || !in->texture_bytes ||
        in->texture_size <= 0 || !in->texture_capture_verified ||
        !in->command_capture_verified || !in->local_origin_verified ||
        !in->system_clip_verified || !in->single_density_verified ||
        nexus_v1_vdp1_texture_command_parse(in->command, in->command_size,
                                             &parsed) != 0 ||
        !parsed.texture_command || parsed.command_type != 0U ||
        parsed.colour_mode != 4U || !parsed.texture_source_range_valid ||
        parsed.texture_byte_count != (uint32_t)in->texture_size ||
        parsed.texture_width == 0U || parsed.texture_height == 0U ||
        in->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        in->sys_clip_h < 0 || in->sys_clip_h > 511 ||
        in->sys_clip_v < 0 || in->sys_clip_v > 511 ||
        (parsed.draw_mode & 0x0100U) != 0U ||
        in->local_origin_x < -4096 || in->local_origin_x > 4095 ||
        in->local_origin_y < -4096 || in->local_origin_y > 4095)
        return 0;
    x0 = parsed.xa;
    y0 = parsed.ya;
    x1 = x0 + (int32_t)parsed.texture_width - 1;
    y1 = y0 + (int32_t)parsed.texture_height - 1;
    if (x1 < -4096 || x1 > 4095 || y1 < -4096 || y1 > 4095)
        return 0;
    memcpy(command, in->command, sizeof(command));
    control = (uint16_t)((le16(command) & (uint16_t)~0x000fU) | 2U);
    write_le16(command, control);
    write_le16(command + 12U, (uint16_t)x0);
    write_le16(command + 14U, (uint16_t)y0);
    write_le16(command + 16U, (uint16_t)x1);
    write_le16(command + 18U, (uint16_t)y0);
    write_le16(command + 20U, (uint16_t)x1);
    write_le16(command + 22U, (uint16_t)y1);
    write_le16(command + 24U, (uint16_t)x0);
    write_le16(command + 26U, (uint16_t)y1);
    adapted = *in;
    adapted.command = command;
    return rasterize_distorted(surface, &adapted, out, 4U);
}

int nexus_v1_vdp1_ymir_capture_rasterize_polyline(
    Nexus_V1_Vdp1YmirCaptureSurface *surface,
    const Nexus_V1_Vdp1YmirCaptureInput *in,
    Nexus_V1_Vdp1YmirCaptureReceipt *out)
{
    Nexus_V1_Vdp1YmirCaptureReceipt rec;
    Nexus_V1_Vdp1TextureCommand cmd;
    Nexus_V1_Vdp1YmirCaptureSurface *tmp;
    Point points[4];
    unsigned int segment;
    int userclip, clipmode, calc, ok = 0;
    uint32_t rgba;
    uint16_t color;
    memset(&rec, 0, sizeof(rec));
    rec.capture_only = 1;
    if (!out) return 0;
    *out = rec;
    if (!surface || !in || !in->command ||
        in->command_size != NEXUS_V1_VDP1_COMMAND_BYTES ||
        !in->command_capture_verified || !in->local_origin_verified ||
        !in->system_clip_verified || !in->single_density_verified ||
        in->sys_clip_h < 0 || in->sys_clip_h > 511 ||
        in->sys_clip_v < 0 || in->sys_clip_v > 511 ||
        in->local_origin_x < -4096 || in->local_origin_x > 4095 ||
        in->local_origin_y < -4096 || in->local_origin_y > 4095 ||
        nexus_v1_vdp1_texture_command_parse(in->command, in->command_size,
                                             &cmd) != 0 ||
        cmd.command_type != 5U || (cmd.control & 0xc000U) != 0U)
        return 0;
    userclip = (cmd.draw_mode & 0x0400U) != 0U;
    clipmode = (cmd.draw_mode & 0x0200U) != 0U;
    calc = cmd.draw_mode & 3U;
    if ((cmd.draw_mode & (0x0100U | 0x0004U)) != 0U ||
        (userclip && (!in->user_clip_verified || in->user_clip_x0 < 0 ||
         in->user_clip_y0 < 0 || in->user_clip_x1 < in->user_clip_x0 ||
         in->user_clip_y1 < in->user_clip_y0))) return 0;

    points[0] = (Point){cmd.xa + in->local_origin_x,
                        cmd.ya + in->local_origin_y};
    points[1] = (Point){cmd.xb + in->local_origin_x,
                        cmd.yb + in->local_origin_y};
    points[2] = (Point){cmd.xc + in->local_origin_x,
                        cmd.yc + in->local_origin_y};
    points[3] = (Point){cmd.xd + in->local_origin_x,
                        cmd.yd + in->local_origin_y};
    color = le16(in->command + 6U);
    rgba = to_rgba_with_msb(color);
    if ((color & 0x8000U) == 0U) rgba &= UINT32_C(0x00ffffff);
    tmp = (Nexus_V1_Vdp1YmirCaptureSurface *)malloc(sizeof(*tmp));
    if (!tmp) return 0;
    *tmp = *surface;
    rec.command_verified = rec.local_origin_verified = 1;
    rec.system_clip_verified = 1;
    rec.user_clip_verified = userclip ? 1 : 0;
    rec.single_density_verified = 1;
    for (segment = 0U; segment < 4U; ++segment) {
        Line line;
        uint32_t clipped;
        Point a = points[segment];
        Point b = points[(segment + 1U) & 3U];
        line_setup(&line, a, b, 0);
        clipped = line_system_clip(&line, in->sys_clip_h, in->sys_clip_v);
        rec.system_clip_prefix_steps_skipped += (int)clipped;
        for ((void)line_step(&line); line_can_step(&line);
             (void)line_step(&line)) {
            if (paint_pixel(tmp, in, line.x, line.y, userclip, clipmode,
                            calc, 0, (cmd.draw_mode & 0x8000U) != 0U, rgba,
                            &rec.system_or_host_clipped_pixels,
                            &rec.user_clipped_pixels))
                ++rec.pixels_written;
        }
    }
    rec.valid = 1;
    rec.renderer_permitted = 0;
    *surface = *tmp;
    ok = 1;
    free(tmp);
    if (!ok) rec.valid = 0;
    *out = rec;
    return ok;
}
