#include "theron_v1_track02_later_route_candidate_intake.h"
#include <stdio.h>
int main(void){Theron_V1Track02LaterRouteCandidateReceipt r;if(!theron_v1_track02_later_route_candidate_intake(0,0,0,0,0,&r)||r.status!=THERON_V1_TRACK02_LATER_ROUTE_UNAVAILABLE)return 1;if(!theron_v1_track02_later_route_candidate_intake("later_route_candidate loader_pc=4000 record=510 raw_sector=511 destination_identity=1",0,0,0,7,&r)||r.status!=THERON_V1_TRACK02_LATER_ROUTE_REJECTED)return 2;puts("test_theron_v1_track02_later_route_candidate_intake: PASS (no corpus)");return 0;}
