#ifndef FIRESTAFF_DM2_V1_FMTOWNS_ANIM_STREAM_H
#define FIRESTAFF_DM2_V1_FMTOWNS_ANIM_STREAM_H

#include <stddef.h>
#include <stdint.h>

/*
 * Read-only FM Towns TWANIM stream admission.
 *
 * HME-242's TITLE, SWOOSH and END are standalone big-endian record streams,
 * invoked by AUTOEXEC.BAT through the separate Phar Lap P3 TWANIM.EXP
 * player.  These declarations deliberately accept byte buffers only: the
 * selected CD image remains the sole media owner and nothing is unpacked to
 * the host filesystem.
 *
 * Record framing: DMWeb, "Animations" (http://dmweb.free.fr/community/
 * documentation/file-formats/animations/).  The exact startup inventories
 * below were independently checked against Greatstone's HME-242 extraction.
 */

typedef struct {
    int valid;
    uint32_t byte_count;
    uint32_t chunk_count;
    uint16_t width;
    uint16_t height;
    uint16_t bit_depth;
    uint16_t an_trailer;
    uint32_t an_count;
    uint32_t pl_count;
    uint32_t en_count;
    uint32_t dl_count;
    uint32_t sd_count;
    uint32_t br_count;
    uint32_t so_count;
    uint32_t do_count;
    uint32_t fo_count;
    uint32_t ne_count;
    uint32_t bn_count;
} DM2_V1_FmtownsAnimStreamReceipt;

typedef struct {
    int valid;
    uint16_t width;
    uint16_t height;
    uint16_t bit_depth;
    uint32_t requested_frame;
    uint32_t decoded_frame_count;
    uint16_t display_duration;
    uint32_t source_bytes_consumed;
    uint32_t compressed_command_count;
    uint32_t output_fnv1a;
} DM2_V1_FmtownsAnimFrameReceipt;

/* TWANIM's PL payload starts with a big-endian colour count, followed by
 * index/R/G/B four-bit entries.  Keep the palette as source nibbles: M11
 * expands them only at the final indexed-palette boundary. */
#define DM2_V1_FMTOWNS_ANIM_PALETTE_COLORS 16u
typedef struct {
    int valid;
    uint16_t color_count;
    uint32_t source_record_offset;
    uint8_t rgb4[DM2_V1_FMTOWNS_ANIM_PALETTE_COLORS][3];
    uint32_t output_fnv1a;
} DM2_V1_FmtownsAnimPaletteReceipt;

/* TITLE owns one SD/SND2 sound definition and invokes it five times through
 * SO records.  These receipts retain pointers into the caller-owned original
 * TITLE buffer; they neither allocate nor convert the sample to host audio.
 * DMWeb documents SD as a big-endian sample-count followed by signed 8-bit
 * mono PCM, while SKWIN's 0759:0E33/0EF0 reconstruction establishes the
 * HME-242 player rate as 5500 Hz despite TITLE's invalid 03E8 SO value. */
#define DM2_V1_FMTOWNS_TITLE_SOUND_EVENT_COUNT 5u
typedef struct {
    uint32_t source_record_offset;
    uint32_t preceding_frame_count;
    uint16_t sound_index;
    uint8_t left_volume;
    uint8_t right_volume;
    uint16_t source_frequency_hz;
    uint16_t player_frequency_hz;
} DM2_V1_FmtownsAnimSoundEventReceipt;

typedef struct {
    int valid;
    uint32_t source_record_offset;
    uint16_t sample_count;
    const int8_t *samples; /* signed 8-bit mono PCM; owned by TITLE buffer */
    uint32_t sample_fnv1a;
    uint32_t event_count;
    DM2_V1_FmtownsAnimSoundEventReceipt
        events[DM2_V1_FMTOWNS_TITLE_SOUND_EVENT_COUNT];
} DM2_V1_FmtownsAnimSoundReceipt;

/* Parses every complete record, rejects unknown tags and rejects trailing or
 * truncated bytes.  It does not render pixels; decoding stays unavailable
 * until a source-owned TWANIM execution handoff consumes this receipt. */
int dm2_v1_fmtowns_anim_stream_parse(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimStreamReceipt *out);

/* Strict retail startup-stream identities.  These validate structure after
 * the caller has already established the media's canonical MD5 identity. */
int dm2_v1_fmtowns_anim_stream_is_hme242_swoosh(
    const DM2_V1_FmtownsAnimStreamReceipt *receipt);
int dm2_v1_fmtowns_anim_stream_is_hme242_title(
    const DM2_V1_FmtownsAnimStreamReceipt *receipt);
int dm2_v1_fmtowns_anim_stream_is_hme242_end(
    const DM2_V1_FmtownsAnimStreamReceipt *receipt);

/* Replays EN/DL image records through SKWIN's `ANIM_DECODE_IMG1` semantics
 * up to `requested_frame` (zero based), into a packed 4bpp canvas.  The
 * caller supplies the original stream bytes and a 32000-byte canvas; no
 * filesystem path or host-made art is accepted. */
int dm2_v1_fmtowns_anim_stream_decode_frame(
    const uint8_t *data, size_t data_size, uint32_t requested_frame,
    uint8_t *out_pixels, size_t out_pixel_capacity,
    DM2_V1_FmtownsAnimFrameReceipt *out);

/* Reads the last complete PL update in an admitted stream.  This is the
 * 0759:1013 palette transaction in SKWIN's TWANIM reconstruction, not an
 * inferred VGA palette. */
int dm2_v1_fmtowns_anim_stream_decode_palette(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimPaletteReceipt *out);

/* Replays PL and FO/NE control records only as far as the requested displayed
 * frame. This is the palette active when SKWIN 0759:0F64 presents that EN/DL
 * image; it does not substitute the stream's final palette for earlier END
 * frames. */
int dm2_v1_fmtowns_anim_stream_decode_palette_for_frame(
    const uint8_t *data, size_t data_size, uint32_t requested_frame,
    DM2_V1_FmtownsAnimPaletteReceipt *out);

/* Decodes the HME-242 TITLE SD/SO plan, including each original event's
 * source offset and preceding image-frame count.  The caller may schedule
 * playback later only from this receipt; no guessed GDAT sample is allowed.
 */
int dm2_v1_fmtowns_anim_stream_decode_title_sound(
    const uint8_t *data, size_t data_size,
    DM2_V1_FmtownsAnimSoundReceipt *out);

#endif /* FIRESTAFF_DM2_V1_FMTOWNS_ANIM_STREAM_H */
