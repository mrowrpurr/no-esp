#include "SpecHelper.h"

int SpecHelper::RunBanditTests(int argc, char* argv[]) {
    // spdlog::set_level(spdlog::level::off);

    // TODO : only add --reporter=spec if --reporter is not already provided as an argument
    auto reporterArgument = "--reporter=spec";

    // Update the arguments to include --reporter=spec (my favorite default reporter!)
    int new_argc = argc + 2;
    char** new_argv = new char*[new_argc];
    new_argv[argc + 1] = nullptr;
    for (int ii = 0; ii < argc; ++ii) {
        new_argv[ii] = argv[ii];
    }
    new_argv[argc] = new char[strlen(reporterArgument) + 1]; // extra char for null-terminated string
    strcpy(new_argv[argc], reporterArgument);

    return bandit::run(new_argc, new_argv);
}
