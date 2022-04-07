file(GLOB_RECURSE UNIT_TESTS_SOURCE *.cpp *.hh)

add_executable(NoEsp.UnitTests ../specHelper.cpp ../specHelper.h ${UNIT_TESTS_SOURCE})

# NoESP headers
target_include_directories(NoEsp.UnitTests PRIVATE ../../include)

# Bandit (testing framework)
find_path(BANDIT_INCLUDE_DIRS "bandit/adapters.h")
target_include_directories(NoEsp.UnitTests PRIVATE ${BANDIT_INCLUDE_DIRS})

# Snowhouse (test assertions)
find_path(SNOWHOUSE_INCLUDE_DIRS "snowhouse/assert.h")
target_include_directories(NoEsp.UnitTests PRIVATE ${SNOWHOUSE_INCLUDE_DIRS})

# spdlog for logging
target_link_libraries(NoEsp.UnitTests PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
