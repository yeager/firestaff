# Pass 154 — static 48ed party-control blocker fix

`48ed3743ab6a` is now a known static no-party dungeon placeholder in the pass113 party-state gate. It cannot satisfy `party_control_ready` even if captures are unique; it is explicitly reported as `static_no_party_hashes_seen` and folded into the direct-start/no-party blocker signature.

Evidence basis: pass151/pass153 showed helper keys, xdotool key/type delivery, and panel clicks remain on static/non-control 48ed sequences.
