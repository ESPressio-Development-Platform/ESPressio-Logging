#pragma once

#include <ESPressio_IObserver.hpp>

namespace ESPressio::Logging {

class ILogSink;

class ILoggingObserver : public virtual Observable::IObserver {
public:
    virtual ~ILoggingObserver() = default;

    /// <summary>Notifies that a Sink has become part of the Logger routing topology.</summary>
    virtual void OnLogSinkRegistered(ILogSink* sink) = 0;

    /// <summary>Notifies that a Sink has been removed from the Logger routing topology.</summary>
    virtual void OnLogSinkUnregistered(ILogSink* sink) = 0;
};

} // namespace ESPressio::Logging
