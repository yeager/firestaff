# DM1 all-graphics phase 65 — in-game capture palette decode

## Change

Fixed `capture_ingame_series` PPM export to decode Firestaff framebuffer bytes with:

- `M11_FB_DECODE_INDEX(raw)`
- `M11_FB_DECODE_LEVEL(raw)`

The capture tool previously used `raw & 0x0f` plus one global startup palette level, which ignored per-pixel light-level encoding. Probe screenshots already decoded this correctly; the normal in-game capture path did not.

## Visual capture

Fresh capture series:

- `verification-m11/decoded-ingame-capture-20260425-145212/01_ingame_start_latest.png`
- `verification-m11/decoded-ingame-capture-20260425-145212/05_ingame_after_cast_latest.png`

Visual review:

- UI colors/text readability improved.
- The second capture remains the most presentable.
- Viewport colored corruption did not fully disappear; the live 3D viewport still shows multicolored/noisy wall/floor artifacts.
- Therefore the remaining live viewport issue is not only capture palette decode; it likely lives in viewport/world rendering, source bitmap interpretation, clipping, masking, or draw-buffer handling.

## Verification

```text
# summary: 399/399 invariants passed
ctest: 4/4 PASS
```
