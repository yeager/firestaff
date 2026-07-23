#include "csb_v1_f0268_f0269_sensor_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed;
#define CHECK(x, m) do { if (x) printf("PASS: %s\n", m); else { ++failed; printf("FAIL: %s\n", m); } } while (0)
int main(void) {
    CSB_V1_RuntimeProfile profile; CSB_V1_DungeonData dungeon;
    CSB_V1_F0268F0269ReceiptPc34 receipt; unsigned char raw[80];
    csb_v1_runtime_init(&profile, NULL); memset(&dungeon, 0, sizeof(dungeon)); memset(raw, 0, sizeof(raw));
    dungeon.raw_data=raw; dungeon.raw_size=80; dungeon.square_bytes=1; dungeon.level_count=1; dungeon.level_widths[0]=1; dungeon.level_heights[0]=1;
    profile.dungeon_handle=&dungeon; profile.current_level=0; profile.game_time=10;
    CHECK(csb_v1_f0268_add_event_pc34(&profile, 7, 0, 0, 2, 9, 15, &receipt)==1 && receipt.valid && receipt.event_time==15, "F0268 adds only an authenticated raw-square event");
    profile.party_state_valid=1; profile.party_state.ChampionCount=2; profile.leader_index=0;
    profile.party_state.Champions[0].CurrentHealth=10; profile.party_state.Champions[1].CurrentHealth=10;
    CHECK(csb_v1_f0269_skill_experience_receipt_pc34(&profile, 8, 300, 0, &receipt)==1 && receipt.recipient_count==2 && receipt.experience_per_recipient==150, "F0269 divides XP by party count for living champions");
    CHECK(csb_v1_f0269_skill_experience_receipt_pc34(&profile, 8, 300, 1, &receipt)==1 && receipt.recipient_count==1 && receipt.experience_per_recipient==300, "F0269 preserves leader-only XP");
    profile.dungeon_handle=NULL;
    CHECK(csb_v1_f0268_add_event_pc34(&profile,7,0,0,2,9,15,&receipt)==0 && !receipt.valid, "missing PC34 source fails closed");
    return failed ? 1 : 0;
}
