# Security Policy

## Reporting a vulnerability

Firestaff does not run network services, does not accept untrusted network input, and does not escalate privileges. The attack surface is small. That said:

If you find a vulnerability in Firestaff — particularly:
- Save-file parsing that could execute code from a malicious save file
- Config-file parsing that could execute code from a malicious config
- Asset-file parsing that could execute code from a malicious DUNGEON.DAT
- Any path-traversal issue in `fs_portable_compat`
- Any integer overflow in parsing code

Please email **bosse@danielnylander.se** with details. We will respond within 7 days.

Please do NOT open public issues for security bugs. We will credit you in the fix commit if you wish.

## Supported versions

Currently only the latest commit on `main` is supported. Tagged releases will be supported for 12 months each once we start tagging.

## Non-vulnerabilities

The following are NOT security issues:
- Determinism mismatches between platforms (please report as regular bugs)
- Original-game bugs that Firestaff faithfully preserves (toggle them off in settings)
- Assertion failures in debug builds (they exist to catch bugs)
