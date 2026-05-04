#ifndef FIRESTAFF_DM1_V1_INPUT_POLL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INPUT_POLL_PC34_COMPAT_H

/* DM1 V1 Input Polling — source-locked from ReDMCSB
 * INPUT.C F0375: keyboard raw scan + cooked key
 * INPUT.C F0376: joystick / gamepad support (optional)
 * G2590_INPUT_ReadKey: read one cooked keyboard event
 *
 * Keyboard mapping follows original DM1 PC layout:
 * - Number keys (1-9): Action hand items
 * - Arrow keys / numpad: Movement (forward, backward, turn, strafe)
 * - Function keys (F1-F10): Spell casting symbols
 * - Space: Wait one frame
 * - ESC / right-click: Menu
 * - Mouse: Click on action hand + movement arrows
 *
 * Raw DOS keyboard interface emulates original 16-bit int 16h scan codes.
 * Cooked keys are ASCII-equivalent for command dispatch.
 * Joystick axes map to movement directions when joystick is connected.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   INPUT.C: F0536_INPUT_Initialize — init input device handler
 *     Amiga: console.device, gameport.device, input.device
 *     Installs F0543_INPUT_DeviceInterruptHandler as ISR
 *   INPUT.C: F0538_INPUT_Deinitialize — remove handler
 *   INPUT.C: F0539_INPUT_Cconis — check if key available (non-blocking)
 *   INPUT.C: F0540_INPUT_Crawcin — read one key (blocking)
 *   INPUT.C: F0543_INPUT_DeviceInterruptHandler — main ISR:
 *     IECLASS_RAWKEY: normalize via RawKeyConvert, store in FIFO
 *       Capslock + arrows → shift+arrows (movement)
 *       Numpad keys remapped to movement directions (A3x):
 *         Num7→TurnLeft, Num9→TurnRight, Num8→Forward,
 *         Num5/Num2→Backward, Num4→StrafeLeft, Num6→StrafeRight
 *     IECLASS_RAWMOUSE: track G1038_i_MouseX, G1039_i_MouseY
 *       Clamp to screen (640×400 on Amiga, 320×200 logical)
 *       Left click → F0359_COMMAND_ProcessClick(x,y,LEFT)
 *       Right click → F0359_COMMAND_ProcessClick(x,y,RIGHT)
 *     Left Alt+Left Amiga = left mouse emulation
 *     Right Alt+Right Amiga = right mouse emulation
 *   INPUT.C: F1097_StoreKeyInBuffer — FIFO insert (64-entry ring)
 *   INPUT.C: F1098_GetFirstKeyFromBuffer — FIFO extract
 *   INPUT.C: F1099_IsKeyBufferNotEmpty — FIFO not-empty check
 *   Key globals:
 *     G1038_i_MouseX, G1039_i_MouseY — mouse position
 *     G0588_i_MouseButtonsStatus — button state bitmask
 *     G0597_B_IgnoreMouseMovements — movement lock
 *     G1043_i_InputBufferCharacterCount — key buffer count (A20 path)
 *     G1045_ai_InputBufferCharacters[256] — key buffer (A20 path)
 *     G3174_i_LastKeyIndex / G3175_i_FirstKeyIndex — FIFO indices (A3x)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Input event types ────────────────────────────────────────────── */
typedef enum {
    DM1_INPUT_NONE = 0,
    DM1_INPUT_KEY_DOWN,
    DM1_INPUT_KEY_UP,
    DM1_INPUT_MOUSE_MOVE,
    DM1_INPUT_MOUSE_LEFT_DOWN,
    DM1_INPUT_MOUSE_LEFT_UP,
    DM1_INPUT_MOUSE_RIGHT_DOWN,
    DM1_INPUT_MOUSE_RIGHT_UP,
    DM1_INPUT_JOYSTICK_AXIS,
    DM1_INPUT_JOYSTICK_BUTTON
} M11_InputEventType;

