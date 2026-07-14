#include "redmcsb_f0929_prim_ftl_load_pc34_compat.h"

#include "redmcsb_f0930_get_header_checksum.h"

#include <string.h>

enum {
    redmcsb_f0929_jump_type = 0x0010u,
    redmcsb_f0929_data_type = 0x0011u,
    redmcsb_f0929_code_type = 0x0012u
};

static uint16_t redmcsb_f0929_u16le(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static uint32_t redmcsb_f0929_u32le(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
           ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static uint16_t redmcsb_f0929_checksum_words(const uint8_t *bytes,
                                               uint32_t byte_count)
{
    uint16_t checksum = 0u;

    while (byte_count != 0u) {
        checksum = (uint16_t)(checksum + redmcsb_f0929_u16le(bytes));
        bytes += 2;
        byte_count -= 2u;
    }
    return checksum;
}

static uint16_t redmcsb_f0929_checksum_bytes(const uint8_t *bytes,
                                               uint32_t byte_count)
{
    uint16_t checksum = 0u;

    while (byte_count != 0u) {
        checksum = (uint16_t)(checksum + *bytes++);
        --byte_count;
    }
    return checksum;
}

static uint8_t *redmcsb_f0929_align_even(uint8_t *address)
{
    uintptr_t value = (uintptr_t)address;

    return (uint8_t *)((value + 1u) & ~(uintptr_t)1u);
}

static void redmcsb_f0929_put_u16le(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static void redmcsb_f0929_put_u32le(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    bytes[2] = (uint8_t)(value >> 16);
    bytes[3] = (uint8_t)(value >> 24);
}

static uint16_t redmcsb_f0929_find_segments(const uint8_t *headers,
                                              uint16_t count,
                                              int16_t *jump_index,
                                              int16_t *data_index,
                                              int16_t *code_index)
{
    uint16_t index;

    *jump_index = -1;
    *data_index = -1;
    *code_index = -1;
    for (index = 0u; index < count; ++index) {
        const uint8_t *header = headers + ((size_t)index * 12u);
        uint16_t type = redmcsb_f0929_u16le(header);
        int16_t *slot = NULL;
        uint16_t error = 0u;

        if (type == redmcsb_f0929_jump_type) {
            slot = jump_index;
            error = 20u;
        } else if (type == redmcsb_f0929_data_type) {
            slot = data_index;
            error = 21u;
        } else if (type == redmcsb_f0929_code_type) {
            slot = code_index;
            error = 22u;
        }
        if (slot != NULL) {
            if (redmcsb_f0929_u16le(header + 2u) != 1u) {
                return error;
            }
            if (*slot != -1) {
                return (uint16_t)(error + 3u);
            }
            *slot = (int16_t)index;
        }
    }
    return (*jump_index == -1 || *data_index == -1 || *code_index == -1)
               ? 2u
               : 0u;
}

uint16_t redmcsb_f0929_prim_ftl_load_pc34_compat(
    int32_t file_handle,
    redmcsb_f0929_ftl_executable_pc34_compat *executable,
    const redmcsb_f0929_callbacks_pc34_compat *callbacks)
{
    uint8_t header[20];
    uint8_t jump_header[40];
    uint8_t *segment_headers = NULL;
    uint8_t *temporary_allocation = NULL;
    uint8_t *memory = NULL;
    uint8_t *data_address;
    uint8_t *a5_address;
    uint8_t *code_address;
    uint8_t *temporary;
    uint16_t segment_count;
    int16_t jump_index;
    int16_t data_index;
    int16_t code_index;
    uint32_t temporary_size;
    uint16_t result;

    executable->memory_address = NULL;
    segment_headers = NULL;
    temporary_allocation = NULL;
    if (callbacks->read(callbacks->context, file_handle, header, 20u) != 0) {
        result = 512u;
        goto done;
    }
    if (redmcsb_f0929_u16le(header) != 0x6160u ||
        redmcsb_f0929_u16le(header + 4u) != 2u || header[6] != 1u ||
        header[7] != 0u) {
        result = 1u;
        goto done;
    }
    segment_count = redmcsb_f0929_u16le(header + 18u);
    if (segment_count > 100u) {
        result = 26u;
        goto done;
    }
    temporary_size = (uint32_t)segment_count * 12u;
    segment_headers = callbacks->allocate(callbacks->context, temporary_size);
    if (segment_headers == NULL) {
        result = 513u;
        goto done;
    }
    if (callbacks->read(callbacks->context, file_handle, segment_headers,
                        temporary_size) != 0) {
        result = 514u;
        goto done;
    }
    if (redmcsb_f0930_get_header_checksum(header, segment_count,
                                           segment_headers) !=
        redmcsb_f0929_u16le(header + 2u)) {
        result = 14u;
        goto done;
    }
    result = redmcsb_f0929_find_segments(segment_headers, segment_count,
                                         &jump_index, &data_index,
                                         &code_index);
    if (result != 0u) {
        goto done;
    }
    if (callbacks->seek(callbacks->context, file_handle,
                        redmcsb_f0929_u32le(segment_headers +
                            ((size_t)jump_index * 12u) + 4u)) != 0) {
        result = 515u;
        goto done;
    }
    if (callbacks->read(callbacks->context, file_handle, jump_header, 40u) !=
        0) {
        result = 516u;
        goto done;
    }
    memory = callbacks->allocate(
        callbacks->context, redmcsb_f0929_u32le(jump_header + 4u) +
                                redmcsb_f0929_u32le(jump_header) +
                                redmcsb_f0929_u32le(jump_header + 20u) + 12u);
    executable->memory_address = memory;
    if (memory == NULL) {
        result = 517u;
        goto done;
    }
    data_address = redmcsb_f0929_align_even(memory);
    a5_address = redmcsb_f0929_align_even(
        data_address + redmcsb_f0929_u32le(jump_header + 4u));
    code_address = redmcsb_f0929_align_even(
        a5_address + redmcsb_f0929_u32le(jump_header));
    temporary_size = redmcsb_f0929_u32le(
        segment_headers + ((size_t)jump_index * 12u) + 8u);
    if (redmcsb_f0929_u32le(segment_headers + ((size_t)data_index * 12u) +
                            8u) > temporary_size) {
        temporary_size = redmcsb_f0929_u32le(
            segment_headers + ((size_t)data_index * 12u) + 8u);
    }
    if (redmcsb_f0929_u32le(segment_headers + ((size_t)code_index * 12u) +
                            8u) > temporary_size) {
        temporary_size = redmcsb_f0929_u32le(
            segment_headers + ((size_t)code_index * 12u) + 8u);
    }
    if (redmcsb_f0929_u16le(jump_header + 30u) != 0u ||
        temporary_size > redmcsb_f0929_u32le(jump_header + 20u)) {
        temporary_allocation = callbacks->allocate(callbacks->context,
                                                   temporary_size + 1u);
        if (temporary_allocation == NULL) {
            result = 518u;
            goto done;
        }
        temporary = redmcsb_f0929_align_even(temporary_allocation);
    } else {
        temporary = code_address;
    }
    temporary_size = redmcsb_f0929_u32le(
        segment_headers + ((size_t)jump_index * 12u) + 8u) - 40u;
    if (callbacks->read(callbacks->context, file_handle, temporary,
                        temporary_size) != 0) {
        result = 519u;
        goto done;
    }
    if ((uint16_t)(redmcsb_f0929_checksum_words(temporary, temporary_size) +
                   redmcsb_f0929_checksum_words(jump_header, 40u) -
                   redmcsb_f0929_u16le(jump_header + 34u)) !=
        redmcsb_f0929_u16le(jump_header + 34u)) {
        result = 15u;
        goto done;
    }
    {
        uint8_t *source = temporary;
        uint8_t *destination =
            a5_address + redmcsb_f0929_u32le(jump_header + 12u);
        uint32_t remaining = temporary_size;

        while (remaining != 0u) {
            uint32_t target = redmcsb_f0929_u32le(source) +
                              (uint32_t)(uintptr_t)code_address;

            redmcsb_f0929_put_u16le(destination + 2u, 0x4ef9u);
            redmcsb_f0929_put_u32le(destination + 4u, target);
            source += 4;
            destination += 8;
            remaining -= 4u;
        }
    }
    if (callbacks->seek(callbacks->context, file_handle,
                        redmcsb_f0929_u32le(segment_headers +
                            ((size_t)data_index * 12u) + 4u)) != 0) {
        result = 520u;
        goto done;
    }
    temporary_size = redmcsb_f0929_u32le(
        segment_headers + ((size_t)data_index * 12u) + 8u);
    if (callbacks->read(callbacks->context, file_handle, temporary,
                        temporary_size) != 0) {
        result = 521u;
        goto done;
    }
    if (redmcsb_f0929_checksum_bytes(temporary, temporary_size) !=
        redmcsb_f0929_u16le(jump_header + 38u)) {
        result = 16u;
        goto done;
    }
    {
        uint8_t *source = temporary + 10u;
        uint8_t *destination = data_address;
        uint32_t compressed = redmcsb_f0929_u32le(temporary + 2u);
        uint32_t relocations = redmcsb_f0929_u32le(temporary + 6u);

        while (compressed != 0u) {
            uint16_t word = redmcsb_f0929_u16le(source);
            source += 2;
            compressed -= 2u;
            destination[0] = (uint8_t)(word & 0xffu);
            destination[1] = (uint8_t)(word >> 8);
            destination += 2;
            if (word == 0u) {
                uint16_t count = redmcsb_f0929_u16le(source);
                source += 2;
                compressed -= 2u;
                (void)memset(destination, 0, count);
                destination += count;
            }
        }
        while (relocations != 0u) {
            int16_t offset = (int16_t)redmcsb_f0929_u16le(source);
            uint8_t *relocated = a5_address + offset;
            uint32_t value = redmcsb_f0929_u32le(relocated);

            value += (uint32_t)(uintptr_t)a5_address;
            redmcsb_f0929_put_u32le(relocated, value);
            source += 2;
            relocations -= 2u;
        }
    }
    if (callbacks->seek(callbacks->context, file_handle,
                        redmcsb_f0929_u32le(segment_headers +
                            ((size_t)code_index * 12u) + 4u)) != 0) {
        result = 528u;
        goto done;
    }
    temporary_size = redmcsb_f0929_u32le(
        segment_headers + ((size_t)code_index * 12u) + 8u);
    if (redmcsb_f0929_u16le(jump_header + 30u) != 0u) {
        temporary = temporary_allocation == NULL ? code_address
                                                 : redmcsb_f0929_align_even(temporary_allocation);
    } else {
        temporary = code_address;
    }
    if (callbacks->read(callbacks->context, file_handle, temporary,
                        temporary_size) != 0) {
        result = 529u;
        goto done;
    }
    if (redmcsb_f0929_u16le(jump_header + 30u) != 0u &&
        (callbacks->decompress_code == NULL ||
         callbacks->decompress_code(callbacks->context, temporary,
                                    code_address) < 0)) {
        result = 27u;
        goto done;
    }
    if (redmcsb_f0929_checksum_bytes(code_address,
                                     redmcsb_f0929_u32le(jump_header + 20u)) !=
        redmcsb_f0929_u16le(jump_header + 36u)) {
        result = 17u;
        goto done;
    }
    executable->a5_world = a5_address;
    executable->jump_table_address =
        a5_address + redmcsb_f0929_u32le(jump_header + 12u) + 2u;
    executable->stack_size = redmcsb_f0929_u32le(jump_header + 16u);
    result = 0u;

done:
    if (segment_headers != NULL) {
        callbacks->free(callbacks->context, segment_headers);
    }
    if (temporary_allocation != NULL) {
        callbacks->free(callbacks->context, temporary_allocation);
    }
    if (result != 0u && executable->memory_address != NULL) {
        callbacks->free(callbacks->context, executable->memory_address);
        executable->memory_address = NULL;
    }
    return result;
}

const char *redmcsb_f0929_prim_ftl_load_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM2B.C:267-544 is F0929_PRIM_05_FTL_Load; "
           "FTL.H:5-26 defines HEADER and SEGMENTHEADER; PRIM.H:183-225 "
           "defines JUMPSEGMENTHEADER, JUMPTABLEENTRY, and DATASEGMENTHEADER.";
}
