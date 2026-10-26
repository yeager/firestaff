# Deterministic Nexus Start-to-menu capture

This evidence was captured on 2026-10-26 with the system-wide Ymir capture
build on TRV2. It uses the hash-verified Japanese retail CUE, authenticated
Japanese Saturn BIOS, and retained title-prompt save-state. No game content was
created or substituted.

The isolated Ymir profile loaded the state into CLI slot 2 and executed this
frame-aligned route:

```text
--debug --load-state 2 --capture-frames 120 --dump-memory \
--capture-start-frame 0 --capture-start-hold 60
```

`title-menu.png` is the X11 image taken after the capture completed. It shows
the retail title menu with `NEW GAME` selected. Its SHA-256 is
`0ad6bbc433936cd853fef476c8eb14be2c9532efcfcc218a43ca1053500ca55c`.

`vdp1-capture-bundle.txt` is the complete VDP1 receipt for that same run. It
records capture frame 59, command cursor 178, 178 command rows for the capture
frame, 10,668 total rows, and zero dropped entries. Its SHA-256 is
`0e5cfa33070e33f24ae0e25ead7ce4fac561ae7c329ef7c59ff12382d3ea519e`.

The temporary profile and unretained RAM/VRAM dumps were removed after the
receipt and image were checked. The evidence proves retail emulator behavior
and a deterministic input route. It does not prove Firestaff's menu renderer,
consumer ownership, or production input dispatch.
