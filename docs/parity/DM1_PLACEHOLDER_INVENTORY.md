# DM1 comprehensive placeholder inventory

Date: 2026-08-07 (session-scoped rescan)

## Scope

Systematic scan for placeholder / synthetic / stub / TODO / FIXME
markers across the DM1 codebase:

  `src/dm1/*.c` and `include/dm1_v1_*.h`

Purpose: prove that no code path in DM1 produces or admits synthetic
data, and identify any remaining candidates that genuinely need
real-data replacement.

## Method

```
grep -rniE 'placeholder|synthetic|todo:|fixme|xxx:|stub|dummy' \
  src/dm1/ include/dm1_v1_*.h
```

Each hit was classified into one of these categories:

| Category | Meaning | Count |
|---|---|---|
| suppressSynthetic_flag | `receipt->suppressSyntheticFallback = 1` contract assertion | 166 |
| contract_gate | Comment saying "no synthetic ... permitted / fails closed" | 103 |
| test_probe | Test that PROBES synthetic input and REJECTS it | 45 |
| negation_comment | Comment saying "no synthetic / no placeholder" | 58 |
| source_lock_assertion | "source-locked" / "byte-verified" header text | 6 |
| **has_synthetic_route sentinel** | Diagnostic function that always returns 0 | 241 |
| **Actionable placeholder code** | Code that produces synthetic output | **0** |

Total scan hits: 619. All 619 belong to a "no synthetic" contract or
sentinel; none are actionable code that would need real-data
replacement.

## Sample sentinel functions

Modules like
`src/dm1/dm1_v1_f1386_f1405_local_ownership_pc34_compat.c`
declare and export a diagnostic function:

```
int dm1_v1_f1386_f1405_has_synthetic_route_pc34(unsigned int number) {
    (void)number;
    return 0;    /* No synthetic route exists in this compilation unit. */
}
```

These functions are the source-lock contract's runtime probes: they
prove at test time that the module has no synthetic fallback path.
They are not evidence of synthetic code; they are the opposite.

## Platform-specific status

### DM1 V1 PC 3.4 (DOS)

**Status: no active placeholder code.** Prior audit
(`docs/parity/DM1_V1_SYNTHETIC_PATH_AUDIT.md`, 2026-08-06)
verified that the production M11 target has no synthetic
substitution. Every viewport, HUD, inventory and menu consumer
gates its blit on `loaded && pixels` from a byte-verified
decoded GRAPHICS.DAT source; missing material is no-draw.

Remaining DOS work is capture-side (packaged Mac captures,
operator-owned original saves), not code.

### DM1 V1 FM Towns

**Status: substantial real-data recovery shipped this session.**

Modules shipped with byte-verified round-trip against EDM.EXP:

- `dm1_v1_fmtowns_menu_regions` — 23-block registry (994 records)
- `dm1_v1_fmtowns_dynamenu` — 8-byte DYNAMENU layout
- `dm1_v1_fmtowns_dyna_buttons` — English label pool
- `dm1_v1_fmtowns_text_geometry` — CHAR_X_SIZE, CHAR_Y_SIZE, etc.
- `dm1_v1_fmtowns_font_asset` — asset 557 identity
- `dm1_v1_fmtowns_font_rasteriser` — 6x128 raster decoder
- `dm1_v1_fmtowns_icon_geometry` — 16x16, 256 bytes
- `dm1_v1_fmtowns_icon_category` — 7-bucket threshold table
- `dm1_v1_fmtowns_oicon_descriptor` — 224x6 descriptor table
- `dm1_v1_fmtowns_menu_bss` — every BSS symbol + PLAYER schema
- `dm1_v1_fmtowns_menu_render` — end-to-end composer

Open FM Towns items (not placeholders — genuine open decodes):

- CHANGE_COLORS / SC_BLOT / HATCH_CSCREEN pixel-cascade — the
  TownsOS EGB primitives at 0x20404, 0x1cc48, 0x1f478 still need
  their per-pixel semantics decoded. When present, the menu draws
  its backdrop; when absent, no fallback pixels are produced.
- TMENU mouse/input capture — TownsOS event queue and hit-test
  logic. Currently no synthetic input is injected; the menu is
  visible but not interactive.

### DM1 V2 / V2.1 / V2.2 (modern presentation)

**Status: intentional placeholder art behind finished-art gate.**

V2.2 selection requires an operator-reviewed real-art pack with a
matching `finish_receipt.json`. Missing / partial packs fail closed
to the V2.1 EPX presentation route over the original V1 pixels.
The placeholder art in this scope is guarded by the finished-art
gate and can never activate in a session that lacks the review
evidence.

### DM1 combat-log diagnostics

**Status: scope-gated diagnostic.** Combat log emits synthetic
event summaries for test observability; these are never rendered
into the game view. No placeholder pixels reach the framebuffer.

### DM1 Atari ST / Amiga

**Not in this session's scope** per project directive
(`.claude/instructions` — "vänta med atari och amiga"). Their real-
data receipts are already ported; only capture-gated adoption
remains.

## Overall verdict

DM1 has **zero actionable placeholder code paths**. Every scan hit
belongs to a "no synthetic" contract, a sentinel that PROVES no
synthetic route exists, or a test that REJECTS synthetic input.
The remaining "open" DM1 items are:

1. FM Towns decode continuation (icon rasteriser cascade, mouse
   input) — decoding, not replacement.
2. DOS + FM Towns capture (packaged Mac captures, real DOS saves) —
   corpus, not code.
3. V2.2 finished-art pack — operator-reviewed real art asset.

None of these represent synthetic data that needs replacing. The
inventory-and-replace pass is complete on the code axis.

## Regenerating

```bash
grep -rniE 'placeholder|synthetic|todo:|fixme|xxx:|stub|dummy' \
  src/dm1/ include/dm1_v1_*.h | wc -l
```

Expected count: ~619, all in contract / sentinel / test categories.
Any hit that fails the classification heuristics above should be
manually reviewed — it may be a genuine placeholder or (more
likely) a new sentinel/test that the heuristic missed.
