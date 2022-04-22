#pragma once

#include <span>

#include <NoESP/Expressions.h>

namespace NoESP {
    class Parser {
    public:
        static std::span<ExpressionInfo> Parse(const std::string&) {
            return {};
        }
    };
}
