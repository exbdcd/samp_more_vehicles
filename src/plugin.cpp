#include "pch.h"

#include "plugin.h"

void Plugin::Init() {
    const auto init = [&]() {
        Log("Plugin loaded!");
        Log("Version: {}", kVersion);
        Log("Build time: {} {}", __DATE__, __TIME__);

        settings.Load();

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(30));

            if (game_sa = module_t("gta_sa.exe"), !game_sa.get_address()) {
                continue;
            }

            if (samp = module_t("samp.dll"), !samp.get_address()) {
                continue;
            }

            DoInit();
            break;
        }
    };

    std::thread(init).detach();
}

void Plugin::Shutdown() {
    if (logFileStream.is_open()) {
        logFileStream.close();
    }
}

void Plugin::WriteToLog(const char* text) {
    if (!logFileStream.is_open()) {
        logFileStream.open(kLogFileName, std::ios::out | std::ios::trunc | std::ios::ate);

        if (logFileStream.is_open()) {
            return;
        }
    }

    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    logFileStream << std::format("[{:%H:%M:%S}] {}\n", time, text);
    logFileStream.flush();
}

void Plugin::DoInit() {
    Log("Do init stuff");

    if (settings.samp_fix_enabled()) {
        for (const auto& samp_version : settings.get_samp_versions()) {

        }
    }
}