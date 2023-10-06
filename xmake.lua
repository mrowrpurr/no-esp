set_languages("c++20")

add_rules("mode.debug", "mode.releasedbg")

local skyrim_scripting_root = os.getenv("SKYRIM_SCRIPTING_PROJECT_ROOT")
includes(path.join(skyrim_scripting_root, "xmake.lua"))

-- TODO: bandit and snowhouse for testing
add_requires("_Log_", "string_format", "spdlog", "simpleini")

includes("*/xmake.lua")
