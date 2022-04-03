
# Exit if no Skyrim version was specified
if(NOT ${SKYRIM_AE} AND NOT ${SKYRIM_SE} AND NOT ${SKYRIM_VR})
	message(FATAL_ERROR "Please specify a version of Skyrim by choosing a CMakeLists.txt preset")
	return()
endif()

# Add support for #ifdef for SKYRIM_AE, SKYRIM_SE, and SKYRIM_VR
if(${SKYRIM_AE})
	message(INFO "Building CMake setup for Skyrim AE: ${SKYRIM_AE}")
	add_compile_definitions(SKYRIM_AE)
	add_compile_definitions(SKYRIM_SUPPORT_AE)
	set(SKYRIM_VERSION AE)
elseif(${SKYRIM_SE})
	message(INFO "Building CMake setup for Skyrim SE: ${SKYRIM_SE}")
	add_compile_definitions(SKYRIM_SE)
	set(SKYRIM_VERSION SE)
elseif(${SKYRIM_VR})
	message(INFO "Building CMake setup for Skyrim VR: ${SKYRIM_VR}")
	add_compile_definitions(SKYRIM_VR)
	set(SKYRIM_VERSION VR)
endif()

message(INFO "Skyrim Version: ${SKYRIM_VERSION}")

# CommonLib
if(SKYRIM_AE)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CommonLib/AE" CommonLibSSE EXCLUDE_FROM_ALL)
	target_link_libraries("${PROJECT_NAME}" PUBLIC CommonLibSSE::CommonLibSSE)
elseif(SKYRIM_SE)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CommonLib/SE" CommonLibSSE EXCLUDE_FROM_ALL)
	target_link_libraries("${PROJECT_NAME}" PUBLIC CommonLibSSE::CommonLibSSE)
elseif(SKYRIM_VR)
	add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/CommonLib/VR" CommonLibVR EXCLUDE_FROM_ALL)
	target_link_libraries("${PROJECT_NAME}" PUBLIC CommonLibVR::CommonLibVR)
endif()

# Get output directory based on configured version of Skyrim
if(SKYRIM_AE)
	if(DEFINED ENV{SKSE_SKYRIM_AE_DEV})
		set(MOD_FOLDER $ENV{SKSE_SKYRIM_AE_DEV})
	elseif(DEFINED ENV{SKYRIM_MODS})
		set(MOD_FOLDER "$ENV{SKYRIM_MODS}/${PROJECT_NAME} - ${SKYRIM_VERSION}")
	else()
		set(MOD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/output/${SKYRIM_VERSION}")
	endif()
elseif(SKYRIM_SE)
	if(DEFINED ENV{SKSE_SKYRIM_SE_DEV})
		set(MOD_FOLDER $ENV{SKSE_SKYRIM_SE_DEV})
	elseif(DEFINED ENV{SKYRIM_MODS})
		set(MOD_FOLDER "$ENV{SKYRIM_MODS}/${PROJECT_NAME} - ${SKYRIM_VERSION}")
	else()
		set(MOD_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/output/${SKYRIM_VERSION}")
	endif()
elseif(SKYRIM_VR)
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
