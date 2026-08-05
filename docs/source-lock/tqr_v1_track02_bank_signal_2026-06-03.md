# Theron V1 Track 02 Bank Signal - 2026-06-03

## Scope

This note records a narrow Track 02 bank/descriptor signal. It does not claim
a dungeon map-grid parser, a dungeon descriptor table, or loader parity.

## Verified Inputs

| Variant | File checked | MD5 | Result |
|---------|--------------|-----|--------|
| US Track 02 ISO | `TQUS19.iso + TQUS02End.iso` | `ceb02343868f80cec899e9b239aff2da` | Supplied retail image contains three exact descriptor/span pairs; the tail alone is not a Track 02 image. |
| US Track 02 raw BIN | `Dungeon Master - Theron's Quest (USA) (Track 02).bin` | `f23601102138f87c33025877767ebf76` | Three exact raw-sector bank anchors found. |
| JP Track 02 raw BIN | `Dungeon Master - Theron's Quest (Japan) (Track 02).bin` | `b7afb338ad31be1025b53f9aff12d73a` | Three exact raw-sector bank anchors found, one raw CD sector earlier than the US anchors. |
| JP Rev 1 Track 02 ISO | `TQJP02End.iso` | `397039af02d50d15c70b74088eb8a1cb` | Image is zero-filled in the available asset, so no dungeon-bank offset is claimed. |

## Supplied Retail US ISO Signal

The supplied retail archive's canonical concatenation has three exact copies of
the descriptor and post-boundary span. The descriptor offsets are `0x5b2406`,
`0x5b4406`, and `0x5b6584`; the corresponding span offsets are `0x207000`,
`0x378000`, and `0x5b8000`. Each descriptor is the same 18-byte little-endian
sequence shown below, and each span is the same 44-byte sequence. This signal
is transport/layout evidence for real bitmap sampling only. It does not bind a
dungeon seed, object table, palette, or square-to-tile meaning.

The production detector requires the full US MD5, all three descriptor offsets,
all three span offsets, and exact occurrence counts. The older `0x1584` /
`0x3000` profile remains available for its separately documented corpus, but is
not substituted for this supplied retail image.

## Legacy US ISO Signal

At byte offset `0x1584`, the US ISO contains one unique little-endian
9-word stride sequence:

```text
20 00 20 04 20 08 20 0c 20 10 20 14 20 18 20 1c 20 20
```

Interpreted as little-endian words, this is:

```text
0x0020, 0x0420, 0x0820, 0x0c20, 0x1020, 0x1420, 0x1820, 0x1c20, 0x2020
```

The stride is `0x0400`, the descriptor is 18 bytes long, and the exact byte
sequence occurs once in the legacy verified US Track 02 ISO corpus.

Immediately after the descriptor, bytes `0x1596..0x2fff` are zero. The next
nonzero run starts at byte offset `0x3000`.

The first 44 bytes at `0x3000..0x302b` are locked as an opaque byte span:

```text
be 80 fe 80 34 81 76 81 d0 81 2a 80 2b 80 38 80
45 80 52 80 5f 80 6c 80 79 80 86 80 a0 80 a5 80
aa 80 af 80 b4 80 b9 80 93 80 00 3f
```

The span occurs once in the legacy verified US Track 02 ISO corpus. The probe treats it as a
boundary signal only: it narrows false positives for the `0x1584` descriptor,
but it does not identify the later table's semantic type or claim map parity.

## Raw BIN Anchors

The cataloged raw Track 02 BINs contain the same descriptor and opaque span
bytes three times. These are not unique byte patterns, so the probe requires
all three exact offsets plus the exact occurrence count.

US raw Track 02 BIN:

| Anchor | Descriptor offset | Descriptor raw sector:user | Span offset | Span raw sector:user |
|--------|-------------------|----------------------------|-------------|----------------------|
| 0 | `0x70be06` | `3141:0x406` | `0x2d53e0` | `1263:0x000` |
| 1 | `0x70e2c6` | `3145:0x406` | `0x47d040` | `2001:0x000` |
| 2 | `0x710904` | `3149:0x584` | `0x712840` | `3153:0x000` |

