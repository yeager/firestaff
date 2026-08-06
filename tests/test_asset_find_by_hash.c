#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define RMDIR(path) _rmdir(path)
static int test_setenv(const char* name, const char* value) {
    return _putenv_s(name, value ? value : "") == 0;
}
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir((path), 0700)
#define RMDIR(path) rmdir(path)
static int test_setenv(const char* name, const char* value) {
    if (value) {
        return setenv(name, value, 1) == 0;
    }
    return unsetenv(name) == 0;
}
#endif

static int write_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(payload, 1U, sizeof(payload) - 1U, fp) != sizeof(payload) - 1U) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void put16(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
}

static void putbe16(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)((v >> 8U) & 0xffU);
    p[1] = (unsigned char)(v & 0xffU);
}

static void put32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xffU);
    p[1] = (unsigned char)((v >> 8U) & 0xffU);
    p[2] = (unsigned char)((v >> 16U) & 0xffU);
    p[3] = (unsigned char)((v >> 24U) & 0xffU);
}

static void putbe32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)((v >> 24U) & 0xffU);
    p[1] = (unsigned char)((v >> 16U) & 0xffU);
    p[2] = (unsigned char)((v >> 8U) & 0xffU);
    p[3] = (unsigned char)(v & 0xffU);
}

/* Minimal valid AmigaDOS OFS disk: it deliberately uses a filesystem file
 * header and data block, rather than pretending an ADF is a generic archive. */
static int write_adf_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char name[] = "GRAPHICS.DAT";
    unsigned char image[4U * 512U] = {0};
    unsigned char* header = image + 2U * 512U;
    unsigned char* data = image + 3U * 512U;
    FILE* fp;
    memcpy(image, "DOS\0", 4U);
    putbe32(header, 2U);
    putbe32(header + 4U, 2U);
    putbe32(header + 8U, 1U);
    putbe32(header + 77U * 4U, 3U);
    putbe32(header + 81U * 4U, (unsigned int)(sizeof(payload) - 1U));
    header[432U] = (unsigned char)(sizeof(name) - 1U);
    memcpy(header + 433U, name, sizeof(name) - 1U);
    putbe32(header + 127U * 4U, 0xfffffffdU);
    putbe32(data, 8U);
    putbe32(data + 4U, 2U);
    putbe32(data + 8U, 1U);
    putbe32(data + 12U, (unsigned int)(sizeof(payload) - 1U));
    memcpy(data + 24U, payload, sizeof(payload) - 1U);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(image, 1U, sizeof(image), fp) != sizeof(image)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

/* Minimal raw GEMDOS/FAT12 .ST image: root GRAPHICS.DAT at cluster 2. */
static int write_atari_st_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    unsigned char image[10U * 512U] = {0};
    unsigned char* root = image + 3U * 512U;
    FILE* fp;
    image[0] = 0x60U;
    put16(image + 11U, 512U);
    image[13] = 1U;
    put16(image + 14U, 1U);
    image[16] = 2U;
    put16(image + 17U, 16U);
    put16(image + 19U, 10U);
    image[21] = 0xf9U;
    put16(image + 22U, 1U);
    memcpy(root, "GRAPHICS", 8U);
    memcpy(root + 8U, "DAT", 3U);
    put16(root + 26U, 2U);
    put32(root + 28U, (unsigned int)(sizeof(payload) - 1U));
    image[512U] = 0xf0U; image[513U] = 0xffU; image[514U] = 0xffU;
    image[1024U] = 0xf0U; image[1025U] = 0xffU; image[1026U] = 0xffU;
    memcpy(image + 4U * 512U, payload, sizeof(payload) - 1U);
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(image, 1U, sizeof(image), fp) != sizeof(image)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

/* One uncompressed MSA track containing the same FAT12 layout as the raw ST
 * fixture. MSA's 0x0e0f header and per-track BE16 length are intentional. */
