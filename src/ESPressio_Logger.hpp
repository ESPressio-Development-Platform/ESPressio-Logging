#pragma once

#include <ESPressio_StopwatchClock.hpp>
#include <ESPressio_SystemClock.hpp>
#include <ESPressio_TimeTraits.hpp>

#include "ESPressio_LogRouter.hpp"

#ifndef ESPRESSIO_LOGGING_ENABLE_TRACE
#define ESPRESSIO_LOGGING_ENABLE_TRACE 0
#endif
#ifndef ESPRESSIO_LOGGING_ENABLE_DEBUG
#define ESPRESSIO_LOGGING_ENABLE_DEBUG 0
#endif
#ifndef ESPRESSIO_LOGGING_ENABLE_INFO
#define ESPRESSIO_LOGGING_ENABLE_INFO 1
#endif
#ifndef ESPRESSIO_LOGGING_ENABLE_WARN
#define ESPRESSIO_LOGGING_ENABLE_WARN 1
#endif
#ifndef ESPRESSIO_LOGGING_ENABLE_ERROR
#define ESPRESSIO_LOGGING_ENABLE_ERROR 1
#endif
#ifndef ESPRESSIO_LOGGING_ENABLE_FATAL
#define ESPRESSIO_LOGGING_ENABLE_FATAL 1
#endif

