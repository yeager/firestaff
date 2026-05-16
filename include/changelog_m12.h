#ifndef FIRESTAFF_CHANGELOG_M12_H
#define FIRESTAFF_CHANGELOG_M12_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Changelog/Version viewer for the Firestaff launcher ──────────
 *
 * Embeds a compact release history displayed as scrollable text
 * in the launcher menu.  Accessible from the main menu via a
 * dedicated "VERSION" menu entry or key shortcut.
 */

#define M12_CHANGELOG_MAX_LINES     64
#define M12_CHANGELOG_VISIBLE_LINES 12

typedef struct {
    int scrollOffset;       /* first visible line index */
    int totalLines;         /* cached line count        */
} M12_ChangelogState;

/* Initialise / reset scroll state. */
void M12_Changelog_Init(M12_ChangelogState* cl);

/* Scroll by delta lines (positive = down, negative = up). */
void M12_Changelog_Scroll(M12_ChangelogState* cl, int delta);

/* Return the total number of changelog lines. */
int  M12_Changelog_LineCount(void);

/* Return the text for line at the given index (0-based).
 * Returns NULL if index is out of range. */
const char* M12_Changelog_GetLine(int index);

/* Return the project version string (e.g. "0.11.0"). */
const char* M12_Changelog_VersionString(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CHANGELOG_M12_H */
