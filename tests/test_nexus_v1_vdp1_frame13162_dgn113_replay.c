#include "firestaff_x68k_media_receipt.h"
#include "nexus_v1_vdp1_capture_compositor.h"
#include "nexus_v1_vdp1_dgn_material_resolver.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
#define strtok_r strtok_s
#endif

#define FRAME_INDEX 13162U
#define TRACE_ROWS 174U
#define TRACE_DRAW_ROWS 162U
#define DGN_JOIN_ROWS 113U
#define VDP1_VRAM_BYTES 0x80000U
#define VDP1_FBS_BYTES 0x80000U
#define COMMAND_BYTES 32U
#define CSV_COLUMNS 22U

typedef struct { uint8_t *data; size_t size; } Blob;

typedef struct {
    unsigned int index;
    uint32_t address;
    uint32_t frame_root;
    int local_x;
    int local_y;
    uint8_t command[COMMAND_BYTES];
} TraceRow;

typedef struct {
    const char *name;
    const char *sha256;
} ManifestEntry;

static Blob read_blob(const char *path)
{
    Blob out = {NULL, 0U};
    FILE *file = fopen(path, "rb");
    long length;
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return out;
    }
    out.data = (uint8_t *)malloc((size_t)length + 1U);
    if (!out.data || fread(out.data, 1U, (size_t)length, file) != (size_t)length) {
        free(out.data);
        out.data = NULL;
        fclose(file);
        return out;
    }
    fclose(file);
    out.data[length] = 0U;
    out.size = (size_t)length;
    return out;
}

static void release_blob(Blob *blob)
{
    free(blob->data);
    blob->data = NULL;
    blob->size = 0U;
}

static int blob_hash_is(const Blob *blob, const char *expected)
{
    char actual[65];
    return blob && blob->data &&
        firestaff_x68k_media_receipt_sha256_hex(
            blob->data, blob->size, actual, sizeof(actual)) == 0 &&
        strcmp(actual, expected) == 0;
}

static void make_path(char *out, size_t capacity, const char *root,
                      const char *relative)
{
    (void)snprintf(out, capacity, "%s/%s", root, relative);
}

static int manifest_has(const Blob *manifest, const char *relative,
                        const char *expected)
{
    const char *line;
    size_t relative_size = strlen(relative);
    if (!manifest || !manifest->data) return 0;
    line = (const char *)manifest->data;
    while (line && *line) {
        const char *end = strchr(line, '\n');
        size_t length = end ? (size_t)(end - line) : strlen(line);
        if (length == 66U + relative_size &&
            memcmp(line, expected, 64U) == 0 && line[64] == ' ' &&
            line[65] == ' ' && memcmp(line + 66U, relative, relative_size) == 0)
            return 1;
        line = end ? end + 1U : NULL;
    }
    return 0;
}

static int parse_uint(const char *text, int base, unsigned long *out)
{
    char *end = NULL;
    unsigned long value;
    if (!text || !*text || !out) return 0;
    value = strtoul(text, &end, base);
    if (end == text || *end != '\0') return 0;
    *out = value;
    return 1;
}

static uint16_t read_le16(const uint8_t *bytes)
{
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8U);
}

static int parse_csv_row(char *line, TraceRow *out)
{
    char *fields[CSV_COLUMNS];
    char *save = NULL;
    char *field;
    size_t count = 0U;
    unsigned long value;
    unsigned int word;

    if (!line || !out) return 0;
    field = strtok_r(line, ",", &save);
    while (field && count < CSV_COLUMNS) {
        fields[count++] = field;
        field = strtok_r(NULL, ",", &save);
    }
    if (count != CSV_COLUMNS || field != NULL ||
        !parse_uint(fields[0], 10, &value) || value != FRAME_INDEX ||
        !parse_uint(fields[1], 10, &value) || value >= TRACE_ROWS) return 0;
    out->index = (unsigned int)value;
    if (!parse_uint(fields[2], 0, &value) || value > VDP1_VRAM_BYTES)
        return 0;
    out->address = (uint32_t)value;
    for (word = 0U; word < 16U; ++word) {
        if (!parse_uint(fields[3U + word], 0, &value) || value > 0xffffUL)
            return 0;
        out->command[word * 2U] = (uint8_t)value;
        out->command[word * 2U + 1U] = (uint8_t)(value >> 8U);
    }
    if (!parse_uint(fields[19], 0, &value) || value > 0xffffffffUL)
        return 0;
    out->frame_root = (uint32_t)value;
    if (!parse_uint(fields[20], 10, &value) || value > 0x7fffUL) return 0;
    out->local_x = (int)value;
    if (!parse_uint(fields[21], 10, &value) || value > 0x7fffUL) return 0;
    out->local_y = (int)value;
    return (out->address & (COMMAND_BYTES - 1U)) == 0U &&
        out->address <= VDP1_VRAM_BYTES - COMMAND_BYTES;
}

