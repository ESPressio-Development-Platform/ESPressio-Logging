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
