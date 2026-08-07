#ifndef DM1_V1_FMTOWNS_TMENU_INPUT_H
#define DM1_V1_FMTOWNS_TMENU_INPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns TMENU.EXP input pipeline schema.
 *
 * Recovered 2026-08-07 by direct disassembly of the shipping
 * HMA-240 Japanese Track 01 TMENU.EXP (235,476 bytes, Phar Lap P3
 * level-1 executable, load offset 0x200, init EIP 0x9408). Every
 * vaddr below is byte-verified against the disc image; no synthetic
 * addresses.
 *
 * TMENU's runtime is a classic poll-dispatch loop at 0x943b..0x9458:
 *
 *   loop:
 *     call SETUP2       ; 0x9aac -- reset queue registers, INT21/AH=19
 *     call POLL_RESUME  ; 0x99ec -- resume pending event handler
 *     call POLL_MAIN    ; 0xc8e0 -- TownsOS input poll
 *     cmp  [ACTIVE], 0  ; 0x557c
 *     je   POLL_MAIN
 *     call EVENT_DISPATCH ; 0xbfac -- dispatch queued event
 *     jmp  SETUP2
 *
 * POLL_MAIN reads TownsOS input state via TBIOS_POLL (0xa130), which
 * bridges from Phar Lap 386|DOS-Extender protected mode into real
 * mode via `mov ax, 0x250f; int 0x21` + `call fs:[0x20]` (fs=0x110
 * real-mode selector) and returns TBIOS status packet at 0x2dbc.
 *
 * The polled state fields at 0x2e52..0x2e78 hold:
 *   0x2e52 : dword event-slot-0 status
 *   0x2e56 : dword sub-field 1 (from 0x2e77)
 *   0x2e5a : dword sub-field 2 (from 0x2e78)
 *   0x2e5e : dword event-slot-1 status
 *   0x2e62 : dword mouse X (from word 0x2e72)
 *   0x2e66 : dword mouse Y low (from byte 0x2e74)
 *   0x2e6a : dword mouse Y mid (from byte 0x2e75)
 *   0x2e6e : dword mouse Y high (from byte 0x2e76)
 *
 * EVENT_DISPATCH consumes queue head at [0x5890] and walks 8-byte
 * event records at [0x575c + eax * 8]; the record's +4 field is a
 * pointer that gets its first word cleared (queue consumed). Text
 * for each event is at [0x3bca + 2 + eax * 10] (10-byte-stride
 * text pool base with 2-byte per-entry header).
 *
 * Event handler dispatch table is at [0x5510 + eax * 4] — dword
 * function pointers indexed by the resolved event id.
 *
 * NB: This module ships ONLY the source-verified schema (function
 * vaddrs, data addresses, table strides, poll semantics). It does
 * NOT implement input processing — full TMENU runtime would require
 * bridging back into TownsOS TBIOS via `fs:[0x20]` real-mode calls,
 * which no consumer can do without an actual TownsOS host.
 *
 * Downstream Firestaff consumers use this schema to:
 *   * Prove a decoded TMENU.EXP image byte-matches the recovered
 *     addresses (via the round-trip test in the tests directory).
 *   * Document why FM Towns menu input remains fail-closed (no
 *     synthetic mouse events; the real path requires host BIOS).
 */

/* Phar Lap P3 header fields (level-1). All offsets from file start. */
#define DM1_V1_FMTOWNS_TMENU_HEADER_SIZE_BYTES          0x180U
#define DM1_V1_FMTOWNS_TMENU_LOAD_IMAGE_OFFSET          0x200U

/* Verified vaddrs (relative to load image base). Every callable is
 * a real function body confirmed by capstone i386 protected-mode
 * disassembly against the shipping Track 01 TMENU.EXP. */
#define DM1_V1_FMTOWNS_TMENU_INIT_EIP_VADDR             0x9408U
#define DM1_V1_FMTOWNS_TMENU_SETUP1_VADDR               0x99e0U
#define DM1_V1_FMTOWNS_TMENU_SETUP2_VADDR               0x9aacU
#define DM1_V1_FMTOWNS_TMENU_POLL_RESUME_VADDR          0x99ecU
#define DM1_V1_FMTOWNS_TMENU_POLL_MAIN_VADDR            0xc8e0U
#define DM1_V1_FMTOWNS_TMENU_EVENT_DISPATCH_VADDR       0xbfacU
#define DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR           0xa130U
#define DM1_V1_FMTOWNS_TMENU_EVENT_HELPER_VADDR         0x30aaU
#define DM1_V1_FMTOWNS_TMENU_EVENT_HELPER_AH8_VADDR     0x30d0U

