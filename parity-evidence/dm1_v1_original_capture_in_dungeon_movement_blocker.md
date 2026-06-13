# DM1 V1 original DOS capture — in-dungeon movement blocker

Status: BLOCKED_DM1_V1_ORIGINAL_IN_DUNGEON_MOVEMENT_MOUSE_CLICK_NO_VIEWPORT_CHANGE

This note continues the capture lane after
`dm1_v1_original_capture_dungeon_entry_reached.md`. It does not redo the
already-proven `dungeon_gameplay` entry route. The new live harness reaches
the dungeon, then attempts an original DM PC movement-panel forward click and
writes a local movement receipt. The click is dispatched, but the viewport
hash remains unchanged across the post-click samples.

No proprietary game frames are committed here. Frames and JSON receipts remain
under the operator-local capture root.

## Host / runtime

- DOSBox Staging 0.82.2 (5e2ba), macOS host
- Capture backend: `dosbox-rawshot` (DOSBox internal rendered screenshot)
- Live capture root:
  `~/firestaff-captures/dm1-v1-in-dungeon-movement-live-20260613-123000/`
- Job-local conf:
  `~/firestaff-captures/dm1-v1-in-dungeon-movement-live-20260613-123000/dosbox_capture.live.conf`
- Conf sha256:
  `c0ea76986b1214a11c48c5f7354abe102ffee5f47741c509c95ed70af3df6084`
- Movement receipt sha256:
  `2e4c0384cc048d5d90a6a09407160fae6e727709161bea6a2b036bca32da524c`

## Conf facts

Single `-conf` file, no layered confs:

- `[sdl]` `output=opengl`, `windowresolution=1024x768`,
  `viewport_resolution=1024x768`, job-local `mapperfile=.../dosbox_capture.mapper.map`
- `[dosbox]` `machine=svga_s3`, `memsize=16`
- `[render]` `frameskip=0`, `aspect=auto`, `glshader=none`
- `[cpu]` `core=dynamic`, `cycles=max`
- `[capture]` `capture_dir=.../dosbox-capture`,
  `default_image_capture_formats=rendered raw`
- `[mouse]` `mouse_capture=onclick`, `mouse_sensitivity=100`,
  `mouse_raw_input=true`, `dos_mouse_driver=true`,
  `dos_mouse_immediate=true`
- `[autoexec]` mounts the DM1 PC 3.4 runtime dir as `C:`, then runs `DM.EXE`

Video mode selector option `1` remains documented and used as VGA.

## Movement attempt

Source target:

- ReDMCSB `COMMAND.C:396-405`, line 398 maps
  `C003_COMMAND_MOVE_FORWARD` to `C070_ZONE_MOVE_FORWARD`.
- PC screen-relative C070 box: `x=263..289`, `y=125..145`.
- Harness click target: center `(276, 135)` in the normalized 320x200
  framebuffer, fraction `(0.8625, 0.675)`.

Live result:

- `enter_dungeon` still reaches `dungeon_gameplay`.
- `dungeon_move_forward_click` dispatch is logged as
  `mouse:left:c070_move_forward`, `ok=true`.
- Before and after states both classify as `dungeon_gameplay`.
- Before viewport sha256:
  `d19a2c8e1fe69e399acf24fb0ce196d8080576b6249238bac7df7d8df3e5e345`
- After viewport sha256:
  `d19a2c8e1fe69e399acf24fb0ce196d8080576b6249238bac7df7d8df3e5e345`
- Before and after normalized full-frame sha256:
  `3a5ab1a8edd2e5a84eb91fa94907bb87dc9a8213db5e2bcaf1c4b2a18187e345`
- Post-click samples: 7; all remained byte-identical at the normalized
  full-frame and viewport-hash level.

## Narrowed blocker

The original-DOS route can now prove all of the following in one executable
live run:

- job-local conf is used and recorded;
- mode `1` is VGA;
- the route reaches `dungeon_gameplay`;
- the in-dungeon C070 forward-arrow mouse click target is source-locked and
  logged;
- the failure is specifically that the click does not mutate the original
  viewport under the current DOSBox/macOS mouse-delivery path.

The next useful work is to isolate the DOSBox guest mouse delivery after
`dungeon_gameplay` without changing the already-proven entrance route. The
failed `mouse_capture=seamless` experiment was also informative: it prevented
the entrance click from reaching `dungeon_gameplay`, so it is not a drop-in
fix for this route.

## Non-claims

- This is not a movement success row.
- This is not original-vs-Firestaff pixel parity.
- No proprietary frame data is committed.
