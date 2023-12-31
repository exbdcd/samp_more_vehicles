#pragma once

#include <simpleini/SimpleIni.h>

const auto kMaxSignatuteBytes = 64;
using signature_t = std::vector<uint8_t>;

struct pattern_t {
    std::string pattern{};
    std::string mask{};

    size_t size() const { return pattern.size(); }
};

struct samp_version_t {
    std::string name{};
    signature_t detect_signature{};
    address_t patch_offset{};
    address_t pChat{};
    address_t CChat_AddMessage{};
    address_t CGame_CreateVehicle{};
    address_t CGame_CreateVehicle_called_from_CVehiclePool_Create{};
};

struct mod_sa_version_t {
    std::string name{};
    std::string module_name{};
    address_t detect_offset{};
    signature_t detect_signature{};
    pattern_t gta_vehicle_get_by_id_pattern;
};

inline class VehicleNames {
public:
    void Add(int mi, const char* name) {
        if (!name || !name[0] || !mi) {
            return;
        }
        names.emplace(mi, name);
    }

    char* GetPtr(int mi) {
        auto name = names.find(mi);
        if (name == names.end()) {
            Add(mi, std::format("Vehicle {}", mi).c_str());
            return GetPtr(mi);
        }
        return name->second.data();
    }

    size_t size() const {
        return names.size();
    }

private:
    std::unordered_map<int, std::string> names{};
} VehicleNames;

class Settings {
public:
    static inline const auto kFileName = "samp_more_vehicles.ini";

private:
    void LoadDefault();

public:
    void Load();
    void UnLoad() {
#if 0
        iniFile.SaveFile(kFileName);
        iniFile.Reset();
#endif
    }

    static bool is_hex_digit(std::string_view sv) {
        return sv.size() == 2 && std::isxdigit(sv[0]) && std::isxdigit(sv[1]);
    }

    static bool parse_byte(std::string_view sv, uint8_t& to) {
        if (!is_hex_digit(sv)) {
            return false;
        }
        try {
            int result = std::stoi(std::string(sv), 0, 16);
            to = static_cast<uint8_t>(result);
            return true;
        }
        catch (...) {
        }
        return false;
    }

    static address_t parse_hex_address(std::string_view sv) {
        auto has0x{ false };
        if (sv.size() > 2 && sv.starts_with("0x")) {
            has0x = true;
        }

        bool is_hex_address = std::all_of(sv.begin() + (has0x ? 3 : 1), sv.end(), [](unsigned char c) {
            return std::isxdigit(c);
            });

        if (is_hex_address) {
            try {
                address_t result = std::stoi(std::string(has0x ? sv.substr(2) : sv), 0, 16);
                return result;
            }
            catch (...) {
            }
        }
        return 0;
    }

    static std::vector<std::string> split_str(std::string_view sv) {
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
    
    bool parse_signature(const char* section, const char* name, signature_t& to);

    bool parse_pattern(const char* section, const char* name, pattern_t& to);

    address_t parse_address(const char* section, const char* name);

    static int parse_model_id(const char* text) {
        if (!text || !text[0]) {
            return 0;
        }

        try {
            int result = std::stoi(std::string(text), 0, 10);
            return result;
        }
        catch (...) {
        }
        return 0;
    }
    
    static std::string signature_to_string(const signature_t& in) {
        std::string result{};
        bool not_first{};

        for (const auto i : in) {
            if (not_first) result += ' ';
            result +=  std::format("{:02X}", i);
            not_first = true;
        }
        return result;
    }

    static std::string pattern_to_string(const pattern_t& in) {
        std::string pattern_str{};

        for (size_t i = 0; i < in.pattern.size(); i++) {
            pattern_str += std::format("\\x{:02X}", static_cast<uint8_t>(in.pattern[i]));
        }

        return std::format("{} (mask: {})", pattern_str, in.mask);
    }

public:
    bool samp_fix_enabled() const {
        return enable_samp_fix;
    }

    bool do_print_warning_in_chat() const {
        return print_warning_in_chat;
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
    bool print_warning_in_chat{};

    std::vector<samp_version_t> samp_versions{};
    std::vector<mod_sa_version_t> mod_sa_versions{};
};