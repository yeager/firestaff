#include "theron_v1_track02_file_cabinet.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 * File cabinet management strings at UD 0x26107B-0x261171.
 * Boot/copy-protection screen at UD 0x26C348-0x26C3B7. */

const char *theron_v1_track02_us_no_space(void) {
    return "  YOU DON'T HAVE ENOUGH SPACE      IN THE FILE CABINET!     ";  /* UD 0x26107B */
}

const char *theron_v1_track02_us_choose_delete(void) {
    return "  CHOOSE A FILE TO DELETE.    ";  /* UD 0x2610B8 */
}

const char *theron_v1_track02_us_sure(void) {
    return "          SURE?               ";  /* UD 0x2610D7 */
}

const char *theron_v1_track02_us_thank_you(void) {
    return "        THANK YOU.            ";  /* UD 0x261115 */
}

const char *theron_v1_track02_us_not_saved(void) {
    return "  THIS GAME WILL NOT BE SAVED!";  /* UD 0x261134 */
}

const char *theron_v1_track02_us_boot_attention(void) {
    return "     ATTENTION!\x01"
           "This disc only works on\x01"
           "the SUPER CD-ROM2 SYSTEM.\x01"
           "      Please use\x01"
           "the SUPER CD-ROM2 SYSTEM. ";  /* UD 0x26C348 */
}
