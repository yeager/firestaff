#ifndef REDMCSB_CHAMPION_STATUS_SLOTBOX_PC34_COMPAT_H
#define REDMCSB_CHAMPION_STATUS_SLOTBOX_PC34_COMPAT_H

typedef struct ChampionStatusSlotBoxCompat { unsigned int slotBoxIndex; unsigned int championIndex; unsigned int handSlot; unsigned int commandId; unsigned int zoneIndex; const char* evidence; } ChampionStatusSlotBoxCompat;
unsigned int CHAMPION_Compat_GetStatusSlotBoxCount(void);
int CHAMPION_Compat_GetStatusSlotBox(unsigned int slotBoxIndex, ChampionStatusSlotBoxCompat* outBox);
const char* CHAMPION_Compat_GetStatusSlotBoxEvidence(void);
#endif