JP raw Track 02 BIN:

| Anchor | Descriptor offset | Descriptor raw sector:user | Span offset | Span raw sector:user |
|--------|-------------------|----------------------------|-------------|----------------------|
| 0 | `0x70b4d6` | `3140:0x406` | `0x2d4ab0` | `1262:0x000` |
| 1 | `0x70d996` | `3144:0x406` | `0x47c710` | `2000:0x000` |
| 2 | `0x70ffd4` | `3148:0x584` | `0x711f10` | `3152:0x000` |

The JP offsets are exactly one 2352-byte raw CD sector before the US offsets.
This is a bank-anchor parity signal only; it still does not decode the table
or promote the runtime dungeon loader.

## Initial Level Candidate

The raw JP/US Track 02 BINs also carry a loader-compatible startup candidate
at a stable offset relative to the first raw descriptor anchor:

```text
descriptor_base = descriptor_offset - 0x1584
candidate       = descriptor_base - 0x92ce
```

For the US raw BIN this resolves to `0x7015b4`. For the JP raw BIN this
resolves to `0x700c84`. Both candidates have the same 12-byte header shape:

```text
width       = 32
height      = 27
dungeonSeed = 0x0108e938
levelIndex  = 0x0026
payload     = 864 grid bytes
```

`theron_v1_track02_load_initial_level_candidate()` only accepts this exact
header after the descriptor table at the related anchor has decoded
successfully. This is a bounded startup handoff. It does not claim a complete
Track 02 dungeon-record format, object table, text table, palette table, or
all-level parser.

The real raw JP/US candidate's first generic loader floor tile is on the top
edge, so the Track 02 startup handoff applies a bounded initial-pose selector
after the header gate. It chooses the first interior floor tile with a passable
neighbor, preferring east/south/west/north. Current real JP/US candidates prove
`start=(2,1,EAST)`; the synthetic candidate fixture proves the same selector on
`start=(1,1,EAST)`.

### CD Record And Following Boundary

The IPL receipt supplies the only accepted Track 02 `INDEX 01` coordinate:
raw sector 224 for JP and 225 for US. Combining that coordinate with the
hash-gated initial candidate places both real payloads at the same Track 02
record and the same MODE1 user-data offsets:

| Fact | JP raw BIN | US raw BIN |
|------|------------|------------|
| Candidate raw sector | 3122 | 3123 |
| Track 02 record | `0x0b52` | `0x0b52` |
| Level start in record user data | `0x114` | `0x114` |
| Level envelope | `0x36c` bytes | `0x36c` bytes |
| Immediate following boundary | `0x480` | `0x480` |
| Remaining user data in that record | `0x380` bytes | `0x380` bytes |

`theron_v1_track02_capture_initial_level_object_boundary()` parses and
fingerprints this real CD-relative boundary only after both the IPL receipt and
descriptor-relative level candidate validate. The following bytes are called
an **object boundary**, not an object table: no original loader read yet assigns
them an object role. The receipt exposes no object records, creates no tiles or
placements, and remains promotion-blocked. This preserves the completed Soul
Room/forcefield media handoff's fail-closed contract.

### Decoded Startup Envelope, Blocked Object Tail

`theron_v1_track02_decode_initial_level_envelope()` is the only real-media
decode surface for this startup payload. It accepts only the authenticated raw
JP/US BIN variants and reuses the IPL record and descriptor-relative candidate
gates before copying through the MODE1 user-data stream. The decoder establishes
this byte layout within the `0x36c`-byte envelope:

