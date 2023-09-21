#include "pch.h"

#include "settings.h"

void Settings::Load() {
    Plugin.Log("Loading settings...");

    iniFile.LoadFile(kFileName);

    enable_samp_fix = iniFile.GetBoolValue("general", "enable_samp_fix", true);
    enable_mod_sa_fix = iniFile.GetBoolValue("general", "enable_mod_sa_fix", true);

    if (!enable_samp_fix && !enable_mod_sa_fix) {
        Plugin.Log("Nothing to load.");
    }
    else {
        std::list<CSimpleIniA::Entry> sections{};
        iniFile.GetAllSections(sections);

        for (const auto& section : sections) {
            const auto name = section.pItem;
            const auto name_sv = std::string_view(name);

            if (enable_samp_fix) {
                if (name_sv.starts_with("samp_")) {
                    samp_version_t samp_version{};

                    samp_version.name = iniFile.GetValue(name, "name", "unknown");

                    const auto detect_signature_str = iniFile.GetValue(name, "detect_signature", "");
                    if (!detect_signature_str[0]) {
                        Plugin.Log("Warning: samp_version '{}' ignored, parameter 'detect_signature' not found!", 
                            samp_version.name);

                        continue;
                    }

                    if (!parse_signature(detect_signature_str, samp_version.detect_signature)) {
                        Plugin.Log("Warning: samp_version '{}' ignored, invalid 'detect_signature' format!", 
                            samp_version.name);

                        continue;
                    }

                    const auto cgame_createvehicle_pattern_str = iniFile.GetValue(name, "cgame_createvehicle_pattern", "");
                    if (!cgame_createvehicle_pattern_str[0]) {
                        Plugin.Log("Warning: samp_version '{}' ignored, parameter 'cgame_createvehicle_pattern' not found!",
                            samp_version.name);

                        continue;
                    }

                    if (!parse_signature(cgame_createvehicle_pattern_str, samp_version.cgame_createvehicle_pattern)) {
                        Plugin.Log("Warning: samp_version '{}' ignored, invalid 'cgame_createvehicle_pattern' format!", 
                            samp_version.name);

                        continue;
                    }


                    Plugin.Log("SAMP version '{}' loaded.", samp_version.name);
                    Plugin.Log("  Detect signature: {}  (size: {})", signature_to_string(samp_version.detect_signature),
                        samp_version.detect_signature.size());
                    Plugin.Log("  CGame::CreateVehicle pattern: {}  (size: {})", signature_to_string(samp_version.cgame_createvehicle_pattern),
                        samp_version.cgame_createvehicle_pattern.size());
                    
                    samp_versions.emplace_back(samp_version);
                }
            }
        }

        if (enable_samp_fix) {
            Plugin.Log("Loaded: {} samp versions", samp_versions.size());
        }
    }

    iniFile.SaveFile(kFileName);

    Plugin.Log("Settings loaded.");
}