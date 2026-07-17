#include "theron_v1_track02_later_route_candidate_manifest_import.h"
#include <stdio.h>
int main(void){Theron_V1Track02LaterRouteCandidateReceipt x;if(!theron_v1_track02_later_route_candidate_manifest_import("/tmp/no-later-manifest",0,0,0,0,&x)||x.status!=THERON_V1_TRACK02_LATER_ROUTE_UNAVAILABLE)return 1;puts("test_theron_v1_track02_later_route_candidate_manifest_import: PASS (no corpus)");return 0;}
