# CSB F0546-F0565 Platform Boundary Source Audit

Authority: ReDMCSB `AMIGA.H`. The source inventory contains declarations but
not PC34 callable bodies, and no authenticated Amiga display/input corpus is
available. These are platform boundaries, not missing portable UI features.

| Range | Declared owner | Required missing evidence | Firestaff disposition |
| --- | --- | --- | --- |
| F0546-F0549 | Amiga mouse sprite ownership | Amiga sprite image, pointer and display-server evidence | Always reject; no pointer/sprite show, hide, or allocation substitute. |
| F0550-F0552 | Amiga video fill/hatch and fatal display | Amiga bitplane and alert behavior | Always reject; no box fill, hatch, or UI/error rendering. |
| F0553-F0556 | Amiga text message-area memory/cursor | Amiga text buffer and font-layout evidence | Always reject; no message-area or text-memory mutation. |
| F0557-F0563 | Amiga scroller task | Amiga scheduler/scroller buffer evidence | Always reject; no scrolling task or message output. |
| F0564-F0565 | Amiga viewport bitplanes/palette | Amiga copper/bitplane/palette corpus | Always reject; no viewport initialization or palette operation. |

An authenticated PC34 `GRAPHICS.DAT` cache proves neither Amiga pointer,
bitplane nor copper semantics. The boundary records that limitation and never
creates a synthetic graphics, UI, or action path.
