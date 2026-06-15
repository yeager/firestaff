/*
 * firestaff_update_check_pc34_compat.h
 *
 * Firestaff self-update-check helpers.  Bounded design:
 * no network dependency.  The M12 launcher fetches the
 * latest release JSON out-of-band (e.g. curl, browser,
 * or the user's package manager) and calls
 * firestaff_update_check_evaluate() to compare the
 * downloaded tag against the current version and produce
 * a user-visible status.
 *
 * v1 (2026-06-14): semver compare + JSON tag_name
 * extractor + a FIRESTAFF_UPDATE_* enum for the
 * launcher's settings-menu UI.  The launcher wires the
 * actual download via the OS package manager / GitHub
 * Releases page; this helper only decides whether the
 * downloaded tag is newer than M12_Changelog_VersionString.
 */
#ifndef REDMCSB_FIRESTAFF_UPDATE_CHECK_PC34_COMPAT_H
#define REDMCSB_FIRESTAFF_UPDATE_CHECK_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Update check result codes. */
typedef enum {
    FIRESTAFF_UPDATE_UP_TO_DATE    = 0, /* current >= latest */
    FIRESTAFF_UPDATE_NEW_AVAILABLE = 1, /* latest > current */
    FIRESTAFF_UPDATE_PARSE_ERROR   = 2, /* JSON malformed */
    FIRESTAFF_UPDATE_DISABLED      = 3, /* FIRESTAFF_NO_UPDATE_CHECK=1 */
} FirestaffUpdateResult;

/* Compare two semver strings ("2.7.14" < "2.7.18").  Returns
 * -1 if a < b, 0 if equal, +1 if a > b.  Pre-release
 * suffixes (e.g. "2.7.18-rc.1") compare against the same
 * base version as a normal release.  Leading "v" or "V"
 * is stripped. */
int firestaff_update_check_compare_semver(const char* a,
                                          const char* b);

/* Extract the "tag_name" field from a GitHub Releases
 * JSON response.  Writes the value (without surrounding
 * quotes, with simple backslash un-escapes) into outBuf
 * up to outCap-1 bytes.  Returns 1 on success, 0 if the
 * field is missing or the body is malformed. */
int  firestaff_update_check_extract_tag(const char* body,
                                       int bodyLen,
                                       char* outBuf, int outCap);

/* Evaluate a GitHub Releases JSON body against the
 * current version.  Returns FIRESTAFF_UPDATE_NEW_AVAILABLE
 * if the embedded tag_name is newer than
 * M12_Changelog_VersionString(), FIRESTAFF_UPDATE_UP_TO_DATE
 * if not, FIRESTAFF_UPDATE_PARSE_ERROR on malformed JSON. */
FirestaffUpdateResult firestaff_update_check_evaluate(
    const char* body, int bodyLen);

/* Honor a build-time escape hatch for offline / CI. */
int  firestaff_update_check_disabled(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_FIRESTAFF_UPDATE_CHECK_PC34_COMPAT_H */
