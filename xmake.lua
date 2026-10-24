if is_plat("windows") then
    add_cxxflags("cl::/std:c++23preview", {force = true})
else
    set_languages("c++23")
end
includes("lib/commonlibsse", "extern/styyx-utils")
local MOD_NAME = "styyx-reduce-loot"
local MOD_VERSION = "2.1.0"
local MOD_DESC = "reduces loot gained"
set_project(MOD_NAME)
set_version(MOD_VERSION)
set_license("proprietary")
set_warnings("allextra")

-- xmake rules
add_rules("mode.debug", "mode.releasedbg")
set_defaultmode("releasedbg")
--add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"}) --useful for clion or vscode
add_rules("plugin.vsxmake.autoupdate")
includes("xmake-rules.lua")

-- commonlib options
if get_config("skyrim_ae") == nil then
    set_config("skyrim_ae", true)
end
set_config("rex_toml", true)
set_config("use-hook-utils", true)
set_config("use-fui", true)



-- add plugin target
target(MOD_NAME)
    add_deps("commonlibsse", {public = true})
    add_deps("styyx-util", {public = true})

    if has_config("skyrim_ae") then
        set_targetdir("build/AE/skse/plugins")
    else
        set_targetdir("build/SE/skse/plugins")
    end

    add_rules("skse-template.plugin", {
        name = MOD_NAME,
        author = "styyx",
        description = MOD_DESC
    })

    if has_config("skyrim_ae") then
        add_defines("SKSE_TEMPLATE_SKYRIM_AE=1", { public = true })
    else
        add_defines("SKSE_TEMPLATE_SKYRIM_SE=1", { public = true })
    end

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

-- builds both game versions. use with ``xmake shiprelease``
-- ship task: builds SE then AE
task("shiprelease")
    set_menu {
        usage       = "xmake shiprelease",
        description = "Build release for SE and AE",
    }
    on_run(function()
        print("Building SE...")
        os.exec("xmake f -m releasedbg --skyrim_ae=false")
        os.exec("xmake")
        print("Building AE...")
        os.exec("xmake f -m releasedbg --skyrim_ae=true")
        os.exec("xmake")
    end)


