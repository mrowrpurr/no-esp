scriptName BindScript hidden
{...}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

function ToForm(string theScriptName, Form theForm, bool addOnce = false) global native
{...}

function ToEditorId(string theScriptName, string editorId, bool addOnce = false) global native
{...}
