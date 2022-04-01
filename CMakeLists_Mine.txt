cmake_minimum_required(VERSION 3.21)

project(no-esp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Get all source files from src/ and include/
file(GLOB_RECURSE no-esp_SOURCES src/*.cpp src/*.h include/*.h)

# Add plugin as a .dll to compile
add_library(no-esp SHARED ${no-esp_SOURCES})

# Include the include/ folder
target_include_directories(no-esp PRIVATE include)

# Compile with C++23 features
target_compile_features(no-esp PRIVATE cxx_std_23)

include(Skyrim)

# Include spdlog for logging
find_package(spdlog CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC spdlog::spdlog)

# Include [TODO] for .ini files
