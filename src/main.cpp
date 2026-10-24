#include "QuickLootAPI.h"
#include "RE/T/TESBoundObject.h"
#include "RE/T/TESObjectWEAP.h"
#include "REX/REX/TOML.h"
#include "SKSE/Impl/PCH.h"
#include "st-hooks.h"
using namespace StyyxUtil;

// Constants:
constexpr auto PLUGIN_NAME             = "dropchances.esl";
constexpr RE::FormID EXCEPTION_LIST_ID = 0x3;
constexpr RE::FormID EXCEPTION_KYWD_ID = 0x4;
constexpr RE::FormID DAE_ART_ID        = 0xA8668; // Skyrim
constexpr RE::FormID DISALLOW_ID       = 0xC27BD;

// checks every x seconds if a follower is near
// 480 seems reasonable imo as it also checks every interior cell leave and enter and load game
// TODO: maybe make it a setting but needs actual playtesting from people playing with followers first
constexpr double UPDATE_TIME = 480.0;

constexpr auto TOML_P_D     = "Data/SKSE/Plugins/drop-chances.toml";
constexpr auto TOML_P_C     = "Data/SKSE/Plugins/drop-chances_custom.toml";
constexpr auto TOML_SEC_SET = "DropSettings";

// glob var
bool IsRelevantMenuOpen    = false;
bool openingActorInventory = false;

// store processed actor + hidden items.
std::unordered_map<RE::TESObjectREFR*, std::unordered_set<RE::FormID>> rejectedItems;
RE::TESObjectREFR* openingRef{};
std::unordered_set<RE::FormID> g_teamMateStorage{};

namespace POOP
{
namespace FORMS
{
inline RE::BGSListForm* exception_formlist{nullptr};
inline RE::BGSListForm* exception_keyword_formlist{nullptr};
inline RE::BGSKeyword* daedric_artifact{nullptr};
inline RE::BGSKeyword* disallow_ench{nullptr};

bool AllFormsValid()
{
    return exception_formlist && exception_keyword_formlist;
}

void LoadForms()
{
    if (!MiscUtil::IsModLoaded(PLUGIN_NAME))
    {
        SKSE::stl::report_and_fail(
            std::format("{} is not loaded, please make sure it is enabled and/or deployed", PLUGIN_NAME));
    }
    auto dh = RE::TESDataHandler::GetSingleton();

    exception_formlist         = dh->LookupForm<RE::BGSListForm>(EXCEPTION_LIST_ID, PLUGIN_NAME);
    exception_keyword_formlist = dh->LookupForm<RE::BGSListForm>(EXCEPTION_KYWD_ID, PLUGIN_NAME);

    daedric_artifact = dh->LookupForm<RE::BGSKeyword>(DAE_ART_ID, "Skyrim.esm");
    disallow_ench    = dh->LookupForm<RE::BGSKeyword>(DISALLOW_ID, "Skyrim.esm");

    if (!AllFormsValid())
    {
        SKSE::stl::report_and_fail(std::format("Could not load all needed forms, make sure the mod is fully "
                                               "updated and you have no old version of "
                                               "{} left",
                                               PLUGIN_NAME));
    }
}

} // namespace FORMS

namespace CONF
{
inline REX::TOML::F32 drop_removal_chance_weapons{TOML_SEC_SET, "fDropRemoveChanceWeapons", 50.0f};
inline REX::TOML::F32 drop_removal_chance_armor{TOML_SEC_SET, "fDropRemoveChanceArmor", 50.0f};
inline REX::TOML::F32 drop_removal_chance_jewelry{TOML_SEC_SET, "fDropRemoveChanceJewelry", 50.0f};

inline REX::TOML::Bool replace_enchanted{TOML_SEC_SET, "bDropEnchanted", false};

void UpdateSettings(const bool a_save = false)
{
    auto t = REX::Singleton<REX::TOML::SettingStore>::GetSingleton();
    t->Init(TOML_P_D, TOML_P_C);
    a_save ? t->Save() : t->Load();
}

} // namespace CONF

void FillInFoll()
{
    auto player             = RE::PlayerCharacter::GetSingleton();
    auto potential_follower = ActorUtil::GetNearbyActors(player, 4096, false);

    for (auto& a : potential_follower)
    {

        if (!a || !a->IsPlayerTeammate())
        {
            continue;
        }
        if (g_teamMateStorage.contains(a->GetFormID()))
        {
            continue;
        }
        g_teamMateStorage.insert(a->GetFormID());
    }
}

struct LocChangeEv : REX::Singleton<LocChangeEv>, RE::BSTEventSink<RE::BGSActorCellEvent>
{

