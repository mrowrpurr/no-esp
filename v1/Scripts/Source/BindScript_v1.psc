scriptName BindScript_v1 hidden
{Attach Papyrus scripts to forms/objects at runtime.}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

function ToForm(string theScriptName, Form theForm, bool addOnce = false) global native
{Bind a new instance of the provided script to the provided form.

If you set the `addOnce` argument to `true`, the script will _only_ be attached to the form
if another script with the same name is not _already_ attached.
    
By default, `BindScript.ToForm()` will add _multiple_ instances of the provided script to the given form.}

function ToEditorId(string theScriptName, string editorId, bool addOnce = false) global native
{If you set the `addOnce` argument to `true`, the script will _only_ be attached to the form
if another script with the same name is not _already_ attached.
    
By default, `BindScript.ToForm()` will add _multiple_ instances of the provided script to the given form (if found by editor ID).}

function SearchObjectsInRadius(float radius, ObjectReference center = None) global native
{Iterate over every Object Reference within the provided radius and attach script(s) when necessary.

By default, the center point is the Player, but you can provide your own Object Reference to be used as a center point for the search.}

function SearchAllObjects() global native
{Iterate over every single loaded Object Reference in the game and attach script(s) when necessary.}
