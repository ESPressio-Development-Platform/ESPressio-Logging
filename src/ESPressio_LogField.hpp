#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <variant>

namespace ESPressio::Logging {

using LogFieldValue = std::variant<
    bool,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float,
    double,
    std::string_view
>;

/**
 * ESPressio Memory Audit
 * Members:
 * - Name (std::string_view): 8 bytes [0 bytes dynamic allocation]
 * - Value (LogFieldValue): 12 bytes [0 bytes dynamic allocation]
 * Total Memory: 20 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
struct LogField {
    std::string_view Name{};
    LogFieldValue Value{};
};

/**
 * ESPressio Memory Audit
 * Members:
 * - Data (LogField*): 4 bytes [0 bytes dynamic allocation]
 * - Count (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogFieldView {
    const LogField* Data = nullptr;
    std::size_t Count = 0;

    constexpr const LogField* begin() const noexcept { return Data; }
    constexpr const LogField* end() const noexcept { return Data == nullptr ? nullptr : Data + Count; }
    constexpr bool Empty() const noexcept { return Count == 0; }
};

template<std::size_t N>
constexpr LogFieldView Fields(const LogField (&fields)[N]) noexcept {
    return LogFieldView{fields, N};
}

} // namespace ESPressio::Logging
