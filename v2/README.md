# `No .esp`

- How can we distinguish between the below argument signatures or whatever?

- And how can we try to keep things happening inside of Skyrim? :) But with TDD!

```
Call ScriptName.FunctionName [Arguments...]
Bind ScriptName [*Selector*] [Properties...]
EventName [Event Conditions] FunctionName [Arguments...]
ActionName [Argments...] [Function||Script w/ arguments]

... AND WICKED BADASS PAPYRUS FUNCTIONS

Examples:

OnKeyPress Key=G Shift=True MyScript.CoolFunction 
OnHit Source=[Spell]:*Fire* Target=PlayerRef MyScript.MyFunction
```
