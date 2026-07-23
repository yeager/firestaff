#include "csb_v1_f0275_wall_click_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int passed, failed;
#define CHECK(c,m) do { if(c){++passed;printf("  PASS: %s\n",m);}else{++failed;printf("  FAIL: %s\n",m);} } while(0)
enum { SENSOR=68, WEAPON=96, OBJECT=27 };
static void w(unsigned char *b,int o,unsigned short v){b[o]=(unsigned char)v;b[o+1]=(unsigned char)(v>>8);}
static unsigned short s(int i,int c){return (unsigned short)((3u<<10)|i|((unsigned)c<<14));}
static unsigned short weapon(void){return (unsigned short)(5u<<10);}
static void fixture(CSB_V1_RuntimeProfile *p,CSB_V1_DungeonData *d,unsigned char raw[160]){
 memset(d,0,sizeof(*d));memset(raw,0,160);d->level_count=1;d->level_widths[0]=3;d->level_heights[0]=3;d->square_bytes=1;d->raw_data=raw;d->raw_size=160;d->square_first_thing_base=66;d->square_first_thing_count=1;d->thing_data_bases[3]=SENSOR;d->thing_type_counts[3]=2;d->thing_data_bases[5]=WEAPON;d->thing_type_counts[5]=1;
 raw[1]=0x10;w(raw,60,0);w(raw,66,s(0,1));w(raw,SENSOR,s(1,2));w(raw,SENSOR+2,1);w(raw,SENSOR+4,0);w(raw,SENSOR+8,0xfffe);w(raw,SENSOR+10,(unsigned short)((OBJECT<<7)|3));w(raw,SENSOR+12,0);w(raw,WEAPON,0xfffe);w(raw,WEAPON+2,OBJECT);
 csb_v1_runtime_init(p,NULL);p->dungeon_handle=d;p->current_level=0;p->leader_index=0;p->party_state_valid=1;p->party_state.LeaderHandThing=weapon();
}
int main(void){CSB_V1_RuntimeProfile p;CSB_V1_DungeonData d;CSB_V1_F0275WallClickReceiptPc34 r;unsigned char raw[160];fixture(&p,&d,raw);
 CHECK(csb_v1_f0275_wall_click_receipt_pc34(&p,0,1,1,&r)==1&&r.click_accepted&&r.sensor_type==1,"C001 accepts a raw wall click");
 CHECK(csb_v1_f0275_wall_click_receipt_pc34(&p,0,1,2,&r)==1&&r.click_accepted&&r.sensor_type==3,"C003 requires matching raw leader-hand type");
 CHECK(csb_v1_f0275_wall_click_receipt_pc34(&p,0,1,3,&r)==1&&!r.click_accepted,"wrong cell has no fallback candidate");
 w(raw,SENSOR,s(3,0));CHECK(csb_v1_f0275_wall_click_receipt_pc34(&p,0,1,2,&r)==0,"stale raw chain fails closed");
 printf("PASSED: %d\nFAILED: %d\n",passed,failed);return failed?1:0;}
