
#ifndef NEXUS_V1_DUNGEON_H
#define NEXUS_V1_DUNGEON_H

#include <stddef.h>
#include <stdint.h>
#include "nexus_v1_dgn_face_material_provenance.h"

/* DM Nexus dungeon level format (.DGN files).
 * Source-lock: DMWeb "Dungeon Master Nexus DGN files", fetched 2026-05-28.
 * Real LEV*.DGN files are 2048-byte block containers. Structure1 contains
 * Structure1B: a 64x64 grid with 8 bytes per cell (0x8000 bytes total).
 * The old Firestaff "raw 32x32 at offset 0" layout is retained only as a
 * synthetic-fixture fallback while older probes are retired. */

#define NEXUS_MAX_MAP_SIZE 64
#define NEXUS_DGN_BLOCK_SIZE 2048
#define NEXUS_DGN_STRUCTURE1B_BYTES 0x8000
#define NEXUS_DGN_STRUCTURE1B_CELL_BYTES 8
#define NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES 4
#define NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT 6
#define NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES 128
#define NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES 16
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE 3
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK 0x3f
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK 0x80
#define NEXUS_DGN_MAX_COLLISION_SECTORS 256
#define NEXUS_DGN_MAX_POST_GRID_0X30_RECORDS 4096
#define NEXUS_DGN_STRUCTURE1F_HEADER_BYTES 16
#define NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT 6
#define NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES 4096
#define NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES 24
#define NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES 4096
#define NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES 8
#define NEXUS_DGN_STRUCTURE1G_HEADER_BYTES 4
#define NEXUS_DGN_MAX_STRUCTURE1G_ENTRIES 256
#define NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX 0x014c
#define NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES 20
#define NEXUS_DGN_MAX_STRUCTURE2_TEXTURES 256
#define NEXUS_DGN_MAX_STRUCTURE3_ENTRIES 4096
#define NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES 40
#define NEXUS_DGN_STRUCTURE3_GEOMETRY_MEASUREMENT_MAX_COMPONENT_ABS 0x100000
/* Corpus identity serialized by test_nexus_v1_dgn_face_mesh_corpus from the
 * documented typed Structure3 rows in hash-verified LEV00--LEV15. */
#define NEXUS_DGN_RETAIL_TYPED_MESH_CORPUS_FNV1A32 0xd3f42b1fU
#define NEXUS_V1_DGN_VIEW_DISTANCE 4
#define NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS 48
#define NEXUS_V1_DGN_RUNTIME_DIRECT_SOURCE_MAX \
    NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS
#define NEXUS_V1_DGN_VIEWPORT_UNITS 1024

/* These are real Structure1 header offsets observed across LEV00..LEV15.
 * Only the first span has a source-backed name: Structure1B collision
 * indexes refer to Structure1C. The remaining spans stay opaque. */
typedef struct {
    int header_offset;
    int relative_offset;
    int size_to_next;
    int present;
    int bounded;
} Nexus_V1_DgnStructure1PostGridPointer;

/* The first bounded post-grid span is the Structure1C collision record
 * table. Every real LEV00..LEV15 form is a four-byte record array whose
 * first byte equals its record count; record zero is not addressable by a
 * Structure1B collision index. Field meanings inside each record remain
 * deliberately undecoded here. DMWeb DGN files: Structure1B bytes 5..7. */
typedef struct {
    int relative_offset;
    int size;
    int record_size;
    int record_count;
    int indexed_record_count;
    int valid;
} Nexus_V1_DgnStructure1CRecordTable;

/* Real-media evidence, LEV00..LEV15: the bounded span selected by header
 * offset 0x24 is exactly 128 zero bytes.  This establishes a reserved span,
 * not a payload interpretation. */
typedef struct {
    int relative_offset;
    int size;
    int valid;
} Nexus_V1_DgnPostGrid0x24ZeroSpan;

/* Real-media evidence, LEV00..LEV15: header offset 0x30 reaches a span
 * bounded by the following 0x34 pointer. It contains a typed prefix of
 * 16-byte rows followed by one opaque tail row. In every prefix row, byte 3
 * stores its six-bit row ordinal and may carry only bit 7 as an observed
 * flag. No other record byte has a gameplay or rendering interpretation. */
typedef struct {
    int relative_offset;
    int size;
    int record_size;
    int record_count;
    int typed_prefix_record_count;
    int opaque_tail_record_count;
    int row_ordinal_prefix_valid;
    /* Byte 3's high bit is observed per-row provenance only. It is never
     * selected through a Structure1B packed reference value. */
    int row_ordinal_flagged_prefix_record_count;
    int first_row_ordinal_flagged_prefix_record;
    int last_row_ordinal_flagged_prefix_record;
    /* Measurements only: byte positions remain deliberately unnamed. */
    int field_distinct_value_count[NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES];
    int referenced_field_distinct_value_count[
        NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES];
    int valid;
} Nexus_V1_DgnPostGrid0x30RecordTable;

/* DMWeb DGN files, Structure1F: a 16-byte header followed by six counted
 * families in this fixed order. These record forms are retained as dungeon
 * semantics; rendering and mechanics do not infer behaviour from fields that
 * the Saturn format reference still marks unknown. */
typedef enum {
    NEXUS_V1_DGN_STRUCTURE1F_ITEMS = 0,
    NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS = 1,
    NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS = 2,
    NEXUS_V1_DGN_STRUCTURE1F_ALCOVES = 3,
    NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS = 4,
    NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS = 5
} Nexus_V1_DgnStructure1FFamily;

