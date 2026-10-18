# Firestaff v3.0.387

## Developer changes

- `DM1, CSB and DM2 startup verification`: verify original-media CLI and menu routes, return-to-menu rescans and direct-game exit behavior across the tested platforms. The DM2 AUTO route selects FM Towns when its authenticated media are installed, reaches New Game, matches the original title palette digest and proceeds to the first champion.
- `DM1 Apple IIGS startup verification`: verify both original ProDOS editions reach the first world through CLI and M12 menu launch; the original intro, entrance, audible DOC playback and complete campaign remain open.
- `Startup verification limits`: SDL dummy video and audio confirm runtime state and captured pixels only; physical display, input, audio output and complete visual parity remain unverified.
