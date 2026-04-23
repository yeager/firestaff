# Firestaff V2 Visual Reset

## Why this reset exists

The first in-game V2 4K image (`verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png`) was rejected by the user as "0%" and "does not look like Dungeon Master at all".

That judgement is correct.

From this point on, V2 visual decisions must be checked against ReDMCSB-first expectations. If Firestaff and ReDMCSB differ without a strong reason, assume Firestaff is wrong.

This reset does **not** approve the current V2 direction. It narrows the next pass to a bounded correction so future V2 image work stays recognizably DM.

## Primary reference rule

Use ReDMCSB / DM1 in-game composition as the primary visual reference.

For V2, enhancement is allowed in:
- resolution
- paint fidelity
- texture clarity
- cleaner separation of layers

Enhancement is **not** allowed to change the screen's core read:
- the viewport must still be the main event
- the dungeon scene must still read instantly as DM space
- the UI must still feel like DM structure, not a fantasy dashboard

## Current failure analysis: why the first V2 4K image fails

### 1. Composition failure
The rejected image reads as a mockup board made of decorated panels, not as a live DM gameplay screen.

What went wrong:
- the screen is organized as a showcase of framed modules
- the eye lands on borders, boxes, and labels before the dungeon
- the whole image reads like a presentation layout rather than a game state

What ReDMCSB preserves:
- one gameplay composition
- one dominant viewport read
- surrounding UI that supports the scene instead of competing with it

### 2. Viewport dominance failure
The viewport frame is large, but the actual playable scene inside it is weak and underfilled.

What went wrong:
- the inner scene is mostly empty decorative brown space
- the monster occupies only a small inset area
- the viewer does not immediately read "walkable dungeon space"

What must be preserved from DM / ReDMCSB:
- the viewport must be the first thing the eye understands
- the dungeon scene must occupy enough of the opening to feel active
- the frame should support the aperture, not overpower it

### 3. Depth reading failure
The image does not construct classic DM depth.

What went wrong:
- no strong corridor / room recession
- no convincing side-wall convergence
- no tile-step rhythm or floor progression carrying distance
- no clear sense of front/mid/far occupancy

What must be preserved:
- immediate first-person dungeon depth
- legible wall/floor/ceiling separation
- classic DM front-facing spatial compression

### 4. Creature staging failure
The skeleton reads as a pasted promo figure, not as a dungeon encounter.

What went wrong:
- centered like an illustration insert
- detached from the scene's perspective
- not anchored to a tile distance or encounter lane
- more like key art than gameplay staging

What must be preserved:
- creatures must sit inside dungeon space
- encounter distance must be readable at a glance
- monster scale must follow the viewport's perspective logic

### 5. UI weighting failure
The UI is too heavy, too segmented, and too self-important.

What went wrong:
- right-side modules read like dashboard panels
- the bottom strip reads like a modern dock
- framing mass competes with the viewport instead of supporting it
- large blocks feel like placeholder UI scaffolding rather than DM structure

What must be preserved:
- dense but functional DM panel logic
- strong right-column identity without becoming a control dashboard
- bottom HUD as compact game structure, not a modern toolbar

### 6. Palette / color character failure
The image has the wrong material and color character.

What went wrong:
- smooth brown/gold fantasy dashboard surfaces
- modern-feeling glow/trim emphasis
- accent blocks that read as synthetic UI scaffolding
- not enough of the harsh, constrained, old-computer DM color character

What must be preserved:
- darker, harsher, more restrained DM-style value grouping
- old-game readability before ornamental finish
- material identity that feels closer to stone / aged frame / bitmap display than polished fantasy UX

### 7. Recognizability failure
Most importantly, the image does not read instantly as Dungeon Master.

It reads as:
- a fantasy-themed interface concept
- a polished UI mockup
- a framed monster showcase

It does **not** read as:
- classic DM in-game exploration/combat
- a ReDMCSB-rooted gameplay screen
- a direct descendant of DM1 composition

## What V2 must preserve from DM / ReDMCSB

These are non-negotiable recognizability anchors.