static int load_checked_in_fixture(const Blob *fixture, TraceRow rows[TRACE_ROWS])
{
    char *copy;
    char *line;
    char *save = NULL;
    unsigned int count = 0U;
    int ok = 0;
    static const char header[] =
        "frame,index,address,word0,word1,word2,word3,word4,word5,word6,word7,"
        "word8,word9,word10,word11,word12,word13,word14,word15,frame_root,"
        "local_x,local_y";

    if (!blob_hash_is(fixture,
            "aa0cd750413a4c6618f91bbeb5953703c65cbc5d8481499135167f858421aebb"))
        return 0;
    copy = (char *)malloc(fixture->size + 1U);
    if (!copy) return 0;
    memcpy(copy, fixture->data, fixture->size);
    copy[fixture->size] = '\0';
    line = strtok_r(copy, "\r\n", &save);
    if (!line || strcmp(line, header) != 0) goto done;
    while ((line = strtok_r(NULL, "\r\n", &save)) != NULL) {
        TraceRow row;
        if (count >= TRACE_ROWS || !parse_csv_row(line, &row) ||
            row.index != count) goto done;
        rows[count++] = row;
    }
    ok = count == TRACE_ROWS;
done:
    free(copy);
    return ok;
}

static int rows_equal(const TraceRow *a, const TraceRow *b)
{
    return a->index == b->index && a->address == b->address &&
        a->frame_root == b->frame_root && a->local_x == b->local_x &&
        a->local_y == b->local_y &&
        memcmp(a->command, b->command, COMMAND_BYTES) == 0;
}

static int verify_external_trace(const Blob *trace,
                                 const TraceRow fixture_rows[TRACE_ROWS],
                                 const Blob *reconstructed_vram)
{
    char *copy;
    char *line;
    char *save = NULL;
    unsigned int seen = 0U;
    int ok = 0;
    static const char header[] =
        "frame,index,address,word0,word1,word2,word3,word4,word5,word6,word7,"
        "word8,word9,word10,word11,word12,word13,word14,word15,frame_root,"
        "local_x,local_y";
    if (!blob_hash_is(trace,
            "a963e0d6e7667ac4a947a546933d0b306ea5caf13a511702a898c30472c12a88") ||
        !blob_hash_is(reconstructed_vram,
            "90b85d0358c7605c8186828d2e42ab46412f504f12c40348197a46db8f770913") ||
        reconstructed_vram->size != VDP1_VRAM_BYTES) return 0;
    copy = (char *)malloc(trace->size + 1U);
    if (!copy) return 0;
    memcpy(copy, trace->data, trace->size);
    copy[trace->size] = '\0';
    line = strtok_r(copy, "\r\n", &save);
    if (!line || strcmp(line, header) != 0) goto done;
    while ((line = strtok_r(NULL, "\r\n", &save)) != NULL) {
        char *frame_end = strchr(line, ',');
        char frame_text[24];
        unsigned long frame;
        TraceRow row;
        size_t frame_size;
        if (!frame_end) goto done;
        frame_size = (size_t)(frame_end - line);
        if (frame_size == 0U || frame_size >= sizeof(frame_text)) goto done;
        memcpy(frame_text, line, frame_size);
        frame_text[frame_size] = '\0';
        if (!parse_uint(frame_text, 10, &frame)) goto done;
        if (frame != FRAME_INDEX) continue;
        if (!parse_csv_row(line, &row) || row.index != seen || seen >= TRACE_ROWS ||
            !rows_equal(&row, &fixture_rows[seen]) ||
            memcmp(row.command, reconstructed_vram->data + row.address,
                   COMMAND_BYTES) != 0) goto done;
        ++seen;
    }
    ok = seen == TRACE_ROWS;
done:
    free(copy);
    return ok;
}

