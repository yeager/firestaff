# CSB F0366-F0385 Command/Viewport Audit

ReDMCSB authority: `COMMAND.C` F0366-F0380 and `MENUS.C` F0381-F0385.
The CSB receipt requires loaded PC34 map bytes and an authenticated
CSBgraphics HUD/palette source. It is read-only and fails closed.

| Symbols | Source responsibility | CSB receipt boundary |
| --- | --- | --- |
| F0366-F0368 | Party move, status-box click, leader selection | Command/party mutation owner required |
| F0369-F0371 | Spell/action click dispatch | Spell/action runtime owner required |
| F0372-F0377 | Dungeon-view wall, hand, throw and hit-test routes | Viewport/inventory owner required |
| F0378-F0380 | Panel click, sleep screen, command queue | Panel/queue owner required |
| F0381-F0385 | Message replacement, charge, action list/name/damage | Menu/text/font/render owner required |

No synthetic command, leader, inventory, queue, font, text, or framebuffer
operation is admitted by this contract.
