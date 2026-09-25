#include "theron_v1_track02.h"
#include "asset_find_by_hash.h"
#include "firestaff_x68k_media_receipt.h"
#include "firestaff_theron_media_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

#define THERON_TEST_US_OGG_SHA256 \
    "c2b296a82898a749503b10edab2523cbb5e7e165ef8c95abafe348fe36bc9c3e"
#define THERON_TEST_JP_OGG_SHA256 \
    "bfac627f0e1ee7debd5bb356065d11f1b3542402e8831b1634d1eab3e119a619"

#if !defined(_WIN32)
static int test_authentic_rar_ogg(const char *archive, int japanese) {
    const char *track02_md5 = japanese ? THERON_TRACK02_MD5_JP_ISO
                                       : THERON_TRACK02_MD5_US_ISO;
    const char *ogg_sha256 = japanese ? THERON_TEST_JP_OGG_SHA256
                                       : THERON_TEST_US_OGG_SHA256;
    const char *cue_member = japanese ? "TQJP.cue" : "TQUS.cue";
    const char *ogg_member = japanese ? "TQJP01.ogg" : "TQUS01.ogg";
    const char *track19 = japanese ? "TQJP19.iso" : "TQUS19.iso";
    const char *track02_tail = japanese ? "TQJP02End.iso" : "TQUS02End.iso";
    char cue_path[ASSET_PATH_MAX];
    char track02_path[ASSET_PATH_MAX];
    char ogg_path[ASSET_PATH_MAX];
    char audio_sha256[65];
    uint8_t *cue_bytes = NULL;
    uint8_t *ogg_bytes = NULL;
    size_t cue_size = 0u;
    size_t ogg_size = 0u;
    FirestaffTheronMediaStatus media;
    FirestaffTheronMediaStatus cue;
    Theron_Track01CddaHandoff handoff;
    Theron_Track01CddaStream stream = {0};
    int ok = 0;

    if (snprintf(cue_path, sizeof(cue_path), "%s::%s", archive, cue_member) >=
            (int)sizeof(cue_path) ||
        snprintf(track02_path, sizeof(track02_path), "%s::@concat(%s,%s)",
                 archive, track19, track02_tail) >= (int)sizeof(track02_path) ||
        snprintf(ogg_path, sizeof(ogg_path), "%s::%s", archive, ogg_member) >=
            (int)sizeof(ogg_path) ||
        FirestaffTheronMedia_ClassifyPathForTrack02(
            archive, track02_md5, &media) != 0 ||
        strcmp(media.candidate_path, track02_path) != 0 ||
        strcmp(media.cue_path, cue_path) != 0 ||
        !asset_read_path_alloc(cue_path, &cue_bytes, &cue_size) ||
        !cue_bytes || cue_size == 0u || cue_size > 64u * 1024u ||
        FirestaffTheronMedia_ParseCue((const char *)cue_bytes, cue_size, &cue) != 0 ||
        !cue.paired_track01_track02 ||
        strcmp(cue.track01_path, japanese ? "TQJP01.wav" : "TQUS01.wav") != 0 ||
        !asset_read_path_alloc(ogg_path, &ogg_bytes, &ogg_size) ||
        !ogg_bytes || ogg_size == 0u || ogg_size > 16u * 1024u * 1024u ||
        firestaff_x68k_media_receipt_sha256_hex(
            ogg_bytes, ogg_size, audio_sha256, sizeof(audio_sha256)) != 0 ||
        strcmp(audio_sha256, ogg_sha256) != 0) {
        fprintf(stderr, "authentic %s RAR CDDA failed source/hash admission\n",
                japanese ? "JP" : "US");
        goto cleanup;
    }
    memset(&handoff, 0, sizeof(handoff));
    handoff.status = THERON_TRACK01_CDDA_AVAILABLE;
    handoff.track02_variant = theron_v1_track02_variant_for_md5(track02_md5);
    snprintf(handoff.cue_path, sizeof(handoff.cue_path), "%s", cue_path);
    snprintf(handoff.audio_path, sizeof(handoff.audio_path), "%s", ogg_path);
    snprintf(handoff.track02_path, sizeof(handoff.track02_path), "%s", track02_path);
    handoff.audio_file_bytes = ogg_size;
    handoff.audio_is_vorbis = 1;
    handoff.original_cdda = 1;
    handoff.playback_handoff_ready = 1;
    handoff.track_number = 1u;
    if (!theron_v1_track01_cdda_stream_start_memory(
            &handoff, ogg_bytes, ogg_size, &stream) ||
        !theron_v1_track01_cdda_stream_pump(&stream) ||
        stream.sectors_queued == 0u) {
        fprintf(stderr, "authentic %s in-memory OGG stream failed to decode\n",
                japanese ? "JP" : "US");
        goto cleanup;
    }
    printf("PASS: authentic %s RAR OGG hash-verified and decoded in memory (%zu bytes)\n",
           japanese ? "JP" : "US", ogg_size);
    ok = 1;
cleanup:
    theron_v1_track01_cdda_stream_stop(&stream);
    free(cue_bytes);
    free(ogg_bytes);
    return ok;
}
#endif

