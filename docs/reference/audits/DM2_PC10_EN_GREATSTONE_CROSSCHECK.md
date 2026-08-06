# DM2 PC 1.0 English Greatstone cross-check

Scope: a read-only comparison of the local, hash-verified DM2 PC English
`GRAPHICS.DAT` corpus and Greatstone's PC 1.0 English extraction catalogue.
It does not copy, unpack, or bundle original game data.

## Inputs

| Source | Observation |
| --- | --- |
| Mounted local PC English `graphics.dat` | 8,639,757 bytes; SHA-256 `c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346` |
| Firestaff `docs/VERIFIED_HASHES.md` | Same PC English identity and size |
| Greatstone [PC 1.0 English GRAPHICS.DAT catalogue](http://greatstone.free.fr/dm/db_data/dm2_pc10_en/graphics.dat/graphics.dat.html) | 5,624 presentational export items |
| DMWeb [DM2 data-file notes](http://dmweb.free.fr/community/documentation/file-formats/dungeon-master-ii-data-files-notes/) | GDAT is a heterogeneous data container, not an image-only list |

## Result

The mounted local file is the source-locked PC English corpus. Firestaff's
ENT1 parser reports 11,854 raw GDAT rows for it. That must not be compared
numerically to Greatstone's 5,624 rendered/exported catalogue items: ENT1 also
contains scalar words, text, raw control streams, palettes and other
non-presentational records. Treating the counts as equal would hide data or
invent a one-to-one mapping.

The real-data regression `test_dm2_v1_gdat_graphicsset_real_data` now verifies
the five G1-referenced graphics styles against their own exact, decodable GDAT
floor/ceiling/control records. Its IMG9 path dispatches original modes 1, 2
and 3 exactly as `SKULLWIN/c_gfx_decode.cpp::decode_img9`; it does not recode
mode 1 as mode 3 or substitute a fallback image.

The broader `test_dm2_v1_gdat_visual_corpus_real_data` walks every `dtImage`
ENT1 row by its exact RAW index. On the mounted source corpus it observes
5,676 image rows, 4,031 distinct RAW image payloads and 18,633,937 decoded
source pixels (census hash `bf5050d3`). Every one decodes through the
SKProject image path. This is a decoder/admission census, not a claim that all
images have a live viewport or HUD owner yet; those consumers must still bind
their exact GDAT address and source draw route.
