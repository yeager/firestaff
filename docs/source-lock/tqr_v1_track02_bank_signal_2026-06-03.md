# Theron V1 Track 02 Bank Signal - 2026-06-03

## Scope

This note records one narrow Track 02 bank/descriptor signal. It does not
claim a dungeon map-grid parser, a dungeon descriptor table, or JP/US parity.

## Verified Inputs

| Variant | File checked | MD5 | Result |
|---------|--------------|-----|--------|
| US Track 02 ISO | `TQUS02End.iso` | `3d8b78571dcd0e6eb8eb4b01eeb7fbba` | One unique bank-stride descriptor candidate found. |
| JP Rev 1 Track 02 ISO | `TQJP02End.iso` | `397039af02d50d15c70b74088eb8a1cb` | Image is zero-filled in the available asset, so no dungeon-bank offset is claimed. |

## US ISO Signal

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
sequence occurs once in the verified US Track 02 ISO.

## Regression Gate

`firestaff_theron_v1_track02_bank_probe` verifies:

- the US file MD5 before asserting offset `0x1584`
- the 9-word `0x0400` stride sequence
- uniqueness of the descriptor bytes within the US ISO
- the JP Rev 1 zero-filled image outcome as insufficient evidence

The probe skips real-data assertions when the Track 02 images are absent.

## Remaining Risk

This is a bank-stride descriptor candidate, not a decoded dungeon map. Later
work still needs to connect the bank signal to loader code and find actual
dungeon descriptors, map grids, object tables, and party/champion seed data.
