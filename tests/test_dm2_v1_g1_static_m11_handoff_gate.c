#include "dm2_v1_viewport_renderer.h"
#include <stdint.h>
#include <stdio.h>

static uint32_t hash(const uint8_t *p, size_t n) { uint32_t h = 2166136261u; while (n--) { h ^= *p++; h *= 16777619u; } return h; }
int main(void)
{
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT] = {0};
    uint8_t px[4] = {1,2,3,4}; uint8_t pal[16] = {0};
    DM2_V1_ViewportState s;
    int item_gdat = dm2_v1_viewport_item_graphic_index(0x10,1,0);
    int ok = 1;
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    for (int miss = 0; miss < 4; ++miss) {
        uint32_t a=11,b=12,c=13,d=14;
        if (miss==0) a=0; if (miss==1) b=0; if (miss==2) c=0; if (miss==3) d=0;
        dm2_v1_viewport_set_g1_scene_static_item_material_direct(&s,1,0x10,1,123,0x1401,1,2,px,2,2,2,pal,hash(pal,16),hash(px,4),a,b,c,d);
        ok &= !s.g1_scene_item_material_ready;
    }
    dm2_v1_viewport_set_g1_scene_static_item_material_direct(&s,1,0x10,1,item_gdat,0x1401,1,2,px,2,2,2,pal,hash(pal,16),hash(px,4),11,12,13,14);
    ok &= s.g1_scene_item_material_ready && s.g1_scene_item_material_raw_gfx256_hash==11 && s.g1_scene_item_material_raw_gfx256_receipt_hash==12 && s.g1_scene_item_material_raw4_hash==13 && s.g1_scene_item_material_raw4_receipt_hash==14;
    s.item_count = 1;
    s.items[0].item_category=0x10; s.items[0].item_type=1; s.items[0].screen_x=80; s.items[0].screen_y=80;
    s.items[0].object_id=0x1401; s.items[0].map_x=1; s.items[0].map_y=2;
    s.items[0].source_g1_weapon=1; s.items[0].source_gdat_field=0;
    s.items[0].source_static_object_admitted=1; s.items[0].source_static_object_cell=3; s.items[0].source_static_object_pass=17;
    s.items[0].source_static_object_clip_rect_id=5081;
    s.items[0].source_static_object_raw_gfx256_hash=11; s.items[0].source_static_object_raw_gfx256_receipt_hash=12;
    s.items[0].source_static_object_raw4_hash=13; s.items[0].source_static_object_raw4_receipt_hash=14;
    /* The static object already joined its source cell/pass/clip route to a
     * real Rect14 row. Rendering must use that handoff instead of deriving a
     * placement from the generic item frame. */
    s.items[0].source_static_object_rect14_applied=1;
    s.items[0].source_static_object_rect14_scale64=0x20;
    s.items[0].source_static_object_rect14_lateral_offset=-3;
    s.items[0].source_static_object_rect14_flip_mirror=1;
    s.items[0].source_static_object_rect14_row_hash=15;
    s.items[0].source_static_object_rect14_placement_hash=16;
    dm2_v1_render_items(&s);
    ok &= s.asset_item_drawn_count==1 && s.last_item_asset_blit_valid &&
          s.last_item_asset_blit.dst_rect.w==1 &&
          s.last_item_asset_blit.dst_rect.h==1 &&
          s.last_item_asset_blit.flip_mirror==1;
    s.items[0].source_static_object_raw4_hash=99; s.asset_item_drawn_count=0; s.blocked_material_mask=0;
    dm2_v1_render_items(&s); ok &= s.asset_item_drawn_count==0;
    s.items[0].source_gdat_field=0xf9; s.items[0].source_static_object_raw4_hash=13; s.asset_item_drawn_count=0; s.blocked_material_mask=0;
    dm2_v1_render_items(&s); ok &= s.asset_item_drawn_count==0;
    dm2_v1_viewport_set_g1_scene_item_material_direct(&s,1,0x10,1,123,0x1401,1,2,px,2,2,2,pal,hash(pal,16),hash(px,4));
    ok &= s.g1_scene_item_material_raw_gfx256_hash==0 && s.g1_scene_item_material_raw4_hash==0;
    if (!ok) fputs("static M11 handoff gate failed\n",stderr);
    return ok ? 0 : 1;
}
