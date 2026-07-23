#ifndef FIRESTAFF_DM1_V1_F0541_F0560_PLATFORM_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0541_F0560_PLATFORM_BOUNDARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Source-owned boundary for F0541-F0560 routes which ReDMCSB selects only
 * for Amiga/IIGS media.  These descriptors deliberately provide no host
 * implementation: PC34 uses its own source-selected input/text/video paths. */
typedef struct {
    const char *symbol;
    const char *source_anchor;
    const char *platform_partition;
    int has_portable_pc34_route;
    const char *pc34_route_or_rationale;
} DM1_V1_F0541F0560PlatformBoundaryPc34;

const DM1_V1_F0541F0560PlatformBoundaryPc34 *
dm1_v1_f0541_f0560_platform_boundary_pc34_at(unsigned int index);
unsigned int dm1_v1_f0541_f0560_platform_boundary_pc34_count(void);
int dm1_v1_f0541_f0560_platform_boundary_has_pc34_route(
    const DM1_V1_F0541F0560PlatformBoundaryPc34 *boundary);
const char *dm1_v1_f0541_f0560_platform_boundary_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