    static void RegisterCellEvent()
    {

        if (auto player = RE::PlayerCharacter::GetSingleton())
        {
            player->AddEventSink<RE::BGSActorCellEvent>(GetSingleton());
        }
    }
    RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorCellEvent* a_event,
                                          RE::BSTEventSource<RE::BGSActorCellEvent>*) override
    {

        if (!a_event)
        {
            return RE::BSEventNotifyControl::kContinue;
        }
        RE::TESObjectCELL* cell = RE::TESForm::LookupByID<RE::TESObjectCELL>(a_event->cellID);

        // exclude exteriors cause there.are.so.many.exterior.cells...
        if (cell && cell->IsExteriorCell())
        {
            return RE::BSEventNotifyControl::kContinue;
        }
        if (a_event->flags == RE::BGSActorCellEvent::CellFlag::kEnter)
        {
            std::erase_if(rejectedItems,
                          [cell](const auto& entry)
                          {
                              auto* reference = entry.first;
                              return !reference || reference->GetParentCell() != cell;
                          });
            FillInFoll();
        }

        if (a_event->flags == RE::BGSActorCellEvent::CellFlag::kLeave)
        {
            FillInFoll();
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

struct Updater
{

    static void InstallUpdater() { HookUtils::WriteVFunc<RE::PlayerCharacter, 0, 0xad, Updater>(); }

    static void Call(RE::PlayerCharacter* a_player, float a_delta)
    {

        static TimerUtil t;
        if (!t.IsRunning())
        {
            t.Start();
            FillInFoll();
        }
        if (t.ElapsedSeconds() >= UPDATE_TIME)
        {
            t.Reset();
            FillInFoll();
        }
        func(a_player, a_delta);
    }
    static inline REL::Relocation<decltype(Call)> func;
};

// Legacy credits:
// https://github.com/Horf/HiddenLoot/blob/97b085ead7b7fd1fd3c8810cf617d2b515ecaf0a/src/LootHook.h#L95
// I started out using the MenuHandling from the above code but changed pretty
// much everything about it now still, credits for the above author
struct MenuEventListener : REX::Singleton<MenuEventListener>, RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
    bool bContainerMenuOpen = false;

    static void RegisterMenu()
    {
        auto ui = RE::UI::GetSingleton();
        if (ui)
        {
            ui->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());
            SKSE::log::info("registered for inventory");
        }
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
    {
        if (!a_event)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (a_event->menuName == RE::ContainerMenu::MENU_NAME)
        {
            // bool and ref reset is my addition
            bContainerMenuOpen = a_event->opening;
            if (!a_event->opening)
            {
                openingActorInventory = false;
                openingRef            = nullptr;
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

struct OpenInvActorHook
{
    static void InstallOpenInvHook() { HookUtils::WriteCall5<OpenInvActorHook>(RELOCATION_ID(24211, 24715), 0x3e7); }

    static void Call(RE::TESObjectREFR* a_ref, RE::ContainerMenu::ContainerMode a_mode)
    {
        auto it    = rejectedItems.find(a_ref);
        openingRef = a_ref;
        if (it == rejectedItems.end())
        {
            openingActorInventory = true;
        }

        func(a_ref, a_mode);

        auto menu = MenuEventListener::GetSingleton();
        if (menu->bContainerMenuOpen)
        {
            openingActorInventory = false;
            openingRef            = nullptr;
        }
    }
    static inline REL::Relocation<decltype(Call)> func;
};

// TODO: check if requiem changes the keyword to something else or just
// changes the keyword's EDID
bool IsItemArtifact(RE::TESBoundObject* a_item)
{

    if (auto kwdf = a_item->As<RE::BGSKeywordForm>(); kwdf && kwdf->HasKeyword(FORMS::daedric_artifact))
    {
        return true;
    }
    return false;
}

bool IsMaybeUniqueItem(RE::TESBoundObject* a_item)
{
    if (auto kwdf = a_item->As<RE::BGSKeywordForm>(); kwdf && kwdf->HasKeyword(FORMS::disallow_ench))
    {
        return true;
    }
    return false;
}

bool IsItemEnchanted(RE::TESBoundObject* a_item, RE::TESObjectREFR* a_ref)
{

    // check pre-enchanted forms. They wouldn't be caught with the extra data
    // bellow i think.
    auto ench = a_item->As<RE::TESEnchantableForm>();
    if (ench)
    {
        if (ench->formEnchanting)
        {
            return true;
        }
    }

    auto inventory = a_ref->GetInventory();

    if (auto it = inventory.find(a_item); it != inventory.end())
    {
        auto entry = it->second.second.get();

        if (entry && entry->GetEnchantment() != nullptr)
        {
            return true;
        }
    }
    return false;
}
// needs more testing. May not work for everything.
// Ebony mail is a contender for not working but that one should be covered by
// the artifact check
// Some quest items aren't actually quest items but the container is the alias
bool IsQuestItem(RE::TESBoundObject* a_item, RE::TESObjectREFR* a_ref)
{

    auto inventory = a_ref->GetInventory();

    if (auto it = inventory.find(a_item); it != inventory.end())
    {

        auto entry = it->second.second.get();

        if (entry && entry->IsQuestObject())
        {
            return true;
        }
    }

    return false;
}

bool IsItemExcluded(RE::TESBoundObject* a_item)
{

    if (FORMS::exception_formlist->HasForm(a_item))
    {
        return true;
    }
    if (a_item->HasKeywordInList(FORMS::exception_keyword_formlist, false))
    {
        return true;
    }
    return false;
}

bool CanHideItem(RE::TESBoundObject* a_item, RE::TESObjectREFR* a_ref)
{

    if (g_teamMateStorage.contains(a_ref->GetFormID()))
    {
        return false;
    }

    if (IsItemExcluded(a_item))
    {
        return false;
    }

    if (IsQuestItem(a_item, a_ref))
    {
        return false;
    }

    auto drop_enchants = CONF::replace_enchanted.GetValue();

    if (!drop_enchants)
    {
        if (IsItemEnchanted(a_item, a_ref))
        {
            return false;
        }
    }

    if (IsItemArtifact(a_item) || IsMaybeUniqueItem(a_item))
    {
        return false;
    }

    return true;
}

float GetActualChance(RE::TESBoundObject* a_item)
{

    if (a_item->Is(RE::FormType::Weapon))
    {
        return CONF::drop_removal_chance_weapons.GetValue();
    }
    if (a_item->Is(RE::FormType::Armor))
    {
        RE::TESObjectARMO* armo = a_item->As<RE::TESObjectARMO>();
        if (armo)
        {
            using slot = RE::BGSBipedObjectForm::BipedObjectSlot;
            switch (armo->GetSlotMask())
            {
                case slot::kAmulet:
                case slot::kCirclet:
                case slot::kRing:
                    return CONF::drop_removal_chance_jewelry.GetValue();
                default:
                    return CONF::drop_removal_chance_armor.GetValue();
            }
        }
        return CONF::drop_removal_chance_armor.GetValue();
    }
    return 0.0f;
}

bool ProcessItem(RE::TESBoundObject* a_item, bool a_original)
{
    if (!a_original)
    {
        return a_original;
    }

    if (openingRef)
    {
        if (auto it = rejectedItems.find(openingRef);
            it != rejectedItems.end() && it->second.contains(a_item->GetFormID()))
        {
            return false;
        }
    }

    if (openingActorInventory && openingRef && CanHideItem(a_item, openingRef))
    {
        auto cmp    = GetActualChance(a_item);
        auto remove = RandomiserUtil::IsPercentageChanceFloat(cmp);

        if (remove)
        {
            // REX::INFO("chance for {} is: {}", a_item->GetName(), cmp);
            rejectedItems[openingRef].insert(a_item->GetFormID());
            return false;
        }
    }
    return true;
}

struct QLoot
{

    static void HandleQuickLoot(QuickLoot::API::Events::ModifyInventoryEvent* a_event)
    {
        auto& inv = a_event->inventory;

        for (auto& items : inv)
        {
            auto obj = items.entry->object;
            if (!obj)
            {
                continue;
            }
            if (!ProcessItem(obj, obj->GetPlayable()))
            {
                inv.erase(&items);
            }
        }
    }

    static void OnOpening(QuickLoot::API::Events::OpeningLootMenuEvent* a_event)
    {
        openingRef            = a_event->container.get().get();
        openingActorInventory = true;
    }

    static void OnClosing(QuickLoot::API::Events::CloseLootMenuEvent* a_event)
    {
        openingRef            = nullptr;
        openingActorInventory = false;
    }

    static void RegisterQuickLoot()
    {
        if (QuickLoot::API::QuickLootAPI::Init("styyx-reduce-loot"))
        {
            SKSE::log::info("Registered for quickloot");
        }
    }

    static void RegisterQuickLootHandler()
    {
        using namespace QuickLoot::API;
        QuickLootAPI::RegisterOpeningLootMenuHandler(OnOpening);
        QuickLootAPI::RegisterModifyInventoryHandler(HandleQuickLoot);
        QuickLootAPI::RegisterCloseLootMenuHandler(OnClosing);
    }
};

namespace PlayableHooks
{


struct Armo
{
    static bool Call(RE::TESBoundObject* a_armo) { return ProcessItem(a_armo, func(a_armo)); }
    static inline REL::Relocation<decltype(Call)> func;
};
struct Weap
{
    static bool Call(RE::TESBoundObject* a_weapon) { return ProcessItem(a_weapon, func(a_weapon)); }
    static inline REL::Relocation<decltype(Call)> func;
};

inline void InstallPlayable()
{
    HookUtils::WriteVFunc<RE::TESObjectARMO, 0, 0x19, Armo>();
    HookUtils::WriteVFunc<RE::TESObjectWEAP, 0, 0x19, Weap>();
}

} // namespace PlayableHooks

} // namespace POOP

void InstallHooksAll()
{

    POOP::Updater::InstallUpdater();
    POOP::OpenInvActorHook::InstallOpenInvHook();
    POOP::PlayableHooks::InstallPlayable();
}

void List(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type)
    {
        case SKSE::MessagingInterface::kPostLoad:
            POOP::QLoot::RegisterQuickLoot();
            break;

        case SKSE::MessagingInterface::kDataLoaded:
            POOP::MenuEventListener::RegisterMenu();
            POOP::LocChangeEv::RegisterCellEvent();
            POOP::QLoot::RegisterQuickLootHandler();
            POOP::FORMS::LoadForms();
            break;

        case SKSE::MessagingInterface::kPostLoadGame:
            POOP::FillInFoll();
            break;
        default:
            break;
    }
}


SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    SKSE::AllocTrampoline(14);
    POOP::CONF::UpdateSettings();
    InstallHooksAll();
    if (!SKSE::GetMessagingInterface()->RegisterListener(List))
    {
        return false;
    }

    return true;
}