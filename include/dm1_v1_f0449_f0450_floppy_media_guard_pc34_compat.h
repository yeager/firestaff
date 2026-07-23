#ifndef FIRESTAFF_DM1_V1_F0449_F0450_FLOPPY_MEDIA_GUARD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0449_F0450_FLOPPY_MEDIA_GUARD_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0449F0450FloppyMediaRequestPc34 {
    const uint8_t *raw_media_bytes;
    size_t raw_media_byte_count;
    uint32_t raw_media_fingerprint;
    int original_pc34_media_verified;
    int no_emulated_media;
    int no_synthetic_availability;
} DM1_V1_F0449F0450FloppyMediaRequestPc34;

typedef struct DM1_V1_F0449F0450FloppyMediaReceiptPc34 {
    int source_body_applicable;
    int media_operation_permitted;
    int fail_closed;
    int write_protection_probe_suppressed;
    int media_change_probe_suppressed;
    int suppress_synthetic_availability;
    const char *source_evidence;
} DM1_V1_F0449F0450FloppyMediaReceiptPc34;

/*
 * The available F0449 and F0450 source bodies are Atari ST-only floppy
 * operations. PC34 callers therefore receive a source-evidenced rejection,
 * never an inferred writable medium or a simulated media-change event.
 */
int dm1_v1_f0449_is_disk_write_protected_guard_pc34(
    const DM1_V1_F0449F0450FloppyMediaRequestPc34 *request,
    DM1_V1_F0449F0450FloppyMediaReceiptPc34 *out_receipt);

int dm1_v1_f0450_force_media_change_detection_guard_pc34(
    const DM1_V1_F0449F0450FloppyMediaRequestPc34 *request,
    DM1_V1_F0449F0450FloppyMediaReceiptPc34 *out_receipt);

const char *dm1_v1_f0449_f0450_floppy_media_guard_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0449_F0450_FLOPPY_MEDIA_GUARD_PC34_COMPAT_H */
