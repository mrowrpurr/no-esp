# TODO

- [x] Bind to references by 0x0 reference form ID (once per script)
- [x] Test that scripts are still attached after a save game
- [x] Support finding forms from other mods
- [x] Bind by calling Papyrus function to Bind a script to an Object
- [x] Remove '*'
- [x] Skyrim SE + VR + AE
- [x] Release on Nexus
- [x] On first location load (aka coc) IF NOT A NEW GAME then load ALL THE THINGS
- [x] Support `;` and `#` and `//` comments in the AutoBinding files (strip them out, incl on the same line `Script 0x123 # foo`
- [ ] Oh shit. Whoops. Only 1 script can register itself for a Base Form!
- [ ] Fill properties via `ScriptName:SomeProperty 0x123 [plugin.esp]`
- [ ] Autobind based on Keyword
- [ ] Autobind based on FormList
- [ ] Add .ini file support
  - [ ] Can toggle logs on and off
  - [ ] Can toggle coc support ON (default: off)
- [ ] Document the Papyrus functions
- [ ] Release Update on Nexus

## Optional:
- [ ] Papyrus function to get a Form by an Editor ID
- [ ] Papyrus function which will process every form in the game for script attachment
- [ ] Papyrus functiuon which will process every object near the player or an object or a point in space

## Later:
- [ ] Skyrim LE

### Maximum Allowed ESPs per Game

- AE: ~4000
- SE: ~4000
- VR: 254
- LE: 255


