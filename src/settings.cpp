#include "pch.h"

#include "settings.h"

void Settings::LoadDefault() {
    if (!iniFile.GetValue("general", "enable_samp_fix")) {
        iniFile.SetValue("general", "enable_samp_fix", "true");
    }

    if (!iniFile.GetValue("general", "enable_mod_sa_fix")) {
        iniFile.SetValue("general", "enable_mod_sa_fix", "false");
    }

    if (!iniFile.GetSection("samp 0.3.7")) {
        iniFile.SetValue("samp 0.3.7", "detect_signature", "F8 03 6A 00 40 50 51 8D 4C 24 0C E8 82 02 01 00");
#if 0
        iniFile.SetValue("samp 0.3.7", "cgame_createvehicle_pattern", "64 A1 00 00 00 00 6A FF 68 ** ** ** ** 50 64 89 25 00 00 00 00 56 57 8B 7C 24 18 8A 84 0F DE FE FF FF");
#endif
        iniFile.SetValue("samp 0.3.7", "patch_offset", "0x00246915");
        iniFile.SetValue("samp 0.3.7", "pChat", "0x0021A0E4");
        iniFile.SetValue("samp 0.3.7", "CChat_AddMessage", "0x000645A0");
        iniFile.SetValue("samp 0.3.7", "CGame_CreateVehicle", "0x9B890");
        iniFile.SetValue("samp 0.3.7", "CGame_CreateVehicle_called_from_CVehiclePool_Create", "0x1B5EB");
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
                        Plugin.Log("Warning: samp_version '{}' ignored: Signature 'detect_signature' not found or invalid format!",
                            samp_version.name);

                        continue;
                    }

#if 0
                    if (!parse_pattern(section_name, "cgame_createvehicle_pattern", samp_version.cgame_createvehicle_pattern)) {
                        Plugin.Log("Warning: samp_version '{}' ignored: Pattern 'cgame_createvehicle_pattern' not found or invalid format!",
                            samp_version.name);

                        continue;
                    }
#endif
                    samp_version.patch_offset = parse_address(section_name, "patch_offset");
                    if (!samp_version.patch_offset) {
                        Plugin.Log("Warning: samp_version '{}' ignored: Offset 'patch_offset' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    samp_version.CGame_CreateVehicle = parse_address(section_name, "CGame_CreateVehicle");
                    if (!samp_version.CGame_CreateVehicle) {
                        Plugin.Log("Warning: samp_version '{}' ignored: Offset 'CGame_CreateVehicle' not found or invalid format!",
                            samp_version.name);
                        continue;
                    }

                    samp_version.CGame_CreateVehicle_called_from_CVehiclePool_Create = parse_address(section_name, "CGame_CreateVehicle_called_from_CVehiclePool_Create");
                    if (!samp_version.CGame_CreateVehicle_called_from_CVehiclePool_Create) {
                        Plugin.Log("Warning: samp_version '{}' ignored: Offset 'CGame_CreateVehicle_called_from_CVehiclePool_Create' not found or invalid format!",
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
                    Plugin.Log("** CGame::CreateVehicle offset: {:#x}", samp_version.CGame_CreateVehicle);
                    Plugin.Log("** CGame::CreateVehicle called from CVehiclePool::Create offset: {:#x}", samp_version.CGame_CreateVehicle_called_from_CVehiclePool_Create);

                    samp_versions.emplace_back(samp_version);
                }
            }
        }

        if (enable_samp_fix) {
            Plugin.Log("Loaded: {} samp versions", samp_versions.size());
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