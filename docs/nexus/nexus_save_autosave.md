# Nexus V1 Autosave

## Status: not implemented

Nexus V1 has Firestaff-native FNXS save/load, but no automatic save policy.
There are no timer-based saves, level-change autosaves, or shutdown autosaves.

The current save commands and quick-resume route require an explicit save
operation. `nexus_v1_tick()` and `nexus_v1_load_level()` do not write a save
implicitly.

This is separate from the unresolved original Saturn save consumer. Any
future autosave policy must write only a Firestaff-native FNXS file until
authentic Saturn save evidence exists.
