#pragma once

#include <urmem/urmem.hpp>

using address_t = uintptr_t;

inline size_t GetModuleSize(char* name) {
    HMODULE hModule = GetModuleHandle(name);
    if (!hModule)
        return 0;
}

inline size_t GetModuleSize(HMODULE module) {
    auto dos_Header = PIMAGE_DOS_HEADER(module);
    auto pe_Header = PIMAGE_NT_HEADERS(long(module) + dos_Header->e_lfanew);
    auto optional_Header = &pe_Header->OptionalHeader;

    return optional_Header->SizeOfCode;
}

class module_t {
public:
    module_t() = default;
    module_t(const module_t&) = delete;

    explicit module_t(const char* name) {
        auto hModule = GetModuleHandle(name);
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