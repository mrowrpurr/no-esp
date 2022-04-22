#pragma once

#include <span>

#include <NoESP/Expression.h>

namespace NoESP {
    class Parser {
    public:
        static std::span<Expression> Parse(const std::string& text) {
            return {};
        }
    };
}
