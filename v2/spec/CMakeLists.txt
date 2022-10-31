find_path(BANDIT_INCLUDE_DIRS "bandit/adapters.h")
find_path(SNOWHOUSE_INCLUDE_DIRS "snowhouse/assert.h")
# [cmake]     find_package(spdlog CONFIG REQUIRED)
# [cmake]     target_link_libraries(main PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

add_subdirectory(actions)
add_subdirectory(parser)
