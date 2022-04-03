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
- [ ] CMake cleaned up and working for all Skyrim versions WITHOUT a local vcpkg! Or with just one!
- [ ] Compiled OK in VSCode for all 3 Skyrim Versions
- [ ] Configurable folder output for .dll and scripts

Features:
- [ ] Process every reference in the game on game load
  - [ ] Capture Main Menu visit or leave
- [ ] Can provide the name of a form type, e.g. `BOOK`
- [ ] Papyrus function which will process every form in the game for script attachment
- [ ] Papyrus function which will process every object near the player or an object or a point in space

# Config:
- [ ] Add .ini file support
  - [ ] .ini - Can toggle logs on and off
  - [ ] .ini - Can set logs to be either Console or File or both

# Logs:
- [ ] Add a .log file specifically for no-esp (rewrite with each game rerun) 

## Later:
- [ ] Skyrim LE
