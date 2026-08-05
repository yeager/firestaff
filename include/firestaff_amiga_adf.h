#ifndef FIRESTAFF_AMIGA_ADF_H
#define FIRESTAFF_AMIGA_ADF_H

#include <stddef.h>
#include <stdint.h>

/* Read-only AmigaDOS OFS ADF intake.  This is shared media infrastructure:
 * it deliberately has no CSB-specific filename or hash policy. */
typedef int (*FirestaffAmigaAdfFileVisitor)(const char *name,
                                            const uint8_t *bytes,
                                            size_t byte_count,
                                            void *user_data);

/* Asset-scanner unit tests compile the scanner as a standalone translation
 * unit. Let that consumer embed the audited reader without imposing a new
 * link dependency on every existing scanner probe. */
#ifndef FIRESTAFF_AMIGA_ADF_API
#define FIRESTAFF_AMIGA_ADF_API
#endif

/* Visit every validated root-volume file in a 512-byte-sector AmigaDOS OFS
 * image. Returns the number of delivered files, or -1 for malformed media.
 * DOS\0 (OFS) disks are accepted; FFS payloads are rejected until their
 * raw-block layout has its own audited implementation. */
FIRESTAFF_AMIGA_ADF_API int firestaff_amiga_adf_visit_ofs_files(
    const uint8_t *image, size_t image_size,
    FirestaffAmigaAdfFileVisitor visitor, void *user_data);

#endif
