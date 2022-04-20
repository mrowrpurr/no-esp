scriptName InitializeMe extends ObjectReference

; When the game loads, try calling a crazy global function
event OnInit()
    CrazyIdeasNativeFunctions.CallThePapyrusFunction()
endEvent
