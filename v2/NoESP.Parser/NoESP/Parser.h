#pragma once

#include <span>

#include <NoESP/Expressions.h>

namespace NoESP {
    class Parser {


    public:
        static std::vector<ExpressionInfo> Parse(const std::string& text) {
            static std::regex TODO{""}; 

            std::vector<ExpressionInfo> expressions;



            // expressions.emplace_back(ExpressionInfo{
            //     .Type=ExpressionType::BindScript,
            //     .BindScriptInfo={ .ScriptName=text }
            // });
            return expressions;
        }
    };
}
