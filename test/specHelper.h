#include <string.h>

#include <spdlog/spdlog.h>
#include <bandit/bandit.h>
#include <snowhouse/snowhouse.h>

using namespace bandit;
using namespace snowhouse;

namespace NoESP {
    template <class... Types>
    void Log(const std::string text, const Types&... args) {
        // Stub function
    };
}

namespace SpecHelper {
    int runBandit(int argc, char *argv[]);
}
