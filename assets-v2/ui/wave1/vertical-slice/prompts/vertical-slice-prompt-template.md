# Firestaff V2 Vertical Slice Prompt Template

Use this template for image-generation or artist handoff notes. Replace bracketed fields only.

## Core style prompt

Create a hand-authored fantasy game UI asset for **Firestaff V2**.

Asset: **[asset id / name]**
Role: **[brief role]**
Canvas: **[exact size]**

Requirements:
- preserve the classic Dungeon Master-era silhouette and proportions for this asset family
- painterly dark-fantasy material treatment
- readable at both the full master size and the exact 50% derived size
- avoid pixel-art enlargement artifacts
- avoid photorealism, plastic UI, glossy sci-fi styling, or fake screenshot content
- keep content/payload zones clean where future icons, portraits, or text must sit
- transparent background where the brief calls for it

Material direction:
- [stone / bronze / iron / lacquered wood mix]
- restrained wear on edges and contact zones
- strong silhouette clarity

Negative guidance:
- no text
- no item icons
- no portraits
- no characters
- no scene backgrounds inside UI openings
- no extra ornaments that reduce readability

Output:
- one clean master suitable for later exact 50% downscale
- edge quality must remain crisp after downscale

## Per-family note blocks

### Viewport frame
- keep the aperture clean for engine compositing
- ornament belongs on the frame, not inside the opening

### Action area
- produce the base shell and separate state overlays, not a single flattened panel

### Status box family
- left and right variants must feel like a designed pair

### Party HUD cell family
- interior must remain open for later item/icon placement