#if !defined(_WIN32)
#include <unistd.h>
#endif

static int write_file(const char *path, const void *contents, size_t length) {
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (length && fwrite(contents, 1u, length, file) != length) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int cue_declares_raw_track02(const char *path) {
    char line[2048];
    FILE *cue = path ? fopen(path, "rb") : NULL;
    int raw = 0;
    if (!cue) return 0;
    while (fgets(line, sizeof(line), cue)) {
        if (strstr(line, "TRACK 02 MODE1/2352")) {
            raw = 1;
            break;
        }
        if (strstr(line, "TRACK 02 MODE1/2048")) break;
    }
    fclose(cue);
    return raw;
}

int main(void) {
#if defined(_WIN32)
    printf("test_theron_v1_track01_cdda_handoff: SKIP (fixture path)\n");
    return 0;
#else
    char directory[] = "firestaff_theron_track01_XXXXXX";
    char cue[512];
    char audio[512];
    char data[512];
    unsigned char sectors[THERON_TRACK01_CDDA_SECTOR_BYTES * 3u] = {0};
    Theron_Track01CddaHandoff handoff;
    Theron_Track01CddaStream stream = {0};
    int failed = 0;
    if (!mkdtemp(directory)) return 1;
    {
        const char *archive = getenv("FIRESTAFF_THERON_RAR");
        if (archive && archive[0] &&
            (!test_authentic_rar_ogg(archive, 0) ||
             !test_authentic_rar_ogg(archive, 1))) {
            rmdir(directory);
            return 1;
        }
    }
    snprintf(cue, sizeof(cue), "%s/original.cue", directory);
    snprintf(audio, sizeof(audio), "%s/track01.bin", directory);
    snprintf(data, sizeof(data), "%s/track02.bin", directory);
    sectors[0] = 0x34u;
    sectors[THERON_TRACK01_CDDA_SECTOR_BYTES] = 0x56u;
    sectors[THERON_TRACK01_CDDA_SECTOR_BYTES * 2u] = 0x78u;
    failed |= !write_file(audio, sectors, sizeof(sectors));
    failed |= !write_file(data, "fixture verified data bytes", strlen("fixture verified data bytes"));
    failed |= !write_file(cue,
        "FILE \"track01.bin\" BINARY\n"
        "  TRACK 01 AUDIO\n"
        "    INDEX 01 00:00:01\n"
        "FILE \"track02.bin\" BINARY\n"
        "  TRACK 02 MODE1/2352\n"
        "    INDEX 01 00:00:00\n",
        strlen("FILE \"track01.bin\" BINARY\n"
               "  TRACK 01 AUDIO\n"
               "    INDEX 01 00:00:01\n"
               "FILE \"track02.bin\" BINARY\n"
               "  TRACK 02 MODE1/2352\n"
               "    INDEX 01 00:00:00\n"));
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            cue, THERON_TRACK02_MD5_US_BIN, &handoff) != THERON_TRACK01_CDDA_AVAILABLE) {
        failed = 1;
    }
    if (!failed && (!handoff.playback_handoff_ready || !handoff.original_cdda ||
                    handoff.index_lba != 1u || handoff.audio_sector_count != 2u ||
                    handoff.audio_start_byte != THERON_TRACK01_CDDA_SECTOR_BYTES ||
                    strcmp(handoff.audio_path, audio) != 0 ||
                    strcmp(handoff.track02_path, data) != 0)) {
        failed = 1;
    }
    /* The two-sector fixture is shorter than the bounded queue.  Filling the
     * queue must wrap only to its CUE-derived Track 01 start, never beyond it.
     * The queue bound is THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS (16 since
     * 2026-07-14): 2 initial sectors + 7 wrap loops of 2 = 16 queued. */
    if (!failed && (!theron_v1_track01_cdda_lifecycle_update(&handoff, 1, &stream) ||
                    stream.sectors_read != 2u ||
                    stream.sectors_queued != THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS ||
                    stream.loop_count != (THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS - 2u) / 2u)) {
        failed = 1;
    }
    if (!failed && (!stream.output_started ||
                    !theron_v1_track01_cdda_lifecycle_update(&handoff, 0, &stream) ||
                    stream.output_started || stream.audio_file || stream.sdl_stream)) {
        failed = 1;
    }
    if (!failed && !write_file(audio, sectors, THERON_TRACK01_CDDA_SECTOR_BYTES - 1u)) {
        failed = 1;
    }
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            cue, THERON_TRACK02_MD5_US_BIN, &handoff) != THERON_TRACK01_CDDA_UNAVAILABLE) {
        failed = 1;
    }
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            data, THERON_TRACK02_MD5_US_BIN, &handoff) != THERON_TRACK01_CDDA_UNAVAILABLE) {
        failed = 1;
    }
    if (!failed && theron_v1_track01_cdda_handoff_from_verified_media(
            cue, "00000000000000000000000000000000", &handoff) != THERON_TRACK01_CDDA_UNVERIFIED) {
        failed = 1;
    }
    if (!failed && (theron_v1_track01_cdda_lifecycle_update(&handoff, 1, &stream) ||
                    stream.output_started || stream.audio_file || stream.sdl_stream)) {
        failed = 1;
    }
    {
        const char *real_cue = getenv("FIRESTAFF_THERON_CUE");
        int real_is_jp = real_cue &&
            (strstr(real_cue, "TQJP") != NULL ||
             strstr(real_cue, "Japan") != NULL);
        int real_raw_track = cue_declares_raw_track02(real_cue);
        const char *real_md5 = real_raw_track
            ? (real_is_jp ? THERON_TRACK02_MD5_JP_BIN
                          : THERON_TRACK02_MD5_US_BIN)
            : (real_is_jp ? THERON_TRACK02_MD5_JP_REV1_ISO
                          : THERON_TRACK02_MD5_US_ISO);
        const char *real_track02_marker = real_raw_track ? ".bin"
            : (real_is_jp ? "TQJP02" : "TQUS02");
        if (real_cue && real_cue[0] &&
            (theron_v1_track01_cdda_handoff_from_verified_media(
                 real_cue, real_md5, &handoff) !=
                 THERON_TRACK01_CDDA_AVAILABLE ||
             !handoff.playback_handoff_ready || !handoff.original_cdda ||
             (real_raw_track
                 ? (handoff.audio_is_vorbis ||
                    handoff.audio_sector_count == 0u ||
                    strstr(handoff.audio_path, ".bin") == NULL)
                 : (strstr(handoff.audio_path, ".wav")
                     ? (handoff.audio_is_vorbis ||
                        handoff.audio_start_byte == 0u ||
                        handoff.audio_sector_count == 0u ||
                        handoff.audio_start_byte > handoff.audio_file_bytes ||
                        handoff.audio_file_bytes - handoff.audio_start_byte !=
                            handoff.audio_sector_count *
                                THERON_TRACK01_CDDA_SECTOR_BYTES)
                     : (!handoff.audio_is_vorbis ||
                        strstr(handoff.audio_path, ".ogg") == NULL))) ||
             strstr(handoff.track02_path, real_track02_marker) == NULL)) {
            fprintf(stderr, "real Track 01 CDDA handoff rejected: %s\n",
                    handoff.unavailable_reason);
            fprintf(stderr, "status=%d ready=%d cdda=%d vorbis=%d audio=%s track02=%s\n",
                    handoff.status, handoff.playback_handoff_ready,
                    handoff.original_cdda, handoff.audio_is_vorbis,
                    handoff.audio_path, handoff.track02_path);
            failed = 1;
        } else if (real_cue && real_cue[0]) {
            Theron_Track01CddaStream real_stream = {0};
            if (!theron_v1_track01_cdda_lifecycle_update(
                    &handoff, 1, &real_stream) ||
                !real_stream.output_started || real_stream.sectors_queued == 0u) {
                fprintf(stderr, "real Track 01 CDDA stream did not start\n");
                failed = 1;
            }
            theron_v1_track01_cdda_stream_stop(&real_stream);
            if (!failed && handoff.audio_is_vorbis) {
                FILE *audio_file = fopen(handoff.audio_path, "rb");
                uint8_t *audio_bytes = NULL;
                Theron_Track01CddaStream memory_stream = {0};
                if (!audio_file || handoff.audio_file_bytes == 0u ||
                    fseek(audio_file, 0L, SEEK_END) != 0 ||
                    ftell(audio_file) < 0L ||
                    (size_t)ftell(audio_file) != handoff.audio_file_bytes ||
                    fseek(audio_file, 0L, SEEK_SET) != 0 ||
                    !(audio_bytes = (uint8_t *)malloc(handoff.audio_file_bytes)) ||
                    fread(audio_bytes, 1u, handoff.audio_file_bytes, audio_file) !=
                        handoff.audio_file_bytes) {
                    fprintf(stderr, "could not load authentic OGG into test memory\n");
                    failed = 1;
                } else if (!theron_v1_track01_cdda_stream_start_memory(
                               &handoff, audio_bytes,
                               handoff.audio_file_bytes, &memory_stream) ||
                           !theron_v1_track01_cdda_stream_pump(&memory_stream) ||
                           memory_stream.sectors_queued == 0u) {
                    fprintf(stderr, "authentic in-memory OGG stream did not start\n");
                    failed = 1;
                }
                theron_v1_track01_cdda_stream_stop(&memory_stream);
                free(audio_bytes);
                if (audio_file) fclose(audio_file);
            }
        }
    }
    remove(cue);
    remove(audio);
    remove(data);
    rmdir(directory);
    printf("test_theron_v1_track01_cdda_handoff: %s\n", failed ? "FAIL" : "PASS");
    return failed;
#endif
}
