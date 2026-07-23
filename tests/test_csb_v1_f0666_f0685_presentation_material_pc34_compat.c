#include "csb_v1_f0666_f0685_presentation_material_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int passed, failed;
#define C(x,m) do{if(x){++passed;printf("PASS: %s\n",m);}else{++failed;printf("FAIL: %s\n",m);}}while(0)
int main(void) {
 CSB_V1_RuntimeProfile p; CSB_V1_DungeonData d; CSB_V1_CSBGraphicsDatRealCache c;
 CSB_V1_CSBGraphicsStartupPackage k; CSB_V1_F0666F0685PresentationMaterialReceiptPc34 r;
 uint8_t raw[80], bytes[64]; const int role=CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;
 memset(&p,0,sizeof(p));memset(&d,0,sizeof(d));memset(&c,0,sizeof(c));memset(&k,0,sizeof(k));memset(raw,0,sizeof(raw));memset(bytes,0,sizeof(bytes));
 d.raw_data=raw;d.raw_size=sizeof(raw);d.square_bytes=1;d.level_count=1;d.level_widths[0]=1;d.level_heights[0]=1;p.dungeon_handle=&d;p.party_state_valid=1;p.graphics_asset.path="/pc34/CSBgraphics.dat";
 c.loaded=1;c.file_buffer=bytes;c.file_size=sizeof(bytes);c.index.count=18;snprintf(c.resolved_path,sizeof(c.resolved_path),"%s",p.graphics_asset.path);snprintf(c.matched_md5,sizeof(c.matched_md5),"%s","0123456789abcdef0123456789abcdef");
 k.valid=k.hud_ready=k.palette_material_complete=1;k.palette_source.valid=1;k.palette_source.source_kind=CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;snprintf(k.palette_source.source_path,sizeof(k.palette_source.source_path),"%s",c.resolved_path);snprintf(k.palette_source.source_md5,sizeof(k.palette_source.source_md5),"%s",c.matched_md5);k.assets[role].present=1;k.assets[role].entry_index=17;k.assets[role].decompressed_size=1;k.image_sources[role].valid=1;k.image_sources[role].source_kind=CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;snprintf(k.image_sources[role].source_path,sizeof(k.image_sources[role].source_path),"%s",c.resolved_path);snprintf(k.image_sources[role].source_md5,sizeof(k.image_sources[role].source_md5),"%s",c.matched_md5);k.image_sources[role].entry_span.entry_index=17;k.image_sources[role].entry_span.compressed_size=1;k.image_sources[role].entry_span.decompressed_size=1;
 C(csb_v1_f0666_f0685_presentation_material_receipt_pc34(&p,&c,&k,&r)&&r.source_bound_mask==CSB_V1_F0666_F0685_SOURCE_MASK&&r.endgame_owner_required&&r.pixel_copy_owner_required,"F0666-F0685 authentic material preserves existing owners");
 k.image_sources[role].source_md5[0]='\0';C(!csb_v1_f0666_f0685_presentation_material_receipt_pc34(&p,&c,&k,&r),"missing package provenance fails closed");
 printf("%d/%d\n",passed,passed+failed);return failed?1:0;
}
