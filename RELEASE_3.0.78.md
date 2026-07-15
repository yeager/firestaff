# Firestaff v3.0.78

Firestaff v3.0.78 packages the latest source-owned runtime work from `main`.

## Highlights since v3.0.77

- **DM1/CSB**: corrected source-region ownership for dungeon presentation,
  title/HUD routes, and ReDMCSB-backed material ordering checks.
- **DM2**: runtime HUD hero types now consume verified GDAT source metadata.
- **Theron's Quest**: Track 02 loading remains fail-closed where the original
  level and object bindings are not yet proven.
- **Nexus**: PRS3 startup tracing now records only authenticated decoder
  evidence; unproven Saturn graphics are not substituted.

## Verification

- Focused source-gate and runtime checks for DM1, CSB, DM2, Theron, and Nexus
  passed locally.
- GitHub Actions builds and packages platform artifacts from the `v3.0.78` tag.
