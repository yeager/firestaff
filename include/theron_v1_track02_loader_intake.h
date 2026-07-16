#ifndef THERON_V1_TRACK02_LOADER_INTAKE_H
#define THERON_V1_TRACK02_LOADER_INTAKE_H

#include <stdint.h>

#include "theron_v1_dungeon_handoff.h"
#include "theron_v1_trace_provenance.h"

/* This is an observation boundary, not a payload decoder. The destination and
 * byte count are retained only when an original later loader read reports
 * them; neither value is interpreted as a memory or record layout. */
typedef struct {
    int authenticated_original_trace;
    int later_than_stage2_transfer;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t byte_count;
} Theron_V1Track02LoaderReadFacts;

typedef struct {
    int observed;
    int authenticated_v3_trace;
    int payload_intake_admitted;
    int initial_envelope_source_bound;
    int initial_envelope_decoded;
    uint32_t record;
    uint32_t record_user_data_offset;
    uint32_t observed_destination;
    uint32_t observed_byte_count;
    uint16_t decoded_header_width;
    uint16_t decoded_header_height;
    uint32_t decoded_header_seed;
    uint16_t decoded_header_identifier;
    uint16_t decoded_header_extension;
    uint32_t decoded_grid_bytes;
    uint32_t decoded_grid_hash;
    uint16_t decoded_grid_row_count;
    uint16_t decoded_grid_row_bytes;
    uint32_t decoded_grid_raw_sector;
    uint32_t decoded_grid_raw_sector_offset;
    uint32_t decoded_grid_first_row_hash;
    uint32_t decoded_grid_last_row_hash;
    const char *status;
} Theron_V1Track02LoaderIntakeReceipt;

/* A coordinate handoff is restricted to the verified raw grid byte and its
 * physical Track 02 placement. It intentionally does not name or interpret
 * the byte as a cell, object, tile, visual, or runtime value. */
