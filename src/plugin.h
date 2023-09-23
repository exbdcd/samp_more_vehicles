#pragma once

#include "memory.h"
#include "settings.h"
#include "mod_sa_fix.h"

inline class Plugin {
public:
    static inline auto kVersion = "1.0";
    static inline auto kModuleName = "samp_more_vehicles.asi";
    static inline auto kLogFileName = "samp_more_vehicles.log";

public:
    void Init(HMODULE hModule);
    void Shutdown();

    void WriteToLog(const char* text);

    template<typename... Args>
    constexpr void Log(std::_Fmt_string<Args...> format, Args &&... args) {
        auto text = std::format(format, std::forward<Args>(args)...);
        WriteToLog(text.c_str());
    }

    std::string path_to(std::string_view path) const {
        return std::format("{}\\{}", modulePath, path);
    }

    Settings& GetSettings() {
        return settings;
    }

    module_t& get_gta_sa_module() {
        return gta_sa;
    }

    module_t& get_samp_module() {
        return samp;
    }

    samp_version_t& get_samp_version() {
        return samp_version;
    }

private:
    void DoInit();
    void DoModSaFix();

public:
    static bool IsVehicleModel(int id);
    void AddChatMessage(const std::string& text);

private:
    std::string modulePath{};

    std::ofstream logFileStream{};

    Settings settings{};

    module_t gta_sa{};
    module_t samp{};

    samp_version_t samp_version{};
} Plugin;