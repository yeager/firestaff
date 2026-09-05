# CSB retail catalog coverage

The native `firestaff_csb_source_text_dump` tool was run on authentic user-owned
media on 2026-09-05. Archive contents remained in memory. Generated catalogs
and diagnostic output stayed in local development storage.

| Edition | Graphics MD5 | Dungeon MD5 | Unique non-header messages | Swedish catalog comparison |
|---|---|---|---:|---|
| Atari ST | `ebf6a57af3f27782e358c0490bfd2f2e` | `6695d2acebce49f95db1d8f3a5c733de` | 221 | Pass |
| FM Towns Japanese | `761d6fc588b31aeaaa9caf3725e111b9` | `7ca51c17ef8bd542ca5f0273672ec1a5` | 218 | 216 undefined messages |

The comparison used `msgcmp --use-untranslated po/csb.sv.po GENERATED.pot`.
It verifies message identity coverage, not translation quality or displayed
glyph correctness. The source extraction admits M564 object names, C699
action names, and the reviewed Utility instruction; it is not a full audit of
every inscription, executable dialog, or Hint Oracle message.

The Japanese extraction reads M564 from graphic 694 (3,629 bytes) and
DYNA_BUTTONS at source offset `0x2a0ec` (336 bytes). The next implementation
step is to bind translation entries to these actual source indices and test
the selected Japanese media's live presentation path. Do not infer complete
retail coverage from the completion percentage of an English-source catalog.

## Catalog admission and native lookup verification

The initial comparison above preceded admission of 216 missing Japanese keys
to every CSB locale. The shared template now contains 437 messages. Swedish
adds 39 reviewed action translations using the corresponding existing English
action translations; the extracted Japanese corpus has 41 nonempty Swedish
entries and 177 untranslated entries (including object names awaiting review).
All 218 authentic keys were checked through `fs_po_load` and
`fs_po_gettext_in_domain`: translated values and untranslated source fallbacks
match the catalog. `po/update.sh --check` passes. These checks do not establish
rendered glyph parity or complete dungeon-text coverage.
