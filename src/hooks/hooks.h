#pragma once

namespace MOD
{
void InstallHooks();

// Example hook stub — replace with your target and args.
// For WriteCall5:  static void Call(...);
// For WriteVFunc:  static void Call(RE::SomeClass* a_this, ...);
struct ExampleHook
{
    static void Call();
    static inline REL::Relocation<decltype(Call)> func;
};
} // namespace MOD
