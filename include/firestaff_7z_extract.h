#ifndef FIRESTAFF_7Z_EXTRACT_H
#define FIRESTAFF_7Z_EXTRACT_H

#include <stddef.h>
#include <stdint.h>

/* Strict in-memory reader for the ordinary one-file, one-folder LZMA2 7z
 * transport used by supplied CSB preservation media. It deliberately rejects
 * encoded headers, encryption, solid/multi-file archives and every method
 * other than LZMA2. The caller owns *out_bytes on success. */
int firestaff_7z_extract_single_lzma2_file(const char *path,
                                           uint8_t **out_bytes,
                                           size_t *out_size,
                                           char *out_name,
                                           size_t out_name_size);

#endif
