#ifndef THERON_V1_TRACK02_LATER_ROUTE_CANDIDATE_INTAKE_H
#define THERON_V1_TRACK02_LATER_ROUTE_CANDIDATE_INTAKE_H
#include "theron_v1_track02_loader_trace_replay_consistency.h"
#include "theron_v1_track02_campaign_media_discovery.h"
typedef enum { THERON_V1_TRACK02_LATER_ROUTE_UNAVAILABLE=0, THERON_V1_TRACK02_LATER_ROUTE_REJECTED, THERON_V1_TRACK02_LATER_ROUTE_CAPTURE_REQUIRED } Theron_V1Track02LaterRouteCandidateStatus;
typedef struct { Theron_V1Track02LaterRouteCandidateStatus status; int observed_trace_row_consumed; int direct_media_consumed; int replay_tail_consumed; int opaque_only; uint16_t loader_pc; uint32_t record; uint32_t raw_sector; uint32_t destination_identity; uint32_t campaign_layout_epoch; } Theron_V1Track02LaterRouteCandidateReceipt;
/* Closed row grammar: `later_route_candidate loader_pc=<hex> record=<hex>
 * raw_sector=<hex> destination_identity=<hex>`. It never assigns a route ID. */
int theron_v1_track02_later_route_candidate_intake(const char *trace_row, const Theron_V1Track02CampaignMediaDiscoveryReceipt *media, const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay, uint32_t layout_epoch, Theron_V1Track02LaterRouteCandidateReceipt *out);
#endif
