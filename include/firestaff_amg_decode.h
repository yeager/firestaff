/*
 * firestaff_amg_decode.h
 *
 * CSB Utility Disk .AMG SND2 decoder.
 *
 * dmweb's Data Files reference identifies TELE2.AMG, SWIPE.AMG,
 * MAGEXPLO.AMG, EXPLOS1.AMG, and DRAGON.AMG from the CSB Amiga
 * Utility Disk as single-item data files of type SND2: a big-endian
 * sample-count word followed by signed 8-bit mono PCM and 0..3 trailing
 * bytes of unknown use.
 *
 * ReDMCSB SWSHSND.C:F0908_InitSound lines 17-23 and 137-143 show the
 * Amiga playback side: sound data is handed to audio.device with
 * ioa_Length and ioa_Period. The .AMG file does not carry a
 * period/speed value, so this decoder exposes only the raw SND2 payload
 * plus helper math for source-cited PAL/NTSC rate calculations when a
 * caller already knows the period or playback speed from game data.
 *
 * Scope:
 *   - Read-only parser/probe support.
 *   - No allocation and no SDL/runtime playback.
 *   - No write-side support.
 *   - Does not claim NAKED.AMG, which is not listed by dmweb as one of
 *     the single-item CSB Amiga SND2 .AMG sound-effect files.
 */

#ifndef FIRESTAFF_AMG_DECODE_H
#define FIRESTAFF_AMG_DECODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRESTAFF_AMG_SND2_HEADER_BYTES 2u
#define FIRESTAFF_AMG_SND2_MAX_TRAILING_BYTES 3u

#define FIRESTAFF_AMG_AMIGA_PAL_CLOCK_HZ  7093790u
#define FIRESTAFF_AMG_AMIGA_NTSC_CLOCK_HZ 7159090u

typedef struct {
    uint16_t sample_count;
    const int8_t* samples;       /* signed 8-bit mono PCM, owned by caller */
    size_t sample_bytes;         /* equals sample_count */
    const uint8_t* trailing;     /* 0..3 bytes, owned by caller */
    size_t trailing_bytes;
} FirestaffAmgSnd2;

/*
 * Decode a CSB Utility Disk .AMG file that is a single SND2 item.
 *
 * Returns:
 *   0  on success.
 *  -1  on invalid arguments or data shorter than the 2-byte header.
 *  -2  when the declared sample count does not fit in data_size.
 *  -3  when more than 3 trailing bytes remain, which violates dmweb's
 *      SND2 bound and usually means this is not a single-item SND2 .AMG.
 */
int FirestaffAmgSnd2_Decode(const uint8_t* data, size_t data_size,
                            FirestaffAmgSnd2* out);

/*
 * dmweb SND2 rate formula for Amiga playback:
 *   sample_rate = CPUclock / (2 * period)
 * Rounded to nearest integer Hz. Returns 0 for invalid inputs.
 */
uint32_t FirestaffAmgSnd2_RateHzForPeriod(uint32_t cpu_clock_hz,
                                          uint16_t period);

/*
 * dmweb also documents the game-side conversion:
 *   period = 72800 / playback_speed
 * The Amiga period register is an integer, so this helper rounds that
 * division to the nearest period before applying RateHzForPeriod().
 * Returns 0 for invalid inputs.
 */
uint32_t FirestaffAmgSnd2_RateHzForPlaybackSpeed(uint32_t cpu_clock_hz,
                                                 uint8_t playback_speed);

int FirestaffAmgSnd2_SelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_AMG_DECODE_H */