1. **Viewport-first read**
   - The player must understand the dungeon scene before noticing style treatment.

2. **Classic DM depth grammar**
   - corridor/room recession
   - floor/wall distance cues
   - readable front/mid/far staging

3. **Creature-as-encounter staging**
   - creatures belong to the dungeon scene, not to a promo layer

4. **DM screen structure**
   - strong viewport
   - meaningful right-side command/info weight
   - compact bottom HUD structure
   - no modern app/dashboard composition

5. **Restrained palette character**
   - clarity over polish
   - retro-leaning value grouping
   - no neon/scaffolding accents unless source-justified

6. **Immediate recognizability at thumbnail size**
   - if the image is shrunk and no longer reads as DM, the pass failed

## What to discard or deprioritize from the current V2 direction

Discard or heavily deprioritize these choices in the next pass:

- mockup-like multi-panel composition
- oversized decorative framing that steals attention from the scene
- modern dashboard / dock / widget logic
- bright cyan or similarly synthetic accent backplates
- pasted-in hero-creature presentation
- smooth fantasy-dashboard polish as the main identity
- empty ornamental space inside the viewport
- any choice that improves "premium UI" at the cost of DM recognizability

## Corrected bounded spec for the next V2 image pass

This is the only target for the next image pass.

### Goal
Produce **one** corrected V2 in-game image that reads immediately as DM/ReDMCSB first, with only bounded high-resolution enhancement.

### Composition constraints
- Use the DM/ReDMCSB gameplay screen as the base composition logic.
- The viewport must remain the dominant read.
- The right-side area may be cleaner/high-res, but must still read as DM command structure rather than a modern sidebar.
- The bottom HUD must stay compact and segmented, not dock-like.
- No title text, capture labels, promo framing, or presentation-board styling.

### Viewport constraints
- Fill the viewport with a real dungeon scene, not a mostly empty ornamental chamber.
- Show readable front-facing depth with clear wall/floor recession.
- Preserve DM-style spatial compression rather than widening into a cinematic view.
- The scene should still work when reduced to 25% scale.

### Creature constraints
- Use at most one clearly staged creature for the next pass.
- Stage it at a readable DM-like encounter distance.
- The creature must feel embedded in the scene's perspective and lighting.
- No portrait-like cutout posing.

### UI constraints
- Keep UI mass subordinate to the viewport.
- Reduce ornamental bevel complexity.
- Avoid empty decorative blocks.
- Keep the right column and bottom row structurally close to ReDMCSB.
- If a UI element looks like a modern panel widget, remove or simplify it.

### Palette / material constraints
- Bias toward restrained dark values and DM-like contrast grouping.
- Do not use cyan/teal support blocks unless directly source-justified.
- Prefer tactile stone/aged-frame/bitmap-display character over glossy fantasy polish.
- High resolution may add texture clarity, but must not change the original value logic.

### Bounded enhancement allowance
Allowed:
- cleaner texture separation
- sharper paint fidelity
- better edge discipline
- subtle material richness
- improved small-detail readability after downscale

Not allowed:
- redesigning the screen grammar
- converting DM into a modern fantasy HUD
- widening or relaxing the original DM spatial feel
- replacing encounter staging with key-art presentation

## Next-pass prompting guidance

Future image prompting for this pass should emphasize:
- ReDMCSB-rooted Dungeon Master in-game screen
- viewport-first composition
- compact right-side command structure
- compact segmented bottom HUD
- embedded creature in classic DM perspective
- restrained palette and old-game read

Future image prompting should explicitly reject:
- dashboard UI
- cinematic monster showcase
- mockup board composition
- neon accent blocks
- oversized ornamental panel framing

## Definition of done for this reset

Do **not** continue broad V2 image production until a new candidate passes all of the following:
- reads instantly as DM at first glance
- preserves ReDMCSB-like composition hierarchy
- shows real dungeon depth
- stages the creature as an encounter, not a poster subject
- keeps UI secondary to the viewport
- avoids the discarded choices listed above

If those are not true, the candidate is not a refinement of V2. It is a wrong-direction redo.
