#pragma once

#include <cstdint>
#include <string_view>

#include <ESPressio_ClockDiscipline.hpp>

#include "ESPressio_LogCategory.hpp"
#include "ESPressio_LogField.hpp"
#include "ESPressio_LogLevel.hpp"

namespace ESPressio::Logging {

struct LogTimestamp {
    uint64_t MonotonicNanoseconds = 0;
    uint64_t SystemNanoseconds = 0;
    Timing::ClockSynchronizationState SystemSynchronizationState =
        Timing::ClockSynchronizationState::Unsynchronized;
};

struct LogRecordView {
    LogTimestamp Timestamp{};
    LogLevel Level = LogLevel::Info;
    LogCategory Category{};
    std::string_view Message{};
    LogFieldView Metadata{};
};

} // namespace ESPressio::Logging
