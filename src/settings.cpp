#include "pch.h"

#include "settings.h"

void Settings::LoadDefault() {
    if (!iniFile.GetValue("general", "enable_samp_fix")) {
        iniFile.SetValue("general", "enable_samp_fix", "true");
    }

    if (!iniFile.GetValue("general", "enable_mod_sa_fix")) {
        iniFile.SetValue("general", "enable_mod_sa_fix", "false");
    }

    if (!iniFile.GetValue("general", "print_warning_in_chat")) {
        iniFile.SetValue("general", "print_warning_in_chat", "true");
    }

    if (!iniFile.GetSection("samp 0.3.7-R1")) {
        iniFile.SetValue("samp 0.3.7-R1", "detect_signature", "F8 03 6A 00 40 50 51 8D 4C 24 0C E8 82 02 01 00");
        iniFile.SetValue("samp 0.3.7-R1", "patch_offset", "0x246915");
        iniFile.SetValue("samp 0.3.7-R1", "pChat", "0x21A0E4");
        iniFile.SetValue("samp 0.3.7-R1", "CChat_AddMessage", "0x645A0");
        iniFile.SetValue("samp 0.3.7-R1", "CGame_CreateVehicle", "0x9B890");
        iniFile.SetValue("samp 0.3.7-R1", "CGame_CreateVehicle_called_from_CVehiclePool_Create", "0x1B5EB");
    }

    if (!iniFile.GetSection("samp 0.3.7-R3")) {
        iniFile.SetValue("samp 0.3.7-R3", "detect_signature", "E8 6D 9A 0A 00 83 C4 1C 85 C0 75 08 50 57 FF 15");
        iniFile.SetValue("samp 0.3.7-R3", "patch_offset", "0xE412");
        iniFile.SetValue("samp 0.3.7-R3", "pChat", "0x26E8C8");
        iniFile.SetValue("samp 0.3.7-R3", "CChat_AddMessage", "0x679F0");
        iniFile.SetValue("samp 0.3.7-R3", "CGame_CreateVehicle", "0x9FB40");
        iniFile.SetValue("samp 0.3.7-R3", "CGame_CreateVehicle_called_from_CVehiclePool_Create", "0x1E98B");
    }

    if (!iniFile.GetSection("samp 0.3.7-R5")) {
        iniFile.SetValue("samp 0.3.7-R5", "detect_signature", "C0 74 06 88 9E 34 02 00 00 88 9E 32 02 00 00 88");
        iniFile.SetValue("samp 0.3.7-R5", "patch_offset", "0xE751");
        iniFile.SetValue("samp 0.3.7-R5", "pChat", "0x26EB80");
        iniFile.SetValue("samp 0.3.7-R5", "CChat_AddMessage", "0x68170");
        iniFile.SetValue("samp 0.3.7-R5", "CGame_CreateVehicle", "0xA0250");
        iniFile.SetValue("samp 0.3.7-R5", "CGame_CreateVehicle_called_from_CVehiclePool_Create", "0x1F0DB");
    }
}

void Settings::Load() {
    Plugin.Log("Loading settings...");

    const auto logFilePath = Plugin.path_to(kFileName).c_str();

    auto error = iniFile.LoadFile(logFilePath);
    if (error != SI_OK) {
        Plugin.Log("SI_Error: {}", error);
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

#if 0
                    if (!parse_pattern(section_name, "cgame_createvehicle_pattern", samp_version.cgame_createvehicle_pattern)) {
                        Plugin.Log("Warning: samp_version '{}' ignored:  Pattern 'cgame_createvehicle_pattern' not found or invalid format!",
                            samp_version.name);

                        continue;
                    }
#endif
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

                    Plugin.Log("SAMP version '{}' loaded.", samp_version.name);
                    Plugin.Log("** Detect signature: {}  (size: {})", signature_to_string(samp_version.detect_signature),
                        samp_version.detect_signature.size());
#if 0
                    Plugin.Log("** CGame::CreateVehicle pattern: {}  (size: {})", pattern_to_string(samp_version.cgame_createvehicle_pattern),
                        samp_version.cgame_createvehicle_pattern.size());
#endif
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