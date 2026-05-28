# Firestaff V2 All-Assets Style Guide

## Purpose

This guide extends the current V2 direction beyond creatures.
It keeps V2 high-resolution while preserving a classic Dungeon Master read.

This is a **V2-only** foundation.
It does not change active V1 parity work.

## Core visual rules

1. **Readable silhouette first**
   - Preserve large forms and distance readability before texture detail.
   - If a 25% thumbnail stops reading as DM-like gameplay art, the pass failed.

2. **Painted, but restrained**
   - Surfaces may be smoother and higher-resolution than DM1.
   - Do not drift into glossy, soft, floaty, or over-rendered fantasy splash-art treatment.

3. **Limited color separation**
   - Use grouped values and a narrow accent range.
   - Prefer DM-like dark bronze, stone, bone, cloth, ember, acid, and muted magical hues over neon UI accents.

4. **Keep some hard edges and chunkiness**
   - Corners, bevels, cutouts, slot frames, and object edges should still feel carved and game-readable.
   - Avoid mushy gradients and airbrushed volume that blur the original roles.

5. **Respect original geometry contracts**
   - If a family is a preserve-scale repaint, keep its DM1 proportion and silhouette intact.
   - If a family is a redraw-native family, preserve role, size hierarchy, and encounter readability.
   - Every graphical asset for every game must start from a source-shape lock: the approved V1/source reference defines the pose, silhouette, footprint, crop role, and depth read. V2.2 may redraw the art, but it must not invent a different creature, object, wall, portrait, icon, or UI shape unless a family brief explicitly marks the asset as redesign-native.

6. **4K-first production**
   - Canonical masters are 4K-scale production assets.
   - 1080p outputs are exact 50% derivatives unless a family brief explicitly documents an exception.

7. **V2.1 and V2.2 share the same visual contract**
   - V2.1 is a high-resolution source-preserving upscale. It should look clean and materially less pixelated than V1/V2.0, while keeping the original asset shape.
   - V2.2 is a new high-resolution modern redraw. It must still match the same source-shape lock before detail, lighting, material polish, or style is accepted.
   - Do not reuse old V2.2 assets that drift away from the source form. Treat them as rejected references, not production inputs.
   - Review V1, V2.0, V2.1, and V2.2 side by side at the same perceived scale before approving an asset.

## Cross-class material language

- Stone, bronze, wood, iron, bone, parchment, cloth, slime, and rune-light should read as tactile but not glossy.
- Wear should be broad and intentional, not noisy micro-scratch filler.
- Highlights should support form reads, not advertise rendering complexity.
- Shadows should stay grouped and readable after downscale.

## Asset-class guidance

These rules apply to all games: DM1, Chaos Strikes Back, DM2, Dungeon Master Nexus, and Theron's Quest. Each game can have its own source files, palette behavior, and render path, but V2 assets still need the same source-shape comparison before they are wired into runtime.

### 1. UI / HUD / frame families
- Preserve the DM screen skeleton and panel silhouettes.
- Keep borders and recesses crisp, weighty, and slightly chunky.
- Use restrained bevel depth and warm/cool separation only where it improves state readability.
- Avoid modern dashboard widgets, glowing backplates, and decorative empty space.

### 2. Creature families
- Preserve depth-ladder readability and encounter staging.
- Favor strong pose read and recognizable mass over anatomy flourish.
- Paint surfaces cleanly, but keep claws, bone edges, horns, teeth, robes, and limb breaks slightly graphic rather than soft.
- Creature detail should support combat reading, not become poster art.

### 3. Environment / structural families
- Walls, floors, ceilings, doors, stairs, and ornaments should read in large planes first.
- Preserve classic DM depth grammar and tile/step rhythm.
- Keep wall ornaments and door trims modular and readable, not baroque.
- Texture density should be lower than modern concept art; form separation matters more than material spectacle.

### 4. Items / icons / action-hand families
- Treat icons as tiny game-readable symbols first, painted objects second.
- Preserve chunky silhouettes, grip shapes, blade heads, bottle bodies, and rune marks.
- Avoid reflective metal glamor and over-detailed sub-parts that disappear at play scale.

### 5. Effects / projectiles / magic
- Effects must read instantly and stay subordinate to gameplay geometry.
- Use layered glow, trails, smoke, sparks, mist, and impacts sparingly.
- Favor shape clarity and timing hooks over bloom-heavy spectacle.
- A V2 effect may be cleaner than DM1, but it should still feel dangerous and gamey rather than cinematic.

### 6. Portrait families
- Portraits should read as compact champion payloads, not splash illustrations.
- Favor bust clarity, face planes, and equipment silhouette.
- Keep backgrounds quiet and value-grouped for HUD readability.

### 7. Title / intro / end / branding
- Branding surfaces may be cleaner and more painted than DM1, but should stay heavy, iconic, and dark-fantasy.
- Prefer bold emblematic compositions over busy montage layouts.
- Avoid chrome, lens-flare polish, or fantasy-book-cover excess.

### 8. Typography / text-bearing families
- Replace old raster text with a restrained V2 text system.
- Typography should feel carved, archival, or utilitarian rather than sleek-app modern.
- Text must support the UI structure without becoming the focal ornament.

## Production guardrails

- Do not claim remastered coverage where the repo only has planning or stubs.
- Prefer explicit manifests, inventories, and class briefs over vague art-direction notes.
- Keep provenance and class status honest: `planned`, `in-progress`, `rebuilt`, or `blocked`.
- If a family is not pipeline-ready yet, land the brief, size contract, and manifest hook instead of faking finished art.
