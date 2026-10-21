#include "config.h"

void MOD::CONFIG::UpdateSetting(bool a_save)
{
    const auto toml = REX::TOML::SettingStore::GetSingleton();
    toml->Init("Data/SKSE/Plugins/MOD.toml", "Data/SKSE/Plugins/MOD_custom.toml");

    if (!a_save)
    {
        toml->Load();
    }
    else
    {
        toml->Save();
    }
}