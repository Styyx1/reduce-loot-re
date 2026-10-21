#include "hooks.h"

#include "st-hooks.h"

namespace MOD
{


void ExampleHook::Call()
{
    func();
}

void InstallHooks()
{ // WriteCall5 example:
    // HookUtils::WriteCall5<ExampleHook>(REL_ID(0, 0), OFFSET(0, 0));

    // WriteVFunc example:
    // HookUtils::WriteVFunc<RE::SomeClass, 0, 0x00, ExampleHook>();

    logs::info("Hooks installed.");
}
} // namespace MOD
