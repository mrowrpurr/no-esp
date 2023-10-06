add_requires("_Log_", "spdlog")

function add_requires_for_specs()
    -- Specs.cpp is not yet released as a package, so you have to require it locally
    local specs_path = os.getenv("SPECS_CPP")
    if specs_path then
        includes(path.join(specs_path, "Specs/xmake.lua"))
    else
        print("SPECS_CPP not set")
        return
    end

    add_requires("string_format", "_Log_", "vcpkg::snowhouse")
end

add_requires_for_specs()

target("NoESP - Specs")
    add_deps("NoESP", "Specs")
    add_packages("vcpkg::snowhouse", "_Log_", "spdlog", "skyrim-commonlib")
    set_kind("binary")
    add_includedirs(".")
    add_files("*.cpp", "**/*.cpp")
