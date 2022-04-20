scriptName InitializeMe extends ObjectReference

; When the game loads, try calling a crazy global function
event OnInit()
    RegisterForSingleUpdate(10)
endEvent

event OnUpdate()
    Debug.MessageBox("CALLING IT..............")
    CrazyIdeasNativeFunctions.CallThePapyrusFunction()
    Debug.MessageBox("CALLED IT!")
endEvent
