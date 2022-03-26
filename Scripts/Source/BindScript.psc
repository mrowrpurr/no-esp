scriptName BindScript hidden
{...}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

function ToObject(string theScriptName, ObjectReference obj) global native
{...}

function ToEditorId(string theScriptName, string editorId) global native
{...}

function ToFormId(string theScriptName, int formId, string optionalPluginFile = "")
{...}
