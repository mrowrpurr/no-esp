#pragma once

#include "Selector.h"

namespace NoESP::Expressions {
    
    enum ExpressionType { Action, BindScript, EventHandler, FunctionCall, Unset };

    struct ActionInfo {

    };

    struct BindScriptInfo {
        // std::string ScriptName;
    };

    struct EventHandlerInfo {

    };

    struct FunctionCallInfo {

    };

    struct ExpressionInfo {
        ExpressionType Type = ExpressionType::Unset;
        ActionInfo ActionInfo;
        BindScriptInfo BindScriptInfo;
        EventHandlerInfo EventHandlerInfo;
        FunctionCallInfo FunctionCallInfo;
    };
}
