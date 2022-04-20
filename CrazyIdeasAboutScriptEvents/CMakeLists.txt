cmake_minimum_required(VERSION 3.21)

project(CrazyIdeasAboutEventsAndShit VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Required for compiling CommonLibSSE
add_compile_options(/bigobj)

# Define your SKSE plugin
add_library(${PROJECT_NAME} SHARED plugin.cpp)

# Link to CommonLibSSE
find_package(CommonLibSSE CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)

if(DEFINED ENV{SKYRIM_MODS})
	# Setup a folder which the output .dll automatically gets copied to!
	set(DLL_FOLDER "$ENV{SKYRIM_MODS}/${PROJECT_NAME}/SKSE/Plugins")

	# SKSE plugin .dll is copied to the specified folder
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/${PROJECT_NAME}.dll"
		VERBATIM
	)
else()
	message(WARNING "Please set SKYRIM_MODS environment variable to the path to your Skyrim mods folder")
endif()
