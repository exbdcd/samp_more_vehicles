#pragma once

#include <urmem/urmem.hpp>

using address_t = uintptr_t;

inline size_t GetModuleSize(HMODULE hModule) {
    auto dos_Header = PIMAGE_DOS_HEADER(hModule);
    auto pe_Header = PIMAGE_NT_HEADERS(long(hModule) + dos_Header->e_lfanew);
    auto optional_Header = &pe_Header->OptionalHeader;

    return optional_Header->SizeOfCode;
}

inline size_t GetModuleSize(char* name) {
    HMODULE hModule = GetModuleHandleA(name);
    if (!hModule)
        return 0;

    return GetModuleSize(hModule);
}

inline std::string GetModuleFileName(HMODULE hModule)
{
    static constexpr auto INITIAL_BUFFER_SIZE = MAX_PATH;
    static constexpr auto MAX_ITERATIONS = 7;
    std::string ret;
    auto bufferSize = INITIAL_BUFFER_SIZE;
    for (size_t iterations = 0; iterations < MAX_ITERATIONS; ++iterations)
    {
        ret.resize(bufferSize);
        auto charsReturned = GetModuleFileNameA(hModule, &ret[0], bufferSize);
        if (charsReturned < ret.length())
        {
            ret.resize(charsReturned);
            return ret;
        }
        else
        {
            bufferSize *= 2;
        }
    }
    return "";
}

class module_t {
public:
    module_t() = default;
    module_t(const module_t&) = delete;

    explicit module_t(const char* name) {
        auto hModule = GetModuleHandleA(name);
        if (!hModule) {
            return;
        }

        name = name;
        address = reinterpret_cast<address_t>(hModule);
        size = GetModuleSize(hModule);
    }

    std::string get_name() const {
        return name;
    }

    address_t get_address() const {
        return address;
    }

    size_t get_size() const {
        return size;
    }

private:
    std::string     name{};
    address_t       address{};
    size_t          size{};
};

int __page_size_get(void);
bool isBadPtr_handlerAny(void* pointer, ULONG size, DWORD dwFlags);
bool isBadPtr_readAny(void* pointer, ULONG size);
bool isBadPtr_writeAny(void* pointer, ULONG size);
int __page_write(void* _dest, const void* _src, uint32_t len);
int __page_read(void* _dest, const void* _src, uint32_t len);
int memcpy_safe(void* _dest, const void* _src, uint32_t len, int check = NULL, const void* checkdata = NULL);
int memcmp_safe(const void* _s1, const void* _s2, uint32_t len);