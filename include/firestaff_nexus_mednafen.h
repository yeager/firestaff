#ifndef FIRESTAFF_NEXUS_MEDNAFEN_H
#define FIRESTAFF_NEXUS_MEDNAFEN_H

/* Explicit retail Nexus launcher.  It is intentionally separate from the
 * native renderer: this starts a user-owned Saturn CUE in Mednafen and never
 * represents emulator output as a Firestaff-native parity receipt. */

#define FIRESTAFF_NEXUS_MEDNAFEN_PATH_MAX 4096

typedef struct Firestaff_NexusMednafenLaunch {
    char emulator[FIRESTAFF_NEXUS_MEDNAFEN_PATH_MAX];
    char disc[FIRESTAFF_NEXUS_MEDNAFEN_PATH_MAX];
    char bios[FIRESTAFF_NEXUS_MEDNAFEN_PATH_MAX];
    int hasBios;
} Firestaff_NexusMednafenLaunch;

/* Discovers the executable, CUE and optional BIOS from explicit arguments or
 * FIRESTAFF_NEXUS_MEDNAFEN/DISC/BIOS.  With no executable override it checks
 * Homebrew's two standard locations.  `dataDir` can be the data root or the
 * Nexus directory itself. */
int Firestaff_NexusMednafen_Discover(const char* dataDir,
                                     const char* emulatorOverride,
                                     const char* discOverride,
                                     const char* biosOverride,
                                     Firestaff_NexusMednafenLaunch* out);

/* Starts Mednafen and waits until it closes.  Returns its exit status, or a
 * negative value if the process could not be launched. */
int Firestaff_NexusMednafen_Launch(
    const Firestaff_NexusMednafenLaunch* launch);

#endif
