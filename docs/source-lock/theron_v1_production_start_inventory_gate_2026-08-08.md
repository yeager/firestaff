# Theron production start-inventory provenance gate

Date: 2026-08-08

`src/theron/theron_v1_champions.c` previously published the DMWeb roster's
starting-equipment list into every production champion's inventory and equip
slots. That list is useful as a fixture description, but it is not a verified
Track 02 object record and there is no recovered T900 start-object consumer
that proves its runtime semantics.

The production build therefore leaves inventory and equipment empty and keeps
the table available only to non-production fixture builds. This is a
source-fidelity gate: missing ownership is represented as missing state rather
than synthetic item state. The remaining task is to recover the original
Track 02 start-object/inventory consumer and bind real object records to it.
