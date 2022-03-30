scriptName BindScript hidden
{Attach Papyrus scripts to forms/objects at runtime.}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

function ToForm(string theScriptName, Form theForm, bool addOnce = false) global native
{If you set the `addOnce` argument to `true`, the script will _only_ be attached to the form
if another script with the same name is not _already_ attached.
    
By default, `BindScript.ToForm()` will add _multiple_ instances of the provided script to the given form.}

function ToEditorId(string theScriptName, string editorId, bool addOnce = false) global native
{If you set the `addOnce` argument to `true`, the script will _only_ be attached to the form
if another script with the same name is not _already_ attached.
    
By default, `BindScript.ToForm()` will add _multiple_ instances of the provided script to the given form (if found by editor ID).}
