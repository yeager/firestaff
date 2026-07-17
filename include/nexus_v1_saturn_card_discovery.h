#ifndef NEXUS_V1_SATURN_CARD_DISCOVERY_H
#define NEXUS_V1_SATURN_CARD_DISCOVERY_H
#include <stdint.h>
#include <stddef.h>
typedef struct { const char *const *paths; size_t path_count; } Nexus_V1_SaturnCardDiscoveryInput;
typedef struct { int valid; int ambiguous; int virtual_candidate_seen; int direct_launch_permitted; uint64_t image_fnv1a64; char path[512]; int opaque_only; } Nexus_V1_SaturnCardDiscoveryReceipt;
int nexus_v1_saturn_card_discover(const Nexus_V1_SaturnCardDiscoveryInput *, Nexus_V1_SaturnCardDiscoveryReceipt *);
#endif
