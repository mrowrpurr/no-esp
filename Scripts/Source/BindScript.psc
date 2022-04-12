scriptName BindScript hidden
{Attach Papyrus scripts to forms/objects at runtime.}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

bool function Bind(string scriptName, string expression) global native

bool function ToKeyword(string scriptName, Keyword theKeyword, string properties = "") global native

bool function ToFormType(string scriptName, string name = "", int id = 0, string properties = "")

bool function ToForm(string scriptName, Form baseForm, string properties = "") global native

bool function ToObject(string scriptName, ObjectReference ref, string properties = "") global native

; TODO - can we attach to aliases?
bool function ToAlias(string scriptName, Alias theAlias, string properties = "") global native

; TODO - can we attach to active magic effects?
bool function ToActiveMagicEffect(string scriptName, ActiveMagicEffect ame, string properties = "") global native
