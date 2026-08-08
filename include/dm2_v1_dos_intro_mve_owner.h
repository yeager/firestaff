#ifndef FIRESTAFF_DM2_V1_DOS_INTRO_MVE_OWNER_H
#define FIRESTAFF_DM2_V1_DOS_INTRO_MVE_OWNER_H

#include "dm2_v1_dos_startup_media.h"

#include <stddef.h>
#include <stdint.h>

/* Opaque BootProfile-owned resident source for the DOS IBMIOP INTRO member.
 * Its bytes originate only from the selected user install and are hash checked
 * before allocation is published.  There is no decoded-file cache and no
 * writable movie access path. */
typedef struct DM2_V1_DosIntroMveOwner DM2_V1_DosIntroMveOwner;

DM2_V1_DosIntroMveOwner *dm2_v1_dos_intro_mve_owner_create(
    const char *install_root, const DM2_V1_DosStartupMediaReceipt *receipt);
void dm2_v1_dos_intro_mve_owner_destroy(DM2_V1_DosIntroMveOwner *owner);

/* The only consumer seam.  The returned pointer remains owned by the
 * BootProfile and is valid until its next scan/reset/cleanup. */
int dm2_v1_dos_intro_mve_owner_readonly(
    const DM2_V1_DosIntroMveOwner *owner, const uint8_t **out_bytes,
    size_t *out_byte_count);

#endif /* FIRESTAFF_DM2_V1_DOS_INTRO_MVE_OWNER_H */
