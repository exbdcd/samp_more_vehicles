#pragma once

#include "memory.h"
#include "settings.h"

inline class Plugin {
public:
    static inline auto kVersion = "1.0";
    static inline auto kLogFileName = "samp_more_vehicles.log";

public:
    void Init();
    void Shutdown();

    void WriteToLog(const char* text);

    template<typename... Args>
    constexpr void Log(std::_Fmt_string<Args...> format, Args &&... args) {
        auto text = std::format(format, std::forward<Args>(args)...);
        WriteToLog(text.c_str());
    }

    Settings& GetSettings() {
        return settings;
    }

private:
    void DoInit();

private:
    std::ofstream logFileStream;

    Settings settings{};

    module_t game_sa{};
    module_t samp{};


} Plugin;