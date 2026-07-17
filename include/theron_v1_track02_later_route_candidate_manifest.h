#ifndef THERON_V1_TRACK02_LATER_ROUTE_CANDIDATE_MANIFEST_H
#define THERON_V1_TRACK02_LATER_ROUTE_CANDIDATE_MANIFEST_H
#include "theron_v1_track02_later_route_candidate_intake.h"
/* Exports one opaque operator request. No route ID or payload field exists. */
int theron_v1_track02_later_route_candidate_manifest_write(const Theron_V1Track02LaterRouteCandidateReceipt *candidate,const char *trace_md5,const char *track02_md5,const char *path);
#endif
