#pragma once

#include <simpleini/SimpleIni.h>

const auto kMaxSignatuteBytes = 32;
using signature_t = std::vector<uint8_t>;

struct samp_version_t {
    std::string name{};
    signature_t detect_signature{};
    signature_t cgame_createvehicle_pattern{};
};

struct mod_sa_version_t {
    std::string module_name{};
};

class Settings {
public:
    static inline const auto kFileName = "samp_more_vehicles.ini";

public:
    void Load();

private:
    bool is_hex_digit(std::string_view sv) {
        return sv.size() == 2 && std::isxdigit(sv[0]) && std::isxdigit(sv[1]);
    }

    bool parse_byte(std::string_view sv, uint8_t& to) {
        if (!is_hex_digit(sv)) {
            return false;
        }
        try {
            int result = std::stoi(std::string(sv));
            to = static_cast<uint8_t>(result);
            return true;
        }
        catch (...) {
        }
        return false;
    }

    std::vector<std::string> split_str(std::string_view sv) {
        std::vector<std::string> result{};

        const auto delimiter = ' ';
        std::string_view::const_iterator start = sv.begin();
        std::string_view::const_iterator end = sv.end();
        std::string_view::const_iterator next = std::find(start, end, delimiter);

        while (next != end) {
            result.emplace_back(start, next);
            start = next + 1;
            next = std::find(start, end, delimiter);
        }

        result.emplace_back(start, next);
        return result;
    }

    bool parse_signature(std::string_view sv, signature_t& to) {
        const auto splits = split_str(sv);

        if (!splits.size() || splits.size() > kMaxSignatuteBytes) {
            return false;
        }

        for (const auto& split : splits) {
            if (!is_hex_digit(split)) {
                return false;
            }

            uint8_t byte{};
            if (!parse_byte(sv, byte))
                return false;

            to.emplace_back(byte);
        }
        return true;
    }
    
    std::string signature_to_string(const signature_t& in) {
        std::string result{};
        bool not_first{};
        for (const auto i : in) {
            result += not_first ? std::format(" {:X}", i) : std::format("{:X}", i);
            not_first = true;
        }
        return result;
    }

public:
    bool samp_fix_enabled() const {
        return enable_samp_fix;
    }

    bool mod_sa_fix_enabled() const {
        return enable_mod_sa_fix;
    }

    const std::vector<samp_version_t>& get_samp_versions() {
        return samp_versions;
    }

    const std::vector<mod_sa_version_t>& get_mod_sa_versions() {
        return mod_sa_versions;
    }

private:
    CSimpleIniA iniFile{};

    bool enable_samp_fix{};
    bool enable_mod_sa_fix{};

    std::vector<samp_version_t> samp_versions{};
    std::vector<mod_sa_version_t> mod_sa_versions{};
};