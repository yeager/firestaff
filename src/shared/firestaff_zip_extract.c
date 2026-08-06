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

int firestaff_zip_extract_memory_by_suffix(const uint8_t *zip_data,
                                           size_t zip_size,
                                           const char *suffix,
                                           uint8_t **out_data,
                                           size_t *out_size) {
    size_t search_start, pos, cd_offset, cd_size;
    uint16_t count, i;
    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0U;
    if (!zip_data || !suffix || !out_data || !out_size || zip_size < 22U) return -1;
    search_start = zip_size > 65557U ? zip_size - 65557U : 0U;
    for (pos = zip_size - 22U;; --pos) {
        if (zr_u32le(zip_data + pos) == 0x06054b50U) break;
        if (pos == search_start) return -1;
    }
    count = zr_u16le(zip_data + pos + 10U);
    cd_size = zr_u32le(zip_data + pos + 12U);
    cd_offset = zr_u32le(zip_data + pos + 16U);
    if (cd_offset > zip_size || cd_size > zip_size - cd_offset) return -1;
    pos = cd_offset;
    for (i = 0U; i < count; ++i) {
        const uint8_t *header;
        uint16_t method, name_size, extra_size, comment_size, local_name, local_extra;
        uint32_t compressed_size, uncompressed_size, local_offset, data_offset;
        char name[512];
        uint8_t *decoded;
        if (pos > cd_offset + cd_size || cd_offset + cd_size - pos < 46U) return -1;
        header = zip_data + pos;
        if (zr_u32le(header) != 0x02014b50U) return -1;
        method = zr_u16le(header + 10U);
        compressed_size = zr_u32le(header + 20U);
        uncompressed_size = zr_u32le(header + 24U);
        name_size = zr_u16le(header + 28U);
        extra_size = zr_u16le(header + 30U);
        comment_size = zr_u16le(header + 32U);
        local_offset = zr_u32le(header + 42U);
        if (name_size == 0U || name_size >= sizeof(name) ||
            (size_t)name_size > cd_offset + cd_size - pos - 46U ||
            (size_t)extra_size > cd_offset + cd_size - pos - 46U - name_size ||
            (size_t)comment_size > cd_offset + cd_size - pos - 46U - name_size - extra_size)
            return -1;
        memcpy(name, header + 46U, name_size);
        name[name_size] = '\0';
        pos += 46U + name_size + extra_size + comment_size;
        if (name[name_size - 1U] == '/' || !str_iends_with(name, suffix)) continue;
        if (local_offset > zip_size || zip_size - local_offset < 30U ||
            zr_u32le(zip_data + local_offset) != 0x04034b50U) return -1;
        local_name = zr_u16le(zip_data + local_offset + 26U);
        local_extra = zr_u16le(zip_data + local_offset + 28U);
        data_offset = local_offset + 30U + local_name + local_extra;
        if (data_offset < local_offset || data_offset > zip_size ||
            compressed_size > zip_size - data_offset || uncompressed_size == 0U) return -1;
        decoded = (uint8_t *)malloc(uncompressed_size);
        if (!decoded) return -1;
        if (method == 0U) {
            if (compressed_size != uncompressed_size) { free(decoded); return -1; }
            memcpy(decoded, zip_data + data_offset, uncompressed_size);
        } else if (method == 8U) {
#ifdef FIRESTAFF_HAS_ZLIB
            z_stream stream;
            int result;
            memset(&stream, 0, sizeof(stream));
            stream.next_in = (Bytef *)(zip_data + data_offset);
            stream.avail_in = compressed_size;
            stream.next_out = decoded;
            stream.avail_out = uncompressed_size;
            if (inflateInit2(&stream, -MAX_WBITS) != Z_OK) { free(decoded); return -1; }
            result = inflate(&stream, Z_FINISH);
            inflateEnd(&stream);
            if (result != Z_STREAM_END || stream.avail_in != 0U || stream.avail_out != 0U) {
                free(decoded);
                return -1;
            }
#else
            free(decoded);
            return -1;
#endif
        } else {
            free(decoded);
            return -1;
        }
        *out_data = decoded;
        *out_size = uncompressed_size;
        return 0;
    }
    return -1;
int firestaff_zip_extract_by_suffix_to_path(const char *zip_path,
                                            const char *suffix,
                                            const char *out_path)
{
    FILE *in = NULL, *out = NULL;
    long size, start, eocd = -1;
    unsigned char *tail = NULL;
    size_t tail_size;
    uint32_t cd_offset, cd_size, pos;
    uint16_t count, i;
    if (!zip_path || !suffix || !out_path) return -1;
    in = fopen(zip_path, "rb");
    if (!in || fseek(in, 0L, SEEK_END) != 0 || (size = ftell(in)) < 22L) goto fail;
    tail_size = (size_t)(size < 65557L ? size : 65557L);
    start = size - (long)tail_size;
    tail = (unsigned char *)malloc(tail_size);
    if (!tail || fseek(in, start, SEEK_SET) != 0 ||
        fread(tail, 1U, tail_size, in) != tail_size) goto fail;
    for (long j = (long)tail_size - 22L; j >= 0L; --j) {
        if (zr_u32le(tail + j) == 0x06054b50U) {
            eocd = start + j;
            count = zr_u16le(tail + j + 10);
            cd_size = zr_u32le(tail + j + 12);
            cd_offset = zr_u32le(tail + j + 16);
            break;
        }
    }
    free(tail); tail = NULL;
    if (eocd < 0L || cd_offset + cd_size > (uint32_t)size) goto fail;
    pos = cd_offset;
    for (i = 0U; i < count && pos + 46U <= cd_offset + cd_size; ++i) {
        unsigned char h[46], local[30];
        char name[512];
        uint16_t method, name_len, extra_len, comment_len, local_name, local_extra;
        uint32_t packed, unpacked, local_offset, data_offset;
        if (fseek(in, (long)pos, SEEK_SET) != 0 || fread(h,1U,sizeof(h),in) != sizeof(h) ||
            zr_u32le(h) != 0x02014b50U) break;
        method=zr_u16le(h+10); packed=zr_u32le(h+20); unpacked=zr_u32le(h+24);
        name_len=zr_u16le(h+28); extra_len=zr_u16le(h+30); comment_len=zr_u16le(h+32);
        local_offset=zr_u32le(h+42); pos += 46U+name_len+extra_len+comment_len;
        if (!name_len || name_len >= sizeof(name) || fread(name,1U,name_len,in) != name_len) break;
        name[name_len]='\0';
        if (name[name_len-1U]=='/' || !str_iends_with(name,suffix)) continue;
        if (fseek(in,(long)local_offset,SEEK_SET)!=0 || fread(local,1U,sizeof(local),in)!=sizeof(local) ||
            zr_u32le(local)!=0x04034b50U) goto fail;
        local_name=zr_u16le(local+26); local_extra=zr_u16le(local+28);
        data_offset=local_offset+30U+local_name+local_extra;
        if (fseek(in,(long)data_offset,SEEK_SET)!=0 || !(out=fopen(out_path,"wb"))) goto fail;
        if (method == 0U) { unsigned char b[32768]; uint32_t left=unpacked; while(left){size_t n=left>sizeof(b)?sizeof(b):left;if(fread(b,1U,n,in)!=n||fwrite(b,1U,n,out)!=n)goto fail;left-=(uint32_t)n;} }
#ifdef FIRESTAFF_HAS_ZLIB
        else if (method == 8U) { unsigned char ib[32768],ob[32768]; uint32_t left=packed,done=0U; z_stream z; int rc=Z_OK; memset(&z,0,sizeof(z)); if(inflateInit2(&z,-MAX_WBITS)!=Z_OK)goto fail; while(rc!=Z_STREAM_END){if(!z.avail_in&&left){size_t n=left>sizeof(ib)?sizeof(ib):left;if(fread(ib,1U,n,in)!=n){inflateEnd(&z);goto fail;}left-=(uint32_t)n;z.next_in=ib;z.avail_in=(uInt)n;}z.next_out=ob;z.avail_out=sizeof(ob);rc=inflate(&z,Z_NO_FLUSH);if(rc!=Z_OK&&rc!=Z_STREAM_END){inflateEnd(&z);goto fail;} {size_t n=sizeof(ob)-z.avail_out;if(n&&fwrite(ob,1U,n,out)!=n){inflateEnd(&z);goto fail;}done+=(uint32_t)n;} } inflateEnd(&z); if(done!=unpacked)goto fail; }
#endif
        else goto fail;
        if (fclose(out)!=0) { out=NULL; goto fail; } fclose(in); return 0;
    }
fail:
    free(tail); if (out) { fclose(out); remove(out_path); } if (in) fclose(in); return -1;
}
