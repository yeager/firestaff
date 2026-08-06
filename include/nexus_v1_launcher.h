#ifndef NEXUS_V1_LAUNCHER_H
#define NEXUS_V1_LAUNCHER_H

/*
 * nexus_v1_launcher.h
 * ===================
 * Nexus V1 launcher — singleton engine lifecycle manager.
 *
 * Owns the Nexus_V1_Engine singleton. Provides:
 *   - launcher_init()       — discover game data, init engine
 *   - launcher_load_level()  — load a dungeon level (0-15)
 *   - launcher_get_engine()  — access singleton (for M11 render loop)
 *
 * Design rationale:
 *   The Nexus V1 engine is a self-contained object with its own game
 *   state, mechanics, and resource management. The launcher acts as a
 *   thin facade that owns the engine pointer and routes M12/M11 calls
 *   into it. This separates launcher concerns (data discovery, profile
 *   validation) from engine concerns (tick, render, save/load).
 *
 * Source: DM Nexus (Saturn) boot flow, NEXUS.C / NEXUS2.C engine
 * lifecycle, ReDMCSB boot/disk loading references.
 */

#include "nexus_v1_engine.h"
#include "nexus_v1_lev_corpus_discovery.h"
#include "nexus_v1_multi_level_capture_campaign_launcher.h"
#include "nexus_v1_owner_material_capture_admission.h"
#include "nexus_v1_owner_material_capture_campaign.h"
#include "nexus_v1_owner_material_capture_campaign_artifact.h"
#include "nexus_v1_owner_material_capture_artifact_preflight.h"
#include "nexus_v1_sal_container_provenance.h"
#include "nexus_v1_sndlev_map_provenance.h"
#include "nexus_v1_slev_sal_asset_discovery.h"
#include "nexus_v1_slev_task_body_capture_plan.h"
#include "firestaff_nexus_v1_boot_profile.h"
#include "nexus_v1_light_runtime.h"
#include "nexus_v1_startup_menu.h"
#include "nexus_v1_structure3_capture_manifest.h"
#include "nexus_v1_title.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Public API ─────────────────────────────────────────────────────── */

/* Initialize the Nexus V1 engine singleton.
 * - Scans data_dir for CUE/BIN (Saturn CD) or extracted files.
 * - Calls nexus_v1_init() on the singleton.
 * - Returns 0 on success, -1 on failure.
 * - Safe to call multiple times; only first call has effect
 *   (subsequent calls return 0 if already initialized). */
int nexus_v1_launcher_init(const char *data_dir);

/* Load a dungeon level (0-15) into the engine.
 * Calls nexus_v1_load_level() on the singleton.
 * Returns 0 on success, -1 if launcher not init'd or level load failed. */
int nexus_v1_launcher_load_level(int level);

typedef struct {
    int valid;
    int opaque_saturn_card_only;
    int native_fnxs_path_used;
    uint64_t card_fnv1a64;
    uint64_t route_epoch;
} Nexus_V1_LauncherSaturnCardStartupReceipt;
int nexus_v1_launcher_select_saturn_card_startup(
    Nexus_V1_Engine *engine, uint64_t route_epoch, uint64_t card_fnv1a64,
    Nexus_V1_LauncherSaturnCardStartupReceipt *out_receipt);
typedef struct { int valid; int opaque_only; uint64_t card_fnv1a64; uint64_t package_fnv1a64; uint64_t route_epoch; } Nexus_V1_LauncherSaturnCardBootBinding;
int nexus_v1_launcher_bind_saturn_card_boot_route(Nexus_V1_Engine *, uint64_t, uint64_t, uint64_t, int, Nexus_V1_LauncherSaturnCardBootBinding *);

/* A selected MENU.BPK PRS3 entry is presentation evidence only. This receipt
 * binds source bytes to M11 while keeping the route explicitly no-draw. */
typedef struct {
    int valid;
    int no_draw_only;
    uint32_t entry_index;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    uint32_t prs3_version;
    uint32_t prs3_pixel_count;
    uint32_t header_first_u32;
    uint32_t header_minus_payload;
    Nexus_V1_BpkPrs3CompressionDescriptorReceipt compression;
} Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt;
int nexus_v1_launcher_menu_bpk_no_draw_presentation_receipt(
    const Nexus_V1_BpkRuntimeUploadReceipt *upload,
    const Nexus_V1_BpkRuntimeUploadRow *row,
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt *out_receipt);

typedef struct {
    int valid;
    int no_draw_only;
    int draw_disabled;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt presentation;
} Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt;
int nexus_v1_launcher_admit_m11_menu_bpk_no_draw_host(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64,
    const Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt *presentation,
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt *out_receipt);
int nexus_v1_launcher_consume_m11_menu_bpk_no_draw_host(
    const Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64,
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt *out_receipt);

typedef struct {
    int valid;
    int opaque_saturn_card_only;
    int no_draw_only;
    int draw_disabled;
    uint64_t card_fnv1a64;
    uint64_t package_fnv1a64;
    uint64_t route_epoch;
    Nexus_V1_LauncherSaturnCardBootBinding card;
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt m11;
} Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt;
int nexus_v1_launcher_bind_saturn_card_m11_no_draw_startup(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t card_fnv1a64, uint64_t package_fnv1a64,
    int direct_card_selected,
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt *out_receipt);

/* External PRS3 material-capture envelope. The body is an opaque witness;
 * declared output is structural metadata, not decoded output. */
#define NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_MAGIC "NXSPRS3M"
#define NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_VERSION 1U
#define NEXUS_V1_M11_PRS3_MATERIAL_CAPTURE_HEADER_BYTES 96U

typedef struct {
    int valid;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    int route_bound;
    int entry_bound;
    int body_bound;
    int declared_output_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int payload_opaque;
    int no_draw_only;
    int decoder_permitted;
    int fallback_visuals_permitted;
} Nexus_V1_LauncherM11Prs3MaterialLocalArtifactReceipt;

int nexus_v1_launcher_verify_m11_prs3_material_local_artifact(
    const Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Prs3MaterialLocalArtifactReceipt *out_receipt);

typedef struct {
    int valid, capture_required, capture_imported, resume_ready, operator_only;
    char bios_sha256[65], disc_sha256[65];
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt route;
    Nexus_V1_LauncherM11Prs3MaterialLocalArtifactReceipt capture;
    int no_draw_only, decoder_permitted, fallback_visuals_permitted;
} Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt;
int nexus_v1_launcher_admit_m12_m11_prs3_material_capture_required(
    const Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_prs3_material_capture(
    const Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *out_receipt);

/* Startup consumes PRS3 evidence as an operator-facing terminal state.  A
 * missing trace settles the scan at capture-required; an imported witness is
 * consumed without granting a decoder, pixels, or a MENU.BPK draw route. */
typedef enum {
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_INVALID = 0,
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_NOT_REQUIRED = 1,
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_CAPTURE_REQUIRED = 2,
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_CAPTURE_CONSUMED = 3
} Nexus_V1_LauncherPrs3StartupState;

typedef struct {
    Nexus_V1_LauncherPrs3StartupState state;
    int valid;
    int scan_complete;
    int startup_can_settle;
    int capture_required;
    int capture_consumed;
    int no_draw_only;
    int decoder_permitted;
    int synthetic_menu_image_permitted;
    const char *status;
} Nexus_V1_LauncherPrs3StartupStateReceipt;

int nexus_v1_launcher_prs3_startup_state(
    const Nexus_V1_MenuBpkRendererHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *capture,
    Nexus_V1_LauncherPrs3StartupStateReceipt *out_receipt);

typedef enum {
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_PRESENT = 0,
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RETURN_TO_IDLE = 1,
    NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RESCAN = 2
} Nexus_V1_LauncherPrs3StartupAction;

typedef struct {
    Nexus_V1_LauncherPrs3StartupStateReceipt prs3;
    int valid;
    int m12_handled;
    int terminal;
    int return_to_idle;
    int rescan_complete;
    int no_draw_only;
    int decoder_permitted;
    int synthetic_menu_image_permitted;
    const char *status;
} Nexus_V1_LauncherM12Prs3StartupTransitionReceipt;

int nexus_v1_launcher_m12_prs3_startup_transition(
    const Nexus_V1_MenuBpkRendererHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt *capture,
    Nexus_V1_LauncherPrs3StartupAction action,
    Nexus_V1_LauncherM12Prs3StartupTransitionReceipt *out_receipt);

/* Active-level SLEV/SAL/MAP/SDDRVS identities are launcher provenance only.
 * The accepted route continues to block script dispatch and SFX playback. */
typedef struct {
    int valid;
    int no_draw_only;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t asset_corpus_fnv1a64;
    Nexus_V1_LevelAuxAdmissionReceipt level_aux;
    Nexus_V1_SlevSalDirectLevelIdentity assets;
    Nexus_V1_SlevSalDirectIdentity sound_driver;
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt startup;
} Nexus_V1_LauncherM11SlevSalNoDrawReceipt;
int nexus_v1_launcher_admit_m11_slev_sal_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt *out_receipt);

/* A reviewed SLEV task-body capture target may reach M11 only through the
 * current direct SLEV/SAL/card/package route. The target is opaque evidence:
 * no opcode, callback, dispatch, codec, or fallback-script meaning is added. */
typedef struct {
    int valid;
    int no_draw_only;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    Nexus_V1_SlevSalDirectIdentity slev;
    Nexus_V1_SlevTaskBodyCaptureTarget task_body;
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt level_aux;
} Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt;
int nexus_v1_launcher_admit_m11_slev_task_body_no_dispatch(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_SlevTaskBodyCapturePlan *plan,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt *out_receipt);