/* ── DM1 PC34 key codes (DOS int 16h scan codes, normalized) ──────── */
typedef enum {
    DM1_KEY_NONE = 0x0000,
    /* Movement keys (arrow/numpad, from F0543 numpad remap) */
    DM1_KEY_FORWARD         = 0x4C00,  /* Arrow Up / Num8 */
    DM1_KEY_BACKWARD        = 0x4D00,  /* Arrow Down / Num5/Num2 */
    DM1_KEY_STRAFE_LEFT     = 0x4F00,  /* Arrow Left / Num4 */
    DM1_KEY_STRAFE_RIGHT    = 0x4E00,  /* Arrow Right / Num6 */
    DM1_KEY_TURN_LEFT       = 0x4600,  /* DEL / Num7 */
    DM1_KEY_TURN_RIGHT      = 0x5F00,  /* Help / Num9 */
    /* Function keys for spell symbols */
    DM1_KEY_SPELL_F1        = 0x003B,
    DM1_KEY_SPELL_F2        = 0x003C,
    DM1_KEY_SPELL_F3        = 0x003D,
    DM1_KEY_SPELL_F4        = 0x003E,
    DM1_KEY_SPELL_F5        = 0x003F,
    DM1_KEY_SPELL_F6        = 0x0040,
    DM1_KEY_SPELL_F7        = 0x0041,
    DM1_KEY_SPELL_F8        = 0x0042,
    DM1_KEY_SPELL_F9        = 0x0043,
    DM1_KEY_SPELL_F10       = 0x0044,
    /* Special */
    DM1_KEY_SPACE           = 0x0020,  /* Wait one tick */
    DM1_KEY_ESCAPE          = 0x001B,  /* Menu */
    DM1_KEY_BACKSPACE       = 0x0008,  /* Delete spell symbol */
    DM1_KEY_DELETE           = 0x007F   /* Shift+DEL */
} M11_DM1KeyCode;

/* ── Mouse button masks (from G0588 bitmask) ─────────────────────── */
#define DM1_MOUSE_LEFT_BUTTON    0x0002  /* MASK0x0002_MOUSE_LEFT_BUTTON */
#define DM1_MOUSE_RIGHT_BUTTON   0x0001  /* MASK0x0001_MOUSE_RIGHT_BUTTON */
#define DM1_MOUSE_LEFT_UP        0x0004  /* MASK0x0004_MOUSE_LEFT_BUTTON_UP (A3x) */
#define DM1_MOUSE_RIGHT_UP       0x0008  /* MASK0x0008_MOUSE_RIGHT_BUTTON_UP (A3x) */

/* ── Input event structure ────────────────────────────────────────── */
typedef struct {
    M11_InputEventType type;
    uint16_t keyCode;       /* Normalized DM1 key code */
    int16_t  mouseX;        /* G1038_i_MouseX (logical: 0–319) */
    int16_t  mouseY;        /* G1039_i_MouseY (logical: 0–199) */
    uint16_t buttonMask;    /* G0588_i_MouseButtonsStatus */
    int16_t  joystickAxis;  /* Axis value (-1, 0, +1) */
    int16_t  joystickId;    /* Axis/button identifier */
} M11_InputEvent;

/* ── Key buffer (ring buffer, matches F1097/F1098/F1099) ──────────── */
#define M11_KEY_BUFFER_SIZE  64  /* 64-entry ring from G3174/G3175 mask 0x3F */

typedef struct {
    uint16_t buffer[M11_KEY_BUFFER_SIZE];
    int      writeIndex;    /* G3174_i_LastKeyIndex */
    int      readIndex;     /* G3175_i_FirstKeyIndex */
} M11_KeyBuffer;

