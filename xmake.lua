-- set required xmake version
set_xmakever("2.8.2")

-- includes (need xmake.lua file in the same directory)
includes("lib/commonlibsse")
includes("extern/styyx-utils")

-- set up for project

local mod_name = "MOD"

set_project(mod_name)
set_version("1.0.0")
set_license("MIT")

-- language and warnings
set_languages("c++23")
set_warnings("allextra")

-- xmake rules
add_rules("mode.debug", "mode.releasedbg")
set_defaultmode("releasedbg")
--add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"}) --useful for clion or vscode
add_rules("plugin.vsxmake.autoupdate")
includes("xmake-rules.lua")

-- commonlib options
set_config("skyrim_ae",true)
set_config("rex_toml", true)
set_config("use-hook-utils", true)
--{{ADDITIONAL CONFIGS}}--


-- add plugin target
target(mod_name)
    add_deps("commonlibsse", {public = true})
    add_deps("styyx-util", {public = true})

    if has_config("skyrim_ae") then
        set_targetdir("build/AE/skse/plugins")
    else
        set_targetdir("build/SE/skse/plugins")
    end

    add_rules("skse-template.plugin", {
        name = mod_name,
        author = "styyx",
        description = "A plugin template for commonlibsse."
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

    after_build(function(target)
        local dist_root = path.join(os.projectdir(), "Distr")
        local runtime   = has_config("skyrim_ae") and "AE" or "SE"
        local plugins   = path.join(dist_root, runtime, "SKSE", "Plugins")
        os.mkdir(plugins)
        os.trycp(target:targetfile(), plugins)
        os.trycp(target:symbolfile(), plugins)
        os.trycp("$(projectdir)/release/**.toml",  plugins)
        os.trycp("$(projectdir)/release/**.json",  plugins)
    end)


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


