# Firestaff v3.0.336

## Developer changes

- `DM1 original-capture diagnostics`: fix short diagnostic-route manifests so
  they report intentionally absent canonical frames instead of crashing while
  preserving the route as rejected parity evidence.

- `DM1 capture-path hygiene`: redact local home-directory prefixes from
  semantic original-route receipts before they are written, preventing
  machine-specific paths from entering generated verification output.
