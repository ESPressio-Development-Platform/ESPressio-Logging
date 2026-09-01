#pragma once

#include <memory>

#include <ESPressio_Memory.hpp>
#include <ESPressio_ThreadSafeObservable.hpp>

#include "ESPressio_ILoggingObserver.hpp"

namespace ESPressio::Logging {

class LoggingObservable final : public Observable::ThreadSafeObservable {
public:
    void SinkRegistered(ILogSink* sink) {
        ExecuteNotification([&](NotificationContext& notification) {
            notification.WithObservers<ILoggingObserver>([&](ILoggingObserver* observer) {
                try { observer->OnLogSinkRegistered(sink); }
                catch (...) {}
            });
        });
    }

    void SinkUnregistered(ILogSink* sink) {
        ExecuteNotification([&](NotificationContext& notification) {
            notification.WithObservers<ILoggingObserver>([&](ILoggingObserver* observer) {
                try { observer->OnLogSinkUnregistered(sink); }
                catch (...) {}
            });
        });
    }
};

inline std::shared_ptr<LoggingObservable> CreateLoggingObservable() {
    return System::Memory::MakeShared<
        LoggingObservable,
        System::Memory::MemoryPolicy::ExternalPreferred
    >();
}

} // namespace ESPressio::Logging
