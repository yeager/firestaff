# Runtime graphics and cheat panel

Firestaff opens the in-game runtime panel with **F10** for Dungeon Master,
Chaos Strikes Back, Dungeon Master II, Theron's Quest and Nexus. The panel is
modal, so a click or key press cannot reach the dungeon while it is open.

## Pages

- **PRES** changes the presentation mode, scaling, aspect policy, integer
  scaling, VSync, FPS overlay, output resolution and window mode.
- **FILT** changes the filters that are implemented by the active game's
  admitted renderer. V1 source-original rendering keeps these controls locked.
- **FX** changes the implemented phosphor, pixel-grid, motion-blur, lighting
  and turn-pan effects. These are presentation effects, not replacement game
  assets.
- **CH** contains the real shared cheat controls: the launcher cheat master
  switch and the slower/normal/faster runtime speed setting. Speed changes are
  applied to the live scheduler immediately.

The panel uses the same per-game configuration slots as the start menu. It is
available for all five games, and changing a control writes the normal
Firestaff configuration immediately. No restart is required.

The presentation page is shared by all games and owns scaling, aspect ratio,
window mode, VSync and the FPS overlay. Source-specific filters are separate:
Theron's admitted V2 chain owns its scanline, palette, dither, EPX and bilinear
settings. DM2 and Nexus show those rows as `SOURCE LOCKED` until their real
runtime filter chains have been decoded and connected; Firestaff does not
pretend that DM1 settings affect them.

## Keyboard and mouse

- Press **F10** to open or close the panel.
- Use **Up/Down** to select a row and **Left/Right**, **Enter** or the action
  key to change it.
- Use **Tab** to cycle pages and **Esc** to close the panel.
- Click a page label to change page, then click a row once to select it and
  again to change it. Click **X** to close.

Only controls backed by a real runtime owner are shown as active. Firestaff
does not fabricate game-specific god modes, infinite items or debug powers
when the original game/source does not provide them. The start-menu cheat
setting and F10 cheat page therefore expose the shared master toggle and live
speed control, while source-specific cheat research remains documented per
game.

## Start menu

The start menu's **Game** options contain the same cheat master switch and
speed choices. A setting selected there is the initial value used by F10; a
change made in F10 is saved back to that game's start-menu configuration.

## Source boundary

Original V1 data remains unchanged by the panel. Enhanced filters and effects
operate only after the corresponding presentation mode and real asset/data
admission gates have succeeded. This keeps the F10 panel useful for live
comparison without replacing missing original assets with synthetic data.
