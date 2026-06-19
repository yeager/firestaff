# V2.2 Asset Style Prompt

Återanvändbar stilmall för V2.2 "Modern" graphics mode (1920×1080, 3D-rendered 2D).

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
- **Size:** 2048x1152 för hero/establishing, 1024x1024 för tiles, 1024x1536 för creatures/items
- **Quality:** high
- **Format:** png
- **Background:** **opaque** (all assets — `gpt-image-2` does NOT support `background: transparent` and silently ignores it; verified via `sips -g hasAlpha` on 19 generated PNGs across batches 1–4)
- **Count:** 1 (kör fler prompts parallellt för batch)
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

Kör flera image_generate-anrop parallellt i samma tool block för snabb batch.
Varje anrop är en egen background task — vänta in alla completion events
innan du visar resultatet.

## "Always Compare" Rule (BOSSe: 2026-06-18)

**Varje V2.2-generering MÅSTE jämföras mot motsvarande original-DM1-sprite.**

1. Extrahera originalsprites: `build/extract_all_sprites <GRAPHICS.DAT>
   .openclaw/tmp/dm1-sprites dm1` (eller motsvarande för andra spel).
2. För varje V2.2-generering, hitta en DM1-sprite som motsvarar (corridor, tile,
   creature, item). Vision-modellen identifierar korrekt subject bättre än
   dimensionsbaserad gissning.
3. Kör pixel-jämförelse (sida-vid-sida) eller konceptuell jämförelse (text).
4. Uppdatera denna stilmall om DM1-insikter avviker från nuvarande master
   prompt (t.ex. palett, silhouette, lighting).
5. Logga jämförelsen i docs/v22-batch-progress.md (eller motsvarande).

## DM1 Style Notes (från vision-analys 2026-06-18)

Analys av 5 original-DM1-sprites (corridor, menu, credits scroll, Firestaff
emblem, Screamer) ger följande kännetecken:

- **Silhuett före detalj** — alla sprites har tydlig, läsbar outline
- **16-färgskänsla** — även i hög upplösning ska paletten kännas begränsad
  och theatralisk (mättade röda/orange/gula flammor, cyan highlights)
- **Höga konturer** — svart outline är ett signum från Atari-ST-eran
- **Symbolisk belysning** — inte realistisk GI; highlights i cyan/gul/vit,
  shadows i svart/blå/mörkröd
- **Theatrical/magisk** — facklor, juveler, scrollar, magiska föremål med
  överdriven mättnad
- **Readable at small size** — sprites ska vara tydliga även om de
  skalas ner (viktigt för tiles och creatures)
- **Composition leder ögat** — starka färgkontraster styr blicken (brand →
  cyan juvel, gul mage → blå silhuett)

**VIKTIGT:** vision-modellen är mer pålitlig än dimensionsbaserad gissning för
att identifiera sprites. Använd alltid vision (image med prompt) för att
matcha V2.2-generering mot rätt original.

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
