#ifndef FIRESTAFF_REDMCSB_F0699_VIDEO_INTERRUPT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0699_VIDEO_INTERRUPT_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB: IMAGE.C F0699_InitVideoInterrupt (lines 392-400), PC 3.4.
 *
 * The original obtains the VIDEO_DRIVER table from DOS interrupt vector 255,
 * stores that table globally, then invokes slot 13 with the addresses of two
 * otherwise-unused global pointers.  The platform-specific vector lookup is
 * caller supplied here; this adapter does not fabricate a video driver.
 */
enum { REDMCSB_F0699_DM_VIDEO_INTERRUPT_PC34 = 255 };

typedef void (*ReDMCSBF0699InitializeUnusedGlobalsPc34Compat)(
    char **first_unused,
    char **second_unused);

typedef struct {
    ReDMCSBF0699InitializeUnusedGlobalsPc34Compat initialize_unused_globals;
} ReDMCSBF0699VideoDriverPc34Compat;

typedef const ReDMCSBF0699VideoDriverPc34Compat *
    (*ReDMCSBF0699GetVectorPc34Compat)(unsigned int interrupt_number,
                                       void *context);

typedef struct {
    const ReDMCSBF0699VideoDriverPc34Compat *video_driver;
    char *first_unused;
    char *second_unused;
} ReDMCSBF0699VideoInterruptPc34Compat;

/* Performs the F0699 PC 3.4 sequence.  False means the host did not expose
 * the original vector/table/slot and leaves the supplied state untouched.
 */
bool F0699_InitVideoInterrupt_PC34(
    ReDMCSBF0699VideoInterruptPc34Compat *state,
    ReDMCSBF0699GetVectorPc34Compat get_vector,
    void *context);

#ifdef __cplusplus
}
#endif

#endif
