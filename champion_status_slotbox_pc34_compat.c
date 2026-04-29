#include "champion_status_slotbox_pc34_compat.h"
#include <string.h>
unsigned int CHAMPION_Compat_GetStatusSlotBoxCount(void){return 8u;}
int CHAMPION_Compat_GetStatusSlotBox(unsigned int slotBoxIndex, ChampionStatusSlotBoxCompat* outBox){ if(!outBox||slotBoxIndex>7u)return 0; memset(outBox,0,sizeof(*outBox)); outBox->slotBoxIndex=slotBoxIndex; outBox->championIndex=slotBoxIndex>>1; outBox->handSlot=slotBoxIndex&1u; outBox->commandId=20u+slotBoxIndex; outBox->zoneIndex=211u+slotBoxIndex; outBox->evidence="DATA.C:977-985 maps status slot boxes 0..7 to C211..C218; COMMAND.C:489-496 maps C020..C027; CHAMPION.C:677-683 routes slotBox<8 to championIndex=slotBox>>1 and hand slot M070"; return 1;}
const char* CHAMPION_Compat_GetStatusSlotBoxEvidence(void){return "ReDMCSB source lock: PC/F20+ status slot boxes 0..7 are champion top-row ready/action hands only; slotBox>>1 selects champion 0..3 and slotBox&1 selects ready/action hand.";}