namespace ESPressio::Logging {

/**
 * ESPressio Memory Audit
 * Members:
 * - _router (LogRouter): 40 bytes known/aligned storage + ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES * (12 bytes) [_stateLock: _owned: owned object: 4 bytes; _stateLock: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 96 bytes; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _mutex: native synchronization state may allocate platform resources lazily; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _condition: native condition-variable state may allocate platform synchronization resources; _observable: pointee: ThreadSafeObservable: Observable: _registrations: Capacity * (12 bytes) element storage; _observable: pointee: ThreadSafeObservable: Observable: _bindings: Capacity * (12 bytes) element storage; _observable: pointee: ThreadSafeObservable: _mutex: _owned: owned object: 4 bytes; _observable: pointee: ThreadSafeObservable: _mutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _observable: pointee: ThreadSafeObservable: _notificationMutex: _owned: owned object: 4 bytes; _observable: pointee: ThreadSafeObservable: _notificationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily]
 * - _monotonicClock (Timing::StopwatchClock<>): 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) [_observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Total Memory: 40 bytes known/aligned storage + ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES * (12 bytes) + 24 bytes known/aligned storage + sizeof(TTick) + sizeof(TTick) [_router: _stateLock: _owned: owned object: 4 bytes; _router: _stateLock: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _router: _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 96 bytes; _router: _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _router: _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _router: _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _mutex: native synchronization state may allocate platform resources lazily; _router: _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _condition: native condition-variable state may allocate platform synchronization resources; _router: _observable: pointee: ThreadSafeObservable: Observable: _registrations: Capacity * (12 bytes) element storage; _router: _observable: pointee: ThreadSafeObservable: Observable: _bindings: Capacity * (12 bytes) element storage; _router: _observable: pointee: ThreadSafeObservable: _mutex: _owned: owned object: 4 bytes; _router: _observable: pointee: ThreadSafeObservable: _mutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _router: _observable: pointee: ThreadSafeObservable: _notificationMutex: _owned: owned object: 4 bytes; _router: _observable: pointee: ThreadSafeObservable: _notificationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _monotonicClock: _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _monotonicClock: _observable: pointee: ThreadSafeObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _monotonicClock: _observable: pointee: ThreadSafeObservable: mutex_: native synchronization state may allocate platform resources lazily]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Logger final {
public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /// <summary>Returns the process-wide ESPressio Logger.</summary>
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    /// <summary>Returns the single Router used by this Logger.</summary>
    LogRouter& Router() noexcept { return _router; }
    const LogRouter& Router() const noexcept { return _router; }

    /// <summary>Performs cheap runtime eligibility checking before message/metadata construction.</summary>
    bool IsEnabled(LogLevel level, const LogCategory& category) const noexcept {
        return _router.IsEnabled(level, category);
    }

    /// <summary>Captures monotonic and System Clock time for a new record.</summary>
    LogTimestamp CaptureTimestamp() {
        const auto monotonic = _monotonicClock.GetTime();
        auto& systemClock = Timing::SystemClock<>::GetInstance();
        const auto system = systemClock.GetTime();
        const auto synchronization = systemClock.GetSynchronizationStatus();

        return LogTimestamp{
            Timing::TimeTraits<Timing::DefaultClockTime>::template ToNanoseconds<uint64_t>(monotonic),
            Timing::TimeTraits<Timing::DefaultClockTime>::template ToNanoseconds<uint64_t>(system),
            synchronization.State
        };
    }

    /// <summary>Routes one borrowed structured record without taking ownership of message or metadata memory.</summary>
    void Log(
        LogLevel level,
        const LogCategory& category,
        std::string_view message,
        LogFieldView metadata = {}
    ) noexcept {
        if (!_router.IsEnabled(level, category)) return;
        try {
            const LogRecordView view{CaptureTimestamp(), level, category, message, metadata};
            const LogRecordLease lease(view);
            _router.Dispatch(lease);
        } catch (...) {
            // Diagnostic infrastructure must not destabilise its caller.
        }
    }

    /// <summary>Routes an already-durable record whose lifetime may be shared by retaining Sinks.</summary>
    void LogShared(ISharedLogRecord& record) noexcept {
        const auto& view = record.GetLogRecordView();
        if (!_router.IsEnabled(view.Level, view.Category)) return;
        const LogRecordLease lease(record);
        _router.Dispatch(lease);
    }

    void Trace(const LogCategory& category, std::string_view message, LogFieldView metadata = {}) noexcept { Log(LogLevel::Trace, category, message, metadata); }
    void Debug(const LogCategory& category, std::string_view message, LogFieldView metadata = {}) noexcept { Log(LogLevel::Debug, category, message, metadata); }
    void Info(const LogCategory& category, std::string_view message, LogFieldView metadata = {}) noexcept { Log(LogLevel::Info, category, message, metadata); }
    void Warn(const LogCategory& category, std::string_view message, LogFieldView metadata = {}) noexcept { Log(LogLevel::Warn, category, message, metadata); }
    void Error(const LogCategory& category, std::string_view message, LogFieldView metadata = {}) noexcept { Log(LogLevel::Error, category, message, metadata); }
    void Fatal(const LogCategory& category, std::string_view message, LogFieldView metadata = {}) noexcept { Log(LogLevel::Fatal, category, message, metadata); }

private:
    Logger() : _monotonicClock(true) {}

    LogRouter _router;
    Timing::StopwatchClock<> _monotonicClock;
};

} // namespace ESPressio::Logging

#if ESPRESSIO_LOGGING_ENABLE_TRACE
#define ESPRESSIO_LOG_TRACE(category, ...) do { const auto& _espressio_log_category = (category); auto& _espressio_logger = ::ESPressio::Logging::Logger::GetInstance(); if (_espressio_logger.IsEnabled(::ESPressio::Logging::LogLevel::Trace, _espressio_log_category)) _espressio_logger.Trace(_espressio_log_category, __VA_ARGS__); } while (0)
#else
#define ESPRESSIO_LOG_TRACE(...) do { } while (0)
#endif

#if ESPRESSIO_LOGGING_ENABLE_DEBUG
#define ESPRESSIO_LOG_DEBUG(category, ...) do { const auto& _espressio_log_category = (category); auto& _espressio_logger = ::ESPressio::Logging::Logger::GetInstance(); if (_espressio_logger.IsEnabled(::ESPressio::Logging::LogLevel::Debug, _espressio_log_category)) _espressio_logger.Debug(_espressio_log_category, __VA_ARGS__); } while (0)
#else
#define ESPRESSIO_LOG_DEBUG(...) do { } while (0)
#endif

#if ESPRESSIO_LOGGING_ENABLE_INFO
#define ESPRESSIO_LOG_INFO(category, ...) do { const auto& _espressio_log_category = (category); auto& _espressio_logger = ::ESPressio::Logging::Logger::GetInstance(); if (_espressio_logger.IsEnabled(::ESPressio::Logging::LogLevel::Info, _espressio_log_category)) _espressio_logger.Info(_espressio_log_category, __VA_ARGS__); } while (0)
#else
#define ESPRESSIO_LOG_INFO(...) do { } while (0)
#endif

#if ESPRESSIO_LOGGING_ENABLE_WARN
#define ESPRESSIO_LOG_WARN(category, ...) do { const auto& _espressio_log_category = (category); auto& _espressio_logger = ::ESPressio::Logging::Logger::GetInstance(); if (_espressio_logger.IsEnabled(::ESPressio::Logging::LogLevel::Warn, _espressio_log_category)) _espressio_logger.Warn(_espressio_log_category, __VA_ARGS__); } while (0)
#else
#define ESPRESSIO_LOG_WARN(...) do { } while (0)
#endif

#if ESPRESSIO_LOGGING_ENABLE_ERROR
#define ESPRESSIO_LOG_ERROR(category, ...) do { const auto& _espressio_log_category = (category); auto& _espressio_logger = ::ESPressio::Logging::Logger::GetInstance(); if (_espressio_logger.IsEnabled(::ESPressio::Logging::LogLevel::Error, _espressio_log_category)) _espressio_logger.Error(_espressio_log_category, __VA_ARGS__); } while (0)
#else
#define ESPRESSIO_LOG_ERROR(...) do { } while (0)
#endif

#if ESPRESSIO_LOGGING_ENABLE_FATAL
#define ESPRESSIO_LOG_FATAL(category, ...) do { const auto& _espressio_log_category = (category); auto& _espressio_logger = ::ESPressio::Logging::Logger::GetInstance(); if (_espressio_logger.IsEnabled(::ESPressio::Logging::LogLevel::Fatal, _espressio_log_category)) _espressio_logger.Fatal(_espressio_log_category, __VA_ARGS__); } while (0)
#else
#define ESPRESSIO_LOG_FATAL(...) do { } while (0)
#endif
