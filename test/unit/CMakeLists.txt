file(GLOB_RECURSE UNIT_TESTS_SOURCE *.cpp *.hh)

add_executable(ThePrototype.UnitTests ../specHelper.cpp ../specHelper.h ${UNIT_TESTS_SOURCE})

# ThePrototype headers
target_include_directories(ThePrototype.UnitTests PRIVATE ../../include)

# Bandit (testing framework)
find_path(BANDIT_INCLUDE_DIRS "bandit/adapters.h")
target_include_directories(ThePrototype.UnitTests PRIVATE ${BANDIT_INCLUDE_DIRS})

# Snowhouse (test assertions)
find_path(SNOWHOUSE_INCLUDE_DIRS "snowhouse/assert.h")
target_include_directories(ThePrototype.UnitTests PRIVATE ${SNOWHOUSE_INCLUDE_DIRS})

# spdlog for logging
target_link_libraries(ThePrototype.UnitTests PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

if(SKYRIM_AE)
    target_link_libraries(ThePrototype.UnitTests PUBLIC CommonLibSSE::CommonLibSSE)
elseif(SKYRIM_SE)
    target_link_libraries(ThePrototype.UnitTests PUBLIC CommonLibSSE::CommonLibSSE)
elseif(SKYRIM_VR)
    target_link_libraries(ThePrototype.UnitTests PUBLIC CommonLibVR::CommonLibVR)
endif()
