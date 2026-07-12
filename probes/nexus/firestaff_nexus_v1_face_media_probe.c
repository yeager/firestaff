/*
 * Canonical Saturn FACE.BIN structural receipt.
 *
 * This real-media probe accepts only the hash-verified 45,104-byte FACE.BIN
 * corpus. It locks the observed FACE/PRS3 framing while deliberately keeping
 * portrait decoding blocked: PRS3 opcode semantics and FACE prefix palette
 * semantics have not been independently reconstructed from the Saturn code.
 */

#include "nexus_v1_ui_surfaces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEXUS_FACE_CANONICAL_SIZE 45104U
#define NEXUS_FACE_FRAME_COUNT 20

static int failures;

static void check(int condition, const char *message) {
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++failures; }
}

static int read_file(const char *path, unsigned char *data, size_t size) {
    FILE *file = fopen(path, "rb");
    int tail;
    if (!file) return 0;
    tail = fread(data, 1U, size, file) == size ? fgetc(file) : 0;
    fclose(file);
    return tail == EOF;
}

int main(void) {
    const char *root = getenv("FIRESTAFF_NEXUS_V1_DATA_DIR");
    char default_root[512];
    char path[640];
    unsigned char data[NEXUS_FACE_CANONICAL_SIZE];
    Nexus_UI_FaceLayout layout;
    Nexus_UI_FaceCompactRecordDescriptor first;
    Nexus_UI_FaceCompactRecordDescriptor last;
    Nexus_UI_FaceCompactRecordDescriptor descriptor;
    Nexus_UI_FaceRecordDecodeInfo decode;
    unsigned char pixels[48 * 48];
    const char *home;

    if (!root || !root[0]) {
        home = getenv("HOME");
        if (!home || !home[0]) {
            puts("SKIP: no Nexus data root configured");
            return 0;
        }
        snprintf(default_root, sizeof(default_root), "%s/.firestaff/data/nexus", home);
        root = default_root;
    }
    snprintf(path, sizeof(path), "%s/FACE.BIN", root);
    if (!read_file(path, data, sizeof(data))) {
        puts("SKIP: canonical Nexus FACE.BIN is not staged");
        return 0;
    }

    memset(&layout, 0, sizeof(layout));
    check(nexus_ui_face_layout_detect(data, (int)sizeof(data), &layout) &&
              layout.header_size == 56 && layout.entry_count == NEXUS_FACE_FRAME_COUNT &&
              layout.entry_size == 0 && layout.portrait_w == 56 && layout.portrait_h == 56,
          "canonical FACE.BIN has 20 variable 56x56 PRS3 frames");
    check(nexus_ui_face_compact_record_descriptor(data, (int)sizeof(data), 0,
                                                  &first) && first.valid &&
              first.prefix_offset == 56U && first.prefix_size == 128U &&
              first.prs3_offset == 184U && first.stream_size == 1796U &&
              first.prs3_version == 1U &&
              first.declared_pixel_count == 3136U,
          "first FACE frame retains canonical prefix, version-1 guard, and PRS3 bounds");
    check(nexus_ui_face_compact_record_descriptor(data, (int)sizeof(data), 19,
                                                  &last) && last.valid &&
              last.prs3_offset == 42960U && last.stream_size == 2126U &&
              last.prs3_size == 2142U && last.prs3_version == 1U &&
              last.declared_pixel_count == 3136U,
              "final FACE frame ends before the canonical two-byte container tail");
    for (int i = 0; i < NEXUS_FACE_FRAME_COUNT; ++i) {
        check(nexus_ui_face_compact_record_descriptor(data, (int)sizeof(data), i,
                                                      &descriptor) &&
                  descriptor.valid && descriptor.prs3_version == 1U &&
                  descriptor.declared_pixel_count == 3136U,
              "every canonical FACE frame selects the DM.BIN version-1 PRS3 route");
    }
    memset(&decode, 0, sizeof(decode));
    memset(pixels, 0xa5, sizeof(pixels));
    check(nexus_ui_expand_face_record_48x48(data + first.prs3_offset,
                                            (int)first.prs3_size,
                                            pixels, (int)sizeof(pixels),
                                            &decode) == 0 &&
              decode.kind == NEXUS_UI_FACE_RECORD_PRS3_UNPROVEN &&
              decode.copied_pixels == 0 && pixels[0] == 0xa5,
          "PRS3 frame remains blocked without synthesized portrait pixels");

    if (failures) {
        fprintf(stderr, "firestaff_nexus_v1_face_media_probe: FAIL (%d)\n", failures);
        return 1;
    }
    puts("RECEIPT: FACE structure proven; PRS3 opcode and palette semantics remain blocked.");
    puts("firestaff_nexus_v1_face_media_probe: PASS");
    return 0;
}
