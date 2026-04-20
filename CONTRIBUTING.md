# Contributing to Firestaff

Firestaff is in early development. As of M10 (engine core complete) we are not yet accepting code contributions — the architecture is still shifting. Once M11 lands (first playable) we will open contribution channels.

Until then, the most useful things you can do:

## Help wanted now

- **Original-game asset hashes**: if you own a legitimate copy of DM1 / CSB / DM2 in any variant (platform × version × language), help us verify the MD5 database. Open an issue with the variant name and the MD5s of the core files. We do NOT want the files themselves — only their hashes.
- **Bug database corrections**: `firestaff-bugs.json` contains 201 entries derived from Christophe Fontanel's ReDMCSB documentation. If you spot an error, file an issue.
- **Testing M10 verify on exotic platforms**: we test on macOS (Apple Silicon), Ubuntu (x86_64), and Windows (x86_64). If you have FreeBSD / Alpine / Raspberry Pi / anything else, run `./run_firestaff_m10_verify.sh` and report.

## Not accepted yet

- Pull requests against engine code (M10). The architecture will be wrong by the time you finish.
- Implementation of UI / rendering (M11). We have a detailed plan; starting would conflict.
- Translations. These come in M12 once strings are extracted.

## Code of conduct

Be respectful. This is a preservation project for a 40-year-old game built by five people who deserve the world. Treat their work with the same care you'd want for your own.

## Legal

By submitting anything (issue, hash, patch if that ever opens), you license it under the MIT licence of the rest of the project.

Firestaff does not distribute, redistribute, or facilitate piracy of Dungeon Master, Chaos Strikes Back, or Dungeon Master II. Any issue or PR that includes original game assets will be closed and deleted.
