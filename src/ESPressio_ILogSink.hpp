#pragma once

#include "ESPressio_LogCategory.hpp"
#include "ESPressio_LogLevel.hpp"
#include "ESPressio_SharedLogRecord.hpp"

namespace ESPressio::Logging {

class ILogSink {
public:
    virtual ~ILogSink() = default;

    /// <summary>Returns whether this Sink wants records for the supplied level/category.</summary>
    virtual bool IsEnabled(LogLevel level, const LogCategory& category) const noexcept = 0;

    /// <summary>Accepts a log record. Borrowed references are valid only until this call returns.</summary>
    /// <remarks>
    /// A Sink crossing a queue/task/thread/deferred boundary must either retain an explicitly
    /// retainable lease or materialise directly into its own bounded durable representation.
    /// It must never retain borrowed pointers from a non-retainable lease.
    /// </remarks>
    virtual void Accept(const LogRecordLease& record) noexcept = 0;
};

} // namespace ESPressio::Logging
