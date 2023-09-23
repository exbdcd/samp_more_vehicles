#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#define VALIDATE_SIZE(struc, size) static_assert(sizeof(struc) == size, "Invalid structure size of " #struc)

#define VALIDATE_OFFSET(struc, member, offset)  \
    static_assert(offsetof(struc, member) == offset, "The offset of " #member " in " #struc " is not " #offset "...")

#include "src/plugin.h"