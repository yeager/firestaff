#include "firestaff_zip_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef FIRESTAFF_HAS_ZLIB
#include <zlib.h>
#endif

static uint16_t zr_u16le(const unsigned char *p) {
    return (uint16_t)((unsigned)p[0] | ((unsigned)p[1] << 8));
}

static uint32_t zr_u32le(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int str_iends_with(const char *s, const char *suffix) {
    size_t slen = strlen(s);
    size_t plen = strlen(suffix);
    if (plen > slen) return 0;
    for (size_t i = 0; i < plen; ++i) {
        if (tolower((unsigned char)s[slen - plen + i]) !=
            tolower((unsigned char)suffix[i]))
            return 0;
    }
    return 1;
}

static const char *basename_of(const char *path) {
    const char *p = strrchr(path, '/');
    return p ? p + 1 : path;
}

typedef enum {
    ZIP_MATCH_SUFFIX,
    ZIP_MATCH_BASENAME
} ZipMatchMode;

static int zip_extract_impl(const char *zip_path,
                            const char *pattern,
                            ZipMatchMode mode,
                            uint8_t **out_data,
                            size_t *out_size)
{
    FILE *fp;
    long fileSize;
    long searchStart;
    long eocdOffset = -1;
    unsigned char *tail;
    size_t tailSize;
    uint32_t cdOffset, cdSize;
    uint16_t entryCount;
    uint32_t pos;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0;
    if (!zip_path || !pattern || !out_data || !out_size)
        return -1;

    fp = fopen(zip_path, "rb");
    if (!fp) return -1;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return -1; }
    fileSize = ftell(fp);
    if (fileSize < 22) { fclose(fp); return -1; }

    tailSize = (size_t)(fileSize < 65557L ? fileSize : 65557L);
    searchStart = fileSize - (long)tailSize;
    tail = (unsigned char *)malloc(tailSize);
    if (!tail) { fclose(fp); return -1; }
    if (fseek(fp, searchStart, SEEK_SET) != 0 ||
        fread(tail, 1, tailSize, fp) != tailSize) {
        free(tail); fclose(fp); return -1;
    }

    for (long j = (long)tailSize - 22; j >= 0; --j) {
        if (tail[j] == 0x50 && tail[j+1] == 0x4b &&
            tail[j+2] == 0x05 && tail[j+3] == 0x06) {
            eocdOffset = searchStart + j;
            entryCount = zr_u16le(tail + j + 10);
            cdSize = zr_u32le(tail + j + 12);
            cdOffset = zr_u32le(tail + j + 16);
            break;
        }
    }
    free(tail);
    if (eocdOffset < 0 || cdOffset + cdSize > (uint32_t)fileSize) {
        fclose(fp); return -1;
    }

    pos = cdOffset;
    for (uint16_t i = 0; i < entryCount && pos + 46U <= cdOffset + cdSize; ++i) {
        unsigned char hdr[46];
        uint16_t method, nameLen, extraLen, commentLen;
        uint32_t compressedSize, uncompressedSize, localOffset;
        char name[512];
        int match;

        if (fseek(fp, (long)pos, SEEK_SET) != 0 ||
            fread(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) break;
        if (zr_u32le(hdr) != 0x02014b50U) break;

        method = zr_u16le(hdr + 10);
        compressedSize = zr_u32le(hdr + 20);
        uncompressedSize = zr_u32le(hdr + 24);
        nameLen = zr_u16le(hdr + 28);
        extraLen = zr_u16le(hdr + 30);
        commentLen = zr_u16le(hdr + 32);
        localOffset = zr_u32le(hdr + 42);

        if (nameLen == 0 || nameLen >= sizeof(name)) {
            pos += 46U + nameLen + extraLen + commentLen;
            continue;
        }
        if (fread(name, 1, nameLen, fp) != nameLen) break;
        name[nameLen] = '\0';
        pos += 46U + nameLen + extraLen + commentLen;

        if (name[nameLen - 1] == '/') continue;

        if (mode == ZIP_MATCH_SUFFIX)
            match = str_iends_with(name, pattern);
        else
            match = (strcasecmp(basename_of(name), pattern) == 0);

        if (!match) continue;

        /* Found matching entry — extract it */
        {
            unsigned char local[30];
            uint16_t localNameLen, localExtraLen;
            uint32_t dataOffset;
            uint8_t *buf;

            if (fseek(fp, (long)localOffset, SEEK_SET) != 0 ||
                fread(local, 1, sizeof(local), fp) != sizeof(local) ||
                zr_u32le(local) != 0x04034b50U) {
                fclose(fp); return -1;
            }
            localNameLen = zr_u16le(local + 26);
            localExtraLen = zr_u16le(local + 28);
            dataOffset = localOffset + 30U + localNameLen + localExtraLen;

            buf = (uint8_t *)malloc((size_t)uncompressedSize);
            if (!buf) { fclose(fp); return -1; }

            if (method == 0) {
                /* Stored */
                if (fseek(fp, (long)dataOffset, SEEK_SET) != 0 ||
                    fread(buf, 1, uncompressedSize, fp) != uncompressedSize) {
                    free(buf); fclose(fp); return -1;
                }
            } else if (method == 8) {
                /* Deflate */
#ifdef FIRESTAFF_HAS_ZLIB
                unsigned char inBuf[32768];
                uint32_t remaining = compressedSize;
                uint32_t produced = 0;
                z_stream zs;
                int ret;

                if (fseek(fp, (long)dataOffset, SEEK_SET) != 0) {
                    free(buf); fclose(fp); return -1;
                }
                memset(&zs, 0, sizeof(zs));
                if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
                    free(buf); fclose(fp); return -1;
                }
                zs.next_out = buf;
                zs.avail_out = uncompressedSize;
                do {
                    if (zs.avail_in == 0 && remaining > 0) {
                        size_t chunk = remaining > sizeof(inBuf)
                                           ? sizeof(inBuf)
                                           : (size_t)remaining;
                        if (fread(inBuf, 1, chunk, fp) != chunk) {
                            inflateEnd(&zs);
                            free(buf); fclose(fp); return -1;
                        }
                        remaining -= (uint32_t)chunk;
                        zs.next_in = inBuf;
                        zs.avail_in = (uInt)chunk;
                    }
                    ret = inflate(&zs, remaining == 0 ? Z_FINISH : Z_NO_FLUSH);
                    if (ret != Z_OK && ret != Z_STREAM_END &&
                        ret != Z_BUF_ERROR) {
                        inflateEnd(&zs);
                        free(buf); fclose(fp); return -1;
                    }
                    produced = uncompressedSize - zs.avail_out;
                } while (ret != Z_STREAM_END &&
                         (remaining > 0 || zs.avail_in > 0));
                inflateEnd(&zs);
                if (produced != uncompressedSize) {
                    free(buf); fclose(fp); return -1;
                }
#else
                free(buf); fclose(fp); return -1;
#endif
            } else {
                free(buf); fclose(fp); return -1;
            }

            fclose(fp);
            *out_data = buf;
            *out_size = (size_t)uncompressedSize;
            return 0;
        }
    }
    fclose(fp);
    return -1;
}

int firestaff_zip_extract_by_suffix(const char *zip_path,
                                    const char *suffix,
                                    uint8_t **out_data,
                                    size_t *out_size)
{
    return zip_extract_impl(zip_path, suffix, ZIP_MATCH_SUFFIX,
                            out_data, out_size);
}

int firestaff_zip_extract_by_name(const char *zip_path,
                                  const char *filename,
                                  uint8_t **out_data,
                                  size_t *out_size)
{
    return zip_extract_impl(zip_path, filename, ZIP_MATCH_BASENAME,
                            out_data, out_size);
}