static int write_atari_msa_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    unsigned char image[10U * 512U] = {0};
    unsigned char header[12] = {0};
    unsigned char packed[10U * 512U];
    unsigned char* root = image + 3U * 512U;
    FILE* fp;
    size_t source = 0u, packed_size = 0u;
    image[0] = 0x60U;
    put16(image + 11U, 512U); image[13] = 1U;
    put16(image + 14U, 1U); image[16] = 2U;
    put16(image + 17U, 16U); put16(image + 19U, 10U);
    image[21] = 0xf9U; put16(image + 22U, 1U);
    memcpy(root, "GRAPHICS", 8U); memcpy(root + 8U, "DAT", 3U);
    put16(root + 26U, 2U);
    put32(root + 28U, (unsigned int)(sizeof(payload) - 1U));
    image[512U] = 0xf0U; image[513U] = 0xffU; image[514U] = 0xffU;
    image[1024U] = 0xf0U; image[1025U] = 0xffU; image[1026U] = 0xffU;
    memcpy(image + 4U * 512U, payload, sizeof(payload) - 1U);
    while (source < sizeof(image)) {
        size_t run = 1u;
        while (source + run < sizeof(image) && image[source + run] == image[source] &&
               run < 65535u) ++run;
        if (run >= 4u || image[source] == 0xe5u) {
            packed[packed_size++] = 0xe5u;
            packed[packed_size++] = image[source];
            putbe16(packed + packed_size, (unsigned int)run);
            packed_size += 2u;
        } else {
            memcpy(packed + packed_size, image + source, run);
            packed_size += run;
        }
        source += run;
    }
    if (packed_size >= sizeof(image)) return 0;
    putbe16(header, 0x0e0fU); putbe16(header + 2U, 10U);
    putbe16(header + 4U, 0U); putbe16(header + 6U, 0U);
    putbe16(header + 8U, 0U); putbe16(header + 10U, (unsigned int)packed_size);
    fp = fopen(path, "wb");
    if (!fp || fwrite(header, 1U, sizeof(header), fp) != sizeof(header) ||
        fwrite(packed, 1U, packed_size, fp) != packed_size) {
        if (fp) fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_stored_zip_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char name[] = "dm2/RENAMED.BIN";
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int payloadSize = (unsigned int)(sizeof(payload) - 1U);
    unsigned int nameLen = (unsigned int)(sizeof(name) - 1U);
    unsigned int centralOffset;
    if (!fp) return 0;

    put32(local, 0x04034b50U);
    put16(local + 4, 20U);
    put16(local + 8, 0U);
    put32(local + 18, payloadSize);
    put32(local + 22, payloadSize);
    put16(local + 26, nameLen);
    if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
        fwrite(name, 1U, nameLen, fp) != nameLen ||
        fwrite(payload, 1U, payloadSize, fp) != payloadSize) {
        fclose(fp);
        return 0;
    }

    centralOffset = (unsigned int)ftell(fp);
    put32(central, 0x02014b50U);
    put16(central + 4, 20U);
    put16(central + 6, 20U);
    put16(central + 10, 0U);
    put32(central + 20, payloadSize);
    put32(central + 24, payloadSize);
    put16(central + 28, nameLen);
    if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
        fwrite(name, 1U, nameLen, fp) != nameLen) {
        fclose(fp);
        return 0;
    }

    put32(eocd, 0x06054b50U);
    put16(eocd + 8, 1U);
    put16(eocd + 10, 1U);
    put32(eocd + 12, (unsigned int)(sizeof(central) + nameLen));
    put32(eocd + 16, centralOffset);
    if (fwrite(eocd, 1U, sizeof(eocd), fp) != sizeof(eocd)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_stored_zip_duplicate_hash_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char* names[] = {"dm2/Z_DUPLICATE.BIN", "dm2/A_DUPLICATE.BIN"};
    FILE* fp = fopen(path, "wb");
    unsigned char local[30] = {0};
    unsigned char central[46] = {0};
    unsigned char eocd[22] = {0};
    unsigned int payloadSize = (unsigned int)(sizeof(payload) - 1U);
    unsigned int centralOffset;
    unsigned int centralSize = 0U;
    int i;
    if (!fp) return 0;

    for (i = 0; i < 2; ++i) {
        unsigned int nameLen = (unsigned int)strlen(names[i]);
        put32(local, 0x04034b50U);
        put16(local + 4, 20U);
        put16(local + 8, 0U);
        put32(local + 18, payloadSize);
        put32(local + 22, payloadSize);
        put16(local + 26, nameLen);
        if (fwrite(local, 1U, sizeof(local), fp) != sizeof(local) ||
            fwrite(names[i], 1U, nameLen, fp) != nameLen ||
            fwrite(payload, 1U, payloadSize, fp) != payloadSize) {
            fclose(fp);
            return 0;
        }
        memset(local, 0, sizeof(local));
    }

    centralOffset = (unsigned int)ftell(fp);
    for (i = 0; i < 2; ++i) {
        unsigned int nameLen = (unsigned int)strlen(names[i]);
        memset(central, 0, sizeof(central));
        put32(central, 0x02014b50U);
        put16(central + 4, 20U);
        put16(central + 6, 20U);
        put16(central + 10, 0U);
        put32(central + 20, payloadSize);
        put32(central + 24, payloadSize);
        put16(central + 28, nameLen);
        if (fwrite(central, 1U, sizeof(central), fp) != sizeof(central) ||
            fwrite(names[i], 1U, nameLen, fp) != nameLen) {
            fclose(fp);
            return 0;
        }
        centralSize += (unsigned int)(sizeof(central) + nameLen);
    }

    put32(eocd, 0x06054b50U);
    put16(eocd + 8, 2U);
    put16(eocd + 10, 2U);
    put32(eocd + 12, centralSize);
    put32(eocd + 16, centralOffset);
    if (fwrite(eocd, 1U, sizeof(eocd), fp) != sizeof(eocd)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void write_tar_octal(unsigned char* field, size_t fieldSize, unsigned int value) {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%0*o", (int)fieldSize - 1, value);
    memcpy(field, tmp, fieldSize - 1U);
    field[fieldSize - 1U] = '\0';
}

static int build_tar_fixture(unsigned char* out, size_t outSize, size_t* outUsed) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char name[] = "dm1/weird-name.payload";
    unsigned char* hdr;
    unsigned int checksum = 0U;
    size_t payloadSize = sizeof(payload) - 1U;
    size_t total = 512U + ((payloadSize + 511U) / 512U) * 512U + 1024U;
    size_t i;
    if (!out || !outUsed || outSize < total) return 0;
    memset(out, 0, total);
    hdr = out;
    memcpy(hdr, name, sizeof(name) - 1U);
    write_tar_octal(hdr + 100, 8U, 0644U);
    write_tar_octal(hdr + 108, 8U, 0U);
    write_tar_octal(hdr + 116, 8U, 0U);
    write_tar_octal(hdr + 124, 12U, (unsigned int)payloadSize);
    write_tar_octal(hdr + 136, 12U, 0U);
    memset(hdr + 148, ' ', 8U);
    hdr[156] = '0';
    memcpy(hdr + 257, "ustar", 5U);
    memcpy(hdr + 263, "00", 2U);
    for (i = 0U; i < 512U; ++i) checksum += hdr[i];
    write_tar_octal(hdr + 148, 8U, checksum);
    memcpy(out + 512U, payload, payloadSize);
    *outUsed = total;
    return 1;
}

static int write_tar_fixture(const char* path) {
    unsigned char tarBytes[4096];
    size_t tarSize = 0U;
    FILE* fp;
    if (!build_tar_fixture(tarBytes, sizeof(tarBytes), &tarSize)) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(tarBytes, 1U, tarSize, fp) != tarSize) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_lha_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    static const char name[] = "dm1/GRAPHICS.DAT";
    FILE* fp = fopen(path, "wb");
    unsigned char header[256] = {0};
    unsigned int payloadSize = (unsigned int)(sizeof(payload) - 1U);
    unsigned int nameLen = (unsigned int)(sizeof(name) - 1U);
    unsigned int headerSize = 22U + nameLen + 2U;
    unsigned int checksum = 0U;
    unsigned int i;
    if (!fp || headerSize > 255U) {
        if (fp) fclose(fp);
        return 0;
    }
    header[0] = (unsigned char)headerSize;
    memcpy(header + 2, "-lh0-", 5U);
    put32(header + 7, payloadSize);
    put32(header + 11, payloadSize);
    header[19] = 0x20U;
    header[20] = 0U;
    header[21] = (unsigned char)nameLen;
    memcpy(header + 22, name, nameLen);
    for (i = 2U; i <= headerSize; ++i) {
        checksum += header[i];
    }
    header[1] = (unsigned char)(checksum & 0xffU);
    if (fwrite(header, 1U, 1U + headerSize, fp) != 1U + headerSize ||
        fwrite(payload, 1U, payloadSize, fp) != payloadSize ||
        fputc(0, fp) == EOF) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

#ifdef FIRESTAFF_HAS_ZLIB
static unsigned int fixture_crc32(const unsigned char* data, size_t size) {
    unsigned int crc = 0xffffffffU;
    size_t i;
    for (i = 0U; i < size; ++i) {
        int bit;
        crc ^= (unsigned int)data[i];
        for (bit = 0; bit < 8; ++bit) {
            crc = (crc & 1U) ? ((crc >> 1U) ^ 0xedb88320U) : (crc >> 1U);
        }
    }
    return crc ^ 0xffffffffU;
}

static int write_gzip_payload(const char* path,
                              const unsigned char* payload,
                              size_t payloadSize) {
    static const unsigned char gzipHeader[10] = {
        0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff
    };
    FILE* fp;
    size_t offset = 0U;
    unsigned int crc;
    unsigned char trailer[8];
    if (!path || !payload || payloadSize > 0xffffffffU) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(gzipHeader, 1U, sizeof(gzipHeader), fp) != sizeof(gzipHeader)) {
        fclose(fp);
        return 0;
    }
    while (offset < payloadSize || (payloadSize == 0U && offset == 0U)) {
        size_t chunk = payloadSize - offset;
        unsigned int len;
        unsigned char hdr[5];
        int finalBlock;
        if (chunk > 65535U) chunk = 65535U;
        finalBlock = (offset + chunk >= payloadSize);
        len = (unsigned int)chunk;
        hdr[0] = finalBlock ? 0x01U : 0x00U;
        hdr[1] = (unsigned char)(len & 0xffU);
        hdr[2] = (unsigned char)((len >> 8U) & 0xffU);
        hdr[3] = (unsigned char)((~len) & 0xffU);
        hdr[4] = (unsigned char)(((~len) >> 8U) & 0xffU);
        if (fwrite(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr) ||
            (chunk > 0U && fwrite(payload + offset, 1U, chunk, fp) != chunk)) {
            fclose(fp);
            return 0;
        }
        offset += chunk;
        if (payloadSize == 0U) break;
    }
    crc = fixture_crc32(payload, payloadSize);
    put32(trailer, crc);
    put32(trailer + 4, (unsigned int)payloadSize);
    if (fwrite(trailer, 1U, sizeof(trailer), fp) != sizeof(trailer)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_tgz_fixture(const char* path) {
    unsigned char tarBytes[4096];
    size_t tarSize = 0U;
    if (!build_tar_fixture(tarBytes, sizeof(tarBytes), &tarSize)) return 0;
    return write_gzip_payload(path, tarBytes, tarSize);
}
#endif

static int write_iso_record(unsigned char* dir, int offset, unsigned int lba,
                            unsigned int size, int isDir,
                            const unsigned char* name, int nameLen) {
    int recLen = 33 + nameLen + ((nameLen & 1) ? 1 : 0);
    if (offset + recLen > 2048) return 0;
    dir[offset] = (unsigned char)recLen;
    put32(dir + offset + 2, lba);
    put32(dir + offset + 6, lba);
    put32(dir + offset + 10, size);
    put32(dir + offset + 14, size);
    dir[offset + 25] = isDir ? 0x02 : 0x00;
    dir[offset + 28] = 1;
    dir[offset + 32] = (unsigned char)nameLen;
    memcpy(dir + offset + 33, name, (size_t)nameLen);
    return recLen;
}

static int write_iso_fixture_payload(const char* path,
                                     const char* payload,
                                     size_t payloadSize);

static int write_iso_fixture(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    return write_iso_fixture_payload(path, payload, sizeof(payload) - 1U);
}

static int write_iso_fixture_payload(const char* path,
                                     const char* payload,
                                     size_t payloadSize) {
    static const unsigned char dot = 0;
    static const unsigned char dotdot = 1;
    static const unsigned char fileName[] = "DUNGEON.DAT;1";
    FILE* fp = fopen(path, "wb");
    unsigned char zero[2048] = {0};
    unsigned char pvd[2048] = {0};
    unsigned char dir[2048] = {0};
    unsigned char fileSector[2048] = {0};
    int offset = 0;
    int recLen;
    if (!fp) return 0;
    for (int i = 0; i < 16; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    pvd[0] = 1;
    memcpy(pvd + 1, "CD001", 5);
    pvd[6] = 1;
    (void)write_iso_record(pvd, 156, 20U, 2048U, 1, &dot, 1);
    if (fwrite(pvd, 1U, sizeof(pvd), fp) != sizeof(pvd)) {
        fclose(fp);
        return 0;
    }
    for (int i = 17; i < 20; ++i) {
        if (fwrite(zero, 1U, sizeof(zero), fp) != sizeof(zero)) {
            fclose(fp);
            return 0;
        }
    }
    recLen = write_iso_record(dir, offset, 20U, 2048U, 1, &dot, 1);
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    recLen = write_iso_record(dir, offset, 20U, 2048U, 1, &dotdot, 1);
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    offset += recLen;
    if (payloadSize > sizeof(fileSector)) {
        fclose(fp);
        return 0;
    }
    recLen = write_iso_record(dir, offset, 21U, (unsigned int)payloadSize,
                              0, fileName, (int)(sizeof(fileName) - 1U));
    if (!recLen) {
        fclose(fp);
        return 0;
    }
    if (fwrite(dir, 1U, sizeof(dir), fp) != sizeof(dir)) {
        fclose(fp);
        return 0;
    }
    memcpy(fileSector, payload, payloadSize);
    if (fwrite(fileSector, 1U, sizeof(fileSector), fp) != sizeof(fileSector)) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static int write_cue_fixture(const char* path,
                             const char* firstPayload,
                             const char* secondPayload) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fprintf(fp,
                "FILE \"%s\" BINARY\n"
                "  TRACK 01 AUDIO\n"
                "    INDEX 01 00:00:00\n"
                "FILE \"%s\" BINARY\n"
                "  Track 02 Mode1/2048\n"
                "    INDEX 01 00:00:00\n",
                firstPayload, secondPayload) < 0) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
}

static void cleanup_fixture(void) {
    remove("asset_find_by_hash_test_tmp/nested/renamed.asset");
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive.zip");
    remove("asset_find_by_hash_test_tmp/archive.apk");
    remove("asset_find_by_hash_test_tmp/archive_as_bin.bin");
    remove("asset_find_by_hash_test_tmp/renamed_zip.payload");
    remove("asset_find_by_hash_test_tmp/archive.tar");
    remove("asset_find_by_hash_test_tmp/archive.tbz2");
    remove("asset_find_by_hash_test_tmp/archive_tar_source.tar");
    remove("asset_find_by_hash_test_tmp/archive.lzh");
    remove("asset_find_by_hash_test_tmp/archive.tgz");
    remove("asset_find_by_hash_test_tmp/archive.7z");
    remove("asset_find_by_hash_test_tmp/kryoflux_tracks.7z");
    remove("asset_find_by_hash_test_tmp/ordinary_raw_member.7z");
    remove("asset_find_by_hash_test_tmp/nested_atari.st.7z");
    remove("asset_find_by_hash_test_tmp/nested_atari.msa.7z");
    remove("asset_find_by_hash_test_tmp/nested_amiga.adf.7z");
    remove("asset_find_by_hash_test_tmp/archive.dmg");
    remove("asset_find_by_hash_test_tmp/renamed_tar.payload");
    remove("asset_find_by_hash_test_tmp/packed_payload.bin");
    remove("asset_find_by_hash_test_tmp/79.1.raw");
    remove("asset_find_by_hash_test_tmp/capture.raw");
    remove("asset_find_by_hash_test_tmp/GRAPHICS.DAT.gz");
    remove("asset_find_by_hash_test_tmp/disc.iso");
    remove("asset_find_by_hash_test_tmp/disc.img");
    remove("asset_find_by_hash_test_tmp/disc.raw");
    remove("asset_find_by_hash_test_tmp/disc_as_zip.zip");
    remove("asset_find_by_hash_test_tmp/renamed_iso.payload");
    remove("asset_find_by_hash_test_tmp/cue_a.payload");
    remove("asset_find_by_hash_test_tmp/cue_b.payload");
    remove("asset_find_by_hash_test_tmp/split.cue");
    remove("asset_find_by_hash_test_tmp/chaos.adf");
    remove("asset_find_by_hash_test_tmp/chaos.st");
    remove("asset_find_by_hash_test_tmp/chaos.msa");
    remove("asset_find_by_hash_test_tmp/nested_amiga.adf");
    RMDIR("asset_find_by_hash_test_tmp/nested");
    RMDIR("asset_find_by_hash_test_tmp");
}

static int path_has_fixture_name(const char* path) {
    return path && strstr(path, "renamed.asset") != NULL;
}

static int path_has_virtual_name(const char* path, const char* container, const char* entry) {
    return path && strstr(path, container) != NULL && strstr(path, "::") != NULL &&
           strstr(path, entry) != NULL;
}

static int path_has_virtual_entry(const char* path, const char* container, const char* entry) {
    char expected[ASSET_PATH_MAX];
    size_t pathLen;
    size_t expectedLen;
    const char* suffix;
    if (!path || !container || !entry) return 0;
    if (snprintf(expected, sizeof(expected), "%s::%s", container, entry) >= (int)sizeof(expected)) {
        return 0;
    }
    pathLen = strlen(path);
    expectedLen = strlen(expected);
    if (pathLen < expectedLen) return 0;
    suffix = path + (pathLen - expectedLen);
    return strcmp(suffix, expected) == 0;
}

static int file_matches_fixture_payload(const char* path) {
    static const char payload[] = "Firestaff hash identity fixture v1\n";
    char buf[128];
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp) return 0;
    n = fread(buf, 1U, sizeof(buf), fp);
    fclose(fp);
    return n == sizeof(payload) - 1U && memcmp(buf, payload, sizeof(payload) - 1U) == 0;
}

static int scan_cache_has_entry(const char* needle) {
    const char* home = getenv("HOME");
    char path[ASSET_PATH_MAX];
    char line[ASSET_PATH_MAX + 64];
    FILE* fp;
    if (!home || !needle ||
        snprintf(path, sizeof(path), "%s/.firestaff/cache/asset_scan_cache.dat", home) >=
            (int)sizeof(path)) {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, needle) != NULL) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int main(void) {
    char outPath[ASSET_PATH_MAX];
    char outPaths[2][ASSET_PATH_MAX];
    int matched[2];
    const char* md5Upper = "08C53652F85ABFE8A075D5DE4D3C8287";
    const char* md5List[] = {
        "00000000000000000000000000000000",
        "08C53652F85ABFE8A075D5DE4D3C8287",
        NULL
    };
    int matchIndex = -1;

    cleanup_fixture();
    if (MKDIR("asset_find_by_hash_test_tmp") != 0 ||
        MKDIR("asset_find_by_hash_test_tmp/nested") != 0 ||
        !write_fixture("asset_find_by_hash_test_tmp/nested/renamed.asset")) {
        cleanup_fixture();
        fprintf(stderr, "fixture setup failed\n");
        return 1;
    }

    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_fixture_name(outPath)) {
        cleanup_fixture();
        fprintf(stderr, "uppercase MD5 recursive lookup failed: %s\n", outPath);
        return 1;
    }

    memset(outPath, 0, sizeof(outPath));
    if (asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                          outPath, 8, 2)) {
        cleanup_fixture();
        fprintf(stderr, "truncated output path should not be reported as a match\n");
        return 1;
    }

    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                outPath, (int)sizeof(outPath),
                                &matchIndex, 2) ||
        matchIndex != 1 ||
        !path_has_fixture_name(outPath)) {
        cleanup_fixture();
        fprintf(stderr, "MD5 list lookup failed: index=%d path=%s\n", matchIndex, outPath);
        return 1;
    }

    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_fixture_name(outPaths[1])) {
        cleanup_fixture();
        fprintf(stderr, "MD5 all-list file lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }

    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_files_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                         outPaths, matched, 2, 2) != 1 ||
        matched[0] || !matched[1] || !path_has_fixture_name(outPaths[1])) {
        cleanup_fixture();
        fprintf(stderr, "ordinary-file all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }

    remove("asset_find_by_hash_test_tmp/nested/renamed.asset");
    if (!write_adf_fixture("asset_find_by_hash_test_tmp/chaos.adf")) {
        cleanup_fixture();
        fprintf(stderr, "ADF fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "chaos.adf", "GRAPHICS.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "ADF filesystem entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "ADF virtual extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] || !matched[1] ||
        !path_has_virtual_entry(outPaths[1], "chaos.adf", "GRAPHICS.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "ADF MD5-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/chaos.adf");
    if (!write_atari_st_fixture("asset_find_by_hash_test_tmp/chaos.st")) {
        cleanup_fixture();
        fprintf(stderr, "Atari ST fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "chaos.st", "GRAPHICS.DAT") ||
        !asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "Atari ST filesystem lookup/extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    if (system("command -v 7zz >/dev/null 2>&1 && "
               "cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z nested_atari.st.7z chaos.st >/dev/null 2>&1") == 0) {
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "nested_atari.st.7z",
                                    "chaos.st::GRAPHICS.DAT") ||
            !asset_extract_virtual_path(outPath,
                                        "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "nested Atari ST filesystem lookup/extraction failed: %s\n", outPath);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
    }
    remove("asset_find_by_hash_test_tmp/nested_atari.st.7z");
    remove("asset_find_by_hash_test_tmp/chaos.st");
    if (!write_atari_msa_fixture("asset_find_by_hash_test_tmp/chaos.msa")) {
        cleanup_fixture();
        fprintf(stderr, "Atari MSA fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "chaos.msa", "GRAPHICS.DAT") ||
        !asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "Atari MSA filesystem lookup/extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    if (system("command -v 7zz >/dev/null 2>&1 && "
               "cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z nested_atari.msa.7z chaos.msa >/dev/null 2>&1") == 0) {
        remove("asset_find_by_hash_test_tmp/chaos.msa");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "nested_atari.msa.7z",
                                    "chaos.msa::GRAPHICS.DAT") ||
            !asset_extract_virtual_path(outPath,
                                        "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "nested Atari MSA lookup/extraction failed: %s\n", outPath);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
        memset(outPaths, 0, sizeof(outPaths));
        memset(matched, 0, sizeof(matched));
        if (asset_find_all_by_md5_list(
                "asset_find_by_hash_test_tmp/nested_atari.msa.7z", md5List,
                outPaths, matched, 2, 2) != 1 ||
            matched[0] || !matched[1] ||
            !path_has_virtual_entry(outPaths[1], "nested_atari.msa.7z",
                                    "chaos.msa::GRAPHICS.DAT")) {
            cleanup_fixture();
            fprintf(stderr, "nested Atari MSA list lookup failed: matched=%d,%d path=%s\n",
                    matched[0], matched[1], outPaths[1]);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/nested_atari.msa.7z");
    }
    remove("asset_find_by_hash_test_tmp/chaos.msa");
    if (!write_stored_zip_fixture("asset_find_by_hash_test_tmp/archive.zip")) {
        cleanup_fixture();
        fprintf(stderr, "ZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "archive.zip", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "stored ZIP entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_name(outPaths[1], "archive.zip", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "MD5 all-list ZIP lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_files_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                         outPaths, matched, 2, 2) != 0 ||
        matched[0] || matched[1]) {
        cleanup_fixture();
        fprintf(stderr, "ordinary-file scan must not open archive containers\n");
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual ZIP extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");

    /* Launcher file pickers may select an archive itself, not its directory. */
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp/archive.zip", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "archive.zip", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "direct ZIP entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    matchIndex = -1;
    if (!asset_find_by_md5_list("asset_find_by_hash_test_tmp/archive.zip", md5List,
                                outPath, (int)sizeof(outPath),
                                &matchIndex, 2) ||
        matchIndex != 1 ||
        !path_has_virtual_name(outPath, "archive.zip", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "direct ZIP MD5-list lookup failed: index=%d path=%s\n",
                matchIndex, outPath);
        return 1;
    }

    remove("asset_find_by_hash_test_tmp/archive.zip");
    if (!write_stored_zip_fixture("asset_find_by_hash_test_tmp/archive.apk")) {
        cleanup_fixture();
        fprintf(stderr, "APK/ZIP-compatible fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "archive.apk", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "APK/ZIP-compatible entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual APK/ZIP-compatible extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive.apk");

    if (!write_stored_zip_fixture("asset_find_by_hash_test_tmp/archive_as_bin.bin")) {
        cleanup_fixture();
        fprintf(stderr, "ZIP-as-BIN fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "archive_as_bin.bin", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "ZIP magic should override BIN/ISO suffix: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual ZIP-as-BIN extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive_as_bin.bin");

    if (!write_stored_zip_fixture("asset_find_by_hash_test_tmp/renamed_zip.payload")) {
        cleanup_fixture();
        fprintf(stderr, "renamed ZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "renamed_zip.payload", "dm2/RENAMED.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "renamed ZIP entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "renamed ZIP extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/renamed_zip.payload");

    if (!write_stored_zip_duplicate_hash_fixture("asset_find_by_hash_test_tmp/archive.zip")) {
        cleanup_fixture();
        fprintf(stderr, "duplicate ZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "archive.zip", "dm2/A_DUPLICATE.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "duplicate ZIP deterministic lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_entry(outPaths[1], "archive.zip", "dm2/A_DUPLICATE.BIN")) {
        cleanup_fixture();
        fprintf(stderr, "duplicate ZIP all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }

    remove("asset_find_by_hash_test_tmp/archive.zip");
    if (!write_tar_fixture("asset_find_by_hash_test_tmp/archive.tar")) {
        cleanup_fixture();
        fprintf(stderr, "TAR fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "archive.tar", "dm1/weird-name.payload")) {
        cleanup_fixture();
        fprintf(stderr, "TAR entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual TAR extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive.tar");

    if (system("command -v bsdtar >/dev/null 2>&1 && "
               "command -v bzip2 >/dev/null 2>&1") == 0) {
        if (!write_tar_fixture("asset_find_by_hash_test_tmp/archive_tar_source.tar") ||
            system("bzip2 -c asset_find_by_hash_test_tmp/archive_tar_source.tar "
                   "> asset_find_by_hash_test_tmp/archive.tbz2") != 0) {
            cleanup_fixture();
            fprintf(stderr, "TBZ2 fixture setup failed\n");
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/archive_tar_source.tar");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "archive.tbz2", "dm1/weird-name.payload")) {
            cleanup_fixture();
            fprintf(stderr, "TBZ2 external archive lookup failed: %s\n", outPath);
            return 1;
        }
        memset(outPaths, 0, sizeof(outPaths));
        memset(matched, 0, sizeof(matched));
        if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                       outPaths, matched, 2, 2) != 1 ||
            matched[0] ||
            !matched[1] ||
            !path_has_virtual_entry(outPaths[1], "archive.tbz2", "dm1/weird-name.payload")) {
            cleanup_fixture();
            fprintf(stderr, "TBZ2 all-list lookup failed: matched=%d,%d path=%s\n",
                    matched[0], matched[1], outPaths[1]);
            return 1;
        }
        if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "virtual TBZ2 extraction failed: %s\n", outPath);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
        remove("asset_find_by_hash_test_tmp/archive.tbz2");
    }

    if (!write_lha_fixture("asset_find_by_hash_test_tmp/archive.lzh")) {
        cleanup_fixture();
        fprintf(stderr, "LZH fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "archive.lzh", "dm1/GRAPHICS.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "LZH entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_entry(outPaths[1], "archive.lzh", "dm1/GRAPHICS.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "LZH all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual LZH extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive.lzh");

#ifdef FIRESTAFF_HAS_ZLIB
    if (!write_tgz_fixture("asset_find_by_hash_test_tmp/archive.tgz")) {
        cleanup_fixture();
        fprintf(stderr, "TGZ fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "archive.tgz", "dm1/weird-name.payload")) {
        cleanup_fixture();
        fprintf(stderr, "TGZ entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_entry(outPaths[1], "archive.tgz", "dm1/weird-name.payload")) {
        cleanup_fixture();
        fprintf(stderr, "TGZ all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual TGZ extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/archive.tgz");

    if (!write_tgz_fixture("asset_find_by_hash_test_tmp/GRAPHICS.DAT.gz")) {
        cleanup_fixture();
        fprintf(stderr, "GZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                          outPath, (int)sizeof(outPath), 2)) {
        cleanup_fixture();
        fprintf(stderr, "GZIP should not treat tar.gz bytes as a single payload: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/GRAPHICS.DAT.gz");

    if (!write_fixture("asset_find_by_hash_test_tmp/nested/renamed.asset")) {
        cleanup_fixture();
        fprintf(stderr, "GZIP source fixture setup failed\n");
        return 1;
    }
    {
        unsigned char inBytes[128];
        FILE* in = fopen("asset_find_by_hash_test_tmp/nested/renamed.asset", "rb");
        size_t inSize;
        if (!in) {
            cleanup_fixture();
            fprintf(stderr, "GZIP source open failed\n");
            return 1;
        }
        inSize = fread(inBytes, 1U, sizeof(inBytes), in);
        fclose(in);
        if (!write_gzip_payload("asset_find_by_hash_test_tmp/GRAPHICS.DAT.gz",
                                inBytes, inSize)) {
            cleanup_fixture();
            fprintf(stderr, "GZIP write failed\n");
            return 1;
        }
    }
    remove("asset_find_by_hash_test_tmp/nested/renamed.asset");
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "GRAPHICS.DAT.gz", "GRAPHICS.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "GZIP single-file lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_entry(outPaths[1], "GRAPHICS.DAT.gz", "GRAPHICS.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "GZIP all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual GZIP extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/GRAPHICS.DAT.gz");
#endif

    if (write_fixture("asset_find_by_hash_test_tmp/packed_payload.bin") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z archive.7z packed_payload.bin >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/packed_payload.bin");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "archive.7z", "packed_payload.bin")) {
            cleanup_fixture();
            fprintf(stderr, "7z external archive lookup failed: %s\n", outPath);
            return 1;
        }
        memset(outPaths, 0, sizeof(outPaths));
        memset(matched, 0, sizeof(matched));
        if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                       outPaths, matched, 2, 2) != 1 ||
            matched[0] ||
            !matched[1] ||
            !path_has_virtual_entry(outPaths[1], "archive.7z", "packed_payload.bin")) {
            cleanup_fixture();
            fprintf(stderr, "7z all-list lookup failed: matched=%d,%d path=%s\n",
                    matched[0], matched[1], outPaths[1]);
            return 1;
        }
        memset(outPaths, 0, sizeof(outPaths));
        memset(matched, 0, sizeof(matched));
        if (asset_find_all_by_md5_list(
                "asset_find_by_hash_test_tmp/archive.7z", md5List,
                outPaths, matched, 2, 2) != 1 ||
            matched[0] || !matched[1] ||
            !path_has_virtual_entry(outPaths[1], "archive.7z",
                                    "packed_payload.bin")) {
            cleanup_fixture();
            fprintf(stderr,
                    "7z direct-root all-list lookup failed: matched=%d,%d path=%s\n",
                    matched[0], matched[1], outPaths[1]);
            return 1;
        }
        if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "virtual 7z extraction failed: %s\n", outPath);
            return 1;
        }
        if (!scan_cache_has_entry("archive.7z::packed_payload.bin")) {
            cleanup_fixture();
            fprintf(stderr, "external archive member MD5 was not cached\n");
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
    }
    remove("asset_find_by_hash_test_tmp/archive.7z");
    remove("asset_find_by_hash_test_tmp/packed_payload.bin");

    /* KryoFlux floppy streams use <track>.<side>.raw members. They are raw
     * flux tracks, not ISO images or loose game files, so an outer archive
     * scanner must not extract/hash each one. Keep ordinary .raw member
     * hashing intact for archive formats that genuinely store a raw payload.
     */
    if (write_fixture("asset_find_by_hash_test_tmp/79.1.raw") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z kryoflux_tracks.7z 79.1.raw >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/79.1.raw");
        memset(outPath, 0, sizeof(outPath));
        if (asset_find_by_md5("asset_find_by_hash_test_tmp/kryoflux_tracks.7z",
                              md5Upper, outPath, (int)sizeof(outPath), 2)) {
            cleanup_fixture();
            fprintf(stderr, "KryoFlux raw track was treated as a loose payload: %s\n",
                    outPath);
            return 1;
        }
    }
    remove("asset_find_by_hash_test_tmp/kryoflux_tracks.7z");
    remove("asset_find_by_hash_test_tmp/79.1.raw");

    /* A numbered raw CD track is also transport, rather than a generic
     * archive member.  In particular, a CSB CD dump can contain dozens of
     * Red Book tracks below the member-size cap; they must not each be
     * extracted for ordinary GRAPHICS/DUNGEON profile hashes.  Whole Track 02
     * searches use the separately registered Theron identities. */
    if (write_fixture("asset_find_by_hash_test_tmp/Chaos (Track 03).bin") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z cd_track_payloads.7z 'Chaos (Track 03).bin' >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/Chaos (Track 03).bin");
        memset(outPath, 0, sizeof(outPath));
        if (asset_find_by_md5("asset_find_by_hash_test_tmp/cd_track_payloads.7z",
                              md5Upper, outPath, (int)sizeof(outPath), 2)) {
            cleanup_fixture();
            fprintf(stderr, "numbered CD track was treated as a generic archive payload: %s\n",
                    outPath);
            return 1;
        }
    }
    remove("asset_find_by_hash_test_tmp/cd_track_payloads.7z");
    remove("asset_find_by_hash_test_tmp/Chaos (Track 03).bin");

    if (write_fixture("asset_find_by_hash_test_tmp/capture.raw") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z ordinary_raw_member.7z capture.raw >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/capture.raw");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp/ordinary_raw_member.7z",
                               md5Upper, outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "ordinary_raw_member.7z",
                                    "capture.raw")) {
            cleanup_fixture();
            fprintf(stderr, "ordinary raw archive member lookup failed: %s\n", outPath);
            return 1;
        }
    }
    remove("asset_find_by_hash_test_tmp/ordinary_raw_member.7z");
    remove("asset_find_by_hash_test_tmp/capture.raw");

    if (write_fixture("asset_find_by_hash_test_tmp/packed_payload.bin") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -tzip archive.dmg packed_payload.bin >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/packed_payload.bin");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "archive.dmg", "packed_payload.bin")) {
            cleanup_fixture();
            fprintf(stderr, "DMG/external archive suffix lookup failed: %s\n", outPath);
            return 1;
        }
        if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "virtual DMG/external archive extraction failed: %s\n", outPath);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
    }
    remove("asset_find_by_hash_test_tmp/archive.dmg");
    remove("asset_find_by_hash_test_tmp/packed_payload.bin");

    if (!write_tar_fixture("asset_find_by_hash_test_tmp/renamed_tar.payload")) {
        cleanup_fixture();
        fprintf(stderr, "renamed TAR fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_entry(outPath, "renamed_tar.payload", "dm1/weird-name.payload")) {
        cleanup_fixture();
        fprintf(stderr, "renamed TAR magic lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "renamed TAR extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/renamed_tar.payload");

    if (write_fixture("asset_find_by_hash_test_tmp/GRAPHICS.DAT") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -tzip amiga_disk.adf GRAPHICS.DAT >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/GRAPHICS.DAT");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "amiga_disk.adf", "GRAPHICS.DAT")) {
            cleanup_fixture();
            fprintf(stderr, "ADF external-container lookup failed: %s\n", outPath);
            return 1;
        }
        if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "virtual ADF external-container extraction failed: %s\n", outPath);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
    }
    remove("asset_find_by_hash_test_tmp/amiga_disk.adf");
    remove("asset_find_by_hash_test_tmp/GRAPHICS.DAT");

    /* Real Amiga releases are commonly distributed as a 7z containing an
     * OFS ADF. The scanner must hash the filesystem member, then materialize
     * that same member rather than treating the disk image as an opaque file. */
    if (write_adf_fixture("asset_find_by_hash_test_tmp/nested_amiga.adf") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -t7z nested_amiga.adf.7z nested_amiga.adf >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/nested_amiga.adf");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "nested_amiga.adf.7z",
                                    "nested_amiga.adf::GRAPHICS.DAT")) {
            cleanup_fixture();
            fprintf(stderr, "nested 7z/ADF filesystem lookup failed: %s\n", outPath);
            return 1;
        }
        if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "nested 7z/ADF filesystem extraction failed: %s\n", outPath);
            return 1;
        }
        remove("asset_find_by_hash_test_tmp/extracted.dat");
        /* The first hash walk has fully inspected the real ADF.  Later
         * independent launcher queries must reuse those verified inner-file
         * digests instead of decompressing this solid 7z again. */
        if (!scan_cache_has_entry("nested_amiga.adf.7z::nested_amiga.adf::GRAPHICS.DAT") ||
            !scan_cache_has_entry("nested_amiga.adf.7z::nested_amiga.adf::@firestaff-adf-complete-v1")) {
            cleanup_fixture();
            fprintf(stderr, "nested 7z/ADF inner-file cache was not recorded\n");
            return 1;
        }
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "nested_amiga.adf.7z",
                                    "nested_amiga.adf::GRAPHICS.DAT")) {
            cleanup_fixture();
            fprintf(stderr, "nested 7z/ADF cached filesystem lookup failed: %s\n",
                    outPath);
            return 1;
        }
        memset(outPaths, 0, sizeof(outPaths));
        memset(matched, 0, sizeof(matched));
        if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                       outPaths, matched, 2, 2) != 1 ||
            matched[0] || !matched[1] ||
            !path_has_virtual_entry(outPaths[1], "nested_amiga.adf.7z",
                                    "nested_amiga.adf::GRAPHICS.DAT")) {
            cleanup_fixture();
            fprintf(stderr, "nested 7z/ADF MD5-list lookup failed: matched=%d,%d path=%s\n",
                    matched[0], matched[1], outPaths[1]);
            return 1;
        }
    }
    remove("asset_find_by_hash_test_tmp/nested_amiga.adf.7z");
    remove("asset_find_by_hash_test_tmp/nested_amiga.adf");

    /* CSB Amiga media is also commonly supplied as ZIP → ADF.  ZIP has an
     * in-process reader for flat members, but the nested disk filesystem must
     * retain the full virtual receipt and materialize the same ADF member. */
    if (write_adf_fixture("asset_find_by_hash_test_tmp/nested_amiga.adf") &&
        system("command -v 7zz >/dev/null 2>&1 && "
               "(cd asset_find_by_hash_test_tmp && "
               "7zz a -tzip nested_amiga.adf.zip nested_amiga.adf >/dev/null 2>&1)") == 0) {
        remove("asset_find_by_hash_test_tmp/nested_amiga.adf");
        /* The archive is created by 7zz, but scan and materialization must
         * use Firestaff's ZIP+ADF readers rather than shelling back out. */
        test_setenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS", "1");
        memset(outPath, 0, sizeof(outPath));
        if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                               outPath, (int)sizeof(outPath), 2) ||
            !path_has_virtual_entry(outPath, "nested_amiga.adf.zip",
                                    "nested_amiga.adf::GRAPHICS.DAT")) {
            cleanup_fixture();
            fprintf(stderr, "nested ZIP/ADF filesystem lookup failed: %s\n", outPath);
            return 1;
        }
        if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
            !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
            cleanup_fixture();
            fprintf(stderr, "nested ZIP/ADF filesystem extraction failed: %s\n", outPath);
            return 1;
        }
        test_setenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS", NULL);
        remove("asset_find_by_hash_test_tmp/extracted.dat");
    }
    remove("asset_find_by_hash_test_tmp/nested_amiga.adf.zip");
    remove("asset_find_by_hash_test_tmp/nested_amiga.adf");

    if (!write_iso_fixture("asset_find_by_hash_test_tmp/disc.iso")) {
        cleanup_fixture();
        fprintf(stderr, "ISO fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "disc.iso", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "ISO entry lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_name(outPaths[1], "disc.iso", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "MD5 all-list ISO lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual ISO extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/disc.iso");

    if (!write_iso_fixture("asset_find_by_hash_test_tmp/disc.img")) {
        cleanup_fixture();
        fprintf(stderr, "IMG ISO fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "disc.img", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "IMG ISO entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual IMG ISO extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/disc.img");

    if (!write_iso_fixture("asset_find_by_hash_test_tmp/disc.raw")) {
        cleanup_fixture();
        fprintf(stderr, "RAW ISO fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "disc.raw", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "RAW ISO entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual RAW ISO extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/disc.raw");

    if (!write_iso_fixture("asset_find_by_hash_test_tmp/disc_as_zip.zip")) {
        cleanup_fixture();
        fprintf(stderr, "ISO-as-ZIP fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "disc_as_zip.zip", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "ISO magic should override ZIP suffix: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "virtual ISO-as-ZIP extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/disc_as_zip.zip");

    if (!write_iso_fixture("asset_find_by_hash_test_tmp/renamed_iso.payload")) {
        cleanup_fixture();
        fprintf(stderr, "renamed ISO fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "renamed_iso.payload", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "renamed ISO entry lookup failed: %s\n", outPath);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "renamed ISO extraction failed: %s\n", outPath);
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/extracted.dat");
    remove("asset_find_by_hash_test_tmp/renamed_iso.payload");

    if (!write_iso_fixture_payload("asset_find_by_hash_test_tmp/cue_a.payload",
                                   "Firestaff non-matching CUE payload v1\n",
                                   strlen("Firestaff non-matching CUE payload v1\n")) ||
        !write_iso_fixture("asset_find_by_hash_test_tmp/cue_b.payload") ||
        !write_cue_fixture("asset_find_by_hash_test_tmp/split.cue",
                           "cue_a.payload",
                           "cue_b.payload")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE fixture setup failed\n");
        return 1;
    }
    memset(outPath, 0, sizeof(outPath));
    if (!asset_find_by_md5("asset_find_by_hash_test_tmp", md5Upper,
                           outPath, (int)sizeof(outPath), 2) ||
        !path_has_virtual_name(outPath, "cue_b.payload", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE mixed-case data track lookup failed: %s\n", outPath);
        return 1;
    }
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    if (asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                   outPaths, matched, 2, 2) != 1 ||
        matched[0] ||
        !matched[1] ||
        !path_has_virtual_name(outPaths[1], "cue_b.payload", "DUNGEON.DAT")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE all-list lookup failed: matched=%d,%d path=%s\n",
                matched[0], matched[1], outPaths[1]);
        return 1;
    }
    if (!asset_extract_virtual_path(outPath, "asset_find_by_hash_test_tmp/extracted.dat") ||
        !file_matches_fixture_payload("asset_find_by_hash_test_tmp/extracted.dat")) {
        cleanup_fixture();
        fprintf(stderr, "split CUE virtual extraction failed: %s\n", outPath);
        return 1;
    }

    /* Missing-extractor diagnostic: an external archive that the scanner
     * cannot open because no supported extractor (7zz/7z/bsdtar) is
     * installed must be recorded so the launcher and --scan-data can name
     * the skipped archive and the tool that would unlock it. The env
     * override forces the "no extractor" branch on hosts that have one
     * installed; on Windows the external shell-out path never exists, so
     * the diagnostic fires regardless. */
    if (!write_fixture("asset_find_by_hash_test_tmp/packed.7z")) {
        cleanup_fixture();
        fprintf(stderr, "external archive fixture setup failed\n");
        return 1;
    }
    test_setenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS", "1");
    asset_scan_clear_missing_extractor_diagnostics();
    memset(outPaths, 0, sizeof(outPaths));
    memset(matched, 0, sizeof(matched));
    (void)asset_find_all_by_md5_list("asset_find_by_hash_test_tmp", md5List,
                                     outPaths, matched, 2, 2);
    if (asset_scan_missing_extractor_count() != 1 ||
        !asset_scan_missing_extractor_path(0) ||
        !strstr(asset_scan_missing_extractor_path(0), "packed.7z") ||
        !asset_scan_missing_extractor_tools(0) ||
        strcmp(asset_scan_missing_extractor_tools(0), "7zz/7z/bsdtar") != 0 ||
        asset_scan_missing_extractor_path(1) != NULL) {
        test_setenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS", NULL);
        cleanup_fixture();
        fprintf(stderr, "missing-extractor diagnostic not recorded: count=%d\n",
                asset_scan_missing_extractor_count());
        return 1;
    }
    test_setenv("FIRESTAFF_TEST_DISABLE_EXTERNAL_ARCHIVE_TOOLS", NULL);
    asset_scan_clear_missing_extractor_diagnostics();
    if (asset_scan_missing_extractor_count() != 0) {
        cleanup_fixture();
        fprintf(stderr, "missing-extractor diagnostic clear failed\n");
        return 1;
    }
    remove("asset_find_by_hash_test_tmp/packed.7z");

    cleanup_fixture();
    return 0;
}
