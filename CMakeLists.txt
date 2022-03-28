cmake_minimum_required(VERSION 3.21)

project(scripts-without-esp VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project files
file(GLOB_RECURSE scripts-without-esp_Files src/*.cpp include/*.h)

# Define your SKSE plugin
add_library(scripts-without-esp SHARED ${scripts-without-esp_Files})

# Setup includes
target_include_directories(scripts-without-esp PRIVATE include)

#################################

# Add options for specifying the version
# Only one version may be provided at a time
option(SKYRIM_AE "Compile plugin for Skyrim AE" ON)
option(SKYRIM_SE "Compile plugin for Skyrim SE" OFF)
option(SKYRIM_VR "Compile plugin for Skyrim VR" OFF)

# Exit if no Skyrim version was specified
if(NOT ${SKYRIM_AE} AND NOT ${SKYRIM_SE} AND NOT ${SKYRIM_VR})
	message(FATAL_ERROR "Run cmake using one of the build/ .bat files for building Skyrim")
	return()
endif()

# Add support for #ifdef for SKYRIM_AE, SKYRIM_SE, and SKYRIM_VR
if(${SKYRIM_AE})
	message(INFO "Building CMake setup for Skyrim AE: ${SKYRIM_AE}")
	add_definitions(-DSKYRIM_AE)
	set(SKYRIM_VERSION AE)
elseif(${SKYRIM_SE})
	message(INFO "Building CMake setup for Skyrim SE: ${SKYRIM_SE}")
	add_definitions(-DSKYRIM_SE)
	set(SKYRIM_VERSION SE)
elseif(${SKYRIM_VR})
	message(INFO "Building CMake setup for Skyrim VR: ${SKYRIM_VR}")
	add_definitions(-DSKYRIM_VR)
	set(SKYRIM_VERSION VR)
endif()

message(INFO "Skyrim Version: ${SKYRIM_VERSION}")

###############

# Compile with C++20 features
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_23)

# Include spdlog
find_package(spdlog CONFIG REQUIRED)
target_link_libraries("${PROJECT_NAME}" PUBLIC spdlog::spdlog)

# CommonLib
if(${SKYRIM_AE})
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CommonLib/AE" CommonLibSSE EXCLUDE_FROM_ALL)
    target_link_libraries("${PROJECT_NAME}" PUBLIC CommonLibSSE::CommonLibSSE)
elseif(${SKYRIM_SE})
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CommonLib/SE" CommonLibSSE EXCLUDE_FROM_ALL)
    target_link_libraries("${PROJECT_NAME}" PUBLIC CommonLibSSE::CommonLibSSE)
elseif(${SKYRIM_VR})
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CommonLib/VR" CommonLibVR EXCLUDE_FROM_ALL)
    target_link_libraries("${PROJECT_NAME}" PUBLIC CommonLibVR::CommonLibVR)
endif()

# Get output directory based on configured version of Skyrim
if(${SKYRIM_AE})
	if(DEFINED ENV{SKSE_SKYRIM_AE_DEV})
		set(MOD_FOLDER $ENV{SKSE_SKYRIM_AE_DEV})
	elseif(DEFINED ENV{SKYRIM_MODS})
		set(MOD_FOLDER "$ENV{SKYRIM_MODS}/${PROJECT_NAME} - ${SKYRIM_VERSION}")
	else()
		set(MOD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/output/${SKYRIM_VERSION}")
	endif()
elseif(${SKYRIM_SE})
	if(DEFINED ENV{SKSE_SKYRIM_SE_DEV})
		set(MOD_FOLDER $ENV{SKSE_SKYRIM_SE_DEV})
	elseif(DEFINED ENV{SKYRIM_MODS})
		set(MOD_FOLDER "$ENV{SKYRIM_MODS}/${PROJECT_NAME} - ${SKYRIM_VERSION}")
	else()
		set(MOD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/output/${SKYRIM_VERSION}")
	endif()
elseif(${SKYRIM_VR})
	if(DEFINED ENV{SKSE_SKYRIM_VR_DEV})
		set(MOD_FOLDER $ENV{SKSE_SKYRIM_VR_DEV})
	elseif(DEFINED ENV{SKYRIM_MODS})
		set(MOD_FOLDER "$ENV{SKYRIM_MODS}/${PROJECT_NAME} - ${SKYRIM_VERSION}")
	else()
		set(MOD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/output/${SKYRIM_VERSION}")
	endif()
endif()

set(DLL_FOLDER "${MOD_FOLDER}/SKSE/Plugins")

# Create output directory if it does not exist
add_custom_command(
	TARGET "${PROJECT_NAME}"
	POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
	VERBATIM
)

# Print the paths that files will be output to when the project is built
message("Build will output .dll and .pdb to ${DLL_FOLDER}")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Scripts")
	message("Build will output Papyrus scripts to ${MOD_FOLDER}")
endif()

# Copy the output .dll and debug .pdb to the configured output directory
add_custom_command(
	TARGET "${PROJECT_NAME}"
	POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}"
	COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Scripts" "${MOD_FOLDER}/Scripts"
	COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${MOD_FOLDER}/Source"
	VERBATIM
)

# Copy Scripts/ and/or Source/ if they are present (at prepare time)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Scripts")
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Scripts" "${MOD_FOLDER}/Scripts"
		VERBATIM
	)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source")
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${MOD_FOLDER}/Source"
		VERBATIM
	)
endif()