typedef struct {
    int relative_offset;
    int size;
    /* The final post-grid span has a complete Structure1F six-family header
     * and matching source tags. It can still fail direct 64x64 validation. */
    int declared;
    uint16_t wall_sensor_first_texture_index;
    uint16_t wall_sensor_first_model_index;
    int family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int family_offset[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int family_record_size[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int total_entry_count;
    int valid;
} Nexus_V1_DgnStructure1FTable;

/* DMWeb DGN Structure1A: header-counted 24-byte model records. */
typedef struct {
    int relative_offset;
    int entry_count;
    int size;
    int valid;
} Nexus_V1_DgnStructure1ATable;

typedef struct {
    uint8_t kind;
    uint8_t structure3_model_index;
    uint8_t z_rotation;
} Nexus_V1_DgnStructure1AModel;

typedef struct {
    Nexus_V1_DgnStructure1FFamily family;
    uint8_t tag;
    uint8_t x;
    uint8_t y;
    /* Items use location/item_id/attribute1/attribute2. Decorations and
     * sensors retain their documented positional/model fields below. */
    uint8_t location;
    uint8_t item_id;
    uint8_t attribute1;
    uint8_t attribute2;
    int8_t offset_x;
    int8_t offset_y;
    uint8_t model_or_aspect;
    uint8_t rotation;
    uint8_t type_or_control;
    uint8_t width;
    uint8_t height;
    uint16_t structure1a_index;
    int structure1a_relation_valid;
    int structure1a_owner_x;
    int structure1a_owner_y;
    uint8_t structure1a_structure3_model_index;
    uint8_t structure1a_z_rotation;
    uint8_t face;
    uint8_t destination_x;
    uint8_t destination_y;
    uint8_t destination_orientation;
    uint32_t raw_record_offset;
    uint32_t raw_record_length;
    uint64_t raw_record_fnv1a64;
} Nexus_V1_DgnStructure1FEntry;

/* Only items, floor decorations, and floor sensors expose documented direct
 * 64x64 cell coordinates. Alcove/wall families bind through Structure1A and
 * remain unresolved until that table has a source-backed parser. */
typedef struct {
    int valid;
    int typed_entry_count;
    int direct_coordinate_entry_count;
    int item_entry_count;
    int floor_decoration_entry_count;
    int floor_sensor_entry_count;
    int direct_coordinate_unique_cell_count;
    int direct_coordinate_duplicate_cell_count;
    int structure1a_bound_entry_count;
} Nexus_V1_DgnStructure1FSpatialReceipt;

/* Alcove and wall Structure1F records carry a big-endian Structure1A index
 * instead of a documented 64x64 cell.  This receipt retains that original
 * index stream at the host boundary without claiming that an index identifies
 * a cell, a model, a trigger, or a Saturn render surface. */
typedef struct {
    int valid;
    int entry_count;
    int alcove_entry_count;
    int wall_decoration_entry_count;
    int wall_sensor_entry_count;
    int zero_index_count;
    int nonzero_index_count;
    int unique_index_count;
    int duplicate_index_count;
    uint16_t highest_index;
} Nexus_V1_DgnStructure1ABoundaryReceipt;

typedef struct {
    int table_valid;
    int table_entry_count;
    int structure1f_bound_entry_count;
    int resolved_entry_count;
    int missing_owner_entry_count;
    int ambiguous_owner_entry_count;
    int out_of_range_index_count;
    int complete;
} Nexus_V1_DgnStructure1ARelationReceipt;

/* Structure1A byte 0 is retained only after the indexed Structure1F record
 * has a complete owner relation. Its original grammar is unknown, so this
 * receipt records raw byte reuse and cannot authorize an object, face, mesh,
 * texture, palette, pixel, or draw command. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_kind_count;
    int unique_kind_count;
    int duplicate_kind_count;
    int zero_kind_count;
    int nonzero_kind_count;
    uint8_t highest_kind;
    int complete;
    int kind_semantics_proven;
} Nexus_V1_DgnStructure1AKindReceipt;

/* Structure1A byte 1 is the documented Structure3 model index. This receipt
 * consumes that byte only after the Structure1F owner relation is complete.
 * It measures original model-reference reuse, but does not parse Structure3
 * bytes, select a face, or authorize a mesh, texture, palette, or pixel. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_model_reference_count;
    int unique_model_index_count;
    int duplicate_model_index_count;
    int zero_model_index_count;
    int nonzero_model_index_count;
    int complete;
} Nexus_V1_DgnStructure3ModelReferenceReceipt;

/* Structure1A byte 2 reaches a Structure1F record only through the complete
 * owner relation. Its original transform grammar is not decoded: this records
 * raw selector reuse only, never a rotation, face, mesh, or draw command. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_selector_count;
    int unique_selector_count;
    int duplicate_selector_count;
    int zero_selector_count;
    int nonzero_selector_count;
    uint8_t highest_selector;
    int complete;
    int transform_semantics_proven;
} Nexus_V1_DgnStructure1ATransformSelectorReceipt;

/* Exact raw Structure1A table identity for a future Saturn transform trace.
 * The 24-byte rows and byte-2 selector column are source provenance only;
 * neither receives a rotation unit, matrix, coordinate, or draw meaning. */
typedef struct {
    int valid;
    int table_byte_offset;
    int entry_count;
    int table_byte_count;
    uint64_t raw_table_fnv1a64;
    uint64_t selector_column_fnv1a64;
    int parsed_model_rows_match;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt selectors;
    int source_table_bound;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1ATransformTableReceipt;

/* Structure1F's Structure1A-bound families retain a raw face-selector byte.
 * This receipt measures that source field only after the owner relation is
 * complete. A selector is not a direction, a Structure3 face, or a draw
 * instruction until original Saturn semantics prove that relationship. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_face_selector_count;
    int unique_face_selector_count;
    int duplicate_face_selector_count;
    int zero_face_selector_count;
    int nonzero_face_selector_count;
    uint8_t highest_face_selector;
    int complete;
    int face_semantics_proven;
} Nexus_V1_DgnStructure1FFaceSelectorReceipt;

/* A Structure1F face-selector byte can be retained beside the resolved
 * Structure1A Structure3-model selector only after the complete owner
 * relation succeeds. This is a raw source-pair receipt: it establishes no
 * Structure3 record boundary, face ordinal, normal, vertex, transform,
 * texture, palette, pixel, or draw instruction. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int attachment_semantics_proven;
} Nexus_V1_DgnStructure3ModelFaceSelectorReceipt;

/* Structure1F's Structure1A-bound families also carry a separate raw
 * rotation-selector byte. It is retained only after the owner relation is
 * complete. Its unit and relation to any face or Structure3 transform remain
 * unproved, so it cannot authorize geometry or drawing. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_rotation_selector_count;
    int unique_rotation_selector_count;
    int duplicate_rotation_selector_count;
    int zero_rotation_selector_count;
    int nonzero_rotation_selector_count;
    uint8_t highest_rotation_selector;
    int complete;
    int rotation_semantics_proven;
} Nexus_V1_DgnStructure1FRotationSelectorReceipt;

/* Face and rotation selectors are read from the same original Structure1F
 * record. This receipt retains only their raw byte-pair reuse after complete
 * Structure1A ownership; it proves no direction, rotation unit, face index,
 * transform, mesh, texture, palette, or pixel meaning. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FFaceRotationPairReceipt;

/* Structure1A-owned Structure1F records retain two signed offset bytes. This
 * receipt preserves only exact raw pair reuse; it does not establish an axis,
 * scale, origin, position, mesh transform, texture coordinate, or draw. */
typedef struct {
    int structure1a_relation_complete;
    int structure1f_bound_entry_count;
    int resolved_offset_pair_count;
    int unique_offset_pair_count;
    int duplicate_offset_pair_count;
    int zero_offset_pair_count;
    int nonzero_offset_pair_count;
    int minimum_offset_x;
    int maximum_offset_x;
    int minimum_offset_y;
    int maximum_offset_y;
    int complete;
    int offset_semantics_proven;
} Nexus_V1_DgnStructure1FOffsetPairReceipt;

/* Wall-decoration and wall-sensor records carry a raw byte at their original
 * payload-selector slot. This receipt records byte reuse only; it is not an
 * object, aspect, model, material, trigger, texture, palette, or pixel map. */
typedef struct {
    int structure1a_relation_complete;
    int wall_payload_entry_count;
    int resolved_payload_selector_count;
    int wall_decoration_selector_count;
    int wall_sensor_selector_count;
    int unique_payload_selector_count;
    int duplicate_payload_selector_count;
    int zero_payload_selector_count;
    int nonzero_payload_selector_count;
    uint8_t highest_payload_selector;
    int complete;
    int payload_semantics_proven;
} Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt;

/* Wall-sensor records carry three raw destination bytes. This receipt keeps
 * exact tuple reuse after Structure1A ownership only. The bytes are not a
 * level, cell, orientation, trigger, teleport, route, or runtime command
 * until original Saturn semantics establish that interpretation. */
typedef struct {
    int structure1a_relation_complete;
    int wall_sensor_entry_count;
    int resolved_destination_count;
    int unique_destination_count;
    int duplicate_destination_count;
    int zero_destination_count;
    int nonzero_destination_count;
    uint8_t highest_destination_x;
    uint8_t highest_destination_y;
    uint8_t highest_destination_orientation;
    int complete;
    int destination_semantics_proven;
} Nexus_V1_DgnStructure1FWallSensorDestinationReceipt;

/* Wall-sensor records retain a separate raw control-selector byte. This is
 * source provenance only: it is not a trigger type, operation, object, script,
 * route, or runtime behavior without original Saturn semantic evidence. */
typedef struct {
    int structure1a_relation_complete;
    int wall_sensor_entry_count;
    int resolved_control_selector_count;
    int unique_control_selector_count;
    int duplicate_control_selector_count;
    int zero_control_selector_count;
    int nonzero_control_selector_count;
    uint8_t highest_control_selector;
    int complete;
    int control_semantics_proven;
} Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt;

/* Structure1A-bound wall sensors retain their adjacent control and
 * destination bytes as one raw four-byte tuple. The tuple is not a trigger,
 * destination, route, operation, mesh, material, texture, palette, pixel, or
 * draw instruction. */
typedef struct {
    int structure1a_relation_complete;
    int wall_sensor_entry_count;
    int resolved_tuple_count;
    int unique_tuple_count;
    int duplicate_tuple_count;
    int zero_tuple_count;
    int nonzero_tuple_count;
    uint32_t highest_tuple;
    int complete;
    int tuple_semantics_proven;
} Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt;

/* Structure1A-bound wall sensors retain their raw model/aspect and rotation
 * bytes as one pair after ownership is complete. This is not a model binding,
 * orientation, trigger, mesh, material, texture, palette, pixel, or draw
 * instruction. */
typedef struct {
    int structure1a_relation_complete;
    int wall_sensor_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt;

/* Structure1A-bound wall decorations retain their raw model/aspect and
 * rotation bytes as one pair after ownership validation. This is not a model
 * binding, orientation, mesh, material, texture, palette, pixel, or draw
 * instruction. */
typedef struct {
    int structure1a_relation_complete;
    int wall_decoration_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt;

/* Alcove records retain one raw payload-selector byte. It is measured only as
 * original byte reuse after Structure1A ownership, not as an item, inventory
 * slot, object, aspect, model, texture, palette, or pixel mapping. */
typedef struct {
    int structure1a_relation_complete;
    int alcove_entry_count;
    int resolved_payload_selector_count;
    int unique_payload_selector_count;
    int duplicate_payload_selector_count;
    int zero_payload_selector_count;
    int nonzero_payload_selector_count;
    uint8_t highest_payload_selector;
    int complete;
    int payload_semantics_proven;
} Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt;

/* Structure1A-bound alcoves retain their raw payload-selector and rotation
 * bytes as one pair after ownership validation. This is not an item binding,
 * orientation, mesh, material, texture, palette, pixel, or draw instruction. */
typedef struct {
    int structure1a_relation_complete;
    int alcove_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt;

/* Direct-coordinate floor-sensor records retain a raw control-selector byte.
 * This receipt records source-byte reuse only; it does not assign a trigger,
 * operation, script, destination, route, or runtime behavior. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_sensor_entry_count;
    int resolved_control_selector_count;
    int unique_control_selector_count;
    int duplicate_control_selector_count;
    int zero_control_selector_count;
    int nonzero_control_selector_count;
    uint8_t highest_control_selector;
    int complete;
    int control_semantics_proven;
} Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt;

/* Direct floor-sensor records retain raw three-byte destination tuples. The
 * tuple remains provenance only, never a cell, orientation, trigger, route,
 * teleport, or runtime command without original Saturn semantic evidence. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_sensor_entry_count;
    int resolved_destination_count;
    int unique_destination_count;
    int duplicate_destination_count;
    int zero_destination_count;
    int nonzero_destination_count;
    uint8_t highest_destination_x;
    uint8_t highest_destination_y;
    uint8_t highest_destination_orientation;
    int complete;
    int destination_semantics_proven;
} Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt;

/* Direct floor-sensor records retain their adjacent raw model/aspect and
 * rotation bytes as one pair. The pair is not a model binding, orientation,
 * trigger, mesh, material, texture, palette, pixel, or draw instruction. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_sensor_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt;

/* Direct floor-sensor records retain their two trailing extent bytes as one
 * raw tuple. The original grammar remains unproved, so it cannot describe a
 * footprint, sensor range, mesh, material, texture, palette, pixel, or draw
 * instruction. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_sensor_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt;

/* Direct floor-decoration records retain a raw payload-selector byte. The
 * receipt preserves byte reuse only; it is not an object, aspect, model,
 * material, texture, palette, pixel, or draw instruction. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_decoration_entry_count;
    int resolved_payload_selector_count;
    int unique_payload_selector_count;
    int duplicate_payload_selector_count;
    int zero_payload_selector_count;
    int nonzero_payload_selector_count;
    uint8_t highest_payload_selector;
    int complete;
    int payload_semantics_proven;
} Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt;

/* Direct floor decorations retain a separate raw rotation-selector byte. Its
 * transform grammar remains unproved: this is never a direction, angle, mesh,
 * texture, palette, pixel, or draw instruction. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_decoration_entry_count;
    int resolved_rotation_selector_count;
    int unique_rotation_selector_count;
    int duplicate_rotation_selector_count;
    int zero_rotation_selector_count;
    int nonzero_rotation_selector_count;
    uint8_t highest_rotation_selector;
    int complete;
    int rotation_semantics_proven;
} Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt;

/* Direct floor-decoration records retain the adjacent raw model/aspect and
 * rotation bytes as one pair. Their original grammar is unproved: this is not
 * a model binding, orientation, mesh, material, texture, palette, pixel, or
 * draw instruction. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_decoration_entry_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int pair_semantics_proven;
} Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt;

/* Direct floor-decoration records preserve their final three documented bytes
 * as a raw tuple. Their original control and extent grammar is unproved, so
 * this is never a placement, size, mesh, texture, palette, pixel, or draw
 * instruction. */
typedef struct {
    int structure1f_spatial_valid;
    int floor_decoration_entry_count;
    int resolved_tuple_count;
    int unique_tuple_count;
    int duplicate_tuple_count;
    int zero_tuple_count;
    int nonzero_tuple_count;
    uint8_t highest_type_or_control;
    uint8_t highest_width;
    uint8_t highest_height;
    int complete;
    int tuple_semantics_proven;
} Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt;

typedef struct { int spatial_valid; int item_count; int resolved_pair_count; int unique_pair_count; int duplicate_pair_count; int complete; int semantics_proven; } Nexus_V1_DgnStructure1FItemAttributePairReceipt;
typedef struct { int spatial_valid; int item_count; int resolved_pair_count; int unique_pair_count; int duplicate_pair_count; int complete; int semantics_proven; } Nexus_V1_DgnStructure1FItemLocationPairReceipt;
/* Item records retain their two documented coordinate bytes as an opaque pair.
 * This preserves raw source distribution only, never an item placement,
 * object, mesh, material, texture, palette, pixel, or draw instruction. */
typedef struct {
    int spatial_valid;
    int item_count;
    int resolved_pair_count;
    int unique_pair_count;
    int duplicate_pair_count;
    int zero_pair_count;
    int nonzero_pair_count;
    uint16_t highest_pair;
    int complete;
    int semantics_proven;
} Nexus_V1_DgnStructure1FItemCoordinatePairReceipt;
typedef struct { int structure1f_spatial_valid; int entry_count; int resolved_pair_count; int unique_pair_count; int duplicate_pair_count; int zero_pair_count; int nonzero_pair_count; int complete; int offset_semantics_proven; } Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt;

/* DMWeb DGN files: the container header names Structure3 with a block offset
 * and block count. The enclosed bytes have no established Saturn payload,
 * vertex, face, texture, palette, or draw grammar, so this is an envelope
 * receipt only. */
typedef struct {
    int declared;
    int block_offset;
    int block_count;
    int byte_offset;
    int byte_size;
    int zero_byte_count;
    int nonzero_byte_count;
    int distinct_byte_value_count;
    int byte_transition_count;
    int first_nonzero_byte_offset;
    int last_nonzero_byte_offset;
    /* Zero-separated nonempty byte spans inside the documented envelope.
     * These are raw candidate search regions, never inferred record, face,
     * or vertex boundaries. */
    int nonzero_byte_run_count;
    int longest_nonzero_byte_run;
    int first_nonzero_byte_run_offset;
    int first_nonzero_byte_run_byte_count;
    int last_nonzero_byte_run_offset;
    int last_nonzero_byte_run_byte_count;
    uint32_t raw_payload_hash;
    /* Structure3 is declared in complete 0x800-byte DGN blocks. These are
     * raw block-boundary observations only, not record, face, or vertex
     * boundaries. */
    int complete_block_count;
    int zero_block_count;
    int nonzero_block_count;
    int first_nonzero_block_index;
    int last_nonzero_block_index;
    int nonzero_block_run_count;
    int longest_nonzero_block_run;
    /* Exact spans of the first and last nonempty block runs. These are raw
     * search regions only, never inferred record or face boundaries. */
    int first_nonzero_block_run_start_block_index;
    int first_nonzero_block_run_block_count;
    int last_nonzero_block_run_start_block_index;
    int last_nonzero_block_run_block_count;
    int valid;
    int face_semantics_proven;
} Nexus_V1_DgnStructure3PayloadReceipt;

/* The first Structure3 word is observed on the retail LEV00--LEV15 corpus
 * as a big-endian count followed by that many increasing big-endian offsets.
 * This proves a bounded directory envelope only. Its entries are not yet
 * called models, faces, vertices, textures, or mesh records. */
typedef struct {
    int payload_valid;
    int directory_declared;
    int entry_count;
    int directory_byte_count;
    int first_entry_offset;
    int last_entry_offset;
    int offsets_strictly_increasing;
    int valid;
    int entry_semantics_proven;
} Nexus_V1_DgnStructure3DirectoryReceipt;

/* DMWeb's Saturn DGN reference names the three count-bounded 12-byte
 * regions in a Structure3 entry as vertices, faces, and per-face normals.
 * This receipt validates only their enclosing boundaries and paired
 * face/normal counts; individual records, face flags, vertex indexes, and
 * all draw/material behaviour remain outside this parser's no-draw gate. */
typedef struct {
    int payload_valid;
    int directory_valid;
    int entry_count;
    int bounded_entry_count;
    int fixed_header_byte_count;
    int first_region_element_count;
    int second_region_element_count;
    int third_region_element_count;
    int complete_third_region_entry_count;
    int zero_tag_entry_count;
    int tag_0x100_entry_count;
    int other_tag_entry_count;
    int boundaries_valid;
    int third_region_boundaries_valid;
    int valid;
    int semantics_proven;
} Nexus_V1_DgnStructure3EntryHeaderReceipt;

/* DMWeb documents Structure3b as four big-endian vertex indexes followed by
 * flags and a fill selector. This receipt validates index bounds and local
 * face-to-vertex incidence, then accounts for each row's raw distinct-index
 * topology and documented fill lanes.
 * It does not bind a selector to any texture, palette, VDP1 command, or draw
 * operation. */
typedef struct {
    int entry_headers_valid;
    int entry_count;
    int vertex_count;
    int face_count;
    int normal_count;
    int triangle_count;
    int quad_count;
    int face_vertex_reference_count;
    int distinct_face_vertex_count;
    int repeated_face_vertex_reference_count;
    int one_distinct_vertex_face_count;
    int two_distinct_vertex_face_count;
    int three_distinct_vertex_face_count;
    int four_distinct_vertex_face_count;
    int linked_face_vertex_reference_count;
    int referenced_vertex_count;
    int unreferenced_vertex_count;
    /* Per-entry coverage is a bounded index observation: whether the face
     * rows mention every vertex in that entry's own table. It does not infer
     * connectivity, winding, visibility, or any drawing behaviour. */
    int fully_referenced_vertex_entry_count;
    int partially_referenced_vertex_entry_count;
    int zero_vertex_entry_count;
    /* Components are formed only by distinct vertex indexes that co-occur
     * within one bounded face row. They do not establish edge direction,
     * winding, surface continuity, or any mesh/draw behaviour. */
    int face_vertex_component_count;
    /* Each entry-local co-occurrence graph is accounted for by its component
     * count only. This neither assigns edges a direction nor infers a
     * surface, winding, visibility, or drawing behaviour. */
    int zero_component_vertex_entry_count;
    int single_component_vertex_entry_count;
    int multiple_component_vertex_entry_count;
    /* An unordered pair is retained only when two distinct indexes co-occur
     * in one bounded face row. This is raw row incidence, not an edge
     * direction, winding, surface, visibility, or draw relation. */
    int face_vertex_cooccurrence_pair_count;
    int face_vertex_adjacency_pair_count;
    int repeated_face_vertex_adjacency_pair_count;
    /* Pair multiplicity is counted only across bounded face-row occurrences
     * in the same entry. It does not establish an edge, winding, surface, or
     * any mesh/draw relation. */
    int single_face_vertex_adjacency_pair_count;
    int shared_face_vertex_adjacency_pair_count;
    int maximum_face_vertex_adjacency_pair_incidence;
    int maximum_vertex_reference_count;
    int textured_face_count;
    int mesh_transparent_face_count;
    int static_texture_fill_count;
    int animated_texture_fill_count;
    int one_off_color_fill_count;
    int unclassified_fill_count;
    int face_vertex_indexes_valid;
    int face_vertex_linkage_valid;
    int face_topology_accounting_valid;
    int face_vertex_entry_coverage_accounting_valid;
    int face_vertex_component_accounting_valid;
    int face_vertex_component_entry_accounting_valid;
    int face_vertex_adjacency_accounting_valid;
    int face_vertex_adjacency_multiplicity_accounting_valid;
    int normal_count_matches_face_count;
    int valid;
    int draw_semantics_proven;
} Nexus_V1_DgnStructure3FaceReceipt;

/* DMWeb's Saturn DGN reference assigns texture-flagged Structure3b fills
 * with a 00xx prefix to Structure2 texture IDs and 08xx fills to Structure1G
 * animated-texture IDs. This receipt validates those identifier joins only.
 * It neither decodes the selected payload nor authorizes palette, VDP1, mesh,
 * transform, clipping, or draw behaviour. */
typedef struct {
    int face_receipt_valid;
    int face_count;
    int textured_face_count;
    int non_textured_face_count;
    int static_texture_selector_count;
    int static_texture_bound_count;
    int static_texture_unbound_count;
    /* Per-level selector reuse is retained from bounded face rows. These are
     * identifier occurrences only: they do not establish texture contents,
     * dimensions, UVs, palette use, animation timing, or draw behaviour. */
    int static_texture_unique_selector_count;
    int static_texture_reused_selector_count;
    int animated_texture_selector_count;
    int animated_texture_bound_count;
    int animated_texture_unbound_count;
    int animated_texture_unique_selector_count;
    int animated_texture_reused_selector_count;
    int unsupported_textured_fill_count;
    int selector_bindings_complete;
    int selector_reuse_accounting_valid;
    int valid;
    int material_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3FaceMaterialReceipt;

/* Structure3b's documented, entry-local vertex indexes also permit a raw
 * edge-incidence measurement. Endpoints are canonicalized only to count
 * repeated index pairs within one entry; this does not establish adjacency,
 * manifoldness requirements, winding, collision, clipping, or draw order. */
typedef struct {
    int face_receipt_valid;
    int entry_count;
    int edge_count;
    int unique_edge_count;
    int single_use_edge_count;
    int shared_edge_count;
    int nonmanifold_edge_count;
    int degenerate_edge_count;
    int maximum_edge_use_count;
    int topology_measurement_complete;
    int valid;
    int topology_semantics_proven;
} Nexus_V1_DgnStructure3EdgeReceipt;

/* DMWeb documents Structure3a and Structure3c as signed 16.16 X/Y/Z
 * vectors. This validates their fixed-point framing, the documented
 * unit-normal invariant, and face-plane/normal coherence with an analytic
 * component-quantization bound. Winding signs are measured rather than
 * assigned a front-face meaning. This does not expose a mesh, choose
 * transforms, decode UVs, or authorize clipping or drawing. */
typedef struct {
    int face_receipt_valid;
    int vertex_count;
    int normal_count;
    int vertex_vector_count;
    int normal_vector_count;
    int nonzero_vertex_vector_count;
    int normal_unit_length_count;
    int normal_non_unit_length_count;
    uint64_t maximum_normal_length_error;
    int normal_face_plane_pair_count; /* one normal-to-edge check per pair */
    int normal_face_plane_within_tolerance_count;
    int normal_face_plane_outside_tolerance_count;
    int degenerate_face_triangle_count;
    int positive_winding_triangle_count;
    int negative_winding_triangle_count;
    int zero_winding_triangle_count;
    uint64_t maximum_normal_face_plane_error;
    int fixed_point_vectors_valid;
    int valid;
    int transform_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3VectorReceipt;

/* This is a raw coordinate measurement over the documented, entry-local
 * Structure3a/3b rows. A face is counted nondegenerate only when at least
 * one three-vertex combination has a nonzero cross product. This establishes
 * neither a surface plane nor a winding, normal, transform, visibility,
 * texture, palette, VDP1, or draw rule. */
typedef struct {
    int face_receipt_valid;
    int vector_receipt_valid;
    int face_count;
    int measurement_face_count;
    int nondegenerate_face_count;
    int degenerate_face_count;
    int maximum_component_absolute_value;
    int cross_product_measurement_safe;
    int accounting_valid;
    int valid;
    int surface_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3FaceGeometryReceipt;

/* This is an entry-local accounting of consecutive vertex-index pairs in
 * documented Structure3b rows. It preserves only raw face-row incidence and
 * traversal direction around each row. It does not establish winding,
 * manifoldness, surface continuity, culling, transform, texture, palette,
 * VDP1, or draw behaviour. */
typedef struct {
    int face_receipt_valid;
    int entry_count;
    int face_count;
    int face_edge_slot_count;
    int nondegenerate_face_edge_reference_count;
    int degenerate_face_edge_reference_count;
    int unique_face_edge_count;
    int boundary_face_edge_count;
    int paired_face_edge_count;
    int multi_incident_face_edge_count;
    int opposite_direction_paired_face_edge_count;
    int same_direction_paired_face_edge_count;
    int maximum_face_edge_incidence;
    int accounting_valid;
    int valid;
    int winding_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3FaceEdgeReceipt;

/* Structure3c is documented as one normal row for each Structure3b face
 * row. This retains only that entry-local ordinal correspondence and the
 * already bounded fixed-point unit check. It does not infer a normal plane,
 * winding, surface, visibility, transform, palette, texture, or draw rule. */
typedef struct {
    int face_receipt_valid;
    int vector_receipt_valid;
    int entry_count;
    int complete_entry_pair_count;
    int face_normal_pair_count;
    int unit_length_face_normal_pair_count;
    int non_unit_length_face_normal_pair_count;
    int pairing_valid;
    int valid;
    int normal_plane_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3FaceNormalPairReceipt;

/* This is an arithmetic cross-check over the already paired Structure3b/3c
 * rows. It measures whether each fixed-point normal is orthogonal to every
 * distinct edge from its face's first vertex, and records the sign of its dot
 * product with one non-collinear face cross product. This is neither a Saturn
 * normal-use rule nor a culling, transform, lighting, texture, palette, VDP1,
 * or draw rule. */
typedef struct {
    int face_receipt_valid;
    int vector_receipt_valid;
    int face_normal_pairing_valid;
    int face_count;
    int measured_face_count;
    int orthogonal_face_count;
    int nonorthogonal_face_count;
    int edge_test_count;
    int orthogonal_edge_test_count;
    int positive_cross_normal_dot_count;
    int negative_cross_normal_dot_count;
    int zero_cross_normal_dot_count;
    int arithmetic_envelope_safe;
    int accounting_valid;
    int valid;
    int normal_plane_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3FaceNormalGeometryReceipt;

/* This is the renderer-facing boundary for the bounded Structure3 mesh
 * evidence. It aggregates only topology, vector, and face/normal-row facts.
 * An original Saturn capture remains mandatory before a normal plane,
 * transform, texture/palette route, or draw command can be authorized. */
typedef struct {
    int source_topology_valid;
    int source_vectors_valid;
    int source_face_geometry_valid;
    int source_face_normal_pairing_valid;
    int source_face_normal_geometry_valid;
    int source_facts_complete;
    int entry_count;
    int vertex_count;
    int face_count;
    int normal_count;
    int face_normal_pair_count;
    int original_capture_required;
    int original_capture_available;
    int normal_plane_semantics_proven;
    int transform_semantics_proven;
    int texture_palette_semantics_proven;
    int draw_semantics_proven;
    int renderer_handoff_ready;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt;

/* Typed source rows from one documented Structure3 entry.  Coordinates and
 * normals retain their original signed 16.16 values; face indexes remain
 * entry-local.  These rows deliberately do not carry transforms, UVs,
 * palette data, VDP1 state, or draw permission. */
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Nexus_V1_DgnStructure3Vector;

typedef struct {
    uint16_t vertex_indexes[4];
    uint8_t flags;
    uint8_t raw_byte_9;
    uint16_t fill_selector;
    int triangle;
} Nexus_V1_DgnStructure3Face;

/* The caller owns all row buffers.  A zero-capacity call reports the exact
 * required counts; insufficient buffers fail without decoding a partial
 * entry.  `data` must match the bounded Structure3 source that loaded
 * `level`, which keeps an arbitrary or mutated payload from being promoted. */
typedef struct {
    int entry_index;
    int vertex_count;
    int face_count;
    int normal_count;
    int vertex_capacity_sufficient;
    int face_capacity_sufficient;
    int normal_capacity_sufficient;
    int source_identity_valid;
    int valid;
    int transform_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3MeshEntryReceipt;

/* DMWeb identifies Structure1A byte 1 as a Structure3 model index and
 * Structure1F byte 1 as the face number inside that model.  Since
 * Structure3c has one normal row per face row, this receipt binds only those
 * three bounded ordinals.  It does not assign placement, transform, normal
 * plane, texture, palette, culling, VDP1, or draw behavior. */
typedef struct {
    int structure1a_relation_complete;
    int structure3_directory_valid;
    int structure3_faces_valid;
    int structure3_face_normal_pairing_valid;
    int structure1f_bound_entry_count;
    int model_entry_bound_count;
    int face_normal_bound_count;
    int out_of_range_model_selector_count;
    int out_of_range_face_selector_count;
    int complete;
    int record_to_face_normal_semantics_proven;
    int normal_plane_transform_or_draw_semantics_proven;
} Nexus_V1_DgnStructure3AttachmentReceipt;

/* A future original-Saturn trace may describe one Structure3 face using this
 * opaque evidence packet. The byte spans are fingerprints only: this type
 * does not decode texture pixels, palette entries, VDP1 command fields,
 * transforms, or culling. `captured_*` buffers supplied to the binder must
 * exactly match these fingerprints, so an altered row, selector, texture
 * span, palette state, VDP state, transform state, or culling state cannot
 * be promoted independently. */
typedef struct {
    uint64_t dgn_fnv1a64;
    uint32_t structure3_payload_fnv1a32;
    uint32_t typed_mesh_corpus_fnv1a32;
    uint32_t entry_index;
    uint32_t face_ordinal;
    uint32_t face_row_fnv1a32;
    uint32_t referenced_vertex_rows_fnv1a32;
    uint32_t normal_row_fnv1a32;
    uint16_t fill_selector;
    uint64_t texture_span_fnv1a64;
    uint64_t palette_state_fnv1a64;
    uint64_t vdp1_state_fnv1a64;
    uint64_t transform_state_fnv1a64;
    uint64_t normal_culling_state_fnv1a64;
    uint64_t vdp1_command_fnv1a64;
    uint64_t first_sequence;
    uint64_t last_sequence;
} Nexus_V1_DgnStructure3FaceCaptureCandidate;

typedef struct {
    int candidate_framing_valid;
    /* Must come from the canonical asset scanner. A fingerprint carried by a
     * capture packet is correlation evidence, not source admission. */
    int dgn_source_hash_verified;
    /* The capture itself must be admitted by a caller-owned original-Saturn
     * source manifest. Matching byte fingerprints alone are never capture
     * provenance. */
    int capture_source_verified;
    int dgn_source_matches;
    int structure3_payload_matches;
    int typed_mesh_corpus_matches;
    int entry_face_matches;
    int face_row_matches;
    int referenced_vertex_rows_match;
    int normal_row_matches;
    int fill_selector_matches;
    int texture_span_matches;
    int palette_state_matches;
    int vdp1_state_matches;
    int transform_state_matches;
    int normal_culling_state_matches;
    int vdp1_command_matches;
    int vdp1_command_format_matches;
    int vdp1_texture_span_size_matches;
    int complete_source_binding;
    /* A packet can be byte-bound without proving that an emulator captured
     * it from original Saturn execution. That provenance must be supplied by
     * a later trace importer; this boundary never permits DGN drawing. */
    int original_saturn_capture_verified;
    int renderer_handoff_ready;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3FaceCaptureBindingReceipt;

/* Correlates only documented Structure1A model-index bytes with documented
 * Structure3 byte/block-run counts. Each zero- and one-based domain is tested
 * separately; neither result establishes any other mapping or decodes a
 * record, face, vertex, mesh, texture, or pixel. */
typedef struct {
    int structure1a_relation_complete;
    int structure3_payload_valid;
    int resolved_model_reference_count;
    int highest_model_index;
    int structure3_block_count;
    int structure3_nonzero_byte_run_count;
    int structure3_nonzero_block_run_count;
    int model_index_exceeds_block_count;
    int model_index_exceeds_nonzero_byte_run_count;
    int model_index_exceeds_nonzero_block_run_count;
    int zero_based_block_ordinal_mapping_disproven;
    int one_based_block_ordinal_mapping_disproven;
    int zero_based_byte_run_ordinal_mapping_disproven;
    int one_based_byte_run_ordinal_mapping_disproven;
    int zero_based_run_ordinal_mapping_disproven;
    int one_based_run_ordinal_mapping_disproven;
    int direct_block_ordinal_mapping_disproven;
    int direct_byte_run_ordinal_mapping_disproven;
    int direct_run_ordinal_mapping_disproven;
    int structure3_directory_valid;
    int structure3_directory_entry_count;
    int zero_based_directory_ordinal_mapping_disproven;
    int one_based_directory_ordinal_mapping_disproven;
    int direct_directory_ordinal_mapping_disproven;
    int face_semantics_proven;
    int valid;
} Nexus_V1_DgnStructure3OrdinalCorrelationReceipt;

/* The runtime host consumes Structure1F only through this receipt. Direct
 * records retain their documented source cells. Structure1A-indexed records
 * may proceed beyond this provenance gate only when the parser's complete
 * owner/model relation receipt proves each original relation; this still
 * grants no face, draw, trigger, mesh, or pixel semantics. */
typedef enum {
    NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_MISSING = 0,
    NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_ABSENT = 1,
    NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_DIRECT = 2,
    NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1F_LAYOUT = 3,
    NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_BLOCKED_STRUCTURE1A_RELATION = 4,
    NEXUS_V1_DGN_STRUCTURE1_HOST_PROVENANCE_READY_RESOLVED_STRUCTURE1A = 5
} Nexus_V1_DgnStructure1HostProvenanceStatus;

typedef struct {
    Nexus_V1_DgnStructure1HostProvenanceStatus status;
    int structure1f_declared;
    int structure1f_valid;
    int structure1f_typed_entry_count;
    Nexus_V1_DgnStructure1FSpatialReceipt structure1f_spatial;
    Nexus_V1_DgnStructure1ABoundaryReceipt structure1a_boundary;
    Nexus_V1_DgnStructure1ARelationReceipt structure1a_relation;
    int can_prepare_runtime_dgn;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1HostProvenanceReceipt;

/* DMWeb DGN files, Structure1G: optional animated-texture declarations.
 * A present table has a counted descriptor prefix and four-byte instruction
 * streams. Image instructions, backward FF FE gotos, and FF FF terminators
 * are validated here; timing flags and runtime stepping remain unexecuted. */
typedef struct {
    int relative_offset;
    int size;
    int descriptor_count;
    int animated_texture_count;
    int animation_data_relative_offset;
    int sequence_count;
    int image_instruction_count;
    int goto_instruction_count;
    int valid;
} Nexus_V1_DgnStructure1GTable;

typedef struct {
    uint8_t animation_id;
    uint16_t first_image_index;
    /* Structure1G uses the global image space: ITEM.IBS owns 0..0x14b,
     * while Structure2 starts at 0x14c. This is a descriptor ID, never a
     * decoded texture or a substitute material selector. */
    uint16_t first_structure2_image_id;
    int first_structure2_image_valid;
    uint16_t sequence_word_offset;
    int sequence_instruction_count;
    int image_instruction_count;
    int goto_instruction_count;
    /* Every non-control instruction is only an index into the already
     * bounded Structure2 descriptor table. This records the complete
     * original-data relation without assigning payload or texture meaning. */
    int structure2_image_instruction_bound_count;
    int structure2_image_instruction_unbound_count;
} Nexus_V1_DgnStructure1GEntry;

/* DMWeb DGN files, Structure2: 20-byte texture descriptors followed by a
 * FFFF terminator and raw palette/image data. Structure1G's global image
 * index is routed here only after subtracting 0x14c and finding this local
 * descriptor ID. Payload decoding remains a separate host gate. */
typedef struct {
    uint16_t image_id;
    uint16_t encoding;
    uint16_t palette_id;
    uint16_t width;
    uint16_t height;
    uint32_t image_relative_offset;
    uint32_t palette_relative_offset;
} Nexus_V1_DgnStructure2Texture;

/* The only established Structure2 payload grammar is the descriptor envelope:
 * Descriptor[20]... FFFF opaque_bytes[].  The bytes after FFFF are retained
 * as a bounded span, but are deliberately not called image or palette records
 * until the retail LEV corpus establishes their offset base and encoding. */
typedef struct {
    int descriptor_bytes;
    int terminator_offset;
    int opaque_payload_offset;
    int opaque_payload_size;
    /* Raw byte composition of the bounded post-FFFF span. These counts are
     * intentionally not a codec, record, image, or palette interpretation. */
    int opaque_payload_zero_byte_count;
    int opaque_payload_nonzero_byte_count;
    /* Pair composition of the same opaque byte span. These are raw bounded
     * two-byte windows only: they do not assert a word grammar, endianness,
     * pixel layout, palette layout, or record format. */
    int opaque_payload_complete_pair_count;
    int opaque_payload_trailing_byte_count;
    int opaque_payload_zero_pair_count;
    int opaque_payload_nonzero_pair_count;
    /* Read-only descriptor-offset correlation. These counters only compare
     * the already parsed numeric fields with the bounded post-FFFF span;
     * they do not assign an offset base, record grammar, pixel codec, or
     * palette meaning to those bytes. */
    int nonzero_descriptor_offset_count;
    int nonzero_descriptor_offsets_in_opaque_payload_count;
    int nonzero_descriptor_offsets_outside_opaque_payload_count;
    /* A target may be inside the opaque span but still point at its final
     * byte.  Count targets for which a two-byte window remains wholly inside
     * the span. This is a byte-boundary receipt, not a payload word grammar. */
    int nonzero_descriptor_offsets_word_bounded_count;
    /* The known LEV00-LEV15 descriptor targets are word-aligned.  This is
     * a measured envelope invariant only; it is not a payload record size. */
    int nonzero_descriptor_offsets_unaligned_count;
    /* Duplicate numeric targets are retained as layout provenance.  A reused
     * target is not a shared palette, image, record, or decoded surface. */
    int nonzero_descriptor_offset_unique_count;
    int nonzero_descriptor_offset_reused_count;
    int local_payload_offset_pattern_observed;
    int local_payload_word_aligned_offset_pattern_observed;
    int local_payload_word_bounded_offset_pattern_observed;
    /* Handoff integrity gate: every present descriptor target must name an
     * aligned, complete two-byte window in this descriptor's bounded opaque
     * span.  An all-zero target set is structurally valid too.  This is only
     * a source-envelope invariant, never a payload, pixel, palette, or
     * record-format claim. */
    int descriptor_offset_envelope_valid;
    int valid;
    int material_or_image_data_proven;
} Nexus_V1_DgnStructure2Payload;

typedef struct {
    int structure1_offset;
    int useful_size;
    int structure1b_relative_offset;
    int structure1b_end_relative_offset;
    int post_grid_offset;
    int post_grid_size;
    Nexus_V1_DgnStructure1PostGridPointer
        post_grid[NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT];
    Nexus_V1_DgnStructure1CRecordTable structure1c;
    Nexus_V1_DgnPostGrid0x24ZeroSpan post_grid_0x24_zero_span;
    Nexus_V1_DgnPostGrid0x30RecordTable post_grid_0x30_records;
    Nexus_V1_DgnStructure1ATable structure1a;
    Nexus_V1_DgnStructure1FTable structure1f;
    Nexus_V1_DgnStructure1GTable structure1g;
    int valid;
} Nexus_V1_DgnStructure1Layout;

typedef struct {
    int dmweb_container;
    int structure1_offset;
    int structure1_size;
    int structure1_useful_size;
    int structure1b_offset;
    int structure1b_size;
    int geometry_offset;
    int geometry_size;
    int structure1c_offset;
    int structure1c_size;
    int structure1c_record_count;
    int structure1c_indexed_record_count;
    int collision_records_valid;
    int post_grid_0x24_zero_span_valid;
    int post_grid_0x24_zero_span_size;
    int post_grid_0x30_record_table_valid;
    int post_grid_0x30_record_count;
    int post_grid_0x30_typed_prefix_record_count;
    int post_grid_0x30_opaque_tail_record_count;
    int post_grid_0x30_row_ordinal_prefix_valid;
    int post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    int post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    int post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    int collision_ref_count;
    int collision_ref_unique_count;
    int max_collision_ref;
    int post_grid_0x30_ref_count;
    int post_grid_0x30_ref_unique_count;
    int max_post_grid_0x30_ref;
    /* A non-zero packed Structure1B reference may only address a row in the
     * observed 0x30 typed prefix. The final row stays opaque and is never
     * render-addressable. */
    int post_grid_0x30_references_valid;
    int post_grid_0x30_invalid_ref_count;
    int first_invalid_post_grid_0x30_ref;
    int post_grid_0x30_record_zero_referenced;
    int post_grid_0x30_ref_value_count;
    int structure1f_declared;
    int structure1f_valid;
    int structure1f_total_entry_count;
    int structure1f_family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int structure1g_present;
    int structure1g_valid;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int mesh_ready;
} Nexus_V1_DgnGeometryInfo;

/* One bounded opaque Structure1C record. The table proof establishes only
 * the count and four-byte record form; individual byte semantics are not
 * inferred. These fields deliberately stay clear for retail DGN data until
 * a Saturn executable or capture proves their geometry grammar. */
typedef struct {
    int valid;
    int circle;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Nexus_V1_DgnCollisionSector;

/* Runtime-facing view of one Structure1B cell. Movement and rendering consume
 * the same documented cell/reference data. Structure1C record bytes are not
 * promoted to collision geometry until their Saturn grammar is evidenced. */
typedef struct {
    int square_type;
    uint16_t collision_ref;
    uint16_t post_grid_0x30_ref;
    uint8_t floor_material_ref;
    uint8_t ceiling_material_ref;
    uint8_t wall_material_refs[4];
    int8_t floor_height[4];
    int8_t ceiling_height[4];
    uint8_t floor_slope;
    uint8_t floor_rotation;
    Nexus_V1_DgnCollisionSector collision_sector;
    int post_grid_0x30_row_prefix_valid;
} Nexus_V1_DgnCellGeometry;

typedef struct {
    int width, height;
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint16_t collision_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_material_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_animation_ids[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t ceiling_material_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t wall_material_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE][4];
    int8_t floor_heights[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_slopes[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_rotations[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint16_t post_grid_0x30_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    Nexus_V1_DgnCollisionSector
        collision_sectors[NEXUS_DGN_MAX_COLLISION_SECTORS];
    int thing_count;
    int creature_count;
    int has_3d_geometry;
    int geometry_offset;
    int geometry_size;
    Nexus_V1_DgnGeometryInfo geometry_info;
    Nexus_V1_DgnStructure1FEntry
        structure1f_entries[NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES];
    int structure1f_entry_count;
    Nexus_V1_DgnStructure1AModel
        structure1a_models[NEXUS_DGN_MAX_STRUCTURE1A_ENTRIES];
    int structure1a_model_count;
    int structure1a_table_valid;
    uint16_t structure1a_owner_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t structure1a_owner_ref_valid[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    Nexus_V1_DgnStructure1GEntry
        structure1g_entries[NEXUS_DGN_MAX_STRUCTURE1G_ENTRIES];
    int structure1g_entry_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    /* A valid Structure1G instruction stream is not enough for a drawable
     * DGN handoff. Every declared global image index must resolve to its
     * local Structure2 descriptor before the host can rely on the table. */
    int structure1g_structure2_bindings_complete;
    Nexus_V1_DgnStructure2Texture
        structure2_textures[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES];
    int structure2_texture_count;
    int structure2_texture_table_valid;
    Nexus_V1_DgnStructure2Payload structure2_payload;
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payload;
    Nexus_V1_DgnStructure3DirectoryReceipt structure3_directory;
    Nexus_V1_DgnStructure3EntryHeaderReceipt structure3_entry_headers;
    Nexus_V1_DgnStructure3FaceReceipt structure3_faces;
    Nexus_V1_DgnStructure3FaceMaterialReceipt structure3_face_materials;
    Nexus_V1_DgnStructure3EdgeReceipt structure3_edges;
    Nexus_V1_DgnStructure3VectorReceipt structure3_vectors;
    Nexus_V1_DgnStructure3FaceGeometryReceipt structure3_face_geometry;
    Nexus_V1_DgnStructure3FaceEdgeReceipt structure3_face_edges;
    Nexus_V1_DgnStructure3FaceNormalPairReceipt structure3_face_normal_pairs;
    Nexus_V1_DgnStructure3FaceNormalGeometryReceipt
        structure3_face_normal_geometry;
    uint16_t structure3_entry_face_counts[NEXUS_DGN_MAX_STRUCTURE3_ENTRIES];
} Nexus_V1_Level;

/* Source-provenance predicate for host routes. A bounded Structure2 payload
 * is usable as a source receipt only when every present descriptor target
 * remains inside that payload's established envelope. This does not decode
 * the payload or grant any pixel, palette, or material permission. */
int nexus_v1_level_structure2_source_envelope_valid(
    const Nexus_V1_Level *level);

typedef enum {
    NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING = 0,
    NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH = 1,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY = 2,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET = 3,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK = 4,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS = 5,
    /* The parsed DGN has no committed canonical Track 1 Structure2 source. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE = 6,
    /* Visible direct Structure1F decorations/sensors have source cells but
     * no Saturn draw/trigger semantics yet. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS = 7,
    /* A packed Structure1B reference reached the opaque Structure1F tail or
     * lies outside the observed typed prefix. It is not drawable geometry. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE = 8,
    /* A Structure1G local descriptor exists, but one of its original
     * Structure2 targets escapes the bounded opaque envelope. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_ENVELOPE = 9,
    /* A span identifies itself as Structure1F but violates its documented
     * direct-coordinate contract, so host code must not omit it. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_LAYOUT = 10,
    /* Structure1A's proven owner/model relation reaches a Structure3 model
     * index, but Structure3 mesh payload/face semantics are still absent. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_MESH = 11,
    /* Structure3's declared block envelope is bounded, but its original
     * face grammar has not yet been decoded. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE3_FACE_SEMANTICS = 12,
    /* Hash-bound SN_FLOOR/SN_WALL bytes are present, but no Saturn
     * executable/capture has proved how Structure1B selects their entries. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1B_SELECTOR = 13,
    /* The engine must retain the canonical identity of the exact LEV bytes
     * that supplied Structure3 face/material provenance. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_CANONICAL_SOURCE = 14
} Nexus_V1_DgnRendererHandoffStatus;

typedef struct {
    Nexus_V1_DgnRendererHandoffStatus status;
    int canonical_source_verified;
    int dmweb_container;
    int mesh_ready;
    int can_render_dgn_mesh;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    int width;
    int height;
    int geometry_offset;
    int geometry_size;
    int collision_ref_count;
    int collision_ref_unique_count;
    int max_collision_ref;
    int post_grid_0x30_ref_count;
    int post_grid_0x30_ref_unique_count;
    int max_post_grid_0x30_ref;
    int post_grid_0x30_references_valid;
    int post_grid_0x30_invalid_ref_count;
    int first_invalid_post_grid_0x30_ref;
    int post_grid_0x30_ref_value_count;
    int post_grid_0x24_zero_span_valid;
    int post_grid_0x30_record_table_valid;
    int post_grid_0x30_record_count;
    int post_grid_0x30_typed_prefix_record_count;
    int post_grid_0x30_opaque_tail_record_count;
    int post_grid_0x30_row_ordinal_prefix_valid;
    int post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    int post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    int post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    int structure1f_declared;
    int structure1f_valid;
    int structure1f_total_entry_count;
    int structure1f_family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int structure1f_typed_entry_count;
    Nexus_V1_DgnStructure1FSpatialReceipt structure1f_spatial;
    Nexus_V1_DgnStructure1ABoundaryReceipt structure1a_boundary;
    Nexus_V1_DgnStructure1ARelationReceipt structure1a_relation;
    Nexus_V1_DgnStructure1AKindReceipt structure1a_kinds;
    Nexus_V1_DgnStructure3ModelReferenceReceipt structure3_model_references;
    Nexus_V1_DgnStructure3DirectoryReceipt structure3_directory;
    Nexus_V1_DgnStructure3EntryHeaderReceipt structure3_entry_headers;
    Nexus_V1_DgnStructure3FaceReceipt structure3_faces;
    Nexus_V1_DgnStructure3FaceMaterialReceipt structure3_face_materials;
    Nexus_V1_DgnStructure3EdgeReceipt structure3_edges;
    Nexus_V1_DgnStructure3VectorReceipt structure3_vectors;
    Nexus_V1_DgnStructure3FaceGeometryReceipt structure3_face_geometry;
    Nexus_V1_DgnStructure3FaceEdgeReceipt structure3_face_edges;
    Nexus_V1_DgnStructure3FaceNormalPairReceipt structure3_face_normal_pairs;
    Nexus_V1_DgnStructure3FaceNormalGeometryReceipt
        structure3_face_normal_geometry;
    Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt structure3_mesh_semantics;
    Nexus_V1_DgnStructure3AttachmentReceipt structure3_attachments;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt structure1a_transform_selectors;
    Nexus_V1_DgnStructure1FFaceSelectorReceipt structure1f_face_selectors;
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt structure3_model_face_selectors;
    Nexus_V1_DgnStructure1FRotationSelectorReceipt structure1f_rotation_selectors;
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt structure1f_face_rotation_pairs;
    Nexus_V1_DgnStructure1FOffsetPairReceipt structure1f_offset_pairs;
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt structure1f_wall_payload_selectors;
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt
        structure1f_wall_sensor_destinations;
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt
        structure1f_wall_sensor_control_selectors;
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt
        structure1f_wall_sensor_control_destination_tuples;
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt
        structure1f_wall_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt
        structure1f_wall_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt
        structure1f_alcove_payload_selectors;
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt
        structure1f_alcove_payload_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt
        structure1f_floor_sensor_control_selectors;
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt
        structure1f_floor_sensor_destinations;
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt
        structure1f_floor_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt
        structure1f_floor_sensor_extent_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt
        structure1f_floor_decoration_payload_selectors;
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt
        structure1f_floor_decoration_rotation_selectors;
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt
        structure1f_floor_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt
        structure1f_floor_decoration_control_extents;
    Nexus_V1_DgnStructure1FItemAttributePairReceipt structure1f_item_attribute_pairs;
    Nexus_V1_DgnStructure1FItemLocationPairReceipt structure1f_item_location_pairs;
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt
        structure1f_item_coordinate_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt structure1f_floor_decoration_offset_pairs;
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payload;
    int structure1g_present;
    int structure1g_valid;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    int structure1g_image_instruction_count;
    int structure1g_goto_instruction_count;
    int structure1g_structure2_image_instruction_bound_count;
    int structure1g_structure2_image_instruction_unbound_count;
    int structure1g_structure2_bindings_complete;
    int structure2_descriptor_offset_envelope_valid;
} Nexus_V1_DgnRendererHandoffReceipt;

typedef enum {
    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR = 1,
    NEXUS_V1_DGN_RENDER_COMMAND_CEILING = 2,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT = 3,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT = 4,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT = 5
} Nexus_V1_DgnRenderCommandKind;

typedef enum {
    NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_NONE = 0,
    /* DMWeb: Structure1B byte4 low nibble 3 declares an animated floor. */
    NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR = 1
} Nexus_V1_DgnAnimatedMaterialRoute;

/* Structure1F direct-cell families have documented 64x64 source
 * coordinates. These bits preserve that source relation on the owning mesh
 * command only; they do not imply an object, trigger, mesh, or draw ABI. */
typedef enum {
    NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_NONE = 0,
    NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_ITEM = 1 << 0,
    NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_FLOOR_DECORATION = 1 << 1,
    NEXUS_V1_DGN_STRUCTURE1F_DIRECT_FAMILY_FLOOR_SENSOR = 1 << 2
} Nexus_V1_DgnStructure1FDirectFamilyMask;

typedef struct {
    Nexus_V1_DgnRenderCommandKind kind;
    int x;
    int y;
    int depth;
    int lateral; /* -1=left, 0=center, +1=right */
    int square_type;
    int wall_dir;
    uint16_t collision_ref;
    uint16_t post_grid_0x30_ref;
    Nexus_V1_DgnCollisionSector collision_sector;
    /* DGN byte3 is signed 1/32 world-unit height. The four values are
     * NW, NE, SE, SW and carry the source-cell slope where present. */
    int8_t floor_height[4];
    int8_t ceiling_height[4];
    uint8_t floor_rotation;
    uint8_t floor_slope;
    /* Screen-space quad in NEXUS_V1_DGN_VIEWPORT_UNITS. The DGN plan
     * owns projection and material selection; hosts only rasterize it. */
    int16_t quad_x[4];
    int16_t quad_y[4];
    int post_grid_0x30_row_prefix_valid;
    uint8_t material_id;
    int animated_texture_declared;
    uint8_t animated_texture_id;
    int animated_texture_structure1g_entry_index;
    uint16_t animated_texture_structure1g_sequence_word_offset;
    uint16_t animated_texture_first_image_index;
    uint16_t animated_texture_structure2_image_id;
    int animated_texture_structure2_image_valid;
    Nexus_V1_DgnAnimatedMaterialRoute animated_texture_host_route;
    /* Exact Structure1F direct-cell provenance for this floor material cell.
     * Direct item, floor-decoration and floor-sensor records name a documented
     * 64x64 floor cell. They therefore never attach to the co-located wall or
     * ceiling commands. A non-zero value still forces the plan down its
     * no-draw semantics gate until the original Saturn draw/trigger ABI is
     * evidenced. */
    uint8_t structure1f_direct_family_mask;
    uint8_t structure1f_direct_entry_count;
    Nexus_V1_DgnRenderCommandKind material_source_kind;
    uint8_t palette_index;
    uint8_t draw_order;
} Nexus_V1_DgnRenderCommand;

/* Direct-coordinate Structure1F records reach only the matching DGN floor
 * command. The copied record is original-data provenance, not an object,
 * trigger, transform, mesh, texture, palette, pixel, or draw instruction. */
typedef struct {
    int command_index;
    int entry_index;
    Nexus_V1_DgnStructure1FEntry entry;
    int draw_authorized;
} Nexus_V1_DgnStructure1FDirectFloorCommandSource;

typedef struct {
    int visible_direct_entry_count;
    int floor_command_source_count;
    int item_floor_command_source_count;
    int floor_decoration_command_source_count;
    int floor_sensor_command_source_count;
    int blocked_capacity_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt;

/* Structure1A-bound Structure1F families name a verified owner cell only
 * after the complete Structure1A relation is available. The floor command is
 * retained solely as the unique visible cell anchor: it is not a claim about
 * a wall face, model, material, texture, palette, pixel or draw operation. */
typedef struct {
    int command_index;
    int entry_index;
    Nexus_V1_DgnStructure1FEntry entry;
    int owner_x;
    int owner_y;
    uint8_t structure3_model_index;
    uint8_t z_rotation;
    int draw_authorized;
} Nexus_V1_DgnStructure1FStructure1ACommandSource;

typedef struct {
    int visible_owned_entry_count;
    int floor_command_source_count;
    int alcove_floor_command_source_count;
    int wall_decoration_floor_command_source_count;
    int wall_sensor_floor_command_source_count;
    int blocked_missing_relation_count;
    int blocked_capacity_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt;

/* A Structure1A model index reaches the level's bounded Structure3 payload
 * through the verified owner-cell source. The payload has no proven model
 * ordinal, face grammar, material or pixel codec, so this is an unresolved
 * topology candidate only and can never authorize a draw. */
typedef struct {
    int command_index;
    int entry_index;
    int owner_x;
    int owner_y;
    /* DMWeb DGN Structure1F wall-family rows bind to Structure1A by their
     * 16-bit table index. Preserve the exact row identity and its raw face
     * selector beside the resolved owner-cell route. Neither field supplies
     * a face transform or any mesh/draw semantics. */
    Nexus_V1_DgnStructure1FFamily structure1f_family;
    uint8_t structure1f_tag;
    uint8_t structure1f_face_selector;
    uint16_t structure1f_structure1a_index;
    int structure1f_binding_proven;
    int structure1f_face_selector_semantics_proven;
    /* Structure1A row byte 0 is retained verbatim after the Structure1F
     * index resolves into the parsed table. It has no assigned meaning. */
    uint8_t structure1a_kind;
    int structure1a_row_binding_proven;
    int structure1a_kind_semantics_proven;
    /* These raw Structure1A row bytes are retained only after matching the
     * Structure1F-derived source values. They do not establish an ordinal,
     * transform, face, mesh, pixel, or draw interpretation. */
    uint8_t structure1a_structure3_model_index;
    uint8_t structure1a_z_rotation;
    int structure1a_model_rotation_binding_proven;
    int structure1a_model_rotation_semantics_proven;
    uint8_t structure3_model_index;
    uint8_t z_rotation;
    int structure3_block_offset;
    int structure3_block_count;
    int structure3_byte_size;
    uint32_t structure3_raw_payload_hash;
    /* Per-candidate exclusions against the only measured Structure3 ordinal
     * domains. They prevent a common but unproved model-index-to-block/run
     * shortcut; they do not identify a mesh span. */
    int model_index_exceeds_block_count;
    int model_index_exceeds_nonzero_byte_run_count;
    int model_index_exceeds_nonzero_block_run_count;
    int direct_ordinal_mapping_disproven;
    int model_ordinal_proven;
    int face_semantics_proven;
    int draw_authorized;
} Nexus_V1_DgnStructure1AStructure3TopologyCandidate;

typedef struct {
    int owner_cell_source_count;
    int topology_candidate_count;
    int structure1f_binding_count;
    int structure1a_row_binding_count;
    int structure1a_model_rotation_binding_count;
    int blocked_invalid_source_count;
    int blocked_payload_count;
    int direct_ordinal_mapping_disproven_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt;

/* Exact Structure1G -> Structure2 -> DGN floor-command provenance. The
 * descriptor fields retain raw original values only: Structure2's payload
 * grammar, palette layout, pixel codec and animation timing remain unproved,
 * so this receipt cannot authorize a draw. */
typedef struct {
    int command_index;
    int structure1g_entry_index;
    uint16_t structure1g_sequence_word_offset;
    uint16_t structure1g_global_image_index;
    int structure1g_sequence_instruction_count;
    int structure1g_sequence_image_instruction_count;
    int structure1g_sequence_goto_instruction_count;
    int structure1g_sequence_bound_image_count;
    int structure1g_sequence_unbound_image_count;
    uint16_t image_id;
    uint16_t encoding;
    uint16_t palette_id;
    uint16_t width;
    uint16_t height;
    uint32_t image_relative_offset;
    uint32_t palette_relative_offset;
    int image_offset_word_bounded;
    int palette_offset_word_bounded;
    int structure2_source_envelope_valid;
    int payload_decoder_proven;
    int draw_authorized;
} Nexus_V1_DgnStructure2FloorCommandSource;

typedef struct {
    int animated_floor_command_count;
    int structure1g_provenance_count;
    int global_image_index_binding_count;
    int complete_sequence_provenance_count;
    int descriptor_offset_envelope_count;
    int source_command_count;
    int blocked_invalid_command_count;
    int blocked_structure1g_provenance_count;
    int blocked_global_image_index_count;
    int blocked_sequence_provenance_count;
    int blocked_descriptor_offset_envelope_count;
    int blocked_missing_descriptor_count;
    int blocked_source_envelope_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure2FloorCommandSourceReceipt;

/* ITEM.IBS is the documented Structure1Fa item-descriptor source.  The
 * regular 16x16 4bpp images are independently bounded; special floor-image
 * records remain excluded until their Saturn pixel encoding is proven. */
#define NEXUS_V1_ITEM_IBS_BYTES 100352
#define NEXUS_V1_ITEM_IBS_DECLARATION_COUNT 243
#define NEXUS_V1_ITEM_IBS_PALETTE_COUNT 8
#define NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT 223
#define NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT 109
#define NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_PACKED_BYTES 2048
#define NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_TEXELS 4096
#define NEXUS_V1_VDP1_COMMAND_BYTES 32
#define NEXUS_V1_VDP1_VRAM_BYTES (512U * 1024U)

typedef struct {
    uint16_t image_id;
    uint16_t encoding;
    uint16_t palette_id;
    uint16_t width;
    uint16_t height;
    uint32_t image_offset;
    uint32_t image_bytes;
    uint32_t packed_4bpp_bytes;
    uint16_t palette_bgr555[16];
    uint8_t packed_4bpp_texels[NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_MAX_PACKED_BYTES];
    int palette_bound;
    int packed_4bpp_valid;
} Nexus_V1_ItemIbsFloorImage;

/* A captured VDP1 command is stored as sixteen little-endian words. The
 * documented coordinate words are framed as signed screen-space values, but
 * their relation to Structure3 transforms, camera state, clipping, and draw
 * ordering remains unproven. */
typedef struct {
    uint16_t control;
    uint16_t link_word;
    uint16_t draw_mode;
    uint16_t colour_control;
    uint16_t texture_source_word;
    uint16_t texture_width;
    uint16_t texture_height;
    int16_t xa;
    int16_t ya;
    int16_t xb;
    int16_t yb;
    int16_t xc;
    int16_t yc;
    int16_t xd;
    int16_t yd;
    uint16_t gouraud_table_word;
    uint8_t command_type;
    uint8_t colour_mode;
    uint8_t texture_bits_per_pixel;
    uint32_t texture_byte_count;
    uint32_t link_byte_offset;
    uint32_t texture_source_byte_offset;
    uint32_t texture_source_byte_end;
    int end_command;
    int texture_command;
    int colour_mode_documented;
    int texture_source_range_valid;
    int link_target_range_valid;
    int four_bpp_colour_bank;
    int coordinate_words_framed;
} Nexus_V1_Vdp1TextureCommand;

/* Sega VDP1 User's Manual, Ch. 6.3--6.5: mode 1 is a 4bpp lookup-table
 * texture. CMDCOLR names its 32-byte VDP1-VRAM table in eight-byte units and
 * each source byte supplies the left/high then right/low pixel code. The
 * helper returns raw 16-bit VDP1 colour codes only; it does not interpret
 * them as RGB, VDP2 CRAM, or host pixels. */
typedef struct {
    int valid;
    int command_mode1_lookup;
    int complete_vdp1_vram_snapshot;
    int texture_lane_matches_vram;
    uint32_t lookup_table_byte_offset;
    int lookup_table_in_vram;
    int texture_high_nibble_first;
    uint32_t output_pixel_count;
    int output_byte_count;
    int no_draw_only;
    int pixel_colour_semantics_proven;
    int palette_or_cram_semantics_proven;
} Nexus_V1_Vdp1LookupDecodeReceipt;

/* A VDP2 palette observation has to carry both the complete 4 KiB colour RAM
 * and the captured register image.  RAMCTL selects CRAM mode at +0x0e and
 * CRAOFB selects the sprite colour-RAM address offset at +0xe6.  These are
 * raw SH-2 little-endian register bytes, not a host palette. */
#define NEXUS_V1_VDP2_CRAM_BYTES 4096U
#define NEXUS_V1_VDP2_RAMCTL_OFFSET 0x0eU
#define NEXUS_V1_VDP2_CRAOFB_OFFSET 0xe6U
#define NEXUS_V1_VDP2_CAPTURE_REGISTERS_MIN_BYTES 0xe8U

typedef struct {
    const uint8_t *colour_ram;
    size_t colour_ram_size;
    const uint8_t *registers;
    size_t registers_size;
    int original_saturn_capture_verified;
} Nexus_V1_Vdp2ColourRamCapture;

typedef enum {
    NEXUS_V1_VDP1_MODE1_PIXEL_BLOCKED = 0,
    NEXUS_V1_VDP1_MODE1_PIXEL_TRANSPARENT = 1,
    NEXUS_V1_VDP1_MODE1_PIXEL_END_CODE = 2,
    NEXUS_V1_VDP1_MODE1_PIXEL_SUPPRESSED_AFTER_END = 3,
    NEXUS_V1_VDP1_MODE1_PIXEL_RGB555 = 4,
    NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB555 = 5,
    NEXUS_V1_VDP1_MODE1_PIXEL_VDP2_CRAM_RGB888 = 6
} Nexus_V1_Vdp1Mode1PixelKind;

/* Source-derived, display-independent result for one mode-1 texture sample.
 * `red`, `green`, and `blue` are the hardware's 8-bit output components; no
 * host framebuffer, blending, priority, or VDP2 composition is implied. */
typedef struct {
    Nexus_V1_Vdp1Mode1PixelKind kind;
    uint8_t texture_index;
    uint16_t raw_colour_code;
    uint16_t colour_ram_address;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Nexus_V1_Vdp1Mode1PalettePixel;

typedef struct {
    int valid;
    int original_saturn_capture_verified;
    int mode1_lookup_bound;
    int vdp2_cram_bound;
    int vdp2_registers_bound;
    uint8_t vdp2_cram_mode;
    uint8_t vdp2_sprite_colour_ram_offset;
    int source_index_zero_transparent;
    int source_index_f_end_code;
    int direct_rgb555_proven;
    int vdp2_cram_address_proven;
    int vdp2_cram_rgb_proven;
    uint32_t output_pixel_count;
    uint64_t colour_ram_fnv1a64;
    uint64_t registers_fnv1a64;
    int no_draw_only;
    int renderer_permitted;
} Nexus_V1_Vdp1Mode1PaletteResolveReceipt;

/* VDP1's 16-colour fetch is high-nibble first, but ITEM.IBS descriptor 0008
 * has not yet been tied to an original VDP1 command stream. These records
 * keep generic Saturn knowledge separate from an asset-bound observation. */
typedef struct {
    uint64_t item_ibs_fnv1a64;
    uint16_t image_id;
    uint16_t encoding;
    uint16_t width;
    uint16_t height;
    uint64_t packed_span_fnv1a64;
    uint64_t palette_fnv1a64;
    uint64_t vdp1_state_fnv1a64;
    uint64_t vdp1_command_fnv1a64;
    uint64_t texture_first_sequence;
    uint64_t texture_last_sequence;
    uint64_t vdp1_command_sequence;
    uint32_t vdp1_texture_source_address;
    uint32_t vdp1_texture_source_bytes;
    uint16_t vdp1_command_source_word;
} Nexus_V1_ItemIbs0008Vdp1CaptureCandidate;

typedef struct {
    int candidate_framing_valid;
    int item_ibs_source_matches;
    int floor_descriptor_matches;
    int packed_span_matches;
    int palette_matches;
    int vdp1_state_matches;
    int vdp1_command_matches;
    int vdp1_command_format_matches;
    int sequence_order_valid;
    int original_vdp1_capture_verified;
    int decode_authorized;
    int fallback_visuals_permitted;
} Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt;

typedef struct {
    int source_hash_verified;
    int descriptor_0008_verified;
    int packed_span_verified;
    int palette_bound;
    int blocked_missing_vdp1_command_provenance;
    int decoded_texel_count;
    int decode_authorized;
    int fallback_visuals_permitted;
} Nexus_V1_ItemIbs0008CodecReceipt;

typedef struct {
    int valid;
    int source_hash_verified;
    uint16_t inventory_association[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint16_t floor_image[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint8_t item_category[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint8_t item_weight[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    /* DMWeb ITEM.IBS declaration references into RLOWFIX.BIN TEXT records.
     * These are source IDs only; they are not display strings. */
    uint16_t item_name_string[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint16_t item_desc_string[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint16_t item_action1_string[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint16_t item_action2_string[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint16_t item_action3_string[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];
    uint16_t palette_bgr555[NEXUS_V1_ITEM_IBS_PALETTE_COUNT][16];
    uint8_t association_palette[256];
    uint8_t association_image[256];
    uint8_t regular_image_texels[NEXUS_V1_ITEM_IBS_REGULAR_IMAGE_COUNT][128];
    Nexus_V1_ItemIbsFloorImage
        floor_images[NEXUS_V1_ITEM_IBS_FLOOR_IMAGE_COUNT];
    int floor_image_count;
} Nexus_V1_ItemIbsBank;

typedef struct {
    int entry_index;
    int command_index;
    int source_x;
    int source_y;
    uint8_t item_id;
    uint8_t palette_index;
    uint8_t image_index;
    const uint16_t *palette_bgr555;
    const uint8_t *packed_4bpp_texels;
    const Nexus_V1_ItemIbsFloorImage *special_floor_image;
} Nexus_V1_DgnStructure1FItemMaterialBinding;

/* A command-indexed source-material handoff for ITEM.IBS descriptor 0008.
 * It deliberately retains the original packed 4bpp bytes: the retail
 * descriptor proves their size and local palette, but not their nibble order
 * or their world-space placement.  Therefore this is a no-draw material
 * consumer, never a renderer fallback. */
typedef struct {
    int command_index;
    int source_entry_index;
    int source_x;
    int source_y;
    uint8_t item_id;
    uint16_t image_id;
    uint16_t encoding;
    uint16_t width;
    uint16_t height;
    uint32_t packed_4bpp_bytes;
    const uint16_t *palette_bgr555;
    const uint8_t *packed_4bpp_texels;
    int source_hash_verified;
    int packed_4bpp_valid;
    int blocked_missing_vdp1_command_provenance;
    int original_vdp1_capture_verified;
    int texel_order_proven;
    int draw_authorized;
} Nexus_V1_DgnCommandPacked4BppMaterial;

typedef struct {
    int source_hash_verified;
    int item_entry_count;
    int command_candidate_count;
    int bound_regular_inventory_count;
    int bound_special_floor_palette_count;
    int bound_special_floor_texture_count;
    int blocked_special_floor_image_count;
    int blocked_missing_command_count;
    int blocked_invalid_item_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1FItemMaterialReceipt;

typedef struct {
    int source_hash_verified;
    int special_floor_binding_count;
    int source_cell_match_count;
    int command_material_count;
    int blocked_missing_vdp1_command_provenance_count;
    int original_vdp1_capture_verified_count;
    int blocked_invalid_binding_count;
    int blocked_invalid_command_count;
    int blocked_source_cell_mismatch_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnCommandPacked4BppMaterialReceipt;

/* Read-only Structure1Fa-to-ITEM.IBS coverage.  This establishes that an
 * actual DGN item's original image reference has a bounded source descriptor;
 * it does not infer where or how the Saturn drew that item. */
typedef struct {
    int source_hash_verified;
    int dgn_item_entry_count;
    int inventory_inherited_item_count;
    int special_floor_reference_count;
    int special_floor_0008_count;
    int blocked_invalid_item_count;
    int blocked_missing_floor_image_count;
    int blocked_unsupported_encoding_count;
    int complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1FItemIbsCoverageReceipt;

typedef struct {
    Nexus_V1_DgnRendererHandoffStatus status;
    /* These are package-to-host provenance facts, not decoder claims. A
     * plan chooses one complete source route and never mixes their surfaces. */
    int static_mns_source_pair_bound;
    int structure1b_selector_binding_proven;
    int structure2_vdp1_palette_binding_proven;
    int item_ibs_vdp1_command_proven;
    int bpk_material_route_bound;
    int uses_static_mns_material_route;
    int uses_bpk_material_route;
    int plan_ready;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    int command_count;
    int floor_count;
    int ceiling_count;
    int wall_count;
    int floor_material_command_count;
    int ceiling_material_command_count;
    int wall_material_command_count;
    int material_semantics_complete;
    int post_grid_0x30_reference_command_count;
    int post_grid_0x30_valid_reference_command_count;
    int first_post_grid_0x30_ref;
    int max_post_grid_0x30_ref;
    int post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    int post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    int post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    int structure1f_declared;
    int structure1f_valid;
    int structure1f_total_entry_count;
    int structure1f_family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int structure1f_typed_entry_count;
    Nexus_V1_DgnStructure1FSpatialReceipt structure1f_spatial;
    Nexus_V1_DgnStructure1ABoundaryReceipt structure1a_boundary;
    Nexus_V1_DgnStructure1ARelationReceipt structure1a_relation;
    Nexus_V1_DgnStructure1AKindReceipt structure1a_kinds;
    Nexus_V1_DgnStructure3ModelReferenceReceipt structure3_model_references;
    Nexus_V1_DgnStructure3DirectoryReceipt structure3_directory;
    Nexus_V1_DgnStructure3EntryHeaderReceipt structure3_entry_headers;
    Nexus_V1_DgnStructure3FaceReceipt structure3_faces;
    /* Renderer planning retains the complete bounded selector join and the
     * face-to-normal ordinal receipt, but neither receipt permits a decode
     * or a draw until original Saturn evidence establishes that route. */
    Nexus_V1_DgnStructure3FaceMaterialReceipt structure3_face_materials;
    Nexus_V1_DgnStructure3VectorReceipt structure3_vectors;
    Nexus_V1_DgnStructure3FaceGeometryReceipt structure3_face_geometry;
    Nexus_V1_DgnStructure3FaceEdgeReceipt structure3_face_edges;
    Nexus_V1_DgnStructure3FaceNormalPairReceipt structure3_face_normal_pairs;
    Nexus_V1_DgnStructure3FaceNormalGeometryReceipt
        structure3_face_normal_geometry;
    Nexus_V1_DgnStructure3AttachmentReceipt structure3_attachments;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt structure1a_transform_selectors;
    Nexus_V1_DgnStructure1FFaceSelectorReceipt structure1f_face_selectors;
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt structure3_model_face_selectors;
    Nexus_V1_DgnStructure1FRotationSelectorReceipt structure1f_rotation_selectors;
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt structure1f_face_rotation_pairs;
    Nexus_V1_DgnStructure1FOffsetPairReceipt structure1f_offset_pairs;
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt structure1f_wall_payload_selectors;
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt
        structure1f_wall_sensor_destinations;
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt
        structure1f_wall_sensor_control_selectors;
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt
        structure1f_wall_sensor_control_destination_tuples;
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt
        structure1f_wall_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt
        structure1f_wall_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt
        structure1f_alcove_payload_selectors;
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt
        structure1f_alcove_payload_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt
        structure1f_floor_sensor_control_selectors;
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt
        structure1f_floor_sensor_destinations;
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt
        structure1f_floor_sensor_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt
        structure1f_floor_sensor_extent_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt
        structure1f_floor_decoration_payload_selectors;
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt
        structure1f_floor_decoration_rotation_selectors;
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt
        structure1f_floor_decoration_model_rotation_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt
        structure1f_floor_decoration_control_extents;
    Nexus_V1_DgnStructure1FItemAttributePairReceipt structure1f_item_attribute_pairs;
    Nexus_V1_DgnStructure1FItemLocationPairReceipt structure1f_item_location_pairs;
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt
        structure1f_item_coordinate_pairs;
    Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt
        structure1f_floor_decoration_offset_pairs;
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payload;
    /* A blocked Structure3 handoff may still retain its proven owner-cell
     * topology route for later diagnostics. These values are raw envelope
     * relations and identity only; they are never mesh, face, material,
     * palette, pixel, or draw semantics. */
    int structure1a_structure3_topology_candidate_count;
    int structure1a_structure3_topology_structure1f_binding_count;
    int structure1a_structure3_topology_structure1f_face_selector_semantics_proven;
    int structure1a_structure3_topology_structure1a_row_binding_count;
    int structure1a_structure3_topology_structure1a_kind_semantics_proven;
    int structure1a_structure3_topology_structure1a_model_rotation_binding_count;
    int structure1a_structure3_topology_structure1a_model_rotation_semantics_proven;
    int structure1a_structure3_topology_blocked_invalid_source_count;
    int structure1a_structure3_topology_blocked_payload_count;
    int structure1a_structure3_topology_direct_ordinal_mapping_disproven_count;
    int structure1a_structure3_topology_complete;
    int structure1a_structure3_payload_block_offset;
    int structure1a_structure3_payload_block_count;
    int structure1a_structure3_payload_nonzero_byte_run_count;
    int structure1a_structure3_payload_nonzero_block_run_count;
    uint32_t structure1a_structure3_payload_raw_hash;
    /* Direct-coordinate Structure1F records whose documented 64x64 source
     * cell appears in this DGN plan. This is provenance only: no record is
     * interpreted as an object, sensor, trigger, or draw command. */
    int structure1f_plan_direct_entry_count;
    int structure1f_plan_item_entry_count;
    int structure1f_plan_floor_decoration_entry_count;
    int structure1f_plan_floor_sensor_entry_count;
    /* Exact direct-cell ownership is retained only on floor commands. The
     * separate entry counts above still block a visible record when a view has
     * no floor command to own it, so this refinement cannot silently omit
     * original data. */
    int structure1f_plan_direct_command_count;
    int structure1f_plan_direct_command_entry_count;
    int structure1f_plan_direct_floor_command_count;
    int structure1f_plan_direct_floor_command_entry_count;
    int structure1f_plan_item_floor_command_count;
    int structure1f_plan_item_floor_command_entry_count;
    int structure1g_present;
    int structure1g_valid;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    int structure1g_image_instruction_count;
    int structure1g_goto_instruction_count;
    int structure1g_structure2_image_instruction_bound_count;
    int structure1g_structure2_image_instruction_unbound_count;
    int structure1g_structure2_bindings_complete;
    int animated_material_command_count;
    int unresolved_animated_material_count;
    int source_cell_count;
    int missing_material_count;
    int first_missing_material_id;
    Nexus_V1_DgnRenderCommandKind first_missing_material_kind;
    int first_blocking_depth;
    int first_blocking_x;
    int first_blocking_y;
    /* Set only by the engine after the loaded level's Structure2 source
     * receipt is bound to its canonical Track 1 materialization. */
    int structure2_source_materialization_bound;
} Nexus_V1_DgnRenderPlanReceipt;

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index);
int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_level_get_collision_ref(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_level_get_material_ref(const Nexus_V1_Level *level, int x, int y,
                                    Nexus_V1_DgnRenderCommandKind kind,
                                    int wall_dir);
int nexus_v1_level_get_cell_geometry(const Nexus_V1_Level *level, int x, int y,
                                     Nexus_V1_DgnCellGeometry *out_cell);
int nexus_v1_level_structure1f_spatial_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FSpatialReceipt *out_receipt);
int nexus_v1_level_structure1a_boundary_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ABoundaryReceipt *out_receipt);
int nexus_v1_level_structure1a_relation_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ARelationReceipt *out_receipt);
int nexus_v1_level_structure1a_kind_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1AKindReceipt *out_receipt);
int nexus_v1_level_structure3_model_reference_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3ModelReferenceReceipt *out_receipt);
int nexus_v1_level_structure1a_transform_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1ATransformSelectorReceipt *out_receipt);
int nexus_v1_level_structure1a_transform_table_receipt(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    Nexus_V1_DgnStructure1ATransformTableReceipt *out_receipt);
int nexus_v1_level_structure1f_face_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFaceSelectorReceipt *out_receipt);
int nexus_v1_level_structure3_model_face_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_rotation_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FRotationSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_face_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt *out_receipt);
int nexus_v1_level_structure1f_offset_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FOffsetPairReceipt *out_receipt);
int nexus_v1_level_structure1f_wall_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_wall_sensor_destination_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt *out_receipt);
int nexus_v1_level_structure1f_wall_sensor_control_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_wall_sensor_control_destination_tuple_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt *out_receipt);
int nexus_v1_level_structure1f_wall_sensor_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt *out_receipt);
int nexus_v1_level_structure1f_wall_decoration_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt *out_receipt);
int nexus_v1_level_structure1f_alcove_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_alcove_payload_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_sensor_control_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_sensor_destination_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_sensor_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_sensor_extent_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_decoration_payload_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_decoration_rotation_selector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_decoration_model_rotation_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_decoration_control_extent_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt *out_receipt);
int nexus_v1_level_structure1f_item_attribute_pair_receipt(const Nexus_V1_Level *, Nexus_V1_DgnStructure1FItemAttributePairReceipt *);
int nexus_v1_level_structure1f_item_location_pair_receipt(const Nexus_V1_Level *, Nexus_V1_DgnStructure1FItemLocationPairReceipt *);
int nexus_v1_level_structure1f_item_coordinate_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt *out_receipt);
int nexus_v1_level_structure1f_floor_decoration_offset_pair_receipt(const Nexus_V1_Level *level, Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt *out_receipt);
int nexus_v1_level_structure3_payload_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3PayloadReceipt *out_receipt);
int nexus_v1_level_structure3_directory_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3DirectoryReceipt *out_receipt);
int nexus_v1_level_structure3_entry_header_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3EntryHeaderReceipt *out_receipt);
int nexus_v1_level_structure3_face_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceReceipt *out_receipt);
int nexus_v1_level_structure3_face_material_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceMaterialReceipt *out_receipt);
/* Copies only parser-validated Structure3b fill selectors from the exact
 * active LEV buffer. This is an identity/provenance bridge, not a texture or
 * palette decoder. */
int nexus_v1_level_collect_structure3_face_material_bindings(
    const Nexus_V1_Level *level, const uint8_t *data, int size,
    Nexus_V1_DgnFaceMaterialBinding *out_bindings, int max_bindings,
    int *out_binding_count);
int nexus_v1_level_structure3_edge_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3EdgeReceipt *out_receipt);
int nexus_v1_level_structure3_vector_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3VectorReceipt *out_receipt);
int nexus_v1_level_structure3_face_geometry_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceGeometryReceipt *out_receipt);
int nexus_v1_level_structure3_face_edge_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceEdgeReceipt *out_receipt);
int nexus_v1_level_structure3_face_normal_pair_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceNormalPairReceipt *out_receipt);

int nexus_v1_level_structure3_face_normal_geometry_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3FaceNormalGeometryReceipt *out_receipt);
int nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt *out_receipt);
int nexus_v1_level_extract_structure3_mesh_entry(
    const Nexus_V1_Level *level, const uint8_t *data, int size,
    int entry_index, Nexus_V1_DgnStructure3Vector *out_vertices,
    int max_vertices, Nexus_V1_DgnStructure3Face *out_faces, int max_faces,
    Nexus_V1_DgnStructure3Vector *out_normals, int max_normals,
    Nexus_V1_DgnStructure3MeshEntryReceipt *out_receipt);
/* Geometry readiness through the restored Structure3 mesh extractor:
 * returns 1 only when every mesh entry of the level extracts as bounded
 * typed source rows and builds to NEXUS_V1_DGN_MESH_READY_GEOMETRY with
 * can_submit_geometry set, textured raster and fallback visuals blocked,
 * and the summed extracted face count equals the level face receipt.
 * On success *out_face_total (when non-NULL) carries that total. This
 * binds no draw semantics and grants no raster or presentation route. */
int nexus_v1_level_structure3_mesh_geometry_ready(
    const Nexus_V1_Level *level, const uint8_t *data, int size,
    int *out_face_total);
int nexus_v1_level_structure3_attachment_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3AttachmentReceipt *out_receipt);
int nexus_v1_dgn_bind_structure3_face_capture_candidate(
    const Nexus_V1_Level *level, const uint8_t *dgn_data, int dgn_size,
    int dgn_source_hash_verified, int capture_source_verified,
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *candidate,
    const uint8_t *captured_texture_span, int captured_texture_span_size,
    const uint8_t *captured_palette_state, int captured_palette_state_size,
    const uint8_t *captured_vdp1_state, int captured_vdp1_state_size,
    const uint8_t *captured_transform_state, int captured_transform_state_size,
    const uint8_t *captured_normal_culling_state,
    int captured_normal_culling_state_size,
    const uint8_t *captured_vdp1_command, int captured_vdp1_command_size,
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt *out_receipt);
int nexus_v1_level_structure3_ordinal_correlation_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt *out_receipt);
int nexus_v1_level_dgn_structure1_host_provenance_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1HostProvenanceReceipt *out_receipt);
const char *nexus_v1_dgn_structure1_host_provenance_status_name(
    Nexus_V1_DgnStructure1HostProvenanceStatus status);
/* Returns non-zero only when the DGN target cell and its collision sector
 * admit a center-to-center party step. */
int nexus_v1_level_move_allowed(const Nexus_V1_Level *level,
                                int from_x, int from_y,
                                int to_x, int to_y);
int nexus_v1_dgn_geometry_info(Nexus_V1_DgnGeometryInfo *out_info,
                               const uint8_t *data,
                               int size);
int nexus_v1_dgn_structure1_layout(Nexus_V1_DgnStructure1Layout *out_layout,
                                   const uint8_t *data,
                                   int size);
int nexus_v1_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt);
const char *nexus_v1_dgn_renderer_handoff_status_name(
    Nexus_V1_DgnRendererHandoffStatus status);
int nexus_v1_level_build_dgn_view_render_plan(
    const Nexus_V1_Level *level,
    int party_x,
    int party_y,
    int party_dir,
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *out_receipt);
/* Copies visible direct-coordinate Structure1F records to their exact floor
 * commands. No semantic promotion or draw permission is granted. */
int nexus_v1_dgn_bind_direct_structure1f_floor_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnStructure1FDirectFloorCommandSource *out_sources,
    int max_sources,
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt *out_receipt);
/* Retains visible Structure1A-owned Structure1F rows on their proven owner
 * cell. It deliberately leaves all visual and runtime semantics unclaimed. */
int nexus_v1_dgn_bind_structure1a_owned_cell_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count,
    Nexus_V1_DgnStructure1FStructure1ACommandSource *out_sources,
    int max_sources,
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt *out_receipt);
/* Retains bounded Structure3 payload identity beside each verified owner-cell
 * source. It must not infer a model ordinal, mesh face, material or draw. */
int nexus_v1_dgn_bind_structure1a_structure3_topology_candidates(
    const Nexus_V1_Level *level,
    const Nexus_V1_DgnStructure1FStructure1ACommandSource *sources,
    int source_count,
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate *out_candidates,
    int max_candidates,
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt *out_receipt);
/* Binds a declared animated floor's verified local Structure2 descriptor to
 * the exact DGN floor command. It emits raw descriptor provenance only and
 * remains fail-closed until an original Saturn payload/VDP1 decoder exists. */
int nexus_v1_dgn_bind_structure2_animated_floor_sources(
    const Nexus_V1_Level *level, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnStructure2FloorCommandSource *out_sources,
    int max_sources, Nexus_V1_DgnStructure2FloorCommandSourceReceipt *out_receipt);
/* Parses only the documented ITEM.IBS regular-icon lane. `source_hash_verified`
 * must come from the caller's canonical Saturn asset receipt; an unverified
 * blob is never promotable to a material source. */
int nexus_v1_item_ibs_parse_verified(const uint8_t *data, int size,
                                     int source_hash_verified,
                                     Nexus_V1_ItemIbsBank *out_bank);
/* Binds direct Structure1Fa item IDs to their authenticated ITEM.IBS regular
 * image/palette pair and the owning DGN mesh command.  It never substitutes a
 * special floor image with an inventory icon. */
int nexus_v1_dgn_bind_structure1f_item_materials(
    const Nexus_V1_Level *level, const Nexus_V1_ItemIbsBank *bank,
    const Nexus_V1_DgnRenderCommand *commands, int command_count,
    Nexus_V1_DgnStructure1FItemMaterialBinding *out_bindings,
    int max_bindings, Nexus_V1_DgnStructure1FItemMaterialReceipt *out_receipt);
/* Consumes authenticated descriptor-0008 bindings into their exact DGN floor
 * command material slots.  It fails closed on every missing source fact and
 * never declares a draw because nibble ordering and item placement are not
 * yet source-proven. */
int nexus_v1_dgn_consume_structure1f_item_floor_materials(
    const Nexus_V1_DgnStructure1FItemMaterialBinding *bindings,
    int binding_count, const Nexus_V1_DgnRenderCommand *commands,
    int command_count, Nexus_V1_DgnCommandPacked4BppMaterial *out_materials,
    int max_materials, Nexus_V1_DgnCommandPacked4BppMaterialReceipt *out_receipt);
/* Inspects direct Structure1Fa item records against an authenticated ITEM.IBS
 * bank. Missing or unproved material data is reported as blocked, never
 * substituted. */
int nexus_v1_dgn_structure1f_item_ibs_coverage(
    const Nexus_V1_Level *level, const Nexus_V1_ItemIbsBank *bank,
    Nexus_V1_DgnStructure1FItemIbsCoverageReceipt *out_receipt);
/* Parses one complete captured VDP1 command record.  This describes only the
 * hardware packet fields and does not prove that any asset reached VDP1. */
int nexus_v1_vdp1_texture_command_parse(
    const uint8_t *command, int command_size,
    Nexus_V1_Vdp1TextureCommand *out_command);
/* Decode only documented VDP1 mode-1 lookup texture samples from a complete,
 * already-authenticated snapshot. This produces raw VDP1 colour codes, never
 * RGBA or a draw. */
int nexus_v1_vdp1_decode_mode1_lookup_texture(
    const uint8_t *command, int command_size,
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *texture_span, int texture_span_size,
    uint16_t *out_colour_codes, size_t out_colour_code_count,
    Nexus_V1_Vdp1LookupDecodeReceipt *out_receipt);
/* Byte-for-byte checker for a separately supplied mode-1 colour-code witness.
 * Equality does not authenticate a witness or permit a renderer. */
int nexus_v1_vdp1_lookup_colour_codes_match(
    const uint16_t *decoded, size_t decoded_count,
    const uint16_t *expected, size_t expected_count);
/* Resolves the documented palette chain for an authenticated Saturn mode-1
 * capture. VDP1 index 0 transparency and index F end-code behaviour are
 * taken from CMDPMOD; direct RGB555 and VDP2 CRAM codes are decoded exactly.
 * It is deliberately no-draw: final VDP1 framebuffer/VDP2 composition still
 * requires a byte-for-byte captured output witness. */
int nexus_v1_vdp1_resolve_mode1_palette_capture(
    const uint8_t *command, int command_size,
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *texture_span, int texture_span_size,
    const Nexus_V1_Vdp2ColourRamCapture *vdp2_capture,
    Nexus_V1_Vdp1Mode1PalettePixel *out_pixels, size_t out_pixel_count,
    Nexus_V1_Vdp1Mode1PaletteResolveReceipt *out_receipt);
/* Exact comparison for a separately captured resolved-pixel witness. It does
 * not authenticate the witness and never authorizes rendering. */
int nexus_v1_vdp1_mode1_palette_pixels_match(
    const Nexus_V1_Vdp1Mode1PalettePixel *resolved, size_t resolved_count,
    const Nexus_V1_Vdp1Mode1PalettePixel *expected, size_t expected_count);
/* Atomically binds one descriptor-0008 image to original Saturn capture
 * bytes. The source ITEM.IBS, packed span, palette, VDP1 state/command, and
 * texture-before-command ordering all have to match. It never authorizes a
 * draw. */
int nexus_v1_item_ibs_bind_0008_vdp1_capture(
    const Nexus_V1_ItemIbsFloorImage *floor,
    const uint8_t *item_ibs, int item_ibs_size, int item_ibs_hash_verified,
    const Nexus_V1_ItemIbs0008Vdp1CaptureCandidate *candidate,
    const uint8_t *captured_texture_span, int captured_texture_span_size,
    const uint8_t *captured_palette_state, int captured_palette_state_size,
    const uint8_t *captured_vdp1_state, int captured_vdp1_state_size,
    const uint8_t *captured_vdp1_command, int captured_vdp1_command_size,
    Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt *out_receipt);
/* Expands descriptor-0008 bytes only after the asset-bound original Nexus
 * VDP1 capture above establishes the byte/nibble route. */
int nexus_v1_item_ibs_decode_0008_vdp1_4bpp(
    const Nexus_V1_ItemIbsFloorImage *floor,
    const Nexus_V1_ItemIbs0008Vdp1CaptureBindingReceipt *capture,
    uint8_t *out_texels, int max_texels,
    Nexus_V1_ItemIbs0008CodecReceipt *out_receipt);

#endif
