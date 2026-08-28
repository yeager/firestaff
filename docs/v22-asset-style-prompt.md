# V2.2 Asset Style Prompt

Reusable style template for the V2.2 "Modern" graphics mode (1920×1080,
3D-rendered 2D).

## Master Style Prompt

> Dark fantasy dungeon asset in the spirit of FTL Games' Dungeon Master (1987),
> rendered as modern 3D cinematic art. Warm torchlight, weathered carved stone,
> volumetric fog, moody atmospheric lighting, PBR materials, high detail, UE5
> quality. **Strong black-ink silhouette, vivid limited-palette contrast, and
> theatrical/symbolic lighting reminiscent of 16-color Atari ST artwork** —
> but rendered with modern PBR detail. Single isolated subject on a clean or
> contextual background. No UI, no text, no HUD.

## Per-Asset Suffixes

| Subject      | Suffix to append                                         |
|--------------|----------------------------------------------------------|
| Floor tile   | `top-down square dungeon floor tile, wet stone, torchlight` |
| Wall tile    | `seamless dungeon wall tile, carved stone, torchlit`     |
| Corner piece | `dungeon corner, two walls meeting, atmospheric lighting`|
| Creature     | `full-body creature sprite, neutral pose, three-quarter view` |
| Item         | `single object, treasure or potion, soft glow, dark backdrop`|
| Entrance     | `first-person dungeon entrance hall, dramatic wide shot`  |

## Parameters

- **Model:** `openai/gpt-image-2`
- **Provider:** openai (configured, OPENAI_API_KEY / Codex OAuth)
- **Size:** 2048x1152 for hero/establishing shots, 1024x1024 for tiles,
  1024x1536 for creatures/items
- **Quality:** high
- **Format:** png
- **Background:** **opaque** (all assets — `gpt-image-2` does NOT support `background: transparent` and silently ignores it; verified via `sips -g hasAlpha` on 19 generated PNGs across batches 1–4)
- **Count:** 1 (run additional prompts in parallel for a batch)
- **aspectRatio:** ignored (warning shown in completion event; rely on `size` instead)

## Output Path

`~/.openclaw/media/tool-image-generation/firestaff-v22-<asset>-<NN>.png`

## Naming

`firestaff-v22-<category>-<descriptor>-<NN>.png`

Examples:
- `firestaff-v22-tile-floor-stone-01.png`
- `firestaff-v22-tile-wall-carved-01.png`
- `firestaff-v22-creature-goblin-01.png`
- `firestaff-v22-item-potion-01.png`

## Batch Recipe

Run multiple image_generate calls in parallel in the same tool block for a
fast batch. Each call is a separate background task; wait for all completion
events before presenting results.

## "Always Compare" Rule (BOSSe: 2026-06-18)

**Every V2.2 generation MUST be compared with its corresponding original DM1 sprite.**

1. Extract original sprites: `build/extract_all_sprites <GRAPHICS.DAT>
   .openclaw/tmp/dm1-sprites dm1` (or the equivalent for other games).
2. For every V2.2 generation, locate its corresponding DM1 sprite (corridor,
   tile, creature, item). The vision model identifies the correct subject more
   reliably than dimension-based guessing.
3. Run a pixel comparison (side by side) or a conceptual comparison (text).
4. Update this style template if the DM1 observations differ from the current
   master prompt (for example palette, silhouette, or lighting).
5. Record the comparison in docs/v22-batch-progress.md (or equivalent).

## DM1 Style Notes (from vision analysis, 2026-06-18)

Analysis of five original DM1 sprites (corridor, menu, credits scroll,
Firestaff emblem, Screamer) identified these characteristics:

- **Silhouette before detail** — all sprites have a clear, readable outline.
- **16-colour feeling** — even at high resolution, the palette should feel
  constrained and theatrical (saturated red/orange/yellow flames and cyan
  highlights).
- **Strong outlines** — black outlines are a signature of the Atari ST era.
- **Symbolic lighting** — not realistic GI; highlights use cyan/yellow/white,
  shadows use black/blue/dark red.
- **Theatrical/magical** — torches, jewels, scrolls, and magical objects have
  exaggerated saturation.
- **Readable at small size** — sprites must remain clear when scaled down,
  which is important for tiles and creatures.
- **Composition directs the eye** — strong colour contrasts guide attention
  (fire → cyan jewel, yellow mage → blue silhouette).

**IMPORTANT:** the vision model is more reliable than dimension-based guessing
when identifying sprites. Always use vision (an image with a prompt) to match
a V2.2 generation to the correct original.

## Reference Index (workspace)

- DM1 sprites: `.openclaw/tmp/dm1-sprites/` (543 PNG, manifest.json)
- V2.2 output: `~/.openclaw/media/tool-image-generation/firestaff-v22-*.png`
- Batch progress: `docs/v22-batch-progress.md`

## Model Constraints — gpt-image-2 (2026-06-19)

- **`background: transparent` is NOT supported.** HTTP 400 ("Transparent
  background is not supported for this model"). Verified across batches 1–4:
  all 19 generated PNGs have `hasAlpha=no` regardless of the `background`
  parameter passed in `image_generate`.
- **Fix:** use `background: opaque` (or omit — default is opaque). All V2.2
  assets are designed for an opaque dark backdrop. When integrating V22
  assets into the V1 draw pipeline, treat them as opaque sprites
  (composite directly, no alpha-blend math needed).
- **`aspectRatio` is silently ignored** — gpt-image-2 ignores `aspectRatio`
  and uses `size` instead. Completion events include a warning:
  "Ignored unsupported overrides for openai/gpt-image-2: aspectRatio=N:N".
  Trust `size` for the actual output dimensions.
- **Output filenames get a UUID suffix.** The `filename` parameter is a
  basename hint, not an exact name. Output file is always
  `firestaff-v22-<name>-<NN>---<UUID>.png`. Use the basename when copying
  to `~/.firestaff/assets/<game>/modern/<category>/<id>.png`.
