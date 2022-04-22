file(GLOB_RECURSE NoESP_Parser_Tests_Source *.cpp *.h)

add_executable(NoESP_Parser_Tests ${NoESP_Parser_Tests_Source} ../SpecHelper.h ../SpecHelper.cpp)

target_include_directories(
    NoESP_Parser_Tests PRIVATE
    ${BANDIT_INCLUDE_DIRS}
    ${SNOWHOUSE_INCLUDE_DIRS}
    ../../NoESP.Types
    ../../NoESP.Parser
)