#include "theron_v1_profile_launch_status.h"
#include <string.h>
int main(void){Theron_V1TraceProvenanceReceipt t={1,0,"x"};Theron_V1ProfileLaunchStatusSnapshot s=theron_v1_profile_launch_status(&t);return !strcmp(s.status,"runtime_unavailable")&&!s.startup_invoked?0:1;}