/* This joins already parsed source receipts only. It does not inspect a SAL
 * descriptor, a MAP row, an SDDRVS command, or a SLEV opcode. */
typedef struct {
    int valid;
    int no_draw_only;
    int no_op_only;
    int commands_opaque;
    int audio_opaque;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    uint64_t task_trace_fnv1a64;
    uint64_t sal_descriptor_fnv1a64;
    uint64_t map_table_fnv1a64;
    uint64_t sound_driver_fnv1a64;
    Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt task;
    Nexus_V1_SalContainerProvenanceReceipt sal_container;
    Nexus_V1_SndlevMapProvenanceReceipt map_table;
} Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt;

typedef struct {
    int original_saturn_trace_bound;
    uint64_t task_trace_fnv1a64;
    uint64_t sal_descriptor_fnv1a64;
    uint64_t map_table_fnv1a64;
    uint64_t sound_driver_fnv1a64;
} Nexus_V1_LauncherSlevTaskSalCaptureBinding;
int nexus_v1_launcher_admit_m11_slev_task_sal_no_op_startup(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM11SlevTaskBodyNoDispatchReceipt *task,
    const Nexus_V1_SalContainerProvenanceReceipt *sal_container,
    const Nexus_V1_SndlevMapProvenanceReceipt *map_table,
    const Nexus_V1_LauncherSlevTaskSalCaptureBinding *binding,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *out_receipt);

#define NEXUS_V1_M11_SLEV_SAL_CAPTURE_MAGIC "NXSLSC01"
#define NEXUS_V1_M11_SLEV_SAL_CAPTURE_VERSION 1U
#define NEXUS_V1_M11_SLEV_SAL_CAPTURE_HEADER_BYTES 96U

/* External original-Saturn SLEV/SAL capture envelope. Its payload remains
 * opaque command evidence and is never retained, dispatched, or played. */
typedef struct {
    int valid;
    Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt route;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    int header_version_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int route_bound;
    int no_draw_only;
    int no_op_only;
    int commands_opaque;
    int audio_opaque;
    int dispatch_permitted;
    int playback_permitted;
    int fallback_script_permitted;
} Nexus_V1_LauncherM11SlevSalCaptureImportReceipt;
int nexus_v1_launcher_import_m11_slev_sal_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11SlevSalCaptureImportReceipt *out_receipt);
typedef struct {
    int valid;
    int route_bound;
    int task_bound;
    int sal_bound;
    int map_bound;
    int sddrvs_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int payload_opaque;
    int no_draw_only;
    int no_op_only;
} Nexus_V1_LauncherM11SlevSalLocalArtifactReceipt;
int nexus_v1_launcher_verify_m11_slev_sal_local_artifact(
    const Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11SlevSalLocalArtifactReceipt *out_receipt);

typedef struct {
    int valid;
    int capture_required;
    int capture_imported;
    int resume_ready;
    int operator_only;
    uint32_t bios_region;
    char bios_sha256[65];
    char disc_sha256[65];
    Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt startup;
    Nexus_V1_LauncherM11SlevSalCaptureImportReceipt capture;
    int no_draw_only;
    int no_op_only;
    int commands_opaque;
    int audio_opaque;
    int dispatch_permitted;
    int playback_permitted;
} Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt;
int nexus_v1_launcher_admit_m12_m11_slev_sal_capture_required(
    const Nexus_V1_LauncherM11SlevTaskSalNoOpStartupReceipt *startup,
    const char *bios_sha256, uint32_t bios_region, const char *disc_sha256,
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_slev_sal_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11SlevSalCaptureRouteReceipt *out_receipt);

/* Champion-to-dungeon handoff for a direct, verified LEV corpus row. This is
 * M12/M11 route evidence only and remains explicitly no-draw. */
typedef struct {
    int valid;
    int no_draw_only;
    int draw_disabled;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    uint32_t level_index;
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt champion_startup;
    Nexus_V1_DgnM11DirectLevNoDrawReceipt dungeon;
} Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt;
int nexus_v1_launcher_admit_direct_lev_m11_dungeon_handoff(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry,
    Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *out_receipt);

/* One immutable, external-capture target for the selected M11 Structure3
 * face/Structure2 descriptor route. It retains identities only, never a DGN
 * payload, candidate bytes, VDP1 command, palette, texture, or decoded data. */
