#pragma once

#include <cstdint>

namespace ESPressio::Logging {

enum class LogLevel : uint8_t {
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

using LogLevelMask = uint8_t;

constexpr LogLevelMask LogLevelBit(LogLevel level) noexcept {
    return static_cast<LogLevelMask>(1u << static_cast<uint8_t>(level));
}

constexpr LogLevelMask NoLogLevels = 0;
constexpr LogLevelMask AllLogLevels =
    LogLevelBit(LogLevel::Trace) |
    LogLevelBit(LogLevel::Debug) |
    LogLevelBit(LogLevel::Info) |
    LogLevelBit(LogLevel::Warn) |
    LogLevelBit(LogLevel::Error) |
    LogLevelBit(LogLevel::Fatal);

constexpr bool ContainsLevel(LogLevelMask mask, LogLevel level) noexcept {
    return (mask & LogLevelBit(level)) != 0;
}

constexpr LogLevelMask AtOrAbove(LogLevel minimum) noexcept {
    LogLevelMask mask = 0;
    for (uint8_t value = static_cast<uint8_t>(minimum);
         value <= static_cast<uint8_t>(LogLevel::Fatal);
         ++value) {
        mask = static_cast<LogLevelMask>(mask | static_cast<LogLevelMask>(1u << value));
    }
    return mask;
}

} // namespace ESPressio::Logging
