
#pragma once

#include "REX/REX/TOML.h"

namespace MOD::CONFIG
{
inline REX::TOML::Bool bExampleBool{"General", "bExampleBool", false};
inline REX::TOML::F32 fExampleFloat{"General", "fExampleFloat", 1.0f};

void UpdateSetting(bool a_save = false);
} // namespace MOD::CONFIG
