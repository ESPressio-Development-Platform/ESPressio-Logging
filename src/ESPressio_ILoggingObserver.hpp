#pragma once

#include <ESPressio_IObserver.hpp>

namespace ESPressio::Logging {

class ILogSink;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members: none; polymorphic/virtual-base object metadata is included in the total.
 * Total Memory: 4 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class ILoggingObserver : public virtual Observable::IObserver {
public:
    virtual ~ILoggingObserver() = default;

    /// <summary>Notifies that a Sink has become part of the Logger routing topology.</summary>
    virtual void OnLogSinkRegistered(ILogSink* sink) = 0;

    /// <summary>Notifies that a Sink has been removed from the Logger routing topology.</summary>
    virtual void OnLogSinkUnregistered(ILogSink* sink) = 0;
};

} // namespace ESPressio::Logging
