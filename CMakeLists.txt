cmake_minimum_required(VERSION 3.21)

project(no-esp VERSION 1.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE NOMINMAX _CRT_SECURE_NO_WARNINGS) # Stop telling me to use strcpy_s please

# Get all source files from src/ and include/
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h include/*.h)

# Add plugin as a .dll to compile
find_package(CommonLibSSE CONFIG REQUIRED)
add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PUBLIC src/PCH.h)

# Include inih for .ini files
find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
target_include_directories(${PROJECT_NAME} PUBLIC ${SIMPLEINI_INCLUDE_DIRS})

add_subdirectory(test)

set(MOD_FOLDER Output)
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
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/no-esp.ini" "${DLL_FOLDER}"
		COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Scripts" "${MOD_FOLDER}/Scripts"
		VERBATIM
)

# Copy Scripts/ to Scripts/ and Scripts/Source to Source/Scripts
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Scripts")
	add_custom_command(
			TARGET "${PROJECT_NAME}"
			POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Scripts" "${MOD_FOLDER}/Scripts"
			COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/Source" "${MOD_FOLDER}/Source/Scripts"
			VERBATIM
	)
endif()

# Also copy over the .pdb when doing a Debug 
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_custom_command(
			TARGET "${PROJECT_NAME}"
			POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}"
			VERBATIM
	)
endif()
