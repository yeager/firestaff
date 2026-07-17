#include "dm2_v1_gdat_door_panel_m11_consumer.h"

#include <stdio.h>
#include <string.h>

static uint32_t hash_bytes(const uint8_t *bytes, size_t count)
{ uint32_t h=2166136261u; while(count--){h^=*bytes++;h*=16777619u;} return h; }

int main(void)
{
    DM2_V1_GdatDoorOverlayM11CommandPlan plan={0};
    DM2_V1_Dm2ViewportM11CompositionReceipt composition={0};
    DM2_V1_GdatDoorPanelM11Receipt receipt;
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH*DM2_VP_HEIGHT], raw[4]={1,2,3,4}, pixels[4]={1,2,3,4};
    int positive, palette_drift, motion_reject;
    memset(framebuffer,0xa5,sizeof(framebuffer)); dm2_v1_viewport_init(&viewport,framebuffer,DM2_VP_WIDTH);
    plan.valid=1; plan.command_count=1; plan.command_hash=7;
    DM2_V1_GdatDoorOverlayM11Command *c=&plan.commands[0];
    c->kind=DM2_V1_GDAT_DOOR_PANEL; c->category=DM2_GDAT_CATEGORY_DOORS; c->pixels=pixels;
    c->width=c->height=2; c->source_width=c->source_height=c->rect_width=c->rect_height=2;
    c->rect_x=12; c->rect_y=22; c->color_key=2; c->material_source_bytes=raw; c->material_source_byte_count=4;
    c->raw_hash=hash_bytes(raw,4); c->decoded_hash=hash_bytes(pixels,4);
    c->palette16[1]=0x31; c->palette16[2]=0x32; c->palette16[3]=0x33; c->palette16[4]=0x34;
    c->palette_hash=hash_bytes(c->palette16,16); c->material_receipt_hash=1; c->geometry_hash=2;
    composition.valid=composition.no_draw=1; composition.identity_hash=9; composition.session_identity=composition.data_epoch=1;
    composition.door_command_hash=7; composition.surface_before=composition.surface_after=viewport.surface_snapshot;
    positive=dm2_v1_gdat_door_panel_m11_receipt_build(&plan,0,&composition,&viewport,&receipt)&&
        dm2_v1_gdat_door_panel_m11_consume(&receipt,&plan,&composition,&viewport)&&
        framebuffer[22*DM2_VP_WIDTH+12]==0x31 && framebuffer[22*DM2_VP_WIDTH+13]==0xa5 &&
        framebuffer[23*DM2_VP_WIDTH+12]==0x33 && framebuffer[23*DM2_VP_WIDTH+13]==0x34;
    ++c->palette16[1]; palette_drift=!dm2_v1_gdat_door_panel_m11_consume(&receipt,&plan,&composition,&viewport)&&framebuffer[22*DM2_VP_WIDTH+12]==0x31; --c->palette16[1];
    c->movement_active=1; motion_reject=!dm2_v1_gdat_door_panel_m11_receipt_build(&plan,0,&composition,&viewport,&receipt);
    printf("positive=%d palette=%d motion=%d\n",positive,palette_drift,motion_reject);
    return positive&&palette_drift&&motion_reject?0:1;
}
