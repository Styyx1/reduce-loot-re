#include "configs/config.h"
#include "hooks/hooks.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    Init(skse);

    MOD::CONFIG::UpdateSetting();
    MOD::InstallHooks();
    return true;
}