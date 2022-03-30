scriptName BindScript hidden
{... something about the whole script}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

function ToForm(string theScriptName, Form theForm, bool addOnce = false) global native
{... something about ToObject}

function ToEditorId(string theScriptName, string editorId, bool addOnce = false) global native
{... something about ToEditorId}

function BindScriptsFromAllObjectsInRadiusOfPlayer(int radius = 1000) global native
{...}

function BindScriptsBySearchingAllFormsInTheGame() global native
{...}

Form function GetFormFromEditorId(string editorId) global native
{...}
