#include "pch.h"

#include "settings.h"
#include "samp_more_vehicles.ini.h"

void Settings::LoadDefault() {
    iniFile.LoadData(samp_more_vehicles_ini, samp_more_vehicles_ini_size);
}

void Settings::Load() {
    Plugin.Log("Loading settings...");

    const auto logFilePath = Plugin.path_to(kFileName).c_str();

    auto error = iniFile.LoadFile(logFilePath);
    if (error != SI_OK) {
        Plugin.Log("Warning: Settings file was not loaded (SI_Error: {})", error);
        Plugin.Log("Using default settings...");

        LoadDefault();
    }

    enable_samp_fix = iniFile.GetBoolValue("general", "enable_samp_fix", true);
    enable_mod_sa_fix = iniFile.GetBoolValue("general", "enable_mod_sa_fix", true);
    print_warning_in_chat = iniFile.GetBoolValue("general", "print_warning_in_chat", true);

    if (!enable_samp_fix && !enable_mod_sa_fix) {
        Plugin.Log("Nothing to load.");
    }
    else {
        std::list<CSimpleIniA::Entry> sections{};
        iniFile.GetAllSections(sections);

        for (const auto& section : sections) {
            const auto section_name = section.pItem;
            const auto section_name_sv = std::string_view(section_name);

            if (enable_samp_fix) {
                if (section_name_sv.starts_with("samp ")) {
                    samp_version_t samp_version{};

                    samp_version.name = section_name_sv.substr(5);

                    if (!parse_signature(section_name, "detect_signature", samp_version.detect_signature)) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Signature 'detect_signature' not found or invalid format!",
                            samp_version.name);

                        continue;
                    }

                    samp_version.patch_offset = parse_address(section_name, "patch_offset");
                    if (!samp_version.patch_offset) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Offset 'patch_offset' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    samp_version.pChat = parse_address(section_name, "pChat");
                    if (!samp_version.pChat) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Offset 'pChat' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    samp_version.CChat_AddMessage = parse_address(section_name, "CChat_AddMessage");
                    if (!samp_version.CChat_AddMessage) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Offset 'CChat_AddMessage' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    samp_version.CGame_CreateVehicle = parse_address(section_name, "CGame_CreateVehicle");
                    if (!samp_version.CGame_CreateVehicle) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Offset 'CGame_CreateVehicle' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    samp_version.CGame_CreateVehicle_called_from_CVehiclePool_Create = parse_address(section_name, "CGame_CreateVehicle_called_from_CVehiclePool_Create");
                    if (!samp_version.CGame_CreateVehicle_called_from_CVehiclePool_Create) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Offset 'CGame_CreateVehicle_called_from_CVehiclePool_Create' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    Plugin.Log("SA:MP version '{}' loaded.", samp_version.name);
                    Plugin.Log("** Detect signature: {}  (size: {})", signature_to_string(samp_version.detect_signature),
                        samp_version.detect_signature.size());

                    Plugin.Log("** Patch offset: {:#x}", samp_version.patch_offset);
                    Plugin.Log("** pChat offset: {:#x}", samp_version.pChat);
                    Plugin.Log("** CChat::AddMessage offset: {:#x}", samp_version.CChat_AddMessage);
                    Plugin.Log("** CGame::CreateVehicle offset: {:#x}", samp_version.CGame_CreateVehicle);
                    Plugin.Log("** CGame::CreateVehicle called from CVehiclePool::Create offset: {:#x}", samp_version.CGame_CreateVehicle_called_from_CVehiclePool_Create);

                    samp_versions.emplace_back(samp_version);
                }
                else if (section_name_sv.starts_with("mod_sa ")) {
                    mod_sa_version_t mod_sa_version{};

                    mod_sa_version.name = section_name_sv.substr(7);

                    mod_sa_version.module_name = iniFile.GetValue(section_name, "module_name", "");
                    if (mod_sa_version.module_name.empty()) {
                        Plugin.Log("Warning: mod_sa_version '{}' ignored:  module_name not found or empty!",
                            mod_sa_version.name);

                        continue;
                    }

                    mod_sa_version.detect_offset = parse_address(section_name, "detect_offset");
                    if (!mod_sa_version.detect_offset) {
                        Plugin.Log("Warning: mod_sa_version '{}' ignored:  Offset 'detect_offset' not found or invalid format!",
                            mod_sa_version.name);
                        continue;
                    }

                    if (!parse_signature(section_name, "detect_signature", mod_sa_version.detect_signature)) {
                        Plugin.Log("Warning: mod_sa_version '{}' ignored:  Signature 'detect_signature' not found or invalid format!",
                            mod_sa_version.name);

                        continue;
                    }

                    if (!parse_pattern(section_name, "gta_vehicle_get_by_id_pattern", mod_sa_version.gta_vehicle_get_by_id_pattern)) {
                        Plugin.Log("Warning: mod_sa_version '{}' ignored:  Pattern 'gta_vehicle_get_by_id_pattern' not found or invalid format!",
                            mod_sa_version.name);

                        continue;
                    }

                    Plugin.Log("mod_sa version '{}' loaded.", mod_sa_version.name);
                    Plugin.Log("** Module name: {}", mod_sa_version.module_name);
                    Plugin.Log("** Detect offset: {:#x}", mod_sa_version.detect_offset);
                    Plugin.Log("** Detect signature: {}  (size: {})", signature_to_string(mod_sa_version.detect_signature),
                        mod_sa_version.detect_signature.size());
                    Plugin.Log("** gta_vehicle_get_by_id_pattern pattern:\n{} (size: {})", pattern_to_string(mod_sa_version.gta_vehicle_get_by_id_pattern),
                        mod_sa_version.gta_vehicle_get_by_id_pattern.size());

                    mod_sa_versions.emplace_back(mod_sa_version);
                }
            }
        }

        if (enable_samp_fix) {
            Plugin.Log("Loaded: {} samp versions", samp_versions.size());
        }

        if (enable_mod_sa_fix) {
            Plugin.Log("Loaded: {} mod_sa versions", mod_sa_versions.size());
        }

        if (iniFile.GetSection("vehicle_names")) {

            std::list<CSimpleIniA::Entry> keys{};
            iniFile.GetAllKeys("vehicle_names", keys);

            for (const auto& key : keys) {
                const auto model_id = parse_model_id(key.pItem);
                if (!model_id) {
                    continue;
                }

                const auto* name = iniFile.GetValue("vehicle_names", key.pItem);
                if (!name) {
                    continue;
                }
                VehicleNames.Add(model_id, name);
            }

            Plugin.Log("Loaded: {} vehicle names", VehicleNames.size());
        }
    }

