#ifndef NEXUS_V1_WARNING_DGT2_SOURCE_ADMISSION_H
#define NEXUS_V1_WARNING_DGT2_SOURCE_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_V1_WARNING_BIN_BYTES 101256U
#define NEXUS_V1_WARNING_BIN_SHA256 \
    "8783fa9defda0a358d0474da56480d476b5511c8ca6d3eb61fe097c5697d44ab"
#define NEXUS_V1_WARNING_BIN_FNV1A64_JA UINT64_C(0x2692625b7981c93e)
#define NEXUS_V1_WARNING_BIN_FNV1A64_EN UINT64_C(0x1baeea0a29d4b5bf)
#define NEXUS_V1_WARNING_BIN_FNV1A64_FR UINT64_C(0xf5e1a22ae1a3e91f)
#define NEXUS_V1_WARNING_BIN_FNV1A64 NEXUS_V1_WARNING_BIN_FNV1A64_JA

typedef struct {
    int sha256_verified;
    /* A runtime caller may pair this raw byte witness with the engine's
     * canonical direct-asset receipt when SHA-256 is not materialized there. */
    int canonical_fnv1a64_verified;
    const char *sha256_hex;
    uint64_t source_fnv1a64;
} Nexus_V1_WarningDgt2SourceIdentity;

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int dgt2_resource_bound;
    int clut_semantics_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t resource_id;
    uint32_t dgt2_offset;
    uint32_t dgt2_length;
    uint64_t dgt2_fnv1a64;
    uint32_t clut_offset;
    uint32_t clut_length;
    uint64_t clut_fnv1a64;
    uint32_t pixel_offset;
    uint32_t pixel_length;
    uint64_t pixel_fnv1a64;
} Nexus_V1_WarningDgt2SourceAdmissionReceipt;

/* Uses the existing bounded RES-star/DGT2 lookup to bind only raw resource-zero
 * spans. SHA-256 attestation or the fixed canonical FNV witness is required.
 * It neither converts the CLUT nor interprets or renders pixel bytes. */
int nexus_v1_warning_dgt2_source_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity,
    Nexus_V1_WarningDgt2SourceAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
