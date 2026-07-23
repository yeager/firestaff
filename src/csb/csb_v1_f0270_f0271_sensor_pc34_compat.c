#include "csb_v1_f0270_f0271_sensor_pc34_compat.h"
#include <string.h>
int csb_v1_f0270_f0271_sensor_receipt_pc34(const CSB_V1_RuntimeProfile *p,int effect,int x,int y,int cell,CSB_V1_F0270F0271ReceiptPc34 *out){
 CSB_V1_F0270F0271ReceiptPc34 r; int t,g; if(!out)return 0; memset(&r,0,sizeof r);r.first_sensor=r.last_sensor=-1;*out=r;
 if(!p||!p->dungeon_handle||!p->dungeon_handle->raw_data||p->dungeon_handle->square_bytes!=1||p->current_level<0||cell<-1||cell>3||csb_v1_dungeon_get_raw_square(p->dungeon_handle,p->current_level,x,y)<0)return 0;
 r.map_index=p->current_level;r.map_x=x;r.map_y=y;r.cell=cell;r.effect=effect;
 if(effect==10){if(!p->party_state_valid||p->party_state.ChampionCount<=0)return 0;r.add_steal_xp=1;r.valid=1;r.source_evidence="ReDMCSB SENSOR.C F0270 C10 -> F0269 loaded party";*out=r;return 1;}
 if(effect!=1&&effect!=2)return 0;
 t=csb_v1_dungeon_get_first_thing(p->dungeon_handle,p->current_level,x,y);
 for(g=0;g<128&&t!=0xfffe&&t!=0xffff;g++){const uint8_t *q;int ty=-1,sz=0;q=csb_v1_dungeon_get_thing_record(p->dungeon_handle,(uint16_t)t,&ty,NULL,&sz);if(!q||sz<2)return 0;if(ty==3&&(cell<0||((t>>14)&3)==cell)){if(r.first_sensor<0)r.first_sensor=t;r.last_sensor=t;r.matching_sensor_count++;}t=(int)((uint16_t)q[0]|((uint16_t)q[1]<<8));}
 if(g>=128||r.matching_sensor_count<2)return 0;r.valid=1;r.source_evidence="ReDMCSB SENSOR.C F0270/F0271 raw C03 rotation admission";*out=r;return 1;
}
