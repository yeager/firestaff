#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_header_pc34_compat.h"

static int write_u16(FILE* f, unsigned short value) {
    unsigned char bytes[2];
    bytes[0] = (unsigned char)(value & 0xFF);
    bytes[1] = (unsigned char)((value >> 8) & 0xFF);
    return fwrite(bytes, 1, 2, f) == 2;
}

static int write_u16_be(FILE* f, unsigned short value) {
    unsigned char bytes[2];
    bytes[0] = (unsigned char)((value >> 8) & 0xFF);
    bytes[1] = (unsigned char)(value & 0xFF);
    return fwrite(bytes, 1, 2, f) == 2;
}

static int write_format0_file(const char* path) {
    FILE* f = fopen(path, "wb");
    unsigned char payloadA[6] = {2, 0, 3, 0, 0xAA, 0xAB};
    unsigned char payloadB[8] = {4, 0, 5, 0, 0xBA, 0xBB, 0xBC, 0xBD};
    if (f == 0) {
        return 0;
    }
    if (!write_u16(f, 2)
     || !write_u16(f, 6)
     || !write_u16(f, 8)
     || !write_u16(f, 11)
     || !write_u16(f, 22)
     || fwrite(payloadA, 1, sizeof(payloadA), f) != sizeof(payloadA)
     || fwrite(payloadB, 1, sizeof(payloadB), f) != sizeof(payloadB)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int write_format1_file(const char* path) {
    FILE* f = fopen(path, "wb");
    unsigned char payloadA[3] = {0x10, 0x11, 0x12};
    unsigned char payloadB[5] = {0x20, 0x21, 0x22, 0x23, 0x24};
    if (f == 0) {
        return 0;
    }
    if (!write_u16(f, 0x8001)
     || !write_u16(f, 2)
     || !write_u16(f, 3)
     || !write_u16(f, 5)
     || !write_u16(f, 13)
     || !write_u16(f, 25)
     || !write_u16(f, 7)
     || !write_u16(f, 8)
     || !write_u16(f, 9)
     || !write_u16(f, 10)
     || fwrite(payloadA, 1, sizeof(payloadA), f) != sizeof(payloadA)
     || fwrite(payloadB, 1, sizeof(payloadB), f) != sizeof(payloadB)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int write_csb_legacy_header_fixture(const char* path) {
    enum { graphic_count = 0x0233, first_graphic_bytes = 239 };
    FILE* f = fopen(path, "wb");
    unsigned short i;
    unsigned char first_graphic[first_graphic_bytes];
    unsigned char graphic[4] = {0, 16, 0, 1};

    if (!f) return 0;
    memset(first_graphic, 0, sizeof(first_graphic));
    first_graphic[0] = 0;
    first_graphic[1] = 224;
    first_graphic[2] = 0;
    first_graphic[3] = 136;
    if (!write_u16_be(f, graphic_count)) goto fail;
    for (i = 0; i < graphic_count; ++i)
        if (!write_u16_be(f, i == 0 ? first_graphic_bytes : sizeof(graphic))) goto fail;
    for (i = 0; i < graphic_count; ++i)
        if (!write_u16_be(f, i == 0 ? first_graphic_bytes : sizeof(graphic))) goto fail;
    if (fwrite(first_graphic, 1, sizeof(first_graphic), f) != sizeof(first_graphic)) goto fail;
    for (i = 1; i < graphic_count; ++i)
        if (fwrite(graphic, 1, sizeof(graphic), f) != sizeof(graphic)) goto fail;
    fclose(f);
    return 1;
fail:
    fclose(f);
    return 0;
}

static int test_format0_header_parse(void) {
    const char* path = "./test_graphics_dat_format0.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    struct MemoryGraphicsDatHeader_Compat header;
    if (!write_format0_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(path, &state, &header)) {
        return 0;
    }
    if (header.format != 0 || header.graphicCount != 2) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.compressedByteCounts[0] != 6 || header.compressedByteCounts[1] != 8) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.decompressedByteCounts[0] != 11 || header.decompressedByteCounts[1] != 22) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.widthHeight[0].Width != 2 || header.widthHeight[0].Height != 3
     || header.widthHeight[1].Width != 4 || header.widthHeight[1].Height != 5) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.fileSize != 24 || state.referenceCount != 0 || state.file != 0) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 1;
}

static int test_format1_header_parse(void) {
    const char* path = "./test_graphics_dat_format1.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    struct MemoryGraphicsDatHeader_Compat header;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(path, &state, &header)) {
        return 0;
    }
    if (header.format != 1 || header.graphicCount != 2) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.compressedByteCounts[0] != 3 || header.compressedByteCounts[1] != 5) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.decompressedByteCounts[0] != 13 || header.decompressedByteCounts[1] != 25) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.widthHeight[0].Width != 7 || header.widthHeight[0].Height != 8
     || header.widthHeight[1].Width != 9 || header.widthHeight[1].Height != 10) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (header.fileSize != 28 || state.referenceCount != 0 || state.file != 0) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 1;
}

static int test_csb_legacy_big_endian_header_parse(void) {
    const char* path = "./test_graphics_dat_csb_legacy.bin";
    struct MemoryGraphicsDatState_Compat state = {0};
    struct MemoryGraphicsDatHeader_Compat header;

    if (!write_csb_legacy_header_fixture(path) ||
        !F0479_MEMORY_LoadGraphicsDatHeader_Compat(path, &state, &header)) return 0;
    if (header.format != 0 || header.graphicCount != 0x0233 ||
        header.compressedByteCounts[0] != 239 ||
        header.decompressedByteCounts[0] != 239 ||
        header.widthHeight[0].Width != 224 || header.widthHeight[0].Height != 136 ||
        header.fileSize != 4741 || state.referenceCount != 0 || state.file != 0) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 1;
}

int main(void) {
    if (!test_format0_header_parse()) {
        fprintf(stderr, "test_format0_header_parse failed\n");
        return 1;
    }
    if (!test_format1_header_parse()) {
        fprintf(stderr, "test_format1_header_parse failed\n");
        return 1;
    }
    if (!test_csb_legacy_big_endian_header_parse()) {
        fprintf(stderr, "test_csb_legacy_big_endian_header_parse failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
