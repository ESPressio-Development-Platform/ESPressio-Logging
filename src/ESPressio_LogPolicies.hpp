#pragma once

#include <cstddef>
#include <cstdint>

namespace ESPressio::Logging {

/// <summary>Defines how a bounded Sink buffer behaves when no capacity remains.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum class LogBufferOverflowPolicy : uint8_t {
    DropOldest = 0,
    DropNewest,
    Reject,
    Block
};

/// <summary>Explicitly bounded or intentionally unlimited byte capacity.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - CapacityMode (Mode): 4 bytes [0 bytes dynamic allocation]
 * - Bytes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogByteCapacity {
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum class Mode : uint8_t { Bounded = 0, Unlimited };

    Mode CapacityMode = Mode::Bounded;
    std::size_t Bytes = 0;

    static constexpr LogByteCapacity Bounded(std::size_t bytes) noexcept {
        return LogByteCapacity{Mode::Bounded, bytes};
    }
    static constexpr LogByteCapacity Unlimited() noexcept {
        return LogByteCapacity{Mode::Unlimited, 0};
    }
    constexpr bool IsValid() const noexcept {
        return CapacityMode == Mode::Unlimited || Bytes > 0;
    }
};

/// <summary>Explicitly bounded or intentionally unlimited item count.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - CapacityMode (Mode): 4 bytes [0 bytes dynamic allocation]
 * - Count (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogCountCapacity {
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum class Mode : uint8_t { Bounded = 0, Unlimited };

    Mode CapacityMode = Mode::Bounded;
    std::size_t Count = 0;

    static constexpr LogCountCapacity Bounded(std::size_t count) noexcept {
        return LogCountCapacity{Mode::Bounded, count};
    }
    static constexpr LogCountCapacity Unlimited() noexcept {
        return LogCountCapacity{Mode::Unlimited, 0};
    }
    constexpr bool IsValid() const noexcept {
        return CapacityMode == Mode::Unlimited || Count > 0;
    }
};

/// <summary>Platform-neutral rolling/retention constraints for persistent Logging Sinks.</summary>
/// <remarks>Unlimited retention is always explicit; zero never silently means unlimited.</remarks>
/**
 * ESPressio Memory Audit
 * Members:
 * - TotalCapacity (LogByteCapacity): 8 bytes [0 bytes dynamic allocation]
 * - FileCapacity (LogByteCapacity): 8 bytes [0 bytes dynamic allocation]
 * - FileCount (LogCountCapacity): 8 bytes [0 bytes dynamic allocation]
 * Total Memory: 24 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct RollingLogPolicy {
    LogByteCapacity TotalCapacity = LogByteCapacity::Bounded(0);
    LogByteCapacity FileCapacity = LogByteCapacity::Bounded(0);
    LogCountCapacity FileCount = LogCountCapacity::Bounded(0);

    constexpr bool IsValid() const noexcept {
        return TotalCapacity.IsValid() && FileCapacity.IsValid() && FileCount.IsValid();
    }
};

/// <summary>Cheap snapshot of Sink health counters; concrete Sinks own the corresponding counters.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - AcceptedRecords (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - FilteredRecords (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - DroppedRecords (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - WriteFailures (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - BytesWritten (uint64_t): 8 bytes [0 bytes dynamic allocation]
 * - QueueHighWaterMark (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 44 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct LogSinkStatistics {
    uint64_t AcceptedRecords = 0;
    uint64_t FilteredRecords = 0;
    uint64_t DroppedRecords = 0;
    uint64_t WriteFailures = 0;
    uint64_t BytesWritten = 0;
    std::size_t QueueHighWaterMark = 0;
};

} // namespace ESPressio::Logging
