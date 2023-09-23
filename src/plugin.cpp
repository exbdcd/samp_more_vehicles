#include "pch.h"

#include "plugin.h"

std::shared_ptr<urmem::patch> patch{};
urmem::hook cgame_createvehicle_hk{};
void* __fastcall HOOK_CGame__CreateVehicle(void* pGame, void* edx, int nModel, float x, float y, float z, float rotation, bool hasSiren);

void Plugin::Init(HMODULE hModule) {
    const auto init = [&]() {
        modulePath = GetModuleFileName(GetModuleHandleA(kModuleName));
        modulePath.resize(modulePath.find_last_of('\\'));

        Log("Plugin loaded!");
        Log("Version: {}", kVersion);
        Log("Build time: {} {}", __DATE__, __TIME__);
        Log("Module path: {}", modulePath);

        settings.Load();

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));

            if (gta_sa = module_t("gta_sa.exe"), !gta_sa.get_address()) {
                continue;
            }

            if (settings.samp_fix_enabled()) {
                if (samp = module_t("samp.dll"), !samp.get_address()) {
                    continue;
                }
            }

            Log("gta_sa.exe address: {:#x}", gta_sa.get_address());

            if (settings.samp_fix_enabled()) {
                Log("samp.dll address: {:#x}", samp.get_address());
            }

            DoInit();
            break;
        }
    };

    std::thread(init).detach();
}

void Plugin::Shutdown() {
    settings.UnLoad();

    if (logFileStream.is_open()) {
        logFileStream.close();
    }
}

void Plugin::WriteToLog(const char* text) {
    if (!logFileStream.is_open()) {
        logFileStream.open(path_to(kLogFileName), std::ios::out | std::ios::trunc | std::ios::ate);

        if (logFileStream.is_open()) {
            return;
        }
    }

    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    logFileStream << std::format("[{:%H:%M:%S}] {}\n", time, text);
    logFileStream.flush();
}

void Plugin::DoInit() {
    Log("** Do init stuff");

    if (!settings.samp_fix_enabled() && !settings.mod_sa_fix_enabled()) {
        Log("Nothing to init.");
        return;
    }

    if (settings.samp_fix_enabled()) {
        for (const auto& samp_version : settings.get_samp_versions()) {
            if (samp_version.detect_signature.empty()) {
                continue;
            }

            if (memcmp_safe((uint8_t*)samp.get_address() + 0xBABE, samp_version.detect_signature.data(),
                samp_version.detect_signature.size())) {

                Log("Detected SA:MP version: {}", samp_version.name);
                this->samp_version = samp_version;
                break;
            }
        }

        if (samp_version.name.empty()) {
            Log("Couldn't detect any SA:MP version!");
        }
        else {
            const auto patch_size = 6;
            const auto patch_addr = samp.get_address() + samp_version.patch_offset;
            Log("Installing patch (offset: {:#x}, size: {})...", samp_version.patch_offset, patch_size);
            urmem::unprotect_memory(samp.get_address() + samp_version.patch_offset, patch_size);
            memset((void*)(patch_addr), 0x90, patch_size);

            Log("Hooking CGame::CreateVehicle (offset: {:#x})...", samp_version.CGame_CreateVehicle);
            cgame_createvehicle_hk.install(samp.get_address() + samp_version.CGame_CreateVehicle,
                urmem::get_func_addr(&HOOK_CGame__CreateVehicle), urmem::hook::type::jmp, 6);
        }
    }

    DoModSaFix();
    Log("Init done.");
}

void Plugin::DoModSaFix() {
    if (!settings.mod_sa_fix_enabled()) {
        return;
    }

    Log("** Do mod_sa fix");
    mod_sa_fix.Init();
}

bool Plugin::IsVehicleModel(int id) {
    const auto kMinId = 400;
    const auto kMaxId = 19999;
    const auto kModelTypeVehicle = 6;

    if (id < kMinId || id > kMaxId) {
        return false;
    }

    static void** modelInfoPtrs = nullptr;
    if (!modelInfoPtrs)
        modelInfoPtrs = (void**)0xA9B0C8;

    if (modelInfoPtrs) {
        void* modelPointer = modelInfoPtrs[id];
        if (modelPointer)
        {
            void** vtbl = *(void***)(modelPointer);
            uint8_t modelType = ((uint8_t(__thiscall*)(void*)) (vtbl[4]))(modelPointer);
            if (modelType == kModelTypeVehicle)
                return true;
        }
    }

    return false;
}

void Plugin::AddChatMessage(const std::string& text) {
    const auto samp_address = get_samp_module().get_address();
    const auto pChat = *(void**)(samp_address + samp_version.pChat);
    const auto func = samp_address + samp_version.CChat_AddMessage;

    ((void(__thiscall*)(void*, unsigned int, const char*))func)(pChat, 0xA9C4E4FF, text.c_str());
}

void* call_create_vehicle(address_t calledFrom, void* pGame, int nModel, float x, float y, float z, float rotation, bool hasSiren) {

    const auto kReplace = 411;
    const auto warningInChat = Plugin.GetSettings().do_print_warning_in_chat();
    const auto samp_address = Plugin.get_samp_module().get_address();
    const auto called_from_cvehiclepool_create = Plugin.get_samp_version().CGame_CreateVehicle_called_from_CVehiclePool_Create;

    if (!Plugin.IsVehicleModel(nModel)) {
        if (calledFrom == called_from_cvehiclepool_create) {
            Plugin.Log("Hook CGame::CreateVehicle: Unknown vehicle model {:d} was not create (because called from CVehiclePool::Create)", nModel);
            if (warningInChat) {
                Plugin.AddChatMessage(std::format("Warning: Unknown vehicle model {:d} was not created!", nModel));
            }
            return nullptr;
        }

        nModel = kReplace;
    }

    auto* result = urmem::call_function<urmem::calling_convention::thiscall, void*, void*, int, float, float, float, float, bool>(
        cgame_createvehicle_hk.get_original_addr() + 6, pGame, nModel, x, y, z, rotation, hasSiren);

    mod_sa_fix.OnAddNewVehicle(result);

    return result;
}

void* __fastcall HOOK_CGame__CreateVehicle(void* pGame, void* edx, int nModel, float x, float y, float z, float rotation, bool hasSiren) {

    const auto samp_address = Plugin.get_samp_module().get_address();
    const auto called_from_cvehiclepool_create = Plugin.get_samp_version().CGame_CreateVehicle_called_from_CVehiclePool_Create;

    address_t called_from{};
    _asm {
        push eax
        mov     eax, [ebp + 4h]
        mov called_from, eax
        pop eax
    }
    called_from -= samp_address;

#if 0
    Plugin.Log("{:#x}", module_t(Plugin.kModuleName).get_address());
    Plugin.Log("HOOK_CGame__CreateVehicle:  nModel: {}, x: {:.2f}, y: {:.2f}, z: {:.2f}, rotation: {:.2f}, hasSiren: {} | called_from: {:#x}",
        nModel, x, y, z, rotation, hasSiren, called_from);
#endif
 
    return call_create_vehicle(called_from, pGame, nModel, x, y, z, rotation, hasSiren);
}