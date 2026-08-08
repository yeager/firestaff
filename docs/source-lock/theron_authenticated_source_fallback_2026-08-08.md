# Theron authenticated source fallback

Firestaff provides an explicit investigation mode for the authenticated US/JP
Track 02 source loader:

```text
FIRESTAFF_THERON_ALLOW_AUTHENTICATED_FALLBACK=1
```

When enabled, a verified Track 02 request may continue after the missing
original CD-consumer capture and retain the real map/object records loaded from
the supplied media. The mode is not an original-runtime claim. It must not be
used as evidence for RNG, creature AI, T700/T900 semantics, VDC/VCE ownership,
portraits, or event-owned audio.

Without the variable, the production route remains fail-closed and reports the
missing semantic handoff. This separation lets development inspect authentic
records without promoting an inferred consumer into the parity runtime.