typedef struct {
    int valid;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    uint32_t level_index;
    uint64_t dgn_fnv1a64;
    uint64_t dgn_byte_count;
    Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt face_descriptor;
    int direct_source_rehashed;
    int original_saturn_capture_required;
    int no_draw_only;
    int payload_materialized;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget;

/* External evidence supplied by an operator-produced original Saturn capture.
 * Lane identities are opaque observations, not texel, palette, or command
 * decoding claims. `replay_sequence` is an explicit monotonic capture order. */
typedef struct {
    int original_saturn_capture_verified;
    uint64_t replay_sequence;
    uint64_t trace_fnv1a64;
    uint64_t trace_byte_count;
    uint64_t texture_candidate_fnv1a64;
    uint64_t palette_candidate_fnv1a64;
    uint64_t vdp1_command_fnv1a64;
    uint64_t vdp1_command_byte_count;
} Nexus_V1_LauncherM11Structure2FaceCaptureReplayEvidence;

typedef struct {
    int valid;
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget target;
    uint64_t trace_fnv1a64;
    uint64_t trace_byte_count;
    uint64_t vdp1_command_fnv1a64;
    uint64_t vdp1_command_byte_count;
    int texture_candidate_bound;
    int palette_candidate_bound;
    int vdp1_binding_observed;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure2FaceCaptureReplayReceipt;

int nexus_v1_launcher_build_m11_structure2_face_capture_replay_target(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *out_target);
int nexus_v1_launcher_admit_m11_structure2_face_capture_replay(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayEvidence *evidence,
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayReceipt *out_receipt);

/* Read-only external original-Saturn VDP1 capture envelope. The opaque payload
 * begins at byte 136; its bytes are never retained or interpreted here. */
#define NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_MAGIC "NXSVDP1C"
#define NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_VERSION 1U
#define NEXUS_V1_M11_STRUCTURE2_FACE_VDP1_CAPTURE_HEADER_BYTES 136U

typedef struct {
    int valid;
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget target;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    uint64_t trace_fnv1a64;
    uint64_t trace_byte_count;
    uint64_t vdp1_command_fnv1a64;
    uint64_t vdp1_command_byte_count;
    int header_version_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int target_bound;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt;

int nexus_v1_launcher_import_m11_structure2_face_vdp1_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt *out_receipt);

/* One immutable replay target for the parser-observed Structure3b topology
 * framing selected by M11. It names source spans only; external payloads are
 * opaque evidence and cannot establish a mesh, transform, or draw route. */
typedef struct {
    int valid;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    uint32_t level_index;
    uint64_t dgn_fnv1a64;
    uint64_t dgn_byte_count;
    Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt topology;
    int direct_source_rehashed;
    int original_saturn_capture_required;
    int no_draw_only;
    int payload_materialized;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget;

#define NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_MAGIC "NXS3TOP1"
#define NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_VERSION 1U
#define NEXUS_V1_M11_STRUCTURE3_TOPOLOGY_CAPTURE_HEADER_BYTES 124U

typedef struct {
    int valid;
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget target;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    int header_version_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int target_bound;
    int topology_opaque;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt;

int nexus_v1_launcher_build_m11_structure3_topology_capture_replay_target(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *out_target);
int nexus_v1_launcher_import_m11_structure3_topology_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *target,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt *out_receipt);

typedef enum {
    NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_INVALID = 0,
    NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_US = 1,
    NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_JP = 2,
    NEXUS_V1_LAUNCHER_SATURN_BIOS_REGION_EU = 3
} Nexus_V1_LauncherSaturnBiosRegion;

/* M12/M11 owns this route transition. A local capture plan is evidence of an
 * external launch request only; without an imported V1 envelope it remains a
 * visible capture-required, no-draw route. */
typedef struct {
    int valid;
    int capture_required;
    int capture_imported;
    int resume_ready;
    int operator_only;
    Nexus_V1_LauncherSaturnBiosRegion bios_region;
    char bios_sha256[65];
    char disc_sha256[65];
    Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget target;
    Nexus_V1_LauncherM11Structure2FaceVdp1CaptureImportReceipt capture;
    int no_draw_only;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt;

int nexus_v1_launcher_admit_m12_m11_vdp1_capture_required(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const char *bios_sha256, Nexus_V1_LauncherSaturnBiosRegion bios_region,
    const char *disc_sha256,
    const Nexus_V1_LauncherM11Structure2FaceCaptureReplayTarget *target,
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_vdp1_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *out_receipt);

typedef struct {
    int valid;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    int bios_bound;
    int disc_bound;
    int dgn_bound;
    int face_bound;
    int descriptor_bound;
    int candidates_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int trace_command_bound;
    int payload_opaque;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure2FaceVdp1LocalArtifactReceipt;

int nexus_v1_launcher_verify_m11_structure2_face_vdp1_local_artifact(
    const Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure2FaceVdp1LocalArtifactReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_vdp1_local_artifact(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Vdp1CaptureRouteReceipt *out_receipt);

/* Operator-only lifecycle for an atomic Structure1F/1A owner + Structure3
 * face + Structure2 material observation. A successful resume retains only
 * the NXS1OMC1 opaque receipt and never establishes owner mapping or draw. */
typedef struct {
    int valid;
    int capture_required;
    int capture_admitted;
    int resume_ready;
    int operator_only;
    Nexus_V1_LauncherSaturnBiosRegion bios_region;
    char bios_sha256[65];
    char disc_sha256[65];
    int topology_candidate_index;
    uint32_t structure3_entry_index;
    uint32_t structure3_face_ordinal;
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget target;
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt capture;
    int no_draw_only;
    int owner_mapping_proven;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt;

int nexus_v1_launcher_admit_m12_m11_owner_material_capture_required(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *bios_sha256, Nexus_V1_LauncherSaturnBiosRegion bios_region,
    const char *disc_sha256,
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target,
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_owner_material_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *out_receipt);
int nexus_v1_launcher_import_m12_m11_owner_material_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *out_trace_receipt,
    Nexus_V1_LauncherM12M11OwnerMaterialCaptureRouteReceipt *out_receipt);

/* Operator-only M12 campaign routing for distinct already-imported NXS1OMC1
 * witnesses. Export carries only hash-bound job metadata; import retains only
 * campaign coverage and cannot enable any draw or decoder path. */
typedef struct {
    int valid;
    int capture_required;
    int captures_imported;
    int resume_ready;
    int operator_only;
    Nexus_V1_LauncherSaturnBiosRegion bios_region;
    char bios_sha256[65];
    char disc_sha256[65];
    uint32_t expected_witness_count;
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt campaign;
    int no_draw_only;
    int owner_mapping_proven;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt;

int nexus_v1_launcher_export_m12_owner_material_capture_campaign_required(
    uint32_t expected_witness_count, const char *bios_sha256,
    Nexus_V1_LauncherSaturnBiosRegion bios_region, const char *disc_sha256,
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *out_receipt);
int nexus_v1_launcher_import_m12_owner_material_capture_campaign(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *route,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *out_receipt);
int nexus_v1_launcher_import_m12_owner_material_capture_campaign_artifact(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *route,
    const uint8_t *artifact_bytes, size_t artifact_byte_count,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt *out_artifact_receipt,
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *out_receipt);

/* One route-selected Structure1F/Structure3/Structure2 witness. Admission
 * remains opaque/no-draw after the campaign artifact and selected preflight
 * both bind, and cannot be used as a rendering authorization. */
typedef struct {
    int valid;
    int capture_required;
    int capture_admitted;
    int resume_ready;
    int operator_only;
    uint32_t witness_index;
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt campaign_route;
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt campaign_artifact;
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt witness;
    int no_draw_only;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt;

int nexus_v1_launcher_admit_m12_owner_material_capture_witness_required(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *campaign_route,
    uint32_t witness_index,
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt *out_receipt);
int nexus_v1_launcher_import_m12_owner_material_capture_witness(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt *route,
    const uint8_t *campaign_artifact_bytes, size_t campaign_artifact_byte_count,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    const Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput *witness_input,
    Nexus_V1_LauncherM12OwnerMaterialCaptureWitnessRouteReceipt *out_receipt);

/* A bounded selection of distinct campaign witnesses. The receipt retains
 * selected row identities only and remains no-draw even after all witnesses
 * have individually passed their external artifact preflight. */
typedef struct {
    int valid;
    int capture_required;
    int captures_admitted;
    int resume_ready;
    int operator_only;
    uint32_t witness_count;
    uint32_t witness_indices[NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES];
    Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt campaign_route;
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt
        witnesses[NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES];
    int no_draw_only;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt;

int nexus_v1_launcher_admit_m12_owner_material_capture_multi_witness_required(
    const Nexus_V1_LauncherM12OwnerMaterialCaptureCampaignRouteReceipt *campaign_route,
    const uint32_t *witness_indices, uint32_t witness_count,
    Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt *out_receipt);

typedef struct {
    int valid;
    int selected_witness;
    int capture_required;
    int operator_only;
    uint32_t level_index;
    uint32_t witness_index;
    uint64_t source_fnv1a64;
    uint64_t descriptor_fnv1a64;
    uint64_t face_row_fnv1a64;
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget capture_target;
    int capture_target_bound;
    int no_draw_only;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11MultiWitnessDungeonCaptureStartReceipt;

int nexus_v1_launcher_admit_m11_multi_witness_dungeon_capture_start(
    const Nexus_V1_Engine *engine,
    const Nexus_V1_LauncherM12OwnerMaterialCaptureMultiWitnessRouteReceipt *route,
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *campaign_input,
    Nexus_V1_LauncherM11MultiWitnessDungeonCaptureStartReceipt *out_receipt);

/* This follows the same M12/M11 capture-required lifecycle as NXSVDP1C, but
 * retains only the opaque Structure3 topology-capture contract. */
typedef struct {
    int valid;
    int capture_required;
    int capture_imported;
    int resume_ready;
    int operator_only;
    Nexus_V1_LauncherSaturnBiosRegion bios_region;
    char bios_sha256[65];
    char disc_sha256[65];
    Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget target;
    Nexus_V1_LauncherM11Structure3TopologyCaptureImportReceipt capture;
    int no_draw_only;
    int decoder_permitted;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt;

int nexus_v1_launcher_admit_m12_m11_structure3_topology_capture_required(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const char *bios_sha256, Nexus_V1_LauncherSaturnBiosRegion bios_region,
    const char *disc_sha256,
    const Nexus_V1_LauncherM11Structure3TopologyCaptureReplayTarget *target,
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_structure3_topology_capture(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *route,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *out_receipt);

/* Local artifact preflight is deliberately redundant with the opaque importer:
 * it makes the operator-side BIOS/disc and topology contract explicit before
 * the live route is allowed to attempt a resume. */
typedef struct {
    int valid;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    int bios_bound;
    int disc_bound;
    int lev_bound;
    int face_bound;
    int vertex_bound;
    int normal_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int topology_opaque;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_LauncherM11Structure3TopologyLocalArtifactReceipt;

int nexus_v1_launcher_verify_m11_structure3_topology_local_artifact(
    const Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM11Structure3TopologyLocalArtifactReceipt *out_receipt);
int nexus_v1_launcher_resume_m12_m11_structure3_topology_local_artifact(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *handoff,
    const Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *route,
    const char *bios_sha256, const char *disc_sha256,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_LauncherM12M11Structure3TopologyCaptureRouteReceipt *out_receipt);

/* One direct SDDRVS identity may accompany a previously admitted direct-LEV
 * dungeon receipt only while every M11/card/package/level identity still
 * matches. It is script provenance only: parsing and dispatch remain blocked. */
typedef struct {
    int valid;
    int no_draw_only;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    Nexus_V1_SlevSalDirectIdentity sound_driver;
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt level_aux;
    Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt dungeon;
} Nexus_V1_LauncherM11SddrvsDungeonNoDrawReceipt;
int nexus_v1_launcher_admit_m11_sddrvs_dungeon_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *dungeon,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SddrvsDungeonNoDrawReceipt *out_receipt);

typedef struct {
    int valid;
    int no_draw_only;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    Nexus_V1_SlevSalDirectLevelIdentity assets;
    Nexus_V1_SalContainerProvenanceReceipt sal_container;
    Nexus_V1_SndlevMapProvenanceReceipt map_table;
    Nexus_V1_LauncherM11SlevSalNoDrawReceipt level_aux;
    Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt dungeon;
} Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt;
int nexus_v1_launcher_admit_m11_slev_sal_dungeon_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt *dungeon,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt *out_receipt);

/* The selected SNDLEV row stays opaque. This joins its bounded identity to an
 * already admitted M11 audio route; it never authorizes selector dispatch or
 * playback. */
typedef struct {
    int valid;
    int no_draw_only;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    uint32_t level_index;
    uint32_t row_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    uint64_t card_fnv1a64;
    uint64_t table_fnv1a64;
    Nexus_V1_SndlevMapRowProvenanceReceipt row;
    Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt dungeon;
} Nexus_V1_LauncherM11SndlevMapRowNoDrawReceipt;
int nexus_v1_launcher_admit_m11_sndlev_map_row_no_draw(
    Nexus_V1_Engine *engine,
    const Nexus_V1_SlevSalAssetDiscoveryReceipt *assets,
    const Nexus_V1_LauncherM11SlevSalDungeonNoDrawReceipt *dungeon,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    uint32_t level_index, uint32_t row_index,
    Nexus_V1_LauncherM11SndlevMapRowNoDrawReceipt *out_receipt);

typedef struct {
    int valid;
    int no_draw_only;
    int draw_disabled;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t dgn_fnv1a64;
    Nexus_V1_LauncherDirectLevM11DungeonHandoffReceipt direct_lev;
} Nexus_V1_LauncherMultiLevelM11DungeonHandoffReceipt;
int nexus_v1_launcher_admit_multi_level_m11_dungeon_handoff(
    Nexus_V1_Engine *engine, const Nexus_V1_LevCorpusDiscoveryReceipt *corpus,
    const Nexus_V1_MultiLevelCaptureCampaignLaunchPlan *plan,
    uint64_t route_epoch, uint64_t package_fnv1a64, uint64_t card_fnv1a64,
    int direct_card_selected, uint32_t level_index,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry,
    Nexus_V1_LauncherMultiLevelM11DungeonHandoffReceipt *out_receipt);

/* Get the current Nexus V1 engine singleton.
 * Returns NULL if launcher not initialized.
 * The returned pointer is owned by the launcher — do not free it. */
Nexus_V1_Engine *nexus_v1_launcher_get_engine(void);

/* Startup's source-owned Structure3 capture boundary. It never exposes a
 * drawable surface or interpretation of opaque capture bytes. */
int nexus_v1_launcher_startup_structure3_capture_intake(
    const char *manifest_text, size_t manifest_size,
    const Nexus_V1_DgnStructure3CaptureImport *capture,
    Nexus_V1_DgnStructure3CaptureHostReceipt *out_receipt);

/* Reads a six-lane external capture through the strict raw importer, then
 * consumes it only against the launcher's currently owned canonical DGN
 * bytes. The resulting engine source remains no-draw. Call release on the
 * receipt when its reader-owned spans are no longer needed. */
int nexus_v1_launcher_startup_structure3_raw_capture_intake(
    const char *manifest_text, size_t manifest_size,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RawCaptureHostReceipt *out_receipt);

/* Launcher-owned boundary for one Structure1F direct-face capture request.
 * It validates only package ownership and no-draw capture identity; it does
 * not consume a transform, material, palette, VDP1 command, or pixels. */
typedef struct {
    int launcher_initialized;
    int active_level_bound;
    int package_source_bound;
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt capture_target;
    Nexus_V1_DmBinVdp1StateWriteReceipt vdp1_state;
    int vdp1_state_source_bound;
    int vdp1_capture_prerequisite_bound;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectFaceCaptureHostReceipt;

int nexus_v1_launcher_dgn_direct_face_capture_intake(
    int structure1f_entry_index, const char *manifest_text, size_t manifest_size,
    Nexus_V1_DgnStructure1FDirectFaceCaptureHostReceipt *out_receipt);

/* Launcher-owned join between a direct Structure1F request and a six-lane
 * original-Saturn capture. All lanes remain opaque and the accepted route is
 * no-draw; callers release raw_capture with the existing raw-capture helper. */
typedef struct {
    Nexus_V1_DgnStructure1FDirectFaceCaptureHostReceipt direct_face;
    Nexus_V1_DgnStructure3RawCaptureHostReceipt raw_capture;
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt joined_capture;
    Nexus_V1_DgnStructure1FVdp1MaterialReceipt material_capture;
    int engine_capture_consumed;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectFaceRawCaptureHostReceipt;

int nexus_v1_launcher_dgn_direct_face_raw_capture_intake(
    int structure1f_entry_index, const char *direct_manifest_text,
    size_t direct_manifest_size, const char *capture_manifest_text,
    size_t capture_manifest_size,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureHostReceipt *out_receipt);

typedef struct {
    int title_surface_loaded;
    int warning_surface_loaded;
    int gameover_surface_loaded;
    int status_bg_surface_loaded;
    int title_screen_loaded;
    int startup_surfaces_loaded;
    int startup_surfaces_expected;
    int startup_surfaces_fallback;
    int faces_loaded;
    int faces_expected;
    int faces_fallback;
    int menu_bpk_upload_receipt_valid;
    int menu_bpk_provenance_bounded;
    uint32_t menu_bpk_provenance_archive_entries;
    uint32_t menu_bpk_provenance_table_entries;
    uint64_t menu_bpk_provenance_expected_bytes;
    Nexus_V1_LauncherMenuBpkNoDrawPresentationReceipt
        menu_bpk_no_draw_presentation;
    Nexus_V1_BpkRuntimeUploadRoute menu_bpk_upload_route;
    int menu_bpk_archive_entries;
    int menu_bpk_surface_entries;
    int menu_bpk_directory_trailer_found;
    int menu_bpk_directory_trailer_at_entry_zero;
    int menu_bpk_directory_trailer_valid;
    int menu_bpk_planned_rows;
    int menu_bpk_blocked_prs3_uploads;
    int menu_bpk_blocks_real_menu_surface_render;
    int menu_bpk_fallback_visuals_permitted;
    Nexus_SfxRuntimeStatus startup_sfx_status;
    int startup_sfx_level_index;
    int startup_cd_track;
    int startup_sfx_blocks_real_playback;
    int startup_assets_ready;
    int startup_audio_handoff_ready;
    int main_menu_route_ready;
    int title_route_ready;
    int real_menu_surface_route_ready;
    int real_menu_surface_route_blocked;
    int save_menu_route_ready;
    int champion_menu_route_ready;
    const char *real_menu_surface_blocker;
    const char *startup_menu_asset_route;
} Nexus_V1_LauncherStartupAssetsReceipt;

typedef struct {
    Nexus_V1_Engine *engine;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    int title_loaded;
    char dungeon_path[512];
    Nexus_V1_StartupLaunchReceipt startup_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt startup_assets;
} Nexus_V1_LauncherBootReceipt;

typedef struct {
    Nexus_V1_Engine *engine;
    Nexus_TitleScreen *title_screen;
    int title_screen_keep;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    int title_loaded;
    char title[64];
    char source_id[32];
    char dungeon_path[512];
    Nexus_V1_StartupLaunchReceipt startup_receipt;
    Nexus_V1_StartupHostReceipt boot_status_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt startup_assets;
    const char *boot_log_line;
} Nexus_V1_LauncherRuntimeReceipt;

typedef enum {
    NEXUS_V1_STARTUP_LAUNCH_GATE_INVALID = 0,
    NEXUS_V1_STARTUP_LAUNCH_GATE_DATA_ERROR = 1,
    NEXUS_V1_STARTUP_LAUNCH_GATE_TITLE_READY = 2,
    NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_ASSET_BLOCKED = 3,
    NEXUS_V1_STARTUP_LAUNCH_GATE_MENU_READY = 4
} Nexus_V1_StartupLaunchGateRoute;

typedef struct {
    Nexus_V1_StartupLaunchGateRoute route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    int engine_ready;
    int level_loaded;
    int title_ready;
    int title_draw_ready;
    int real_menu_ready;
    int save_menu_ready;
    int champion_menu_ready;
    int fallback_visuals_permitted;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
    const char *boot_log_line;
} Nexus_V1_StartupLaunchGateReceipt;

typedef enum {
    NEXUS_V1_STARTUP_ASSET_HANDOFF_INVALID = 0,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_DATA_ERROR = 1,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_TITLE_READY = 2,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_MENU_BLOCKED = 3,
    NEXUS_V1_STARTUP_ASSET_HANDOFF_MAIN_MENU_READY = 4
} Nexus_V1_StartupAssetHandoffRoute;

typedef struct {
    Nexus_V1_StartupAssetHandoffRoute route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_MenuBpkRendererHandoffReceipt menu_bpk_renderer_handoff;
    int title_asset_handoff_ready;
    int real_menu_asset_handoff_ready;
    int audio_asset_handoff_ready;
    int main_menu_route_ready;
    int saturn_asset_handoff_ready;
    int real_asset_route_ready;
    int menu_bpk_renderer_handoff_valid;
    int menu_bpk_prs3_blocks_real_menu_route;
    int blocks_main_menu_route;
    int fallback_visuals_permitted;
    const char *title_asset_route;
    const char *menu_asset_route;
    const char *audio_asset_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupAssetHandoffReceipt;

typedef struct {
    Nexus_V1_Engine *engine;
    Nexus_V1_StartupHostReceipt host_receipt;
    int resumed;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    char dungeon_path[512];
    int nglt_decoded;
    char diagnostic[256];
    const char *log_line;
} Nexus_V1_LauncherResumeReceipt;

void nexus_v1_launcher_boot_receipt_clear(
    Nexus_V1_LauncherBootReceipt *receipt);
void nexus_v1_launcher_runtime_receipt_clear(
    Nexus_V1_LauncherRuntimeReceipt *receipt);
void nexus_v1_launcher_startup_launch_gate_receipt_clear(
    Nexus_V1_StartupLaunchGateReceipt *receipt);
void nexus_v1_launcher_startup_asset_handoff_receipt_clear(
    Nexus_V1_StartupAssetHandoffReceipt *receipt);
const char *nexus_v1_launcher_startup_launch_gate_route_name(
    Nexus_V1_StartupLaunchGateRoute route);
const char *nexus_v1_launcher_startup_asset_handoff_route_name(
    Nexus_V1_StartupAssetHandoffRoute route);
int nexus_v1_launcher_startup_launch_gate_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupLaunchGateReceipt *out_receipt);
int nexus_v1_launcher_startup_asset_handoff_from_runtime_receipt(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    Nexus_V1_StartupAssetHandoffReceipt *out_receipt);

typedef struct {
    int title_active;
    int title_frame;
    int save_select_active;
    int champion_select_active;
    const char *save_dir;
    unsigned int slot_mask;
    int save_selected_row;
    int save_row_count;
    Nexus_V1_Engine *engine;
    int champion_cursor;
    int champion_frame;
    uint64_t launcher_route_epoch;
    uint64_t menu_bpk_package_fnv1a64;
    uint64_t saturn_card_fnv1a64;
    int saturn_card_direct_selected;
} Nexus_V1_StartupRuntimeState;

typedef struct {
    Nexus_V1_StartupRuntimeState runtime;
} Nexus_V1_LauncherRuntimeStartupSnapshot;

typedef enum {
    NEXUS_V1_STARTUP_MENU_PRESENTATION_INVALID = 0,
    NEXUS_V1_STARTUP_MENU_PRESENTATION_SAVE = 1,
    NEXUS_V1_STARTUP_MENU_PRESENTATION_CHAMPION = 2
} Nexus_V1_StartupMenuPresentationKind;

typedef struct {
    Nexus_V1_StartupMenuPresentationKind kind;
    int route_ready;
    int route_blocked;
    int draw_command_count;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    int host_caller_valid;
    int package_capture_consumed_by_host;
    int display_callers_use_package_receipt;
    int suppress_fallback_visuals;
    int blocked_route_suppresses_all_draws;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupMenuPresentationReceipt;

typedef struct {
    Nexus_V1_StartupTitleRouteReceipt title_route;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    int route_ready;
    int route_blocked;
    int title_draw_ready;
    int save_menu_ready;
    int champion_menu_ready;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupTitleHandoffReceipt;

typedef enum {
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_INVALID = 0,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_ASSET_BLOCKED = 1,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_NOT_START = 2,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_DGN_BLOCKED = 3,
    NEXUS_V1_STARTUP_RUNTIME_HANDOFF_READY_RENDER_STATE = 4
} Nexus_V1_StartupRuntimeHandoffRoute;

typedef struct {
    Nexus_V1_StartupRuntimeHandoffRoute route;
    Nexus_V1_StartupChampionExecution champion_execution;
    Nexus_V1_StartupHostActionReceipt host_action_receipt;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_V1_DgnRenderPlanReceipt render_plan;
    Nexus_V1_DgnStructure1HostProvenanceReceipt structure1_host_provenance;
    Nexus_V1_DgnStructure2SourceReceipt structure2_source;
    Nexus_V1_DgnStaticMaterialSourceReceipt static_material_sources;
    Nexus_ScriptRuntimeReceipt script_receipt;
    int runtime_ready;
    int dgn_render_ready;
    int dgn_viewport_render_ready;
    int hud_ready;
    int dgn_render_blocked;
    int structure1_host_provenance_consumed;
    int dgn_static_material_source_consumed;
    int structure2_source_materialization_bound;
    int structure2_vdp1_palette_binding_proven;
    int item_ibs_vdp1_command_proven;
    int script_runtime_ready;
    int script_runtime_blocked;
    int level_loaded;
    int party_x;
    int party_y;
    int party_dir;
    int command_count;
    int viewport_rasterized_command_count;
    int viewport_material_surface_count;
    int viewport_floor_material_surface_count;
    int viewport_ceiling_material_surface_count;
    int viewport_wall_material_surface_count;
    int dgn_viewport_host_route_status;
    int dgn_viewport_host_route_ready;
    int dgn_viewport_host_route_consumed;
    int dgn_viewport_host_route_package_consumed;
    int dgn_viewport_host_route_blocks_runtime;
    int dgn_viewport_capture_ready;
    uint32_t dgn_viewport_frame_hash;
    int dgn_render_floor_count;
    int dgn_render_ceiling_count;
    int dgn_render_wall_count;
    int dgn_floor_material_command_count;
    int dgn_ceiling_material_command_count;
    int dgn_wall_material_command_count;
    int dgn_material_semantics_complete;
    int bpk_material_surface_count;
    int bpk_truecolor_material_surface_count;
    int bpk_prs3_material_surface_count;
    int dgn_material_plan_consumed;
    int dgn_commands_copied_from_material_plan;
    uint32_t dgn_command_buffer_hash;
    int dgn_command_buffer_exact;
    int dgn_material_viewport_consumed;
    int bpk_material_path_consumed;
    int viewport_written_pixels;
    int fallback_visuals_permitted;
    const char *asset_route;
    const char *dgn_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupRuntimeHandoffReceipt;

typedef struct {
    Nexus_V1_StartupRuntimeHandoffRoute route;
    Nexus_V1_StartupChampionExecution champion_execution;
    Nexus_V1_StartupHostActionReceipt host_action_receipt;
    Nexus_V1_StartupRuntimeHandoffReceipt runtime_handoff;
    int host_action_valid;
    int runtime_route_ready;
    int runtime_route_blocked;
    Nexus_SfxRuntimeStatus startup_sfx_status;
    int startup_sfx_level_index;
    int startup_cd_track;
    int startup_audio_handoff_ready;
    int startup_sfx_blocks_real_playback;
    Nexus_V1_DgnRendererHandoffStatus dgn_handoff_status;
    Nexus_V1_DgnRendererHandoffStatus dgn_render_plan_status;
    Nexus_V1_DgnRenderCommandKind first_dgn_render_command_kind;
    int dgn_render_plan_ready;
    int dgn_render_command_count;
    int dgn_render_floor_count;
    int dgn_render_ceiling_count;
    int dgn_render_wall_count;
    int dgn_floor_material_command_count;
    int dgn_ceiling_material_command_count;
    int dgn_wall_material_command_count;
    int dgn_material_semantics_complete;
    int structure2_source_materialization_bound;
    int structure2_vdp1_palette_binding_proven;
    int item_ibs_vdp1_command_proven;
    int dgn_static_material_source_consumed;
    int dgn_viewport_render_ready;
    int dgn_viewport_rasterized_command_count;
    int dgn_viewport_material_surface_count;
    int dgn_viewport_floor_material_surface_count;
    int dgn_viewport_ceiling_material_surface_count;
    int dgn_viewport_wall_material_surface_count;
    int dgn_viewport_host_route_status;
    int dgn_viewport_host_route_ready;
    int dgn_viewport_host_route_consumed;
    int dgn_viewport_host_route_package_consumed;
    int dgn_viewport_host_route_blocks_runtime;
    int dgn_viewport_capture_ready;
    uint32_t dgn_viewport_frame_hash;
    int bpk_material_surface_count;
    int bpk_truecolor_material_surface_count;
    int bpk_prs3_material_surface_count;
    int dgn_material_plan_consumed;
    int dgn_commands_copied_from_material_plan;
    uint32_t dgn_command_buffer_hash;
    int dgn_command_buffer_exact;
    int dgn_material_viewport_consumed;
    int bpk_material_path_consumed;
    int dgn_viewport_written_pixels;
    int dgn_blocks_real_mesh_render;
    Nexus_ScriptRuntimeStatus script_runtime_status;
    int script_runtime_ready;
    int script_runtime_blocked;
    int script_candidate_source_bytes;
    int script_rules_loaded;
    int consumed_by_nexus;
    int fallback_visuals_permitted;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupRuntimeRouteReceipt;

typedef enum {
    NEXUS_V1_STARTUP_ROUTE_PROOF_INVALID = 0,
    NEXUS_V1_STARTUP_ROUTE_PROOF_ASSET_BLOCKED = 1,
    NEXUS_V1_STARTUP_ROUTE_PROOF_TITLE_READY = 2,
    NEXUS_V1_STARTUP_ROUTE_PROOF_MENU_READY = 3,
    NEXUS_V1_STARTUP_ROUTE_PROOF_RUNTIME_READY = 4
} Nexus_V1_StartupRouteProofRoute;

typedef struct {
    Nexus_V1_StartupRouteProofRoute route;
    Nexus_V1_StartupLaunchGateReceipt launch_gate;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupTitleHandoffReceipt title_handoff;
    Nexus_V1_StartupMenuPresentationReceipt menu_presentation;
    Nexus_V1_StartupRuntimeRouteReceipt runtime_route_receipt;
    Nexus_V1_StartupRuntimeHandoffReceipt runtime_handoff;
    int saturn_asset_boot_ready;
    int title_route_ready;
    int menu_route_ready;
    int title_art_loaded;
    int warning_art_loaded;
    int startup_surfaces_real_ready;
    int faces_real_ready;
    int save_load_menu_route_ready;
    int startup_ui_route_ready;
    int full_start_graphics_ready;
    int runtime_route_ready;
    int graphics_ready;
    int audio_ready;
    int title_menu_route_ready;
    int menu_runtime_route_ready;
    int first_runtime_route_ready;
    int audio_runtime_route_ready;
    int audio_runtime_route_blocked;
    int dgn_viewport_host_route_status;
    int dgn_viewport_host_route_ready;
    int dgn_viewport_host_route_consumed;
    int dgn_viewport_host_route_package_consumed;
    int dgn_viewport_host_route_blocks_runtime;
    int dgn_viewport_capture_ready;
    uint32_t dgn_viewport_frame_hash;
    Nexus_SfxRuntimeStatus startup_sfx_status;
    int startup_sfx_level_index;
    int startup_cd_track;
    int startup_sfx_blocks_real_playback;
    int script_runtime_route_ready;
    int script_runtime_route_blocked;
    int full_startup_route_ready;
    Nexus_ScriptRuntimeStatus script_runtime_status;
    int script_candidate_source_bytes;
    int fallback_visuals_permitted;
    const char *asset_route;
    const char *startup_ui_blocker;
    const char *title_route;
    const char *menu_route;
    const char *runtime_route;
    const char *first_runtime_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupRouteProofReceipt;

typedef enum {
    NEXUS_V1_STARTUP_FULL_START_INVALID = 0,
    NEXUS_V1_STARTUP_FULL_START_BLOCKED_ASSETS = 1,
    NEXUS_V1_STARTUP_FULL_START_WARNING_TITLE_READY = 2,
    NEXUS_V1_STARTUP_FULL_START_MENU_READY = 3
} Nexus_V1_StartupFullStartRoute;

typedef struct {
    Nexus_V1_StartupFullStartRoute route;
    Nexus_V1_StartupLaunchGateReceipt launch_gate;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;
    Nexus_V1_LauncherStartupAssetsReceipt assets;
    Nexus_V1_StartupHostReceipt host_receipt;
    int warning_art_loaded;
    int title_art_loaded;
    int gameover_art_loaded;
    int warning_capture_surface_ready;
    int title_capture_surface_ready;
    int gameover_capture_surface_ready;
    int title_status_ready;
    int warning_status_ready;
    int save_status_ready;
    int champion_status_ready;
    int boot_warning_title_ready;
    int startup_surfaces_real_ready;
    int faces_real_ready;
    int menu_bpk_route_ready;
    int save_menu_route_ready;
    int champion_menu_route_ready;
    int audio_track02_ready;
    int cd_track;
    Nexus_SfxRuntimeStatus sfx_status;
    int sfx_blocks_real_playback;
    int full_start_graphics_ready;
    int full_start_menu_ready;
    int m11_host_route_ready;
    Nexus_V1_LauncherM11MenuBpkNoDrawHostReceipt
        m11_menu_bpk_no_draw_host;
    Nexus_V1_LauncherSaturnCardM11NoDrawStartupReceipt
        saturn_card_m11_no_draw_startup;
    int fallback_visuals_permitted;
    const char *m11_host_route;
    const char *startup_ui_blocker;
    const char *asset_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupFullStartReceipt;

typedef struct {
    Nexus_V1_StartupFullStartReceipt full_start;
    Nexus_V1_StartupTitleHandoffReceipt title_handoff;
    Nexus_V1_StartupMenuPresentationReceipt presentation;
    Nexus_V1_StartupSaveRouteReceipt save_route;
    int title_handoff_valid;
    int presentation_valid;
    int save_route_valid;
    int m11_ready;
    int m12_ready;
    int redraw;
    int draw_command_count;
    int save_row_count;
    int selected_row;
    const char *consumer_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupFullStartConsumerReceipt;

typedef enum {
    NEXUS_V1_STARTUP_CAPTURE_INVALID = 0,
    NEXUS_V1_STARTUP_CAPTURE_BLOCKED = 1,
    NEXUS_V1_STARTUP_CAPTURE_TITLE = 2,
    NEXUS_V1_STARTUP_CAPTURE_SAVE = 3,
    NEXUS_V1_STARTUP_CAPTURE_CHAMPION = 4,
    NEXUS_V1_STARTUP_CAPTURE_MENU_IDLE = 5
} Nexus_V1_StartupCaptureRoute;

typedef struct {
    Nexus_V1_StartupFullStartConsumerReceipt consumer;
    char phase[32];
    char animation[32];
    int startup_active;
    int startup_frame;
    int animation_active;
    int title_frame;
    int title_frame_max;
    int title_ready;
    int boot_warning_frames;
    int boot_start_ready_frames;
    int boot_frame_in_phase;
    int title_frames_until_ready;
    int title_hold_frame;
    int title_prompt_visible;
    int title_reveal_y0;
    int title_reveal_y1;
    int title_reveal_h;
    int warning_surface_loaded;
    int title_surface_loaded;
    int gameover_surface_loaded;
    int warning_capture_surface_ready;
    int title_capture_surface_ready;
    int gameover_capture_surface_ready;
    int warning_capture_frame;
    int title_capture_frame;
    int save_capture_frame;
    int champion_capture_frame;
    int dungeon_capture_frame;
    int gameover_capture_frame;
    int saturn_warning_frame;
    int saturn_title_capture_frame;
    int saturn_save_capture_frame;
    int saturn_champion_capture_frame;
    int saturn_dungeon_capture_frame;
    int saturn_title_ready_frame;
    int saturn_gameover_capture_frame;
    int saturn_timing_exact;
    int saturn_capture_frames_exact;
    int package_route_matches_capture_route;
    int full_start_package_receipt_ready;
    int host_display_caller_expected;
    int route_ready;
    int m11_ready;
    int m12_ready;
    int graphics_ready;
    int audio_ready;
    int save_menu_ready;
    int champion_menu_ready;
    int fallback_visuals_permitted;
    int capture_valid;
    int capture_route_ready;
    int capture_command_count;
    int title_capture_ready;
    int save_capture_ready;
    int champion_capture_ready;
    int blocked_draw_suppressed;
    int title_route_active;
    int save_route_active;
    int champion_route_active;
    int menu_idle_active;
    int warning_visible;
    Nexus_V1_StartupCaptureRoute capture_route;
    Nexus_V1_StartupDrawKind first_capture_draw_kind;
    const char *capture_route_expected_consumer_route;
    const char *consumer_route;
    const char *asset_route;
    const char *startup_ui_blocker;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupFullStartPackageReceipt;

typedef struct {
    int handled;
    int supported;
    int data_ready;
    int version_ready;
    int startup_menu_ready;
    int full_start_graphics_ready;
    int startup_contract_ready;
    int packaged_capture_expected;
    int packaged_capture_ready;
    int startup_step_count;
    int startup_step_ready_count;
    int boot_warning_frames;
    int boot_start_ready_frames;
    int title_frame_max;
    int title_frames_until_ready;
    int title_hold_frame;
    int title_prompt_visible;
    int title_reveal_y0;
    int title_reveal_y1;
    int title_reveal_h;
    int warning_surface_loaded;
    int title_surface_loaded;
    int gameover_surface_loaded;
    int warning_capture_surface_ready;
    int title_capture_surface_ready;
    int gameover_capture_surface_ready;
    int warning_capture_frame;
    int title_capture_frame;
    int save_capture_frame;
    int champion_capture_frame;
    int dungeon_capture_frame;
    int gameover_capture_frame;
    int saturn_warning_frame;
    int saturn_title_capture_frame;
    int saturn_save_capture_frame;
    int saturn_champion_capture_frame;
    int saturn_dungeon_capture_frame;
    int saturn_title_ready_frame;
    int saturn_gameover_capture_frame;
    int saturn_timing_exact;
    int saturn_capture_frames_exact;
    int full_start_package_receipt_ready;
    int host_display_caller_expected;
    int capture_command_count;
    Nexus_V1_StartupCaptureRoute capture_route;
    Nexus_V1_StartupDrawKind first_capture_draw_kind;
    const char *game_id;
    const char *card_title_label;
    const char *card_subtitle_label;
    const char *timing_summary_label;
    const char *ready_status_label;
    const char *ready_detail_label;
    const char *path_label;
    const char *contract_label;
    const char *capture_label;
    const char *capture_route_label;
    const char *first_capture_draw_label;
    const char *next_step_label;
    const char *active_proof_label;
    const char *status_label;
    const char *detail_label;
    const char *launch_status_label;
    const char *launch_detail_label;
    const char *blocked_status_label;
    const char *blocked_detail_label;
} Nexus_V1_M12StartupPackageReceipt;

typedef struct {
    Nexus_V1_StartupFullStartPackageReceipt package;
    Nexus_V1_M12StartupPackageReceipt m12_package;
    Nexus_V1_StartupCaptureRoute capture_route;
    Nexus_V1_StartupDrawKind first_draw_kind;
    int command_count;
    int max_commands;
    int copied_command_count;
    int timing_frame;
    int timing_frame_max;
    int timing_ready;
    int saturn_timing_exact;
    int saturn_capture_frames_exact;
    int active_capture_frame;
    int saturn_active_capture_frame;
    int warning_visible;
    int prompt_visible;
    int m11_ready;
    int m12_ready;
    int capture_ready;
    int display_ready;
    int blocked;
    int fallback_visuals_permitted;
    const char *route_label;
    const char *first_draw_label;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupReceiptBundle;

typedef enum {
    NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_INVALID = 0,
    NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_BLOCKED_ASSETS = 1,
    NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_TITLE_CAPTURE = 2,
    NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_MENU_CAPTURE = 3,
    NEXUS_V1_STARTUP_REAL_ASSET_OWNERSHIP_RUNTIME_HANDOFF = 4
} Nexus_V1_StartupRealAssetOwnershipRoute;

typedef struct {
    Nexus_V1_StartupRealAssetOwnershipRoute route;
    Nexus_V1_StartupReceiptBundle startup_bundle;
    Nexus_V1_StartupAssetHandoffReceipt asset_handoff;
    Nexus_V1_StartupRuntimeRouteReceipt runtime_route;
    Nexus_V1_MenuBpkRendererHandoffReceipt menu_bpk_handoff;
    Nexus_V1_DgnRendererHandoffReceipt dgn_handoff;
    Nexus_V1_DgnRenderPlanReceipt dgn_render_plan;
    Nexus_V1_DgnStaticMaterialSourceReceipt static_material_sources;
    Nexus_V1_DgnRenderCommand dgn_commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    int receipt_owner_is_nexus;
    int title_menu_receipt_owned;
    int capture_receipt_owned;
    int real_asset_receipt_owned;
    int consumes_bpk_menu_handoff;
    int consumes_prs3_blocker;
    int consumes_dgn_handoff;
    int title_capture_uses_real_assets;
    int menu_capture_uses_real_assets;
    int full_start_package_consumed;
    int package_capture_consumed_by_host;
    int package_route_matches_capture_route;
    int host_route_consumes_package_route;
    int host_route_consumes_capture_matrix;
    int dgn_route_consumes_startup_package;
    int dgn_route_saturn_capture_exact;
    int host_ownership_route_matches_capture_route;
    int package_route_consumes_host_ownership;
    int dgn_route_consumes_host_ownership;
    int startup_route_consumption_complete;
    int non_title_saturn_capture_route_complete;
    int dungeon_startup_route_consumption_complete;
    int startup_route_consumes_package_capture;
    int title_route_consumes_package_capture;
    int save_route_consumes_package_capture;
    int champion_route_consumes_package_capture;
    int dungeon_route_consumes_package_capture;
    int startup_route_saturn_capture_exact;
    int title_route_saturn_capture_exact;
    int save_route_saturn_capture_exact;
    int champion_route_saturn_capture_exact;
    int dungeon_route_saturn_capture_exact;
    int dungeon_capture_route_consumed;
    int startup_host_package_route_complete;
    int title_host_package_route_complete;
    int save_host_package_route_complete;
    int champion_host_package_route_complete;
    int dungeon_host_package_route_complete;
    unsigned int host_package_route_complete_mask;
    unsigned int host_package_route_expected_mask;
    int host_package_route_matrix_complete;
    unsigned int host_saturn_exact_capture_mask;
    int host_saturn_route_timing_matrix_complete;
    int host_package_route_timing_matrix_complete;
    unsigned int host_all_route_complete_mask;
    unsigned int host_all_route_expected_mask;
    int host_all_route_matrix_complete;
    unsigned int host_saturn_all_exact_capture_mask;
    unsigned int host_saturn_all_expected_capture_mask;
    int host_saturn_all_route_timing_matrix_complete;
    int host_all_route_timing_matrix_complete;
    int title_menu_capture_route_joined;
    int bpk_menu_route_joined;
    int runtime_dgn_route_joined;
    int first_host_draw_uses_package;
    int blocked_route_suppresses_startup_draws;
    int blocked_route_suppresses_dgn_draws;
    int saturn_timing_exact;
    int saturn_capture_frames_exact;
    int active_capture_frame;
    int saturn_active_capture_frame;
    int host_route_consumes_active_capture_frame;
    int host_route_consumes_dungeon_capture_frame;
    int host_route_capture_matrix_ready;
    int host_route_capture_matrix_exact;
    int host_saturn_non_title_capture_count;
    unsigned int host_saturn_non_title_capture_mask;
    unsigned int host_saturn_expected_capture_mask;
    int saturn_save_capture_frame;
    int saturn_champion_capture_frame;
    int saturn_dungeon_capture_frame;
    int runtime_dgn_handoff_ready;
    int runtime_dgn_viewport_render_ready;
    int no_fallback_visuals_enforced;
    int fallback_visuals_permitted;
    int blocked_draw_suppressed;
    int capture_ready;
    int display_ready;
    int startup_draw_command_count;
    int dgn_draw_command_count;
    int dgn_viewport_rasterized_command_count;
    int dgn_viewport_written_pixels;
    int dgn_viewport_material_surface_count;
    int dgn_viewport_floor_material_surface_count;
    int dgn_viewport_ceiling_material_surface_count;
    int dgn_viewport_wall_material_surface_count;
    int dgn_viewport_host_route_status;
    int dgn_viewport_host_route_ready;
    int dgn_viewport_host_route_consumed;
    int dgn_viewport_host_route_package_consumed;
    int dgn_viewport_host_route_blocks_runtime;
    int dgn_viewport_capture_ready;
    uint32_t dgn_viewport_frame_hash;
    int dgn_material_surface_coverage_complete;
    int dgn_material_semantics_complete;
    int dgn_static_material_source_consumed;
    int runtime_dgn_material_path_consumed;
    int host_route_consumes_dgn_material_path;
    int bpk_material_surface_count;
    int bpk_truecolor_material_surface_count;
    int bpk_prs3_material_surface_count;
    int copied_dgn_command_count;
    Nexus_V1_StartupCaptureRoute capture_route;
    Nexus_V1_StartupDrawKind first_startup_draw_kind;
    Nexus_V1_DgnRenderCommandKind first_dgn_draw_kind;
    const char *receipt_owner;
    const char *dungeon_capture_route;
    const char *asset_route;
    const char *asset_blocker;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupRealAssetOwnershipReceipt;

typedef struct {
    int active_frame;
    int expected_title_frame;
    int warning_boundary;
    int title_boundary;
    int start_ready_boundary;
    int warning_surface_verified;
    int title_surface_verified;
    int timing_verified;
    int menu_bpk_prs3_blocked;
    Nexus_V1_StartupDrawKind expected_draw_kind;
    int command_verified;
    int consumer_ready;
    const char *status;
} Nexus_V1_StartupTitleTransitionCaptureReceipt;

typedef struct {
    Nexus_V1_StartupRealAssetOwnershipReceipt ownership;
    int receipt_owner_is_nexus;
    int host_caller_ready;
    int host_startup_capture_ready;
    int host_runtime_dgn_ready;
    /* DM.BIN geometry provenance only; this does not authorize HUD pixels. */
    int hud_geometry_source_bound;
    int hud_layout_count;
    int hud_hit_rect_count;
    int host_runtime_dgn_viewport_render_ready;
    int host_execute_startup_draws;
    int host_execute_dgn_draws;
    int bpk_handoff_consumed;
    int prs3_blocker_consumed;
    int dgn_handoff_consumed;
    int suppress_fallback_visuals;
    int suppress_legacy_placeholder_visuals;
    int no_fallback_visuals_enforced;
    int startup_command_count;
    int copied_startup_command_count;
    int dgn_command_count;
    int dgn_viewport_rasterized_command_count;
    int dgn_viewport_written_pixels;
    int dgn_viewport_material_surface_count;
    int dgn_viewport_floor_material_surface_count;
    int dgn_viewport_ceiling_material_surface_count;
    int dgn_viewport_wall_material_surface_count;
    int dgn_viewport_host_route_status;
    int dgn_viewport_host_route_ready;
    int dgn_viewport_host_route_consumed;
    int dgn_viewport_host_route_package_consumed;
    int dgn_viewport_host_route_blocks_runtime;
    int dgn_viewport_capture_ready;
    uint32_t dgn_viewport_frame_hash;
    int dgn_material_surface_coverage_complete;
    int dgn_material_semantics_complete;
    int host_runtime_dgn_material_path_consumed;
    int host_route_consumes_dgn_material_path;
    int bpk_material_surface_count;
    int bpk_truecolor_material_surface_count;
    int bpk_prs3_material_surface_count;
    int copied_dgn_command_count;
    int copied_dgn_material_plan_complete;
    int title_timing_frame;
    int title_timing_frame_max;
    int title_timing_ready;
    int full_start_package_consumed;
    int package_capture_consumed_by_host;
    int package_route_matches_capture_route;
    int host_route_consumes_package_route;
    int host_route_consumes_capture_matrix;
    int dgn_route_consumes_startup_package;
    int dgn_route_saturn_capture_exact;
    int dungeon_capture_route_consumed;
    int host_ownership_route_matches_capture_route;
    int package_route_consumes_host_ownership;
    int dgn_route_consumes_host_ownership;
    int startup_route_consumption_complete;
    int non_title_saturn_capture_route_complete;
    int dungeon_startup_route_consumption_complete;
    int startup_route_consumes_package_capture;
    int title_route_consumes_package_capture;
    int save_route_consumes_package_capture;
    int champion_route_consumes_package_capture;
    int dungeon_route_consumes_package_capture;
    int startup_route_saturn_capture_exact;
    int title_route_saturn_capture_exact;
    int save_route_saturn_capture_exact;
    int champion_route_saturn_capture_exact;
    int dungeon_route_saturn_capture_exact;
    int startup_host_package_route_complete;
    int title_host_package_route_complete;
    int save_host_package_route_complete;
    int champion_host_package_route_complete;
    int dungeon_host_package_route_complete;
    unsigned int host_package_route_complete_mask;
    unsigned int host_package_route_expected_mask;
    int host_package_route_matrix_complete;
    unsigned int host_saturn_exact_capture_mask;
    int host_saturn_route_timing_matrix_complete;
    int host_package_route_timing_matrix_complete;
    unsigned int host_all_route_complete_mask;
    unsigned int host_all_route_expected_mask;
    int host_all_route_matrix_complete;
    unsigned int host_saturn_all_exact_capture_mask;
    unsigned int host_saturn_all_expected_capture_mask;
    int host_saturn_all_route_timing_matrix_complete;
    int host_all_route_timing_matrix_complete;
    int startup_bundle_consumed;
    int display_callers_use_package_receipt;
    int single_saturn_startup_owner_ready;
    int title_menu_capture_route_joined;
    int runtime_dgn_route_joined;
    int blocked_route_suppresses_all_draws;
    int saturn_warning_frame;
    int saturn_title_capture_frame;
    int saturn_save_capture_frame;
    int saturn_champion_capture_frame;
    int saturn_dungeon_capture_frame;
    int saturn_title_ready_frame;
    int saturn_gameover_capture_frame;
    int saturn_timing_exact;
    int saturn_capture_frames_exact;
    int host_active_capture_frame;
    int host_saturn_active_capture_frame;
    int host_route_consumes_active_capture_frame;
    int host_route_consumes_dungeon_capture_frame;
    int host_route_capture_matrix_ready;
    int host_route_capture_matrix_exact;
    int host_saturn_non_title_capture_count;
    unsigned int host_saturn_non_title_capture_mask;
    unsigned int host_saturn_expected_capture_mask;
    Nexus_V1_StartupCaptureRoute capture_route;
    Nexus_V1_StartupRealAssetOwnershipRoute ownership_route;
    const char *host_route;
    const char *dungeon_capture_route;
    const char *startup_package_route;
    const char *status_scope;
    const char *status;
} Nexus_V1_StartupHostCallerReceipt;

void nexus_v1_launcher_startup_title_transition_capture_receipt_clear(
    Nexus_V1_StartupTitleTransitionCaptureReceipt *receipt);
int nexus_v1_launcher_startup_title_transition_capture_receipt_from_host(
    const Nexus_V1_StartupHostCallerReceipt *host_receipt,
    int title_frame,
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    Nexus_V1_StartupTitleTransitionCaptureReceipt *out_receipt);

typedef struct {
    Nexus_V1_StartupHostCallerReceipt title_host;
    Nexus_V1_StartupHostCallerReceipt save_host;
    Nexus_V1_StartupHostCallerReceipt champion_host;
    int title_route_complete;
    int save_route_complete;
    int champion_route_complete;
    int dungeon_route_complete;
    int dungeon_capture_route_consumed;
    int dgn_material_surface_coverage_complete;
    int dgn_material_semantics_complete;
    int dgn_material_path_consumed;
    int bpk_material_surface_count;
    int bpk_truecolor_material_surface_count;
    int bpk_prs3_material_surface_count;
    int dgn_mesh_runtime_complete;
    int dgn_viewport_runtime_complete;
    int dgn_viewport_host_route_status;
    int dgn_viewport_host_route_ready;
    int dgn_viewport_host_route_consumed;
    int dgn_viewport_host_route_package_consumed;
    int dgn_viewport_host_route_blocks_runtime;
    int dgn_viewport_capture_ready;
    uint32_t dgn_viewport_frame_hash;
    int startup_package_consumed_by_all_routes;
    int host_route_matrix_complete;
    int saturn_timing_matrix_complete;
    int no_fallback_visuals_enforced;
    int fallback_visuals_permitted;
    unsigned int complete_route_mask;
    unsigned int expected_route_mask;
    int saturn_title_capture_frame;
    int saturn_save_capture_frame;
    int saturn_champion_capture_frame;
    int saturn_dungeon_capture_frame;
    unsigned int saturn_non_title_capture_mask;
    unsigned int saturn_expected_capture_mask;
    int saturn_non_title_capture_count;
    int saturn_non_title_capture_complete;
    int all_nexus_startup_routes_complete;
    int all_nexus_runtime_routes_complete;
    int complete_support_ready;
    const char *status_scope;
    const char *status;
} Nexus_V1_CompleteSupportReceipt;

void nexus_v1_launcher_startup_runtime_state_clear(
    Nexus_V1_StartupRuntimeState *state);
void nexus_v1_launcher_runtime_startup_snapshot_clear(
    Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot);
void nexus_v1_launcher_startup_menu_presentation_receipt_clear(
    Nexus_V1_StartupMenuPresentationReceipt *receipt);
void nexus_v1_launcher_startup_title_handoff_receipt_clear(
    Nexus_V1_StartupTitleHandoffReceipt *receipt);
void nexus_v1_launcher_startup_runtime_handoff_receipt_clear(
    Nexus_V1_StartupRuntimeHandoffReceipt *receipt);
void nexus_v1_launcher_startup_runtime_route_receipt_clear(
    Nexus_V1_StartupRuntimeRouteReceipt *receipt);
void nexus_v1_launcher_startup_route_proof_receipt_clear(
    Nexus_V1_StartupRouteProofReceipt *receipt);
void nexus_v1_launcher_startup_full_start_receipt_clear(
    Nexus_V1_StartupFullStartReceipt *receipt);
void nexus_v1_launcher_startup_full_start_consumer_receipt_clear(
    Nexus_V1_StartupFullStartConsumerReceipt *receipt);
void nexus_v1_launcher_startup_full_start_package_receipt_clear(
    Nexus_V1_StartupFullStartPackageReceipt *receipt);
void nexus_v1_launcher_m12_startup_package_receipt_clear(
    Nexus_V1_M12StartupPackageReceipt *receipt);
void nexus_v1_launcher_startup_receipt_bundle_clear(
    Nexus_V1_StartupReceiptBundle *receipt);
void nexus_v1_launcher_startup_real_asset_ownership_receipt_clear(
    Nexus_V1_StartupRealAssetOwnershipReceipt *receipt);
void nexus_v1_launcher_startup_host_caller_receipt_clear(
    Nexus_V1_StartupHostCallerReceipt *receipt);
void nexus_v1_launcher_complete_support_receipt_clear(
    Nexus_V1_CompleteSupportReceipt *receipt);
const char *nexus_v1_launcher_startup_real_asset_ownership_route_name(
    Nexus_V1_StartupRealAssetOwnershipRoute route);
int nexus_v1_launcher_complete_support_receipt_from_host_routes(
    const Nexus_V1_StartupHostCallerReceipt *title_host,
    const Nexus_V1_StartupHostCallerReceipt *save_host,
    const Nexus_V1_StartupHostCallerReceipt *champion_host,
    Nexus_V1_CompleteSupportReceipt *out_receipt);
int nexus_v1_launcher_m12_startup_package_from_data_gate(
    int supported,
    int data_ready,
    int version_ready,
    Nexus_V1_M12StartupPackageReceipt *out_receipt);
int nexus_v1_launcher_m12_startup_package_from_full_start_package(
    const Nexus_V1_StartupFullStartPackageReceipt *package,
    Nexus_V1_M12StartupPackageReceipt *out_receipt);
int nexus_v1_launcher_startup_host_facts_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupHostFacts *out_facts);
int nexus_v1_launcher_startup_host_facts_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupHostFacts *out_facts);
int nexus_v1_launcher_startup_assets_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt);
int nexus_v1_launcher_startup_assets_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_LauncherStartupAssetsReceipt *out_receipt);
int nexus_v1_launcher_startup_advance_idle_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupIdleReceipt *out_receipt);
int nexus_v1_launcher_startup_advance_idle_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupIdleReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_save_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_save_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_save_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_save_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_save_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupSaveRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_title_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_title_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_route_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_route_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_title_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_title_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_title_pointer_handoff_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupTitleHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_firestaff_input_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_pointer_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
int nexus_v1_launcher_startup_execute_champion_pointer_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_StartupChampionExecution *out_execution,
    Nexus_V1_StartupHostActionReceipt *out_receipt);
const char *nexus_v1_launcher_startup_runtime_handoff_route_name(
    Nexus_V1_StartupRuntimeHandoffRoute route);
const char *nexus_v1_launcher_dgn_visual_blocker_from_render_plan(
    const Nexus_V1_DgnRenderPlanReceipt *render_plan);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_execution_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_handoff_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeHandoffReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_route_from_champion_execution(
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input(
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_route_from_champion_pointer(
    const Nexus_V1_StartupRuntimeState *state,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_route_from_champion_firestaff_input_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt);
int nexus_v1_launcher_startup_runtime_route_from_champion_pointer_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRuntimeRouteReceipt *out_receipt);
const char *nexus_v1_launcher_startup_route_proof_route_name(
    Nexus_V1_StartupRouteProofRoute route);
int nexus_v1_launcher_startup_route_proof_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt);
int nexus_v1_launcher_startup_route_proof_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    const Nexus_V1_StartupChampionExecution *execution,
    const Nexus_V1_StartupHostActionReceipt *host_action,
    Nexus_V1_DgnRenderCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupRouteProofReceipt *out_receipt);
const char *nexus_v1_launcher_startup_full_start_route_name(
    Nexus_V1_StartupFullStartRoute route);
int nexus_v1_launcher_startup_full_start_receipt_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupFullStartReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupFullStartReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_consumer_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartConsumerReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_consumer_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartConsumerReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_package_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_package_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_package_export_presentation(
    const Nexus_V1_StartupFullStartPackageReceipt *package,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int nexus_v1_launcher_startup_full_start_package_build_commands_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt);
int nexus_v1_launcher_startup_full_start_package_build_commands_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupFullStartPackageReceipt *out_receipt);
int nexus_v1_launcher_startup_receipt_bundle_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupReceiptBundle *out_receipt);
int nexus_v1_launcher_startup_receipt_bundle_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupReceiptBundle *out_receipt);
int nexus_v1_launcher_startup_real_asset_ownership_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupRealAssetOwnershipReceipt *out_receipt);
int nexus_v1_launcher_startup_real_asset_ownership_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupRealAssetOwnershipReceipt *out_receipt);
int nexus_v1_launcher_startup_host_caller_receipt_from_runtime_state(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_StartupRuntimeState *state,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_startup_commands,
    int max_startup_commands,
    Nexus_V1_DgnRenderCommand *out_dgn_commands,
    int max_dgn_commands,
    Nexus_V1_StartupHostCallerReceipt *out_receipt);
int nexus_v1_launcher_startup_host_caller_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeReceipt *runtime,
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    int menu_input,
    Nexus_V1_StartupLoadSaveFn load_save,
    void *load_userdata,
    Nexus_V1_StartupDrawCommand *out_startup_commands,
    int max_startup_commands,
    Nexus_V1_DgnRenderCommand *out_dgn_commands,
    int max_dgn_commands,
    Nexus_V1_StartupHostCallerReceipt *out_receipt);
int nexus_v1_launcher_startup_presentation_build_save_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_save_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_save_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_save_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_presentation_build_champion_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_presentation_build_champion_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands);
int nexus_v1_launcher_startup_champion_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_champion_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    Nexus_V1_StartupDrawCommand *out_commands,
    int max_commands,
    Nexus_V1_StartupMenuPresentationReceipt *out_receipt);
int nexus_v1_launcher_startup_presentation_execute(
    const Nexus_V1_StartupDrawCommand *commands,
    int command_count,
    const Nexus_V1_StartupDrawExecutor *executor);
int nexus_v1_launcher_startup_presentation_receipt_from_runtime_state(
    const Nexus_V1_StartupRuntimeState *state,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int nexus_v1_launcher_startup_presentation_receipt_from_snapshot(
    const Nexus_V1_LauncherRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int nexus_v1_launcher_startup_resume_status_host_receipt(
    Nexus_V1_StartupResumeStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt);
int nexus_v1_launcher_startup_boot_status_host_receipt(
    Nexus_V1_StartupBootStatus status,
    Nexus_V1_StartupHostReceipt *out_receipt);

int nexus_v1_launcher_boot_level0_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherBootReceipt *out_receipt);
int nexus_v1_launcher_boot_level0_runtime_startup(
    const char *data_dir,
    Nexus_TitleScreen *title,
    Nexus_V1_LauncherRuntimeReceipt *out_receipt);
int nexus_v1_launcher_resume_from_save_path(
    const char *save_path,
    Nexus_V1_LightRuntime *light_runtime,
    Nexus_V1_LauncherResumeReceipt *out_receipt);

/* Shutdown the launcher and free the engine singleton.
 * Safe to call multiple times. */
void nexus_v1_launcher_shutdown(void);

#ifdef __cplusplus
}
#endif


#endif /* NEXUS_V1_LAUNCHER_H */
