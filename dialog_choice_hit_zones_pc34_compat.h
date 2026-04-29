#ifndef REDMCSB_DIALOG_CHOICE_HIT_ZONES_PC34_COMPAT_H
#define REDMCSB_DIALOG_CHOICE_HIT_ZONES_PC34_COMPAT_H

typedef struct DialogChoiceHitZoneCompat { unsigned int choice; unsigned int commandId; unsigned int zoneIndex; const char* name; const char* evidence; } DialogChoiceHitZoneCompat;
unsigned int DIALOG_Compat_GetChoiceHitZoneCount(void);
int DIALOG_Compat_GetChoiceHitZone(unsigned int choice, DialogChoiceHitZoneCompat* outZone);
const char* DIALOG_Compat_GetChoiceHitZoneEvidence(void);
#endif
