# 1.0.0

- [x] Bind to references by 0x0 reference form ID (once per script)
- [x] Test that scripts are still attached after a save game
- [x] Support finding forms from other mods
- [x] Bind by calling Papyrus function to Bind a script to an Object
- [x] Remove '*'
- [x] Skyrim SE + VR + AE
- [x] Release on Nexus

# 1.1.0

- [x] On first location load (aka coc) IF NOT A NEW GAME then load ALL THE THINGS
- [x] Support `;` and `#` and `//` comments in the AutoBinding files (strip them out, incl on the same line `Script 0x123 # foo`
- [x] Oh shit. Whoops. Only 1 script can register itself for a Base Form!
- [x] Fill properties via `ScriptName:SomeProperty 0x123 [plugin.esp]`
- [x] Autobind based on Keyword
- [x] Autobind based on FormList
- [x] Document the Papyrus functions
- [x] Record a new demo!
- [x] Fix: when the script doesn't exist, it blows up
- [x] Double check: BindScript.ToForm and .ToEditorID don't blow up if None/not found or non-existent script!
- [x] Papyrus function to get a Form by an Editor ID
- [x] Release Update on Nexus

# 1.1.1

- [x] Ignore files that don't end in .txt
- [x] Can provide a *partial* Editor ID (`*foo`, `foo*`, `*foo*`, `*foo*bar*`, `/^foo.+bar$/`)
- [x] Update log for script bound to include the hex form ID

# 1.2.0

Project Stuff:
- [x] CMake cleaned up and working for all Skyrim versions WITHOUT a local vcpkg! Or with just one!
- [x] Compiled OK in VSCode for all 3 Skyrim Versions
- [x] .bat files for compiling any version
- [ ] Document configurable folder output for .dll and scripts

Features:
- [x] Process every reference in the game on game load
  - [x] Capture Main Menu visit or leave
- [ ] Can provide the name of a form type, e.g. `BOOK`
- [x] Papyrus function which will process every form in the game for script attachment
- [x] Papyrus function which will process every object near the player or an object or a point in space
- [x] .ini config for searching nearby on a regular basis

# Config:
- [x] Add .ini file support
  - [x] .ini - Can toggle console logs on and off
  - [x] .ini - Can disable on load object reference search

# Logs:
- [x] Add a .log file specifically for no-esp (rewrite with each game rerun) 

# Property Setting:
- [ ] Try getting property types FROM OBJECTS rather than the raw type <---
- [ ] Can you list properties from the Main Menu? NO - CANNOT GET TYPES!
  - [ ] What if not loaded, can you Process via linker from that context?
  - [ ] Goal: eager loading of property types to not do checking at runtime
- [ ] Support setting property values in-line for simple literal values and anything based on an editor ID or formID/esp pair

## Soon:
- [ ] Refactor System into files that make sense!

## Later:
- [ ] Skyrim LE