static int json_string_value(const char *begin, const char *end,
                             const char *key, char *out, size_t capacity)
{
    char needle[64];
    const char *found;
    const char *value;
    const char *close;
    size_t length;
    (void)snprintf(needle, sizeof(needle), "\"%s\":", key);
    found = strstr(begin, needle);
    if (!found || (end && found >= end)) return 0;
    value = strchr(found + strlen(needle), '"');
    if (!value || (end && value >= end)) return 0;
    ++value;
    close = strchr(value, '"');
    if (!close || (end && close >= end)) return 0;
    length = (size_t)(close - value);
    if (!length || length >= capacity) return 0;
    memcpy(out, value, length);
    out[length] = '\0';
    return 1;
}

static int json_number_value(const char *json, const char *key,
                             unsigned long expected)
{
    char needle[64];
    const char *found;
    char *end = NULL;
    unsigned long value;
    (void)snprintf(needle, sizeof(needle), "\"%s\":", key);
    found = strstr(json, needle);
    if (!found) return 0;
    value = strtoul(found + strlen(needle), &end, 10);
    return end != found + strlen(needle) && value == expected;
}

static int parse_execution_json(const Blob *execution,
                                const TraceRow rows[TRACE_ROWS],
                                uint8_t executed[TRACE_ROWS])
{
    const char *cursor;
    unsigned int count = 0U;
    unsigned int executed_count = 0U;
    int ok = 0;
    if (!blob_hash_is(execution,
            "2417d30d651f385ef384881d3b14524576069c22ac1f2fe53f71f9938e3355b4") ||
        !strstr((const char *)execution->data, "\"frame_index\": 13162") ||
        !json_number_value((const char *)execution->data, "trace_row_count", TRACE_ROWS) ||
        !json_number_value((const char *)execution->data, "flow_transitions_verified", 173U) ||
        !strstr((const char *)execution->data, "\"renderer_permitted\": false")) return 0;
    memset(executed, 0, TRACE_ROWS);
    cursor = strstr((const char *)execution->data, "\"commands\": [");
    if (!cursor) return 0;
    cursor += strlen("\"commands\": [");
    while (count < TRACE_ROWS) {
        const char *index_field = strstr(cursor, "\"index\":");
        const char *object_end;
        char address_text[32];
        unsigned long index;
        unsigned long address;
        char *number_end = NULL;
        int was_executed;
        if (!index_field) goto done;
        index = strtoul(index_field + strlen("\"index\":"), &number_end, 10);
        if (number_end == index_field + strlen("\"index\":")) goto done;
        object_end = strchr(number_end, '}');
        if (!object_end || !json_string_value(number_end, object_end,
                "address", address_text, sizeof(address_text)) ||
            index != count || !parse_uint(address_text, 0, &address) ||
            address != rows[count].address) goto done;
        {
            const char *executed_field = strstr(number_end, "\"executed\":");
            was_executed = executed_field && executed_field < object_end &&
                strncmp(executed_field + strlen("\"executed\":"), " true", 5U) == 0;
            if (!was_executed && (!executed_field || executed_field >= object_end ||
                strncmp(executed_field + strlen("\"executed\":"), " false", 6U) != 0))
                goto done;
        }
        executed[count] = (uint8_t)was_executed;
        if (was_executed) ++executed_count;
        ++count;
        cursor = object_end + 1;
    }
    ok = count == TRACE_ROWS && executed_count == 168U &&
        strstr((const char *)execution->data, "\"skipped\": 5") != NULL &&
        strstr((const char *)execution->data, "\"non_end\": 168") != NULL;
done:
    return ok;
}

