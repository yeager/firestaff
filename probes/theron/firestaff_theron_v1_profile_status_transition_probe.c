#include "theron_v1_profile_status_transition.h"
#include <string.h>
int main(void){Theron_V1ProfileLaunchStatusSnapshot s={"runtime_unavailable",0};Theron_V1ProfileStatusTransition t=theron_v1_profile_status_transition("raw_track_required_ready","ready",&s);return !strcmp(t.status,"runtime_unavailable")&&!t.startup_invoked&&!t.session_allocated?0:1;}
