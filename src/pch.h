#pragma once
#include <RE/Skyrim.h>

#include <SKSE/SKSE.h>

#include <styyx-utils.h>

using namespace StyyxUtil;
using namespace std::literals;
namespace logs = SKSE::log;

#if defined(SKYRIM_SUPPORT_AE) || defined(ENABLE_SKYRIM_AE)
#define SKSE_TEMPLATE_SKYRIM_AE 1
#elif defined(SKYRIM_SUPPORT_VR) || defined(ENABLE_SKYRIM_VR)
#define SKSE_TEMPLATE_SKYRIM_VR 1
#else
#define SKSE_TEMPLATE_SKYRIM_SE 1
#endif

#ifdef SKSE_TEMPLATE_SKYRIM_AE
#define REL_ID(se, ae) REL::ID(ae)
#define REL_ID_3(se, ae, vr) REL::ID(ae)
#define OFFSET(se, ae) ae
#define OFFSET_3(se, ae, vr) ae
#elif defined(SKSE_TEMPLATE_SKYRIM_VR)
#define REL_ID(se, ae) REL::ID(se)
#define REL_ID_3(se, ae, vr) REL::ID(vr)
#define OFFSET(se, ae) se
#define OFFSET_3(se, ae, vr) vr
#else
#define REL_ID(se, ae) REL::ID(se)
#define REL_ID_3(se, ae, vr) REL::ID(se)
#define OFFSET(se, ae) se
#define OFFSET_3(se, ae, vr) se
#endif

#define DLLEXPORT __declspec(dllexport)
