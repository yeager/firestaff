; Theron's Quest US Track 02 — static RNG-consumer body boundaries
;
; These are two US-BIN source windows statically associated with the $4644
; preconsumer. They are source evidence only: runtime execution, HuC6280 bank
; selection, RAM-loaded helper state and semantic return-value ownership still
; require a valid live-game capture before any host RNG or spawn path may
; consume them.
;
; Source image: raw 2352-byte Track 02 BIN, MD5
; f23601102138f87c33025877767ebf76.
;
; $C96B -> $CA69 inclusive: 255 bytes, raw offset $A47EB,
; FNV-1a 0xE689C658.  The span ends at the first RTS of this entry.
; Its visible body calls $45E3, $5C75, $5DC9, $D3CF, $5147, $511F,
; $5D78, $D597, $5D52 and $4540, so a byte match alone does not establish
; which bank owns those callees at runtime.
;
; $CC4C -> $CD13 inclusive: 200 bytes, raw offset $A4ACC,
; FNV-1a 0x4AD0801E. The window contains an RTS at $CC6A and continues with
; a separately decoded region at $CC6B; the final byte at $CD13 is PLY, not
; an RTS. See theron-us-cc4c-consumer.asm for the linear MAME listing and its
; strict runtime/semantic limits. The byte span includes calls to $5B64,
; $4540, $45E3 and $5A76; static address operands do not prove their runtime
; bank mappings or call effects.
;
; Firestaff receipt fields:
;   Theron_V1Huc6280DisassemblyReceipt.spawn_rng_c96b_*
;   Theron_V1Huc6280DisassemblyReceipt.spawn_rng_cc4c_*
;
; Neither receipt field authorizes dynamic spawn, AI, loot or T700 state.
