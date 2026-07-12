#include "dm2_v1_wall_ornament.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    uint32_t offsets[5] = {0, 2, 4, 6, 8};
    uint32_t sizes[5] = {2, 2, 2, 2, 2};
    uint8_t raw[10] = {1,0, 13,0, 0,0, 2,0, 0x34,0x12};
    DM2_V1_GdatEntry entries[5]; DM2_V1_AssetLoader loader;
    DM2_V1_WallOrnamentReceipt receipt;
    memset(&loader, 0, sizeof(loader)); memset(entries, 0, sizeof(entries));
    for (int i=0;i<5;++i) { entries[i].cls1=DM2_GDAT_CATEGORY_WALL_GFX; entries[i].cls2=7; entries[i].cls3=(i==4)?DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET:DM2_GDAT_ENTRY_TYPE_WORD_VALUE; entries[i].cls4=(uint8_t)(i==0?4:i==1?5:i==2?7:i==3?10:0xfd); entries[i].data_index=(uint16_t)i; }
    loader.loaded=1; loader.entries=entries; loader.entry_count=5; loader.raw_offsets=offsets; loader.raw_sizes=sizes; loader.raw_data_count=5; loader.data=raw; loader.data_size=sizeof(raw);
    if (!dm2_v1_wall_ornament_receipt(&loader,7,&receipt) || !receipt.valid || receipt.position!=13 || receipt.alcove_type!=2 || receipt.item_inside_displacement!=0x1234u || dm2_v1_wall_ornament_receipt(&loader,8,&receipt)) return 1;
    puts("DM2 wall ornament receipt: PASS"); return 0;
}
