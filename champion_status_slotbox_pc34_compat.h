#ifndef REDMCSB_CHAMPION_STATUS_SLOTBOX_PC34_COMPAT_H
#define REDMCSB_CHAMPION_STATUS_SLOTBOX_PC34_COMPAT_H

typedef struct ChampionStatusSlotBoxCompat {
    unsigned int slotBoxIndex;
    unsigned int championIndex;
    unsigned int handSlot;
    unsigned int commandId;
    unsigned int zoneIndex;
    const char* evidence;
} ChampionStatusSlotBoxCompat;

typedef struct ChampionStatusNameBoxCompat {
    unsigned int championIndex;
    unsigned int commandId;
    unsigned int left;
    unsigned int right;
    unsigned int top;
    unsigned int bottom;
    unsigned int fillWidth;
    unsigned int fillHeight;
    const char* evidence;
} ChampionStatusNameBoxCompat;

unsigned int CHAMPION_Compat_GetStatusSlotBoxCount(void);
int CHAMPION_Compat_GetStatusSlotBox(unsigned int slotBoxIndex, ChampionStatusSlotBoxCompat* outBox);
const char* CHAMPION_Compat_GetStatusSlotBoxEvidence(void);

unsigned int CHAMPION_Compat_GetStatusNameBoxCount(void);
int CHAMPION_Compat_GetStatusNameBox(unsigned int championIndex, ChampionStatusNameBoxCompat* outBox);
const char* CHAMPION_Compat_GetStatusNameBoxEvidence(void);

#endif