typedef struct {
    int handed_off;
    int authenticated_v3_trace;
    uint16_t raw_grid_x;
    uint16_t raw_grid_y;
    uint8_t raw_grid_byte;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridCoordinateReceipt;

/* A row handoff retains one complete verified source row as opaque bytes.
 * Neither the row nor its contents acquire dungeon, cell, object, tile, or
 * visual semantics at this boundary. */
typedef struct {
    int handed_off;
    int authenticated_v3_trace;
    uint16_t raw_grid_y;
    uint16_t raw_grid_bytes;
    uint8_t raw_grid_row[THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH];
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    uint32_t raw_grid_row_hash;
    const char *status;
} Theron_V1Track02RawGridRowReceipt;

/* A complete source-verified initial grid for the downstream dungeon handoff.
 * The bytes are atomic and opaque: this boundary assigns no dungeon, cell,
 * object, tile, visual, or runtime semantics to them. */
typedef struct {
    int handed_off;
    int authenticated_v3_trace;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint8_t raw_grid[THERON_V1_INITIAL_ENVELOPE_HEADER_WIDTH *
                     THERON_V1_INITIAL_ENVELOPE_HEADER_HEIGHT];
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    uint32_t raw_grid_hash;
    const char *status;
} Theron_V1Track02RawGridReceipt;

/* A runtime consumer receives the exact source-verified startup grid.  The
 * bytes are read-only and remain deliberately unclassified: the loader trace
 * proves their transfer, not cell, object, tile, palette, or visual meaning.
 * A consumer must explicitly accept the receipt; rejection leaves the caller
 * without a route rather than enabling any generated substitute. */
typedef int (*Theron_V1Track02RawGridConsumer)(
    const Theron_V1Track02RawGridReceipt *grid,
    void *context);

typedef struct {
    int delivered;
    int authenticated_v3_trace;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridRuntimeReceipt;

/* The initial-grid handoff is not an object-table handoff. This receipt lets
 * callers record that they reached verified raw bytes while still refusing to
 * project those bytes into objects, triggers, monsters, or fallback visuals. */
typedef struct {
    int projection_blocked;
    int authenticated_v3_trace;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridObjectTableProjectionReceipt;

/* A caller may have observed another Track 02 read and be tempted to treat the
 * verified startup grid as an object table. This receipt keeps that path
 * fail-closed: it records the proposed read facts, rejects the startup
 * envelope record as an object-table source, and requires a separate original
 * later read before object bytes can acquire semantics. */
typedef struct {
    int object_table_read_blocked;
    int authenticated_v3_trace;
    int candidate_read_seen;
    int candidate_read_authenticated;
    int startup_record_rejected_as_object_table;
    int separate_object_table_record_required;
    int later_loader_read_required;
    int no_fallback;
    uint32_t candidate_record;
    uint32_t candidate_record_user_data_offset;
    uint32_t candidate_destination;
    uint32_t candidate_byte_count;
    uint32_t startup_grid_record;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02ObjectTableReadBlockReceipt;

/* The verified initial grid is not a bitmap route. This negative receipt is
 * separate from object-table blocking so callers cannot treat either route as
 * implicitly covered by the other. */
typedef struct {
    int bitmap_route_blocked;
    int authenticated_v3_trace;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridBitmapRouteReceipt;

/* Mirrors the object-table read-claim blocker for visuals. A proposed bitmap
 * read cannot use the verified startup grid or the startup-envelope record as
 * source material; a separate original later loader read is required before
 * any bitmap bytes, palette route, or fallback visual can be admitted. */
typedef struct {
    int bitmap_read_blocked;
    int authenticated_v3_trace;
    int candidate_read_seen;
    int candidate_read_authenticated;
    int startup_record_rejected_as_bitmap;
    int separate_bitmap_record_required;
    int later_loader_read_required;
    int palette_binding_required;
    int no_fallback_visual;
    uint32_t candidate_record;
    uint32_t candidate_record_user_data_offset;
    uint32_t candidate_destination;
    uint32_t candidate_byte_count;
    uint32_t startup_grid_record;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02BitmapReadBlockReceipt;

/* A narrow level route for the verified startup grid. This is the first
 * dungeon-facing positive handoff from Track 02, but it still admits only the
 * source-owned raw grid. Bitmap and object routes stay blocked until their
 * own original-data receipts exist. */
typedef struct {
    int level_route_admitted;
    int authenticated_v3_trace;
    int bitmap_route_blocked;
    int object_route_blocked;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridLevelRouteReceipt;

/* Dungeon-named handoff for consumers that should not infer bitmap/object
 * readiness from the older level-route receipt. This admits the same opaque
 * raw grid and explicitly keeps non-grid dungeon semantics closed. */
typedef struct {
    int dungeon_route_admitted;
    int authenticated_v3_trace;
    int bitmap_route_blocked;
    int object_route_blocked;
    int no_fallback;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridDungeonRouteReceipt;

/* The startup grid is not a proven dungeon-record or object-table read. This
 * negative receipt preserves the exact raw grid identity while naming the
 * missing evidence required before any later-record semantics can be bound. */
typedef struct {
    int dungeon_record_blocked;
    int object_table_record_blocked;
    int authenticated_v3_trace;
    int later_loader_read_required;
    int no_fallback;
    uint32_t expected_dungeon_record;
    uint32_t observed_raw_grid_record;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02RawGridDungeonRecordEvidenceReceipt;

/* Mirrors the object/bitmap read-claim blockers for dungeon records. A
 * proposed dungeon read cannot use the verified startup grid or startup
 * envelope record as its source; grammar binding stays closed until a
 * separate original later loader read proves real dungeon bytes. */
typedef struct {
    int dungeon_read_blocked;
    int authenticated_v3_trace;
    int candidate_read_seen;
    int candidate_read_authenticated;
    int startup_record_rejected_as_dungeon;
    int separate_dungeon_record_required;
    int later_loader_read_required;
    int grammar_binding_required;
    int no_fallback_dungeon;
    uint32_t candidate_record;
    uint32_t candidate_record_user_data_offset;
    uint32_t candidate_destination;
    uint32_t candidate_byte_count;
    uint32_t startup_grid_record;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02DungeonReadBlockReceipt;

/* Aggregates object-table and dungeon-record read evidence at the handoff
 * boundary. Even authenticated non-startup read facts do not create object or
 * dungeon semantics here; they only clear the startup-alias blocker while the
 * decoder/grammar bindings remain required and fallback visuals stay closed. */
typedef struct {
    int handoff_blocked;
    int authenticated_v3_trace;
    int object_read_seen;
    int dungeon_read_seen;
    int object_read_authenticated;
    int dungeon_read_authenticated;
    int object_startup_record_rejected;
    int dungeon_startup_record_rejected;
    int object_later_read_proven;
    int dungeon_later_read_proven;
    int object_decoder_binding_required;
    int dungeon_grammar_binding_required;
    int object_handoff_blocked;
    int dungeon_handoff_blocked;
    int no_fallback_visuals;
    int no_synthetic_handoff;
    uint32_t object_candidate_record;
    uint32_t object_candidate_record_user_data_offset;
    uint32_t object_candidate_destination;
    uint32_t object_candidate_byte_count;
    uint32_t dungeon_candidate_record;
    uint32_t dungeon_candidate_record_user_data_offset;
    uint32_t dungeon_candidate_destination;
    uint32_t dungeon_candidate_byte_count;
    uint32_t startup_grid_record;
    uint16_t raw_grid_width;
    uint16_t raw_grid_height;
    uint32_t raw_grid_bytes;
    uint32_t raw_grid_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *status;
} Theron_V1Track02ObjectDungeonHandoffGateReceipt;

/* This binds an observed read to the existing accepted-trace provenance
 * boundary.  The transfer facts remain opaque observations. */
typedef struct {
    const Theron_V1TraceProvenanceReceipt *trace_provenance;
    int later_than_stage2_transfer;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t byte_count;
} Theron_V1AuthenticatedTrack02LoaderReadFacts;

/* Binds one authenticated non-startup loader read to the real CD placement
 * described by a raw Track 02 BIN+CUE admission receipt. This proves only the
 * physical record/window covered by the read; object and dungeon semantics
 * remain closed until their own decoders/grammars consume the bytes. */
typedef struct {
    int cd_record_read_proven;
    int raw_cue_admission_consumed;
    int authenticated_later_loader_read;
    int startup_record_rejected;
    int object_semantics_blocked;
    int dungeon_semantics_blocked;
    int decoder_binding_required;
    int grammar_binding_required;
    int no_fallback_visuals;
    int no_synthetic_handoff;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t byte_count;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02LaterReadCdRecordReceipt;

/* Records whether later object/dungeon byte handoff is backed by canonical
 * raw Track 02 media. Missing local media produces a positive fail-closed
 * receipt instead of allowing synthetic bytes or fallback visuals. */
typedef struct {
    int gate_evaluated;
    int raw_media_bound;
    int raw_media_missing_blocked;
    int raw_cue_admission_consumed;
    int canonical_raw_bin_required;
    int canonical_raw_bin_present;
    int iso_image_blocked;
    int parser_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t cue_track02_index01_raw_sector;
    size_t raw_track02_bytes;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02LaterReadRawMediaGateReceipt;

/* Pairs the original later-loader read rows that are expected to feed the
 * object-table and dungeon-record paths. This is a table-of-reads receipt
 * only: destinations and byte windows remain opaque and non-runtime. */
typedef struct {
    int table_bound;
    int raw_media_gate_consumed;
    int object_cd_record_consumed;
    int dungeon_cd_record_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int distinct_records;
    int non_overlapping_raw_windows;
    int parser_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_handoff;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_destination;
    uint32_t object_byte_count;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_destination;
    uint32_t dungeon_byte_count;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonLoaderReadTableReceipt;

typedef enum {
    THERON_V1_TRACK02_LAYOUT_ROLE_NONE = 0,
    THERON_V1_TRACK02_LAYOUT_ROLE_OBJECT_TABLE = 1,
    THERON_V1_TRACK02_LAYOUT_ROLE_DUNGEON_RECORD = 2
} Theron_V1Track02LayoutRole;

/* Moves a proven later CD-record read one step closer to object/dungeon
 * layout code without decoding it. The receipt binds the byte window to a
 * requested layout role, while parser semantics, runtime handoff, rendering,
 * and fallback visuals remain closed. */
typedef struct {
    int layout_window_bound;
    int cd_record_read_consumed;
    int object_layout_bound;
    int dungeon_layout_bound;
    int parser_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int decoder_or_grammar_required;
    int no_synthetic_layout;
    Theron_V1Track02LayoutRole role;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t destination;
    uint32_t layout_bytes;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02LaterReadLayoutReceipt;

/* Pairs the object-table and dungeon-record layout windows without decoding
 * either. The pair proves only that the two role-bound windows come from the
 * same Track 02 media and do not overlap; parser/runtime/rendering remain
 * blocked. */
typedef struct {
    int layout_pair_bound;
    int object_layout_consumed;
    int dungeon_layout_consumed;
    int same_track02_media;
    int distinct_records;
    int non_overlapping_windows;
    int parser_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_layout;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_layout_bytes;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonLayoutPairReceipt;

/* Joins the read-table/layout binding to the non-overlapping layout pair.
 * This proves the paired layout windows are still the exact windows carried
 * forward from original loader reads; no format semantics are assigned. */
typedef struct {
    int bridge_bound;
    int read_layout_binding_consumed;
    int layout_pair_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int read_windows_preserved;
    int non_overlapping_windows;
    int parser_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_layout;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_destination;
    uint32_t object_layout_bytes;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_destination;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt;

/* Binds the raw-media loader-read table to the role-bound layout windows.
 * This preserves loader destinations and byte windows across the handoff but
 * still admits no object fields, dungeon grammar, runtime route, or visuals. */
typedef struct {
    int binding_bound;
    int read_table_consumed;
    int object_layout_consumed;
    int dungeon_layout_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int destinations_preserved;
    int layout_windows_match_reads;
    int parser_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_layout;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_destination;
    uint32_t object_layout_bytes;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_destination;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt;

/* Hands the exact role-bound layout bytes to a future decoder. This verifies
 * the original Track 02 media again and copies opaque source bytes only;
 * parser semantics, runtime handoff, rendering, and fallback visuals remain
 * closed. */
typedef struct {
    int bytes_handed_off;
    int layout_receipt_consumed;
    int object_layout_bytes;
    int dungeon_layout_bytes;
    int exact_source_bytes;
    int parser_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02LayoutRole role;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t track02_record;
    uint32_t record_user_data_offset;
    uint32_t layout_bytes;
    uint32_t layout_hash;
    uint32_t raw_track02_sector;
    uint32_t raw_sector_offset;
    uint32_t raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02LaterReadLayoutBytesReceipt;

/* Pairs the opaque object-table and dungeon-record byte handoff receipts
 * against the already-bound layout pair. This still admits no object or
 * dungeon parser semantics and never opens rendering or fallback visuals. */
typedef struct {
    int byte_pair_bound;
    int layout_pair_consumed;
    int object_bytes_consumed;
    int dungeon_bytes_consumed;
    int same_track02_media;
    int non_overlapping_windows;
    int exact_source_bytes;
    int parser_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt;

/* Binds copied opaque bytes back to the loader-read/layout-pair bridge. This
 * proves the byte hashes belong to the same source windows carried from the
 * original loader reads; the bytes still remain opaque. */
typedef struct {
    int bridge_bound;
    int read_layout_pair_bridge_consumed;
    int byte_pair_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int source_windows_preserved;
    int byte_hashes_recorded;
    int parser_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt;

/* Final pre-decoder source-byte gate for the paired object/dungeon windows.
 * It consumes the raw-media gate plus the paired opaque byte receipt, but
 * still does not admit parser semantics, runtime handoff, rendering, or
 * fallback visuals. */
typedef struct {
    int gate_evaluated;
    int raw_media_gate_consumed;
    int byte_pair_consumed;
    int source_bytes_ready;
    int raw_media_missing_blocked;
    int decoder_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonDecoderGateReceipt;

/* Binds the pre-decoder gate back to the loader-read-to-byte bridge. This is
 * the last non-semantic integrity check before topology evidence. */
typedef struct {
    int gate_bound;
    int read_to_bytes_bridge_consumed;
    int decoder_gate_consumed;
    int source_bytes_ready;
    int raw_media_missing_blocked;
    int same_track02_media;
    int source_windows_preserved;
    int byte_hashes_preserved;
    int decoder_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt;

/* Predecode evidence over the verified object/dungeon byte windows. This
 * records only source-window topology and hashes; it assigns no object-table
 * fields, dungeon-record grammar, runtime objects, or visuals. */
typedef struct {
    int evidence_recorded;
    int decoder_gate_consumed;
    int source_bytes_ready;
    int raw_media_missing_blocked;
    int same_track02_media;
    int non_overlapping_windows;
    int object_window_before_dungeon;
    int dungeon_window_before_object;
    int decoder_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    uint32_t gap_bytes;
    uint32_t total_span_bytes;
    uint32_t predecode_evidence_hash;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt;

/* Post-predecoder gate that ties topology evidence back to the full
 * loader-read-to-byte chain. It marks only evidence readiness, not decoder
 * or runtime admission. */
typedef struct {
    int readiness_bound;
    int read_to_decoder_gate_consumed;
    int predecode_evidence_consumed;
    int topology_ready;
    int raw_media_missing_blocked;
    int same_track02_media;
    int source_windows_preserved;
    int byte_hashes_preserved;
    int topology_hash_preserved;
    int decoder_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    uint32_t gap_bytes;
    uint32_t total_span_bytes;
    uint32_t predecode_evidence_hash;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt;

/* Binds post-predecode topology evidence to the existing source-locked
 * initial dungeon-level handoff. This proves same-media adjacency only; the
 * object/dungeon windows stay opaque and cannot enter runtime or rendering. */
typedef struct {
    int level_handoff_bound;
    int post_predecode_gate_consumed;
    int initial_level_handoff_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int initial_level_source_locked;
    int initial_level_boundary_opaque;
    int topology_evidence_preserved;
    int object_records_blocked;
    int dungeon_records_blocked;
    int decoder_semantics_blocked;
    int dungeon_grammar_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t initial_level_track02_record;
    uint32_t initial_level_user_data_offset;
    uint32_t initial_level_raw_track02_sector;
    uint32_t initial_level_raw_sector_offset;
    uint16_t initial_level_width;
    uint16_t initial_level_height;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    uint32_t gap_bytes;
    uint32_t total_span_bytes;
    uint32_t predecode_evidence_hash;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt;

/* Final pre-grammar barrier for object/dungeon evidence. It preserves the
 * original CD-read/layout/topology chain but refuses object-table or
 * dungeon-record grammar admission until an original parser witness exists. */
typedef struct {
    int grammar_gate_evaluated;
    int level_handoff_gate_consumed;
    int source_topology_ready;
    int raw_media_missing_blocked;
    int same_track02_media;
    int original_cd_read_evidence_preserved;
    int topology_evidence_preserved;
    int object_table_grammar_required;
    int dungeon_record_grammar_required;
    int object_table_grammar_admitted;
    int dungeon_record_grammar_admitted;
    int decoder_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    uint32_t gap_bytes;
    uint32_t total_span_bytes;
    uint32_t predecode_evidence_hash;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt;

/* Binds the grammar barrier back to the original loader-read layout binding.
 * This proves the grammar candidate still uses the exact CD-read records,
 * destinations, and byte windows; it still admits no grammar. */
typedef struct {
    int read_evidence_bound;
    int grammar_gate_consumed;
    int read_layout_binding_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int original_cd_read_destinations_preserved;
    int layout_windows_preserved;
    int topology_evidence_preserved;
    int object_table_grammar_required;
    int dungeon_record_grammar_required;
    int object_table_grammar_admitted;
    int dungeon_record_grammar_admitted;
    int decoder_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_destination;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_destination;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    uint32_t predecode_evidence_hash;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt;

/* External original-parser witness facts. These are trace facts only: they
 * name the exact loader-read windows that the original object/dungeon parser
 * consumed, but they do not describe object fields or dungeon records. */
typedef struct {
    int original_loader_trace;
    int original_parser_trace;
    int object_table_parser_entered;
    int dungeon_record_parser_entered;
    int no_fallback_visuals;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_destination;
    uint32_t object_byte_count;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_destination;
    uint32_t dungeon_byte_count;
    uint32_t dungeon_raw_track02_offset;
    const char *track02_md5;
} Theron_V1Track02ObjectDungeonParserGrammarWitnessFacts;

/* Admits only the fact that an original parser trace consumed these exact
 * object/dungeon windows. Field decoding and runtime use stay blocked. */
typedef struct {
    int parser_witness_bound;
    int grammar_read_evidence_consumed;
    int original_loader_trace_consumed;
    int original_parser_trace_consumed;
    int raw_media_missing_blocked;
    int same_track02_media;
    int object_table_parser_witnessed;
    int dungeon_record_parser_witnessed;
    int object_table_grammar_admitted;
    int dungeon_record_grammar_admitted;
    int object_table_fields_blocked;
    int dungeon_record_fields_blocked;
    int decoder_semantics_blocked;
    int runtime_handoff_blocked;
    int rendering_blocked;
    int fallback_visuals_blocked;
    int no_synthetic_bytes;
    Theron_V1Track02Variant raw_track02_variant;
    uint32_t object_track02_record;
    uint32_t object_record_user_data_offset;
    uint32_t object_destination;
    uint32_t object_layout_bytes;
    uint32_t object_layout_hash;
    uint32_t object_raw_track02_offset;
    uint32_t dungeon_track02_record;
    uint32_t dungeon_record_user_data_offset;
    uint32_t dungeon_destination;
    uint32_t dungeon_layout_bytes;
    uint32_t dungeon_layout_hash;
    uint32_t dungeon_raw_track02_offset;
    uint32_t predecode_evidence_hash;
    const char *track02_md5;
    const char *status;
} Theron_V1Track02ObjectDungeonParserGrammarWitnessReceipt;

/* Accepts only a provenance-authenticated later read of the source-locked
 * initial envelope. It deliberately leaves payload intake blocked until
 * independent evidence establishes what the observed transfer means. */
int theron_v1_track02_loader_intake_observe(
    const Theron_V1Track02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Admits an observation only when it came through the existing authenticated
 * trace path. It does not promote the receipt to payload intake. */
int theron_v1_track02_loader_intake_observe_authenticated_trace(
    const Theron_V1AuthenticatedTrack02LoaderReadFacts *facts,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Joins an authenticated later-read observation to the independently
 * source-verified initial-envelope receipt. The observed read must cover the
 * real envelope, but this remains a boundary binding rather than a decode. */
int theron_v1_track02_loader_intake_bind_initial_envelope(
    const Theron_V1Track02LoaderIntakeReceipt *observation,
    const Theron_V1DungeonHandoffReceipt *initial_envelope,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Decodes the source-bound initial envelope only from the complete, canonical
 * raw Track 02 image. The raw bytes are independently rehashed and must still
 * agree with the selected receipt and runtime-admitted loader observation.
 * It promotes the literal header, grid span, and its bounded raw-sector row
 * partition, but assigns no cell, object, visual, or post-grid-tail
 * semantics. */
int theron_v1_track02_loader_intake_decode_initial_envelope(
    const Theron_V1Track02LoaderIntakeReceipt *source_bound_receipt,
    const Theron_V1DungeonHandoffReceipt *initial_envelope,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02LoaderIntakeReceipt *out_receipt);

/* Hands off one coordinate only after independently rehashing the canonical
 * raw BIN and rechecking the decoded envelope header and complete grid hash.
 * Invalid provenance, altered bytes, or coordinates outside the verified grid
 * produce no receipt. */
int theron_v1_track02_loader_intake_handoff_raw_grid_coordinate(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint16_t raw_grid_x,
    uint16_t raw_grid_y,
    Theron_V1Track02RawGridCoordinateReceipt *out_receipt);

/* Hands off one complete source-verified grid row after independently
 * rehashing the canonical raw BIN and rechecking its literal envelope and
 * complete-grid receipt. The returned bytes remain opaque. */
int theron_v1_track02_loader_intake_handoff_raw_grid_row(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint16_t raw_grid_y,
    Theron_V1Track02RawGridRowReceipt *out_receipt);

/* Hands off the complete literal grid only after independently rehashing the
 * canonical raw BIN and requiring the exact source-locked 32x27 receipt.
 * This is the atomic raw-data boundary for a later dungeon consumer; no
 * fallback visual or inferred semantic route is enabled here. */
int theron_v1_track02_loader_intake_handoff_raw_grid(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02RawGridReceipt *out_receipt);

/* Delivers the complete, rehashed initial grid to a caller-owned runtime
 * consumer. This is the final original-media boundary before a future
 * evidence-backed dungeon decoder: it never creates a level, object list, or
 * visual fallback, and a rejecting consumer leaves no successful receipt. */
int theron_v1_track02_loader_intake_deliver_raw_grid_to_runtime(
    const Theron_V1Track02LoaderIntakeReceipt *decoded_receipt,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    Theron_V1Track02RawGridConsumer consumer,
    void *consumer_context,
    Theron_V1Track02RawGridRuntimeReceipt *out_receipt);

/* Explicitly blocks object-table projection from the verified startup grid.
 * A successful receipt is a negative handoff: it proves no object route or
 * substitute visual was admitted from these bytes. */
int theron_v1_track02_loader_intake_block_raw_grid_object_table_projection(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridObjectTableProjectionReceipt *out_receipt);

/* Blocks a proposed object-table read claim from consuming the startup grid.
 * This is negative evidence only: it never admits object bytes, even when a
 * candidate read is supplied. */
int theron_v1_track02_loader_intake_block_object_table_read_claim(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *candidate_read,
    Theron_V1Track02ObjectTableReadBlockReceipt *out_receipt);

/* Explicitly blocks bitmap/visual projection from the verified startup grid.
 * The receipt is negative evidence only and cannot create a generated visual
 * or asset substitute. */
int theron_v1_track02_loader_intake_block_raw_grid_bitmap_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridBitmapRouteReceipt *out_receipt);

/* Blocks a proposed bitmap read claim from consuming the startup grid. This
 * never admits bitmap bytes or a visual fallback. */
int theron_v1_track02_loader_intake_block_bitmap_read_claim(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *candidate_read,
    Theron_V1Track02BitmapReadBlockReceipt *out_receipt);

/* Admits only the source-verified raw grid as a level route. The paired
 * bitmap and object routes are explicitly unavailable with no fallback. */
int theron_v1_track02_loader_intake_admit_raw_grid_level_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridLevelRouteReceipt *out_receipt);

/* Admits only the source-verified raw grid as the dungeon-facing handoff.
 * Bitmap/object projection and visual fallback remain unavailable. */
int theron_v1_track02_loader_intake_admit_raw_grid_dungeon_route(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridDungeonRouteReceipt *out_receipt);

/* Records that no later dungeon-record or object-table read has been proven
 * from this startup grid. This is intentionally negative evidence. */
int theron_v1_track02_loader_intake_block_raw_grid_dungeon_record_evidence(
    const Theron_V1Track02RawGridReceipt *grid,
    Theron_V1Track02RawGridDungeonRecordEvidenceReceipt *out_receipt);

/* Blocks a proposed dungeon-record read claim from consuming the startup
 * grid. This is negative evidence only: it requires a separate later read and
 * grammar binding before dungeon semantics can open. */
int theron_v1_track02_loader_intake_block_dungeon_read_claim(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *candidate_read,
    Theron_V1Track02DungeonReadBlockReceipt *out_receipt);

/* Gates the combined object-table/dungeon handoff after startup-grid proof.
 * It records candidate later-read facts but never promotes them to object,
 * dungeon, bitmap, or fallback visual output at this boundary. */
int theron_v1_track02_loader_intake_gate_object_dungeon_handoff(
    const Theron_V1Track02RawGridReceipt *grid,
    const Theron_V1Track02LoaderReadFacts *object_read,
    const Theron_V1Track02LoaderReadFacts *dungeon_read,
    Theron_V1Track02ObjectDungeonHandoffGateReceipt *out_receipt);

/* Proves the physical CD record/window for an authenticated later non-startup
 * read over a real raw Track 02 BIN+CUE admission. The receipt carries no
 * object-table, dungeon-record, bitmap, palette, or fallback semantics. */
int theron_v1_track02_loader_intake_admit_later_cd_record_read(
    const Theron_V1Track02RawCueAdmissionReceipt *raw_cue,
    const Theron_V1Track02LoaderReadFacts *read,
    Theron_V1Track02LaterReadCdRecordReceipt *out_receipt);

/* Gates later object/dungeon byte handoff on canonical raw Track 02 media.
 * A missing raw-cue receipt yields an explicit blocked receipt; a present
 * receipt must already be the real BIN+CUE admission. */
int theron_v1_track02_loader_intake_gate_later_read_raw_media(
    const Theron_V1Track02RawCueAdmissionReceipt *raw_cue,
    Theron_V1Track02LaterReadRawMediaGateReceipt *out_receipt);

/* Binds the object/dungeon later-loader read rows against the admitted raw
 * media. Missing media is an explicit no-fallback blocker; present media
 * still leaves parser/runtime/rendering closed. */
int theron_v1_track02_loader_intake_bind_object_dungeon_loader_read_table(
    const Theron_V1Track02LaterReadRawMediaGateReceipt *raw_media_gate,
    const Theron_V1Track02LaterReadCdRecordReceipt *object_read,
    const Theron_V1Track02LaterReadCdRecordReceipt *dungeon_read,
    Theron_V1Track02ObjectDungeonLoaderReadTableReceipt *out_receipt);

/* Binds a proven later CD-record read to an object-table or dungeon-record
 * layout role, but does not decode, render, or publish runtime semantics. */
int theron_v1_track02_loader_intake_bind_later_read_layout(
    const Theron_V1Track02LaterReadCdRecordReceipt *cd_record,
    Theron_V1Track02LayoutRole role,
    Theron_V1Track02LaterReadLayoutReceipt *out_receipt);

/* Binds loader-read table evidence to role-bound layout windows while keeping
 * parser/runtime/rendering closed. Missing raw media stays fail-closed. */
int theron_v1_track02_loader_intake_bind_read_table_to_layouts(
    const Theron_V1Track02ObjectDungeonLoaderReadTableReceipt *read_table,
    const Theron_V1Track02LaterReadLayoutReceipt *object_layout,
    const Theron_V1Track02LaterReadLayoutReceipt *dungeon_layout,
    Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt *out_receipt);

/* Binds object-table and dungeon-record layout receipts as a non-overlapping
 * pair from the same Track 02 media. This is still not decode/render proof. */
int theron_v1_track02_loader_intake_bind_object_dungeon_layout_pair(
    const Theron_V1Track02LaterReadLayoutReceipt *object_layout,
    const Theron_V1Track02LaterReadLayoutReceipt *dungeon_layout,
    Theron_V1Track02ObjectDungeonLayoutPairReceipt *out_receipt);

/* Verifies the non-overlapping layout pair still matches the loader-read
 * table layout binding. Missing raw media remains a positive blocker. */
int theron_v1_track02_loader_intake_bridge_read_layout_binding_to_layout_pair(
    const Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt
        *read_layout_binding,
    const Theron_V1Track02ObjectDungeonLayoutPairReceipt *layout_pair,
    Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt *out_receipt);

/* Copies exact raw bytes for a bound object-table or dungeon-record layout
 * window from verified Track 02 media. The bytes stay opaque. */
int theron_v1_track02_loader_intake_handoff_later_layout_bytes(
    const Theron_V1Track02LaterReadLayoutReceipt *layout,
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    const char *raw_track02_md5,
    uint8_t *out_bytes,
    size_t out_capacity,
    Theron_V1Track02LaterReadLayoutBytesReceipt *out_receipt);

/* Binds the opaque object/dungeon byte receipts to their layout pair. This is
 * a source-byte pairing gate only, not decode or render admission. */
int theron_v1_track02_loader_intake_bind_object_dungeon_layout_bytes_pair(
    const Theron_V1Track02ObjectDungeonLayoutPairReceipt *layout_pair,
    const Theron_V1Track02LaterReadLayoutBytesReceipt *object_bytes,
    const Theron_V1Track02LaterReadLayoutBytesReceipt *dungeon_bytes,
    Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *out_receipt);

/* Verifies copied opaque bytes still match the loader-read/layout-pair
 * bridge. Missing raw media remains a positive blocker. */
int theron_v1_track02_loader_intake_bridge_read_layout_pair_to_bytes(
    const Theron_V1Track02ObjectDungeonReadLayoutPairBridgeReceipt
        *read_layout_pair_bridge,
    const Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *byte_pair,
    Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt *out_receipt);

/* Gates the paired object/dungeon bytes for a future decoder. Missing raw
 * media is a positive blocked receipt; present media must match the exact
 * paired byte windows and remains non-rendering/non-semantic. */
int theron_v1_track02_loader_intake_gate_object_dungeon_decoder_bytes(
    const Theron_V1Track02LaterReadRawMediaGateReceipt *raw_media_gate,
    const Theron_V1Track02ObjectDungeonLayoutBytesPairReceipt *byte_pair,
    Theron_V1Track02ObjectDungeonDecoderGateReceipt *out_receipt);

/* Verifies the pre-decoder gate still matches the full loader-read to copied
 * byte bridge. Missing raw media remains a positive blocker. */
int theron_v1_track02_loader_intake_bind_read_to_bytes_to_decoder_gate(
    const Theron_V1Track02ObjectDungeonReadToBytesBridgeReceipt
        *read_to_bytes_bridge,
    const Theron_V1Track02ObjectDungeonDecoderGateReceipt *decoder_gate,
    Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt *out_receipt);

/* Records source-byte topology for the gated object/dungeon windows. This is
 * the next handoff receipt toward a future decoder, not format admission. */
int theron_v1_track02_loader_intake_record_object_dungeon_predecode_evidence(
    const Theron_V1Track02ObjectDungeonDecoderGateReceipt *decoder_gate,
    Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt *out_receipt);

/* Consumes topology evidence only when it still matches the full
 * loader-read-to-decoder gate. This remains non-semantic and no-render. */
int theron_v1_track02_loader_intake_gate_object_dungeon_post_predecode(
    const Theron_V1Track02ObjectDungeonReadToDecoderGateReceipt
        *read_to_decoder_gate,
    const Theron_V1Track02ObjectDungeonPredecodeEvidenceReceipt
        *predecode_evidence,
    Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt *out_receipt);

/* Requires a source-locked startup dungeon-level handoff before preserving
 * object/dungeon topology evidence for a future decoder. Missing raw media
 * remains an explicit positive blocker. */
int theron_v1_track02_loader_intake_gate_object_dungeon_level_handoff(
    const Theron_V1Track02ObjectDungeonPostPredecodeGateReceipt
        *post_predecode_gate,
    const Theron_V1DungeonHandoffReceipt *initial_level,
    Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt *out_receipt);

/* Preserves real object/dungeon loader-read evidence at the grammar boundary
 * while failing closed until an original object-table/dungeon-record grammar
 * witness is available. */
int theron_v1_track02_loader_intake_gate_object_dungeon_grammar_admission(
    const Theron_V1Track02ObjectDungeonLevelHandoffGateReceipt
        *level_handoff_gate,
    Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt *out_receipt);

/* Binds the grammar-admission blocker to the original read-table/layout
 * binding. This is positive loader/CD-read evidence only, not grammar proof. */
int theron_v1_track02_loader_intake_bind_grammar_admission_to_loader_reads(
    const Theron_V1Track02ObjectDungeonGrammarAdmissionGateReceipt
        *grammar_gate,
    const Theron_V1Track02ObjectDungeonReadTableLayoutBindingReceipt
        *read_layout_binding,
    Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt *out_receipt);

/* Binds an original parser witness to the already-preserved loader-read
 * evidence. This admits grammar provenance only, not decoded fields. */
int theron_v1_track02_loader_intake_admit_object_dungeon_parser_witness(
    const Theron_V1Track02ObjectDungeonGrammarReadEvidenceGateReceipt
        *grammar_read_gate,
    const Theron_V1Track02ObjectDungeonParserGrammarWitnessFacts *witness,
    Theron_V1Track02ObjectDungeonParserGrammarWitnessReceipt *out_receipt);

#endif
