#include "nexus_v1_launcher.h"
#include "nexus_v1_saturn_save_capture.h"
#include <string.h>
int main(void){Nexus_V1_Engine e;Nexus_V1_SaturnSaveCaptureReceipt s;Nexus_V1_LauncherSaturnCardStartupReceipt r;memset(&e,0,sizeof(e));memset(&s,0,sizeof(s));s.valid=1;s.status=NEXUS_V1_SATURN_SAVE_CAPTURE_ADMITTED_OPAQUE;s.opaque_only=s.title_route_bound=s.champion_route_bound=1;s.image_bytes=8192;s.image_fnv1a64=9;if(!nexus_v1_engine_set_saturn_save_capture_receipt(&e,2,&s)||!nexus_v1_launcher_select_saturn_card_startup(&e,2,9,&r)||!r.valid||r.native_fnxs_path_used)return 1;if(nexus_v1_launcher_select_saturn_card_startup(&e,3,9,&r)||nexus_v1_launcher_select_saturn_card_startup(&e,2,10,&r))return 1;return 0;}
