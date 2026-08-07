#ifndef DM1_V1_FMTOWNS_PHARLAP_BRIDGE_H
#define DM1_V1_FMTOWNS_PHARLAP_BRIDGE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked Phar Lap 386|DOS-Extender real-mode bridge slots.
 *
 * Every DM1 FM Towns binary (EDM.EXP, JDM.EXP, TMENU.EXP) transitions
 * from 32-bit protected mode into real-mode BIOS via Phar Lap's
 * selector-based indirect call:
 *
 *   push fs
 *   push 0x110              ; Phar Lap real-mode data selector
 *   pop  fs
 *   call ptr fs:[slot]      ; ff 1d disp32, fs-prefixed
 *   pop  fs
 *
 * Four slots are used by the shipping DM1 disc, byte-verified by
 * scanning each Phar Lap image for the `64 ff 1d <disp32>` opcode
 * pattern:
 *
 *   Slot        EDM.EXP  JDM.EXP  TMENU.EXP  Purpose (verified)
 *   -----       -------  -------  ---------  ------------------
 *   fs:[0x20]     70       70       70       TBIOS (mouse/keyboard/timer/video)
 *   fs:[0x40]     20       20       20       Phar Lap secondary dispatch
 *   fs:[0x48]      1        1        1       rare — timing/exit
 *   fs:[0x80]      2        2        1       rare — hardware init
 *
 * Total call sites per binary: 93 (EDM), 93 (JDM), 92 (TMENU).
 *
 * The counts and slot addresses are BYTE-VERIFIED from the shipping
 * Japanese Rev 1 disc (SHA-256 hashes in
 * docs/dm1/fmtowns_real_data_hashes.json). Any DM1 FM Towns emulator
 * bridge that implements these four slots covers the entire runtime
 * requirement — no other real-mode transition escapes.
 */

#define DM1_V1_FMTOWNS_PHARLAP_REALMODE_SELECTOR   0x110U

/* The four verified dispatch slots. */
#define DM1_V1_FMTOWNS_PHARLAP_SLOT_TBIOS          0x20U   /* 70 calls */
#define DM1_V1_FMTOWNS_PHARLAP_SLOT_SECONDARY      0x40U   /* 20 calls */
#define DM1_V1_FMTOWNS_PHARLAP_SLOT_TIMING         0x48U   /*  1 call  */
#define DM1_V1_FMTOWNS_PHARLAP_SLOT_HARDWARE_INIT  0x80U   /*  1-2 calls */

/* Byte-verified call-site counts, per binary. */
typedef struct {
    uint32_t slot_tbios;
    uint32_t slot_secondary;
    uint32_t slot_timing;
    uint32_t slot_hardware_init;
    uint32_t total;
} dm1_v1_fmtowns_pharlap_call_profile_t;

/* Recovered profiles from the shipping Japanese Rev 1 disc. */
extern const dm1_v1_fmtowns_pharlap_call_profile_t
    dm1_v1_fmtowns_pharlap_profile_edm_exp_pc34;
extern const dm1_v1_fmtowns_pharlap_call_profile_t
    dm1_v1_fmtowns_pharlap_profile_jdm_exp_pc34;
extern const dm1_v1_fmtowns_pharlap_call_profile_t
    dm1_v1_fmtowns_pharlap_profile_tmenu_exp_pc34;

/* Return 1 if the four canonical slot addresses match what
 * DM1/CSB/DM2 FM Towns binaries expect, 0 otherwise. Pure
 * compile-time constant check — a downstream emulator bridge that
 * differs on any slot value must fail this gate before it can be
 * admitted as source-compatible. */
int dm1_v1_fmtowns_pharlap_slot_layout_is_valid_pc34(void);

/* Scan a Phar Lap P3 executable image for `call far fs:[disp32]`
 * instructions (opcode bytes 64 ff 1d disp32) and count how many
 * target each of the four canonical slots. Fills *out with the
 * observed counts. Returns 1 on success; 0 if inputs are NULL or
 * `image` is smaller than 7 bytes (min instruction size). Zero
 * out-slot counts are legitimate for tools/probes; the DM1 runtime
 * requires at least the TBIOS slot to be non-zero. */
int dm1_v1_fmtowns_pharlap_count_call_sites_pc34(
    const uint8_t                          *image,
    unsigned long                           image_size,
    dm1_v1_fmtowns_pharlap_call_profile_t  *out);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_PHARLAP_BRIDGE_H */