static int write_rgba(const char *path,
                      const Nexus_V1_Vdp1RgbaCaptureFramebuffer *framebuffer)
{
    FILE *file;
    size_t pixel;
    if (!path || !*path || !framebuffer) return 0;
    file = fopen(path, "wb");
    if (!file) return 0;
    for (pixel = 0U; pixel < NEXUS_FB_W * NEXUS_FB_H; ++pixel) {
        uint32_t argb = framebuffer->rgba_buffer[pixel];
        uint8_t rgba[4] = {(uint8_t)(argb >> 16U), (uint8_t)(argb >> 8U),
            (uint8_t)argb, (uint8_t)(argb >> 24U)};
        if (fwrite(rgba, 1U, sizeof(rgba), file) != sizeof(rgba)) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0;
}

static int write_receipt(const char *path, const char *rgba_path,
                         const char *capture_sha, const char *dgn_sha,
                         const uint32_t *indices, unsigned int count,
                         const Nexus_V1_Vdp1CommandSequenceReceipt *sequence,
                         const Nexus_V1_Vdp1RgbaCaptureSequenceReceipt *replay)
{
    char rgba_sha[65];
    Blob rgba = read_blob(rgba_path);
    FILE *file;
    unsigned int i;
    if (!rgba.data || rgba.size != NEXUS_FB_W * NEXUS_FB_H * 4U ||
        firestaff_x68k_media_receipt_sha256_hex(
            rgba.data, rgba.size, rgba_sha, sizeof(rgba_sha)) != 0) {
        release_blob(&rgba);
        return 0;
    }
    release_blob(&rgba);
    file = fopen(path, "wb");
    if (!file) return 0;
    fprintf(file, "format=firestaff-nexus-frame13162-dgn113-vdp1-replay-v1\n");
    fprintf(file, "frame=13162\nwidth=320\nheight=224\npixel_format=RGBA8\n");
    fprintf(file, "capture_manifest_verified=true\ntrace_rows=%u\ntrace_draw_rows=%d\n",
            TRACE_ROWS, sequence->draw_count);
    fprintf(file, "command_order_verified=true\nlocal_state_updates=%d\nuser_clip_updates=%d\n",
            sequence->local_coordinate_count, sequence->user_clip_count);
    fprintf(file, "dgn_image_clut_joins=%u\nsource_joins_verified=%d\npalette_joins_verified=%d\n",
            count, replay->source_joins_verified, replay->palette_joins_verified);
    fprintf(file, "capture_gap_commands=%d\nwritten_pixels=%d\ngouraud_applied=%d\n",
            replay->capture_gap_commands, replay->written_pixels,
            replay->gouraud_commands_applied);
    fprintf(file, "capture_only=true\nrenderer_permitted=false\nfull_frame_parity_claim=false\n");
    fprintf(file, "capture_vdp1_fbs_sha256=%s\nretail_lev00_dgn_sha256=%s\n",
            capture_sha, dgn_sha);
    fprintf(file, "rgba_sha256=%s\ndgn_join_command_indices=", rgba_sha);
    for (i = 0U; i < count; ++i)
        fprintf(file, "%s%u", i ? "," : "", indices[i]);
    fputc('\n', file);
    return fclose(file) == 0;
}

int main(int argc, char **argv)
{
    char path[4096];
    char receipt_path[4096];
    Blob manifest = {0}, raw_trace = {0}, reconstructed_vram = {0};
    Blob fbs = {0}, snapshot_vram = {0}, bundle = {0};
    Blob execution = {0}, execution_fixture = {0}, fixture = {0}, dgn = {0};
    TraceRow rows[TRACE_ROWS];
    uint8_t executed[TRACE_ROWS] = {0};
    Nexus_V1_Vdp1CommandSequenceReceipt sequence;
    Nexus_V1_Vdp1RgbaCaptureSequenceReceipt rgba_receipt;
    Nexus_V1_Vdp1RgbaCaptureSequenceInput rgba_input;
    Nexus_V1_Vdp1DgnMaterialResolverInput resolver_input;
    Nexus_V1_Vdp1CaptureCompositeInput materials[TRACE_ROWS];
    uint8_t command_bytes[TRACE_ROWS][COMMAND_BYTES];
    uint32_t joined_indices[TRACE_ROWS];
    int joined_origin_x[TRACE_ROWS], joined_origin_y[TRACE_ROWS];
    uint8_t joined_origin_verified[TRACE_ROWS];
    uint32_t offsets[TRACE_ROWS];
    const char *capture_root;
    const char *dgn_path;
    const char *fixture_path;
    const char *execution_path;
    const char *rgba_path;
    uint8_t sequence_commands[TRACE_ROWS * COMMAND_BYTES];
    Blob registers = {0};
    Nexus_V1_Vdp1RgbaCaptureFramebuffer *rgba = NULL;
    unsigned int joined = 0U;
    unsigned int clip_updates = 0U;
    unsigned int local_updates = 0U;
    int clip_x0 = 0, clip_y0 = 0, clip_x1 = 0, clip_y1 = 0;
    int clip_is_verified = 0;
    int result = 1;
    char fbs_sha[65];
    char dgn_sha[65];

    if (argc != 6) {
        fprintf(stderr, "usage: %s <frame-13162-capture-root> <LEV00.DGN> "
            "<checked-in-trace.csv> <checked-in-execution.json> <output.rgba>\n", argv[0]);
        return 2;
    }
    capture_root = argv[1];
    dgn_path = argv[2];
    fixture_path = argv[3];
    execution_path = argv[4];
    rgba_path = argv[5];
    if (snprintf(receipt_path, sizeof(receipt_path), "%s.receipt.txt", rgba_path) >=
            (int)sizeof(receipt_path)) {
        fprintf(stderr, "output receipt path is too long\n");
        return 2;
    }

#define LOAD_CAPTURE(blob, relative) do { \
        make_path(path, sizeof(path), capture_root, relative); \
        blob = read_blob(path); \
        if (!blob.data) { \
            fprintf(stderr, "SKIP: authenticated capture file is unavailable: %s\n", path); \
            result = 77; \
            goto done; \
        } \
    } while (0)
    LOAD_CAPTURE(manifest, "SHA256SUMS.txt");
    LOAD_CAPTURE(raw_trace, "dumps/vdp1-command-trace.csv");
    LOAD_CAPTURE(snapshot_vram, "dumps/vdp1-vram.bin");
    LOAD_CAPTURE(fbs, "dumps/vdp1-fbs.bin");
    LOAD_CAPTURE(registers, "dumps/vdp1-registers.bin");
    LOAD_CAPTURE(bundle, "dumps/vdp1-capture-bundle.txt");
    LOAD_CAPTURE(reconstructed_vram,
        "vdp1-vram-command-trace-reconstructed-le.bin");
    LOAD_CAPTURE(execution, "vdp1-command-execution-frame13162.json");
#undef LOAD_CAPTURE
    fixture = read_blob(fixture_path);
    execution_fixture = read_blob(execution_path);
    dgn = read_blob(dgn_path);
    if (!fixture.data || !execution_fixture.data || !dgn.data) {
        fprintf(stderr, "SKIP: checked-in trace/receipt or retail LEV00.DGN is unavailable\n");
        result = 77;
        goto done;
    }
    if (!blob_hash_is(&fbs,
            "8852b60717358bb10416cec2ebf8d8c6c049025fb78f1d621e938e3305b51c25") ||
        fbs.size != VDP1_FBS_BYTES ||
        !blob_hash_is(&snapshot_vram,
            "9974b1716c4ee91aa1d44d3ed2705cc09ac27b17c642e68bdaf96293fd36c4e7") ||
        snapshot_vram.size != VDP1_VRAM_BYTES ||
        !blob_hash_is(&registers,
            "13fc8537361274b6665fa85257edaa5a64789cde92ba326489a9defc9ae880c8") ||
        registers.size != 32U ||
        !blob_hash_is(&bundle,
            "1a32f6cfa3def2ce1c54f66a94a3b3a9ac75b01ad267edc3033ef8c86b3be982") ||
        !blob_hash_is(&execution,
            "2417d30d651f385ef384881d3b14524576069c22ac1f2fe53f71f9938e3355b4") ||
        execution.size != execution_fixture.size ||
        memcmp(execution.data, execution_fixture.data, execution.size) != 0 ||
        !blob_hash_is(&dgn,
            "24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a")) {
        fprintf(stderr, "frame-13162 capture, execution receipt, or LEV00.DGN hash mismatch\n");
        goto done;
    }
    {
        static const ManifestEntry expected[] = {
            {"./dumps/vdp1-command-trace.csv", "a963e0d6e7667ac4a947a546933d0b306ea5caf13a511702a898c30472c12a88"},
            {"./dumps/vdp1-vram.bin", "9974b1716c4ee91aa1d44d3ed2705cc09ac27b17c642e68bdaf96293fd36c4e7"},
            {"./dumps/vdp1-fbs.bin", "8852b60717358bb10416cec2ebf8d8c6c049025fb78f1d621e938e3305b51c25"},
            {"./dumps/vdp1-registers.bin", "13fc8537361274b6665fa85257edaa5a64789cde92ba326489a9defc9ae880c8"},
            {"./dumps/vdp1-capture-bundle.txt", "1a32f6cfa3def2ce1c54f66a94a3b3a9ac75b01ad267edc3033ef8c86b3be982"},
            {"./vdp1-command-execution-frame13162.json", "2417d30d651f385ef384881d3b14524576069c22ac1f2fe53f71f9938e3355b4"},
            {"./vdp1-vram-command-trace-reconstructed-le.bin", "90b85d0358c7605c8186828d2e42ab46412f504f12c40348197a46db8f770913"}
        };
        size_t i;
        for (i = 0U; i < sizeof(expected) / sizeof(expected[0]); ++i) {
            if (!manifest_has(&manifest, expected[i].name, expected[i].sha256)) {
                fprintf(stderr, "capture manifest omits or changes %s\n", expected[i].name);
                goto done;
            }
        }
    }
    if (!load_checked_in_fixture(&fixture, rows) ||
        !verify_external_trace(&raw_trace, rows, &reconstructed_vram) ||
        !parse_execution_json(&execution_fixture, rows, executed)) {
        fprintf(stderr, "frame-13162 trace fixture, capture trace, or execution JSON is inconsistent\n");
        goto done;
    }
    if (!strstr((const char *)bundle.data, "capture_frame_index=13162\n") ||
        !strstr((const char *)bundle.data, "capture_next_command_index=174\n") ||
        !strstr((const char *)bundle.data, "trace_file_complete=1\n") ||
        !strstr((const char *)bundle.data, "trace_rows_for_capture_frame=174\n") ||
        !strstr((const char *)bundle.data, "trace_dropped_total=0\n")) {
        fprintf(stderr, "Ymir capture bundle does not prove a complete, drop-free frame\n");
        goto done;
    }
    memset(&sequence, 0, sizeof(sequence));
    for (unsigned int i = 0U; i < TRACE_ROWS; ++i) {
        offsets[i] = rows[i].address;
        memcpy(sequence_commands + (size_t)i * COMMAND_BYTES,
               rows[i].command, COMMAND_BYTES);
    }
    if (!nexus_v1_vdp1_command_sequence_trace(
            sequence_commands, offsets, (int)TRACE_ROWS, &sequence) ||
        !sequence.valid || !sequence.complete || !sequence.command_order_verified ||
        !sequence.end_record_verified || !sequence.display_origin_verified ||
        sequence.command_count != (int)TRACE_ROWS ||
        sequence.draw_count != (int)TRACE_DRAW_ROWS ||
        sequence.user_clip_count != 2 || sequence.local_coordinate_count != 4) {
        fprintf(stderr, "frame-13162 command ordering or state trace failed closed\n");
        goto done;
    }
    /* The Ymir CSV records the local state before a LOCAL record is applied,
     * while the sequence parser receipt reports the state effective for the
     * current record. Bind the final captured LOCAL transition directly. */
    if (read_le16(rows[7].command) % 16U != 10U ||
        read_le16(rows[7].command + 12U) != 160U ||
        read_le16(rows[7].command + 14U) != 112U ||
        sequence.command_origin_x[7] != 160 ||
        sequence.command_origin_y[7] != 112 ||
        rows[8].local_x != 160 || rows[8].local_y != 112) {
        fprintf(stderr, "frame-13162 LOCAL origin transition is not bound to the DGN draw window\n");
        goto done;
    }

    memset(&resolver_input, 0, sizeof(resolver_input));
    resolver_input.dgn_bytes = dgn.data;
    resolver_input.dgn_byte_count = (int)dgn.size;
    resolver_input.source_hash_verified = 1;
    resolver_input.palette_slot_base = 0;
    memset(materials, 0, sizeof(materials));
    for (unsigned int i = 0U; i < TRACE_ROWS; ++i) {
        uint16_t control = (uint16_t)rows[i].command[0] |
            ((uint16_t)rows[i].command[1] << 8U);
        unsigned int type = control & 15U;
        Nexus_V1_Vdp1TextureCommand parsed;
        Nexus_V1_Vdp1CaptureCompositeInput material;
        uint32_t palette_offset;
        int resolved;

        if (type == 8U) {
            if ((control & 0xc000U) != 0U || clip_updates >= 2U) {
                fprintf(stderr, "unsupported frame-13162 user-clip control row\n");
                goto done;
            }
            clip_x0 = (int)(((uint16_t)rows[i].command[12] |
                ((uint16_t)rows[i].command[13] << 8U)) & 0x1fffU);
            clip_y0 = (int)(((uint16_t)rows[i].command[14] |
                ((uint16_t)rows[i].command[15] << 8U)) & 0x1fffU);
            clip_x1 = (int)(((uint16_t)rows[i].command[20] |
                ((uint16_t)rows[i].command[21] << 8U)) & 0x1fffU);
            clip_y1 = (int)(((uint16_t)rows[i].command[22] |
                ((uint16_t)rows[i].command[23] << 8U)) & 0x1fffU);
            clip_is_verified = 1;
            ++clip_updates;
        } else if (type == 10U) {
            ++local_updates;
        }
        if (nexus_v1_vdp1_texture_command_parse(rows[i].command,
                COMMAND_BYTES, &parsed) != 0) {
            fprintf(stderr, "unparseable captured command %u\n", i);
            goto done;
        }
        if (!parsed.texture_command || parsed.command_type != 2U ||
            parsed.colour_mode != 1U) continue;
        memset(&material, 0, sizeof(material));
        resolved = nexus_v1_vdp1_dgn_material_resolver(
            reconstructed_vram.data, (int)reconstructed_vram.size,
            rows[i].command, COMMAND_BYTES, &parsed, rows[i].address,
            &material, &resolver_input);
        if (resolved && material.transparent_capture_noop_verified) continue;
        if (!resolved) continue;
        if (!resolved || !material.dgn_source_hash_verified ||
            !material.dgn_image || !material.dgn_palette ||
            material.dgn_image_size != (int)parsed.texture_byte_count ||
            material.dgn_palette_size != 32 || !executed[i] ||
            !clip_is_verified || clip_x0 != 0 || clip_y0 != 0 ||
            clip_x1 != 320 || clip_y1 != 224 || i >= 169U ||
            (parsed.draw_mode & UINT16_C(0x0400)) == 0U ||
            (parsed.draw_mode & UINT16_C(0x0200)) != 0U ||
            rows[i].local_x != sequence.command_origin_x[i] ||
            rows[i].local_y != sequence.command_origin_y[i] ||
            sequence.command_origin_x[i] != 160 ||
            sequence.command_origin_y[i] != 112) {
            fprintf(stderr, "DGN/CLUT source or captured clip/origin join failed at row %u\n", i);
            goto done;
        }
        if (joined >= DGN_JOIN_ROWS) {
            fprintf(stderr, "more DGN joins than the pinned authentic capture inventory\n");
            goto done;
        }
        palette_offset = (((uint32_t)parsed.colour_control & ~UINT32_C(3)) << 3U);
        if (palette_offset > VDP1_VRAM_BYTES - 32U) {
            fprintf(stderr, "invalid captured palette span at row %u\n", i);
            goto done;
        }
        memcpy(command_bytes[joined], rows[i].command, COMMAND_BYTES);
        materials[joined] = material;
        materials[joined].command = command_bytes[joined];
        materials[joined].command_size = COMMAND_BYTES;
        materials[joined].texture_span = reconstructed_vram.data +
            parsed.texture_source_byte_offset;
        materials[joined].texture_span_size = (int)parsed.texture_byte_count;
        materials[joined].palette_state = reconstructed_vram.data + palette_offset;
        materials[joined].palette_state_size = 32;
        materials[joined].original_saturn_capture_verified = 1;
        materials[joined].screen_origin_x = sequence.command_origin_x[i];
        materials[joined].screen_origin_y = sequence.command_origin_y[i];
        materials[joined].system_clip_state_verified = 1;
        materials[joined].system_clip_x = 320;
        materials[joined].system_clip_y = 224;
        materials[joined].user_clip_state_verified = 1;
        materials[joined].user_clip_x0 = clip_x0;
        materials[joined].user_clip_y0 = clip_y0;
        materials[joined].user_clip_x1 = clip_x1;
        materials[joined].user_clip_y1 = clip_y1;
        joined_indices[joined] = i;
        joined_origin_x[joined] = sequence.command_origin_x[i];
        joined_origin_y[joined] = sequence.command_origin_y[i];
        joined_origin_verified[joined] = 1U;
        ++joined;
    }
    if (joined != DGN_JOIN_ROWS || clip_updates != 2U || local_updates != 4U) {
        fprintf(stderr, "incomplete authentic window: DGN joins=%u/%u clips=%u locals=%u\n",
            joined, DGN_JOIN_ROWS, clip_updates, local_updates);
        goto done;
    }
    rgba = (Nexus_V1_Vdp1RgbaCaptureFramebuffer *)calloc(1U, sizeof(*rgba));
    if (!rgba) {
        fprintf(stderr, "cannot allocate capture-only RGBA framebuffer\n");
        goto done;
    }
    memset(&rgba_input, 0, sizeof(rgba_input));
    rgba_input.commands = materials;
    rgba_input.vdp1_vram = reconstructed_vram.data;
    rgba_input.vdp1_vram_size = (int)reconstructed_vram.size;
    rgba_input.vdp1_vram_hash_verified = 1;
    rgba_input.command_trace_indices = joined_indices;
    rgba_input.command_origin_x = joined_origin_x;
    rgba_input.command_origin_y = joined_origin_y;
    rgba_input.command_origin_verified = joined_origin_verified;
    rgba_input.command_count = (int)joined;
    rgba_input.trace_draw_command_count = (int)TRACE_DRAW_ROWS;
    rgba_input.trace_row_count = (int)TRACE_ROWS;
    rgba_input.trace_complete_verified = 1;
    rgba_input.command_order_verified = 1;
    rgba_input.local_coordinate_state_verified = 1;
    rgba_input.end_record_verified = 1;
    rgba_input.system_clip_state_verified = 1;
    rgba_input.system_clip_x = 320;
    rgba_input.system_clip_y = 224;
    memset(&rgba_receipt, 0, sizeof(rgba_receipt));
    if (!nexus_v1_vdp1_capture_composite_mode1_rgba_sequence(
            rgba, &rgba_input, &rgba_receipt) || !rgba_receipt.valid ||
        !rgba_receipt.capture_only || rgba_receipt.renderer_permitted ||
        rgba_receipt.command_count != (int)DGN_JOIN_ROWS ||
        rgba_receipt.source_joins_verified != (int)DGN_JOIN_ROWS ||
        rgba_receipt.palette_joins_verified != (int)DGN_JOIN_ROWS ||
        rgba_receipt.capture_gap_commands != (int)(TRACE_DRAW_ROWS - DGN_JOIN_ROWS) ||
        rgba_receipt.user_clip_commands_verified != (int)DGN_JOIN_ROWS ||
        rgba_receipt.gouraud_commands_unrendered != 0) {
        fprintf(stderr, "capture-only sequence compositor failed closed or returned an incomplete receipt\n");
        goto done;
    }
    if (!write_rgba(rgba_path, rgba) ||
        firestaff_x68k_media_receipt_sha256_hex(fbs.data, fbs.size,
            fbs_sha, sizeof(fbs_sha)) != 0 ||
        firestaff_x68k_media_receipt_sha256_hex(dgn.data, dgn.size,
            dgn_sha, sizeof(dgn_sha)) != 0 ||
        !write_receipt(receipt_path, rgba_path, fbs_sha, dgn_sha,
            joined_indices, joined, &sequence, &rgba_receipt)) {
        fprintf(stderr, "could not write replay output and receipt\n");
        goto done;
    }
    printf("PASS: authentic frame %u replayed %u uniquely DGN/CLUT-joined VDP1 commands; "
        "rows=%u draws=%u gaps=%d written_pixels=%d; output=%s receipt=%s; "
        "capture_only=true renderer_permitted=false (no screenshot parity claim)\n",
        FRAME_INDEX, joined, TRACE_ROWS, sequence.draw_count,
        rgba_receipt.capture_gap_commands, rgba_receipt.written_pixels,
        rgba_path, receipt_path);
    result = 0;
done:
    free(rgba);
    release_blob(&manifest);
    release_blob(&raw_trace);
    release_blob(&reconstructed_vram);
    release_blob(&fbs);
    release_blob(&registers);
    release_blob(&snapshot_vram);
    release_blob(&bundle);
    release_blob(&execution);
    release_blob(&execution_fixture);
    release_blob(&fixture);
    release_blob(&dgn);
    return result;
}