| Byte range | Corpus-backed result | Semantic status |
|------------|----------------------|-----------------|
| `0x000..0x001` | big-endian width `32` | decoded header field |
| `0x002..0x003` | big-endian height `27` | decoded header field |
| `0x004..0x007` | big-endian seed `0x0108e938` | decoded header field |
| `0x008..0x009` | big-endian level index `0x0026` | decoded header field |
| `0x00a..0x00b` | big-endian value `0x0103` | opaque fingerprint only |
| `0x00c..0x36b` | exact `0x360`-byte (32 x 27) grid span | byte-faithful envelope only |

The grid's extent and fingerprint are decoded, but byte values have no new tile,
collision, trigger, object, palette, or visual meaning here. The immediate
following boundary remains `0x480` in the CD record, and the remaining `0x380`
user-data bytes remain an unparsed tail. No object record, count, placement, or
table is returned. The receipt is invalidated for malformed, unknown, or
non-corpus media and does not permit fallback visuals.

## Regression Gate

`firestaff_theron_v1_track02_bank_probe` verifies:

- the supplied retail US file MD5 before asserting its three descriptor/span
  offsets
- the legacy US file MD5 before asserting offset `0x1584`
- the 9-word `0x0400` stride sequence
- uniqueness of the descriptor bytes within the US ISO
- zero-fill after the descriptor through the next nonzero run at `0x3000`
- uniqueness of the 16-byte `0x3000` prefix within the US ISO
- uniqueness of the 44-byte opaque post-boundary span at `0x3000`
- a negative fixture where the descriptor bytes exist without the boundary
  prefix, which must not pass
- a negative fixture where the old 16-byte boundary prefix exists without the
  full 44-byte post-boundary span, which must not pass
- the JP Rev 1 zero-filled image outcome as insufficient evidence
- the US raw Track 02 BIN descriptor/span anchors at three exact offsets
- the JP raw Track 02 BIN descriptor/span anchors at three exact offsets
- raw 2352-byte sector coordinates for every JP/US raw BIN anchor
- synthetic no-data positive fixtures for both raw BIN layouts
- a negative raw fixture where one of the three descriptor anchors is missing

The probe skips real-data assertions when the Track 02 images are absent.

`firestaff_theron_v1_track02_level_handoff_probe` additionally verifies:

- the synthetic descriptor-window to level-loader positive handoff
- the synthetic initial-level candidate positive handoff
- a corrupted initial-level header rejection fixture
- real US raw BIN candidate `0x7015b4` loading as 32x27
- real JP raw BIN candidate `0x700c84` loading as 32x27
- authenticated JP/US envelope header values, `0x360`-byte grid extent, and
  fail-closed extension/object-tail receipt state
- malformed input, unknown media, and a known-MD5-labelled non-corpus sector
  cannot produce an envelope, grid, object claim, or fallback route
- the older descriptor-window handoff remains no-claim on real JP/US raw BIN
  descriptor windows until the actual per-window semantics are decoded

## Post-Descriptor Cross-Region Layout Receipt (2026-07-11)

`firestaff_theron_v1_track02_nonstartup_sector_receipt_probe` now captures
the two MD5-verified raw BINs together and compares structurally
post-descriptor windows without interpreting their bytes. All three replicated
anchors agree on the following physical/container facts:

- descriptor entry ordering;
- 0x0400-byte window length;
- offset relative to that anchor's descriptor table; and
- whether the raw span begins in MODE1 user data, crosses a sector boundary,
  contains non-user-data bytes, or stays logically contiguous.

Every corresponding JP/US raw-span fingerprint differs. This establishes a
shared descriptor-relative container layout only; it is explicit negative
evidence against treating these windows as one region-neutral bitmap, palette,
level, object, or text payload. The comparison receipt is permanently
`opaque_only` and `promotion_blocked`; it exports no payload bytes and cannot
enable a decoder or runtime route.

## Remaining Risk

This now includes one bounded initial-level startup handoff and a shared
opaque post-descriptor container-layout receipt, but not a decoded full dungeon
map. Later work still needs original-loader evidence to decode the surrounding
dungeon records, all levels, object tables, transitions, text/palette payloads,
and party/champion seed data.
