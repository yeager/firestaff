#ifndef FIRESTAFF_LZEXE_UNPACK_V091_H
#define FIRESTAFF_LZEXE_UNPACK_V091_H

#ifdef __cplusplus
extern "C" {
#endif

/* Unpack the LZEXE 0.91 load module from an MZ executable into a malloc-owned
 * buffer. Returns 1 on success; caller releases outBytes with free(). */
int V1_LZEXE_UnpackLoadModule091(const unsigned char* data,
                                 unsigned int dataBytes,
                                 unsigned char** outBytes,
                                 unsigned int* outByteCount);

#ifdef __cplusplus
}
#endif

#endif
