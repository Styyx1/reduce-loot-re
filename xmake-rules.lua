local PLUGIN_CPP = [[
#include <SKSE/SKSE.h>

#if defined(SKSE_TEMPLATE_SKYRIM_VR)
SKSEPluginInfo(
    .Version = { ${PLUGIN_VERSION_MAJOR}, ${PLUGIN_VERSION_MINOR}, ${PLUGIN_VERSION_PATCH}, 0 },
    .Name = "${PLUGIN_NAME}",
    .Author = "${PLUGIN_AUTHOR}",
    .SupportEmail = "${PLUGIN_EMAIL}",
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
)
#elif defined(SKYRIM_SUPPORT_AE)
extern "C" __declspec(dllexport)
constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion({ ${PLUGIN_VERSION_MAJOR}, ${PLUGIN_VERSION_MINOR}, ${PLUGIN_VERSION_PATCH}, 0 });
    v.PluginName("${PLUGIN_NAME}");
    v.AuthorName("${PLUGIN_AUTHOR}");
    v.AuthorEmail("${PLUGIN_EMAIL}");
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });
    return v;
}();
#else
extern "C" __declspec(dllexport)
bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "${PLUGIN_NAME}";
    a_info->version = REL::Version{ ${PLUGIN_VERSION_MAJOR}, ${PLUGIN_VERSION_MINOR}, ${PLUGIN_VERSION_PATCH}, 0 }.pack();

    if (a_skse->IsEditor()) {
        SKSE::log::critical("Loaded in editor, marking as incompatible");
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (ver < SKSE::RUNTIME_SSE_1_5_39) {
        SKSE::log::critical("Unsupported runtime version {}", ver.string());
        return false;
    }

    return true;
}
#endif
]]

local PLUGIN_RC = [[
#include <winres.h>

1 VERSIONINFO
FILEVERSION ${PLUGIN_VERSION_MAJOR}, ${PLUGIN_VERSION_MINOR}, ${PLUGIN_VERSION_PATCH}, 0
PRODUCTVERSION ${PROJECT_VERSION_MAJOR}, ${PROJECT_VERSION_MINOR}, ${PROJECT_VERSION_PATCH}, 0
FILEFLAGSMASK 0x17L
#ifdef _DEBUG
    FILEFLAGS 0x1L
#else
    FILEFLAGS 0x0L
#endif
FILEOS 0x4L
FILETYPE 0x1L
FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "FileDescription", "${PLUGIN_DESCRIPTION}"
            VALUE "FileVersion", "${PLUGIN_VERSION}.0"
            VALUE "InternalName", "${PLUGIN_NAME}"
            VALUE "LegalCopyright", "${PLUGIN_AUTHOR} | ${PLUGIN_LICENSE}"
            VALUE "ProductName", "${PROJECT_NAME}"
            VALUE "ProductVersion", "${PROJECT_VERSION}.0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
]]

rule("skse-template.plugin")
    add_deps("win.sdk.resource")

    on_config(function(target)
        import("core.base.semver")
        import("core.project.project")

        target:set("arch", "x64")
        target:set("kind", "shared")

        local conf = target:extraconf("rules", "skse-template.plugin")
        local conf_dir = path.join(target:autogendir(), "rules", "skse-template", "plugin")
        os.mkdir(conf_dir)

        local conf_map = {
            PLUGIN_AUTHOR = conf.author or "",
            PLUGIN_DESCRIPTION = conf.description or "",
            PLUGIN_EMAIL = conf.email or "",
            PLUGIN_LICENSE = (target:license() or "Unknown") .. " License",
            PLUGIN_NAME = conf.name or target:name(),
            PLUGIN_VERSION = target:version() or "0.0.0",
            PLUGIN_VERSION_MAJOR = semver.new(target:version() or "0.0.0"):major(),
            PLUGIN_VERSION_MINOR = semver.new(target:version() or "0.0.0"):minor(),
            PLUGIN_VERSION_PATCH = semver.new(target:version() or "0.0.0"):patch(),
            PROJECT_NAME = project.name() or "",
            PROJECT_VERSION = project.version() or "0.0.0",
            PROJECT_VERSION_MAJOR = semver.new(project.version() or "0.0.0"):major(),
            PROJECT_VERSION_MINOR = semver.new(project.version() or "0.0.0"):minor(),
            PROJECT_VERSION_PATCH = semver.new(project.version() or "0.0.0"):patch(),
        }

        local function render(data)
            return data:gsub("(%${([^\n]-)})", function(_, key)
                local value = conf_map[key:trim()]
                assert(value ~= nil, "cannot get variable(%s)", key)
                return tostring(value)
            end)
        end

        local function write_file(name, data)
            local file_path = path.join(conf_dir, name)
            local file = io.open(file_path, "w")
            assert(file, "cannot open " .. file_path)
            file:write(render(data), "\n")
            file:close()
            target:add("files", file_path)
        end

        write_file("plugin.cpp", PLUGIN_CPP)
        write_file("version.rc", PLUGIN_RC)
    end)
