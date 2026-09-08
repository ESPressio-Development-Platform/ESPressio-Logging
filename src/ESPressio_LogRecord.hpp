#pragma once

#include <cstdint>
#include <string_view>

#include <ESPressio_ClockDiscipline.hpp>

#include "ESPressio_LogCategory.hpp"
#include "ESPressio_LogField.hpp"
#include "ESPressio_LogLevel.hpp"

namespace ESPressio::Logging {

/**
 * ESPressio Memory Audit
 * Members:
 * - MonotonicNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - SystemNanoseconds (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - SystemSynchronizationState (Timing::ClockSynchronizationState): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 20 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogTimestamp {
    uint64_t MonotonicNanoseconds = 0;
    uint64_t SystemNanoseconds = 0;
    Timing::ClockSynchronizationState SystemSynchronizationState =
        Timing::ClockSynchronizationState::Unsynchronized;
};

/**
 * ESPressio Memory Audit
 * Members:
 * - Timestamp (LogTimestamp): 20 bytes [0 bytes dynamic allocation]
 * - Level (LogLevel): 1 bytes [0 bytes dynamic allocation]
 * - Category (LogCategory): 16 bytes [0 bytes dynamic allocation]
 * - Message (std::string_view): 8 bytes [0 bytes dynamic allocation]
 * - Metadata (LogFieldView): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 56 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogRecordView {
    LogTimestamp Timestamp{};
    LogLevel Level = LogLevel::Info;
    LogCategory Category{};
    std::string_view Message{};
    LogFieldView Metadata{};
};

} // namespace ESPressio::Logging
