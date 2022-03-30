scriptName FormFromEditorID hidden
{}

; BUG in VSCode - first function's documentation is not shown in editor
function _vscode_hack()
endFunction

Form function Get(string editorId) global native
{Returns the form found using the provided editor ID.
Returns `None` if not found.

> _Recommended: po3's Tweaks. Without it, most of the game's editor IDs cannot be used to lookup Forms._}