#if 0
    iniFile.SaveFile(logFilePath);
#endif

    Plugin.Log("Settings loaded.");
}

bool Settings::parse_signature(const char* section, const char* name, signature_t& to) {
    if (!section || !section[0] || !name || !name[0]) {
        return false;
    }

    const auto signature_str = iniFile.GetValue(section, name, "");
    if (!signature_str || !signature_str[0]) {
        return false;
    }

    std::string_view signature_str_sv{ signature_str };
    const auto splits = split_str(signature_str_sv);
    if (!splits.size() || splits.size() > kMaxSignatuteBytes) {
        return false;
    }

    for (const auto& split : splits) {
        uint8_t byte{};
        if (!parse_byte(split, byte)) {
            return false;
        }

        to.emplace_back(byte);
    }
    return true;
}

bool Settings::parse_pattern(const char* section, const char* name, pattern_t& to) {
    if (!section || !section[0] || !name || !name[0]) {
        return false;
    }

    const auto pattern_str = iniFile.GetValue(section, name, "");
    if (!pattern_str || !pattern_str[0]) {
        return false;
    }

    std::string_view pattern_str_sv{ pattern_str };
    const auto splits = split_str(pattern_str_sv);
    if (!splits.size() || splits.size() > kMaxSignatuteBytes) {
        return false;
    }

    for (const auto& split : splits) {
        auto mask = 'x';
        uint8_t byte{};

        if (split != "**") {

            if (!is_hex_digit(split)) {
                return false;
            }

            if (!parse_byte(split, byte))
                return false;
        }
        else {
            mask = '?';
        }

        to.pattern += static_cast<char>(byte);
        to.mask += mask;
    }
    return true;
}

address_t Settings::parse_address(const char* section, const char* name) {
    if (!section || !section[0] || !name || !name[0]) {
        return false;
    }

    return iniFile.GetLongValue(section, name, 0);
}