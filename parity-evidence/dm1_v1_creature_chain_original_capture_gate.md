# DM1 V1 creature-chain original-capture gate

Status: BLOCKED_DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_GATE_LOCKED

This is a blocker gate. It prepares the two route rows needed for original DOS creature-chain comparison and does not promote pixel parity.

## Required rows
- creature_chain_d2c_trolin_front viewSquare=D2C raw=320x200 viewportCrop=0,33,224,136
- creature_chain_d1c_trolin_front viewSquare=D1C raw=320x200 viewportCrop=0,33,224,136

## Source audit
- PASS DUNVIEW.C:4547-5586 f0115_creature_chain_draw_route - creature chain evidence must bind to the original F0115 pose/bitmap/palette/flip path
- PASS DUNVIEW.C:1656-1685 pc_i34e_creature_aspect_table - Trolin type 16 aspect row resolves to firstNative 51, coordinate/transparent byte 0x04, and replacement sets 0x65
- PASS DEFS.H:2392-2392 pc_i34e_first_creature_graphic - front native bitmap index for Trolin is 584 + 51 = 635

## Document audit
- PASS gap_doc_names_missing_original
- PASS gap_doc_requires_d2c
- PASS gap_doc_requires_d1c
- PASS runbook_scope_has_creature_chain
- PASS contract_path_documented
- PASS runbook_names_creature_chain_d2c_trolin_front
- PASS runbook_names_creature_chain_d1c_trolin_front

## Decision

The creature-chain original comparison route is narrowed to two Trolin viewport rows, D2C then D1C, with canonical PC 3.4 asset hashes and 320x200/raw plus 224x136 viewport-crop requirements. The lane remains blocked on real original DOS screenshots.

## Non-claims
- No original creature screenshot is supplied by this contract.
- No original-vs-Firestaff pixel parity is promoted by this contract.
- No game logic or rendering behavior is changed by this contract.