/* ── Input system state ───────────────────────────────────────────── */
typedef struct {
    /* Mouse state — from G1038/G1039/G0588 */
    int16_t  mouseX;
    int16_t  mouseY;
    uint16_t mouseButtons;
    int      ignoreMouseMovements;   /* G0597_B_IgnoreMouseMovements */
    int      leftButtonDown;         /* G1050_B_LeftMouseButtonDown */
    int      rightButtonDown;        /* G1051_B_RightMouseButtonDown */

    /* Screen bounds for mouse clamping */
    int16_t  screenMaxX;   /* 319 (logical) or 639 (raw Amiga) */
    int16_t  screenMaxY;   /* 199 (logical) or 399 (raw Amiga) */

    /* Key buffer */
    M11_KeyBuffer keyBuffer;

    /* Keyboard alt/amiga emulation flags */
    int leftAmigaAltDown;       /* G1046_B_LeftAmigaAndLeftAltDown */
    int rightAmigaAltDown;      /* G1047_B_RightAmigaAndRightAltDown */

    /* Random seed from first input timestamp */
    int      randomInitialized; /* G1048_B_RandomNumberInitialized */
    uint32_t randomSeed;        /* G0349_ul_LastRandomNumber */

    /* Input activity flag */
    int mouseOrKeyboardInput;   /* G1044_B_MouseOrKeyboardInput */

    /* Initialization state */
    int initialized;
} M11_InputState;

/* ── Initialization ───────────────────────────────────────────────── */

/* Initialize input system (F0536_INPUT_Initialize path). */
void m11_input_init(M11_InputState *state);

/* Deinitialize input system (F0538_INPUT_Deinitialize). */
void m11_input_deinit(M11_InputState *state);

/* ── Key buffer operations (F1097/F1098/F1099) ────────────────────── */

/* Store a key code in the ring buffer. Returns 1 if stored, 0 if full. */
int m11_input_store_key(M11_InputState *state, uint16_t keyCode);

/* Get first key from buffer. Returns 0 if empty. */
uint16_t m11_input_get_key(M11_InputState *state);

/* Check if key buffer is not empty. */
int m11_input_key_available(const M11_InputState *state);

/* Discard all buffered input (F0357_COMMAND_DiscardAllInput equivalent). */
void m11_input_discard_all(M11_InputState *state);

/* ── Mouse event processing (from F0543 IECLASS_RAWMOUSE handler) ── */

/* Process a mouse movement delta. Clamps to screen bounds. */
void m11_input_mouse_move(M11_InputState *state, int16_t dx, int16_t dy);

/* Set absolute mouse position. */
void m11_input_mouse_set_position(M11_InputState *state, int16_t x, int16_t y);

/* Process mouse button down. Returns the button mask set. */
uint16_t m11_input_mouse_button_down(M11_InputState *state, int isLeft);

/* Process mouse button up. Returns the button mask cleared. */
uint16_t m11_input_mouse_button_up(M11_InputState *state, int isLeft);

/* Get current mouse position. */
void m11_input_mouse_get_position(const M11_InputState *state,
                                  int16_t *x, int16_t *y);

/* ── Keyboard event processing (from F0543 IECLASS_RAWKEY handler) ── */

/* Process a raw key event. Normalizes and stores in buffer.
 * Returns the normalized key code, or 0 if swallowed. */
uint16_t m11_input_process_raw_key(M11_InputState *state,
                                   uint16_t rawScanCode,
                                   int isKeyUp,
                                   int capsLock,
                                   int shiftHeld);

/* Map a numpad scan code to movement key code (A3x numpad remap). */
uint16_t m11_input_numpad_to_movement(uint16_t normalizedCode);

/* ── Joystick support (optional, from F0376) ──────────────────────── */

/* Process joystick axis. Maps to movement direction. */
uint16_t m11_input_joystick_axis(M11_InputState *state,
                                 int16_t axisX, int16_t axisY);

/* ── Polling ──────────────────────────────────────────────────────── */

/* Check if any input has occurred since last check (F0539 equivalent).
 * Clears the flag after checking. */
int m11_input_any_activity(M11_InputState *state);

/* Wait for keyboard or mouse activity (F0541 equivalent — non-blocking poll). */
int m11_input_wait_for_activity(const M11_InputState *state);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *m11_input_poll_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_INPUT_POLL_PC34_COMPAT_H */
