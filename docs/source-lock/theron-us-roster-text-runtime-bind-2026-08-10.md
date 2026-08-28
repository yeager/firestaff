# Theron US Roster Text: Runtime Binding (2026-08-10)

The verified startup runtime path now uses
`theron_v1_track02_catalog_startup_roster_names()` directly on the authentic
Track 02 file before forcefield party initialisation. For known US and JP BINs,
the source-bound catalog replaces the host's optional name list. Every name
retains its raw offset/provenance in the catalog and is then passed to the
existing party/HUD text path.

This closes a concrete discrepancy in which a host-supplied name could override
a real codon record. If the catalog cannot be verified, the handoff is rejected
without a partial party state.

The boundary is deliberate: this proves Firestaff's text binding, but not that
the original HuC6280 text consumer has the same byte/font/VDC route. US titles,
font graphics, JP portraits, and the original's full presentation still require
a source-bound runtime capture.