/* Data addresses. */
#define DM1_V1_FMTOWNS_TMENU_TBIOS_STATUS_PACKET_VADDR  0x2dbcU
#define DM1_V1_FMTOWNS_TMENU_ACTIVE_FLAG_VADDR          0x557cU
#define DM1_V1_FMTOWNS_TMENU_ACTIVE_SAVED_VADDR         0x5580U
#define DM1_V1_FMTOWNS_TMENU_CURRENT_RECORD_VADDR       0x5578U
#define DM1_V1_FMTOWNS_TMENU_HANDLER_TABLE_VADDR        0x5510U
#define DM1_V1_FMTOWNS_TMENU_EVENT_QUEUE_HEAD_VADDR     0x5890U
#define DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_BASE_VADDR    0x575cU
#define DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_POOL_VADDR      0x3bcaU
#define DM1_V1_FMTOWNS_TMENU_EVENT_TARGET_ID_VADDR      0x3dceU

/* Polled TBIOS state field offsets (relative to base 0x2e52). */
#define DM1_V1_FMTOWNS_TMENU_STATE_BASE_VADDR           0x2e52U
#define DM1_V1_FMTOWNS_TMENU_STATE_SLOT0_STATUS_OFFSET  0x00U
#define DM1_V1_FMTOWNS_TMENU_STATE_SUB1_OFFSET          0x04U
#define DM1_V1_FMTOWNS_TMENU_STATE_SUB2_OFFSET          0x08U
#define DM1_V1_FMTOWNS_TMENU_STATE_SLOT1_STATUS_OFFSET  0x0cU
#define DM1_V1_FMTOWNS_TMENU_STATE_MOUSE_X_OFFSET       0x10U
#define DM1_V1_FMTOWNS_TMENU_STATE_MOUSE_Y_LO_OFFSET    0x14U
#define DM1_V1_FMTOWNS_TMENU_STATE_MOUSE_Y_MID_OFFSET   0x18U
#define DM1_V1_FMTOWNS_TMENU_STATE_MOUSE_Y_HI_OFFSET    0x1cU

/* Source status packet raw byte offsets (relative to 0x2dbc base). */
#define DM1_V1_FMTOWNS_TMENU_RAW_MOUSE_X_WORD_VADDR     0x2e72U
#define DM1_V1_FMTOWNS_TMENU_RAW_MOUSE_Y_LO_BYTE_VADDR  0x2e74U
#define DM1_V1_FMTOWNS_TMENU_RAW_MOUSE_Y_MID_BYTE_VADDR 0x2e75U
#define DM1_V1_FMTOWNS_TMENU_RAW_MOUSE_Y_HI_BYTE_VADDR  0x2e76U
#define DM1_V1_FMTOWNS_TMENU_RAW_SUB1_BYTE_VADDR        0x2e77U
#define DM1_V1_FMTOWNS_TMENU_RAW_SUB2_BYTE_VADDR        0x2e78U

/* Event record stride (bytes). */
#define DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_STRIDE_BYTES  8U

/* Event text pool stride (bytes per record). */
#define DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_STRIDE_BYTES    10U

/* Event text pool header size (bytes before the first glyph). */
#define DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_HEADER_BYTES    2U

/* Phar Lap real-mode selector used for `call fs:[0x20]`. */
#define DM1_V1_FMTOWNS_TMENU_PHARLAP_REALMODE_SELECTOR  0x110U

/* Return the vaddr of the event record for `event_slot`. Returns
 * 0 if slot exceeds a bounded 4096-slot ceiling (which no real
 * TMENU session ever approaches — the queue is a small ring). */
uint32_t dm1_v1_fmtowns_tmenu_event_record_vaddr_pc34(uint32_t event_slot);

/* Return the vaddr of the event text buffer for `event_slot`,
 * including the 2-byte header offset. Returns 0 for out-of-range. */
uint32_t dm1_v1_fmtowns_tmenu_event_text_vaddr_pc34(uint32_t event_slot);

/* Return the vaddr of the handler function pointer for `event_id`.
 * Returns 0 for out-of-range. The dispatch is `call [table + id*4]`. */
uint32_t dm1_v1_fmtowns_tmenu_handler_vaddr_pc34(uint32_t event_id);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_TMENU_INPUT_H */
