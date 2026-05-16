#include "dialog_choice_hit_zones_pc34_compat.h"
#include <string.h>
static const DialogChoiceHitZoneCompat kZones[] = {
 {1u,210u,456u,"BOTTOM_BUTTON","COMMAND.C:1906-1921 maps C210 choice 1 to C456_ZONE_DIALOG_BOTTOM_BUTTON"},
 {2u,211u,457u,"TOP_BUTTON","COMMAND.C:1906-1921 maps C211 choice 2 to C457_ZONE_DIALOG_TOP_BUTTON"},
 {3u,212u,460u,"BOTTOM_LEFT_BUTTON","COMMAND.C:1906-1921 maps C212 choice 3 to C460_ZONE_DIALOG_BOTTOM_LEFT_BUTTON"},
 {4u,213u,461u,"BOTTOM_RIGHT_BUTTON","COMMAND.C:1906-1921 maps C213 choice 4 to C461_ZONE_DIALOG_BOTTOM_RIGHT_BUTTON"}
};
unsigned int DIALOG_Compat_GetChoiceHitZoneCount(void){return 4u;}
int DIALOG_Compat_GetChoiceHitZone(unsigned int choice, DialogChoiceHitZoneCompat* outZone){ if(!outZone||choice<1u||choice>4u)return 0; *outZone=kZones[choice-1u]; return 1; }
const char* DIALOG_Compat_GetChoiceHitZoneEvidence(void){return "ReDMCSB source lock: COMMAND.C defines dialog choice mouse input arrays for C210..C213 using viewport/screen-centered dialog zone IDs C456/C457/C460/C461, and COMMAND.C:2459-2461 converts command C210..C213 to G0335_ui_SelectedDialogChoice = command - C209.";}
