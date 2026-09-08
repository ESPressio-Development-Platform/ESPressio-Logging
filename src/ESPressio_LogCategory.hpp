#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ESPressio::Logging {

using LogCategoryId = uint64_t;

/// <summary>Computes the stable 64-bit FNV-1a identity used for a category name.</summary>
constexpr LogCategoryId HashLogCategory(std::string_view name) noexcept {
    LogCategoryId hash = 14695981039346656037ULL;
    for (char character : name) {
        hash ^= static_cast<uint8_t>(character);
        hash *= 1099511628211ULL;
    }
    return hash;
}

/// <summary>Open, application-extensible log category containing a compact identity and non-owning display name.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - Id (LogCategoryId): 8 bytes [0 bytes dynamic allocation]
 * - Name (std::string_view): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 16 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogCategory {
    LogCategoryId Id = 0;
    std::string_view Name{};

    constexpr LogCategory() noexcept = default;
    constexpr LogCategory(LogCategoryId id, std::string_view name) noexcept : Id(id), Name(name) {}

    template<std::size_t N>
    static constexpr LogCategory Named(const char (&name)[N]) noexcept {
        const std::string_view view{name, N - 1};
        return LogCategory{HashLogCategory(view), view};
    }

    constexpr bool operator==(const LogCategory& other) const noexcept { return Id == other.Id; }
    constexpr bool operator!=(const LogCategory& other) const noexcept { return !(*this == other); }
};

namespace Categories {
inline constexpr LogCategory Logging = LogCategory::Named("Logging");
inline constexpr LogCategory Application = LogCategory::Named("Application");
}

} // namespace ESPressio::Logging
