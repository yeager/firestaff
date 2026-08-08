#include "dm2_v1_dos_intro_mve_owner.h"

#include "dm2_v1_mve_stream.h"

#include <stdlib.h>
#include <string.h>

struct DM2_V1_DosIntroMveOwner {
    uint8_t *bytes;
    size_t byte_count;
    uint32_t mve_offset;
    int valid;
};

DM2_V1_DosIntroMveOwner *dm2_v1_dos_intro_mve_owner_create(
    const char *install_root, const DM2_V1_DosStartupMediaReceipt *receipt)
{
    DM2_V1_DosIntroMveOwner *owner;
    DM2_V1_MveStreamReceipt stream;

    if (!receipt || !receipt->valid || !receipt->complete ||
        !receipt->intro_verified || !receipt->intro_has_interplay_mve ||
        receipt->intro_mve_header_offset == 0u)
        return NULL;
    owner = (DM2_V1_DosIntroMveOwner *)calloc(1u, sizeof(*owner));
    if (!owner || !dm2_v1_dos_startup_media_load_intro_verified(
                      install_root, receipt, &owner->bytes,
                      &owner->byte_count) ||
        !dm2_v1_mve_stream_parse(owner->bytes, owner->byte_count, &stream) ||
        !stream.valid || stream.mve_offset != receipt->intro_mve_header_offset ||
        stream.width != 320u || stream.height != 200u ||
        stream.timer_rate_us == 0u || stream.timer_subdivision == 0u) {
        dm2_v1_dos_intro_mve_owner_destroy(owner);
        return NULL;
    }
    owner->mve_offset = stream.mve_offset;
    owner->valid = 1;
    return owner;
}

void dm2_v1_dos_intro_mve_owner_destroy(DM2_V1_DosIntroMveOwner *owner)
{
    if (!owner) return;
    if (owner->bytes) {
        memset(owner->bytes, 0, owner->byte_count);
        free(owner->bytes);
    }
    memset(owner, 0, sizeof(*owner));
    free(owner);
}

int dm2_v1_dos_intro_mve_owner_readonly(
    const DM2_V1_DosIntroMveOwner *owner, const uint8_t **out_bytes,
    size_t *out_byte_count)
{
    if (out_bytes) *out_bytes = NULL;
    if (out_byte_count) *out_byte_count = 0u;
    if (!owner || !owner->valid || !owner->bytes || owner->byte_count == 0u ||
        owner->mve_offset == 0u || !out_bytes || !out_byte_count)
        return 0;
    *out_bytes = owner->bytes;
    *out_byte_count = owner->byte_count;
    return 1;
}
