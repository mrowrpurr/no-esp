add_requires("skyrim-commonlib")

target("NoESP.Plugin")
    set_basename("no-esp")
    add_deps("NoESP")
    add_packages("simpleini", "spdlog", "_Log_", "string_format")
    add_files("*.cpp")
    add_deps("SkyrimScripting.Plugin")
    add_packages("skyrim-commonlib")
    add_rules("@skyrim-commonlib/plugin", {
        mod_folders = os.getenv("SKYRIM_SCRIPTING_MOD_FOLDERS")
    })