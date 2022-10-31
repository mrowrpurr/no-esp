file(GLOB_RECURSE NoESP_Actions_Tests_Source *.cpp *.h)

add_executable(NoESP_Actions_Tests ${NoESP_Actions_Tests_Source} ../SpecHelper.h ../SpecHelper.cpp)

target_include_directories(
    NoESP_Actions_Tests PRIVATE
    ${BANDIT_INCLUDE_DIRS}
    ${SNOWHOUSE_INCLUDE_DIRS}
    ../../NoESP.Types
    ../../NoESP.Actions
)