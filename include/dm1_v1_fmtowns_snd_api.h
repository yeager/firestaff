#ifndef DM1_V1_FMTOWNS_SND_API_H
#define DM1_V1_FMTOWNS_SND_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 sound API surface (SND_*).
 *
 * Recovered from EDM.EXP SYM1 by grouping symbols under 'SND_'.
 * These are the byte-verified entry points DM1's sound subsystem
 * exposes for FM/PCM/CDDA/timer/volume control. Every vaddr is
 * a real function in the shipping EDM.EXP; a hosted-BIOS or
 * Tsugaru integration can hook these directly by name.
 *
 * The API mirrors the standard FM Towns sound driver interface
 * documented in the Fujitsu Technical Data Book — DM1 wraps each
 * hardware primitive through one of these functions.
 */

typedef struct {
    const char *name;
    uint32_t    vaddr;
    const char *category;   /* "core" | "fm" | "pcm" | "vol" | "cdda" */
    const char *purpose;
} dm1_v1_fmtowns_snd_api_entry_t;

#define DM1_V1_FMTOWNS_SND_API_COUNT  36U

extern const dm1_v1_fmtowns_snd_api_entry_t
    dm1_v1_fmtowns_snd_api_entries[DM1_V1_FMTOWNS_SND_API_COUNT];

/* Return the entry for `name`, NULL on miss. */
const dm1_v1_fmtowns_snd_api_entry_t *
dm1_v1_fmtowns_snd_api_lookup_pc34(const char *name);

/* Return the vaddr of a named SND API function, or 0. */
uint32_t dm1_v1_fmtowns_snd_api_vaddr_pc34(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_SND_API_H */
