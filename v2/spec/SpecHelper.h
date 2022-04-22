#include <string.h>
#include <iostream>

// #include <spdlog/spdlog.h>
#include <bandit/bandit.h>
#include <snowhouse/snowhouse.h>

using namespace bandit;
using namespace snowhouse;

namespace NoESP {
    template <class... Types>
    void Log(const std::string text, const Types&... args) {
         std::cout << std::format(text, args...) + "\n";
    };
}

namespace SpecHelper {
    int RunBanditTests(int argc, char *argv[]);
}
