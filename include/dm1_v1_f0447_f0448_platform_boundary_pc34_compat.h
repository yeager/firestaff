#ifndef FIRESTAFF_DM1_V1_F0447_F0448_PLATFORM_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0447_F0448_PLATFORM_BOUNDARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_F0447F0448SourcePlatformPc34 {
    DM1_V1_F0447_F0448_SOURCE_PLATFORM_PC34 = 1,
    DM1_V1_F0447_F0448_SOURCE_PLATFORM_ATARI_ST = 2
} DM1_V1_F0447F0448SourcePlatformPc34;

typedef struct DM1_V1_F0447F0448PlatformRequestPc34 {
    DM1_V1_F0447F0448SourcePlatformPc34 requested_platform;
    int original_source_branch_verified;
    int no_platform_emulation;
    int no_synthetic_memory_manager;
} DM1_V1_F0447F0448PlatformRequestPc34;

typedef struct DM1_V1_F0447F0448PlatformReceiptPc34 {
    int source_body_applicable;
    int execution_permitted;
    int fail_closed;
    int hang_suppressed;
    int memory_manager_suppressed;
    int suppress_platform_emulation;
    const char *source_evidence;
} DM1_V1_F0447F0448PlatformReceiptPc34;

/*
 * ReDMCSB only supplies Atari ST implementations for these entry points.
 * Both calls therefore reject on PC34 instead of recreating copy protection,
 * supervisor-mode memory probing, or a host-specific memory manager.
 */
int dm1_v1_f0447_hang_if_false_boundary_pc34(
    const DM1_V1_F0447F0448PlatformRequestPc34 *request,
    DM1_V1_F0447F0448PlatformReceiptPc34 *out_receipt);

int dm1_v1_f0448_initialize_memory_manager_boundary_pc34(
    const DM1_V1_F0447F0448PlatformRequestPc34 *request,
    DM1_V1_F0447F0448PlatformReceiptPc34 *out_receipt);

const char *dm1_v1_f0447_f0448_platform_boundary_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0447_F0448_PLATFORM_BOUNDARY_PC34_COMPAT_H */
