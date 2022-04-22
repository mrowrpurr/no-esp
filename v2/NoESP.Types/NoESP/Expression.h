#pragma once

namespace NoESP {
    enum ExpressionType { BindScript, Unset };

    struct Expression {
        ExpressionType Type = ExpressionType::Unset;
        std::string ScriptName;
        std::string FunctionName;
    };
}
