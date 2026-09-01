# ESPressio-Logging

Platform-neutral, structured Logging abstractions for the ESPressio Development Platform.

## Design goals

`ESPressio-Logging` defines what a log record is, whether it should be emitted, and how it is routed. It deliberately does not know how UART, filesystems, sockets, encryption algorithms, ESP32 APIs, Arduino APIs, or any other concrete output mechanism works. Concrete Logging Sinks live beside the context they adapt, for example Serial Sinks in `ESPressio-Serial`, persistent Sinks in `ESPressio-Persistence`, and Socket Sinks in `ESPressio-Sockets`.

The core design is intentionally zero-copy-friendly. A normal synchronous log call creates only a borrowed `LogRecordView`; its message and structured metadata remain non-owning views and the same record is routed to every interested Sink. A Sink may not retain borrowed data after `ILogSink::Accept()` returns. When a temporal boundary is crossed (queue, Task, lifetime Thread, deferred callback, etc.), the Sink must either retain an explicitly shareable record or materialise directly into its own bounded durable representation. In short: **borrow across calls; own across time**.

## Levels

The levels are `Trace`, `Debug`, `Info`, `Warn`, `Error`, and `Fatal`. Filtering is mask-based rather than minimum-threshold-only, so configurations such as `Info=on`, `Warn=off`, `Error=on`, `Fatal=on` are valid. `AtOrAbove(...)` remains available as a convenience for conventional severity-threshold behaviour.

Runtime filtering is the intersection of the global level mask, any category-specific level mask, and the receiving Sink's own `IsEnabled(level, category)` decision.

### Compile-time elimination

Each level has an independent compile-time switch:

- `ESPRESSIO_LOGGING_ENABLE_TRACE` (default `0`)
- `ESPRESSIO_LOGGING_ENABLE_DEBUG` (default `0`)
- `ESPRESSIO_LOGGING_ENABLE_INFO` (default `1`)
- `ESPRESSIO_LOGGING_ENABLE_WARN` (default `1`)
- `ESPRESSIO_LOGGING_ENABLE_ERROR` (default `1`)
- `ESPRESSIO_LOGGING_ENABLE_FATAL` (default `1`)

Use the corresponding `ESPRESSIO_LOG_TRACE`, `ESPRESSIO_LOG_DEBUG`, `ESPRESSIO_LOG_INFO`, `ESPRESSIO_LOG_WARN`, `ESPRESSIO_LOG_ERROR`, and `ESPRESSIO_LOG_FATAL` macros when code must be physically absent from a build when that level is disabled. Disabled macros do not evaluate their arguments, so expensive diagnostic expressions and diagnostic-only string construction are not performed.

Direct `Logger::Debug(...)`/etc. methods remain available for code whose inclusion is intentionally controlled elsewhere, but they cannot provide preprocessing-time elimination of their call-site arguments.

## Categories

Categories are open and application-extensible. They are not a closed ESPressio enum. `LogCategory::Named("Laser-Trigger")`, `LogCategory::Named("NoiseMonitor")`, or any other static application category produces a compact stable FNV-1a identity while retaining a non-owning name view for human-readable output.

`Categories::Logging` and `Categories::Application` are the only core categories currently supplied by this library. Individual ESPressio libraries should publish their own category constants in their own context rather than growing a central category enum here.

## Structured metadata

`LogRecordView` carries a non-owning `LogFieldView`. Fields use a small typed `std::variant` containing scalar values or `std::string_view`; the core does not allocate a map, vector, owning string, or serialized payload merely to represent metadata.

## Time

Every record carries both:

- monotonic nanoseconds, used for ordering and elapsed-time reasoning; and
- System Clock nanoseconds plus the current `ClockSynchronizationState`, used for Mesh-wide comparable time when the ESPressio System Clock has been synchronized.

The default `Logger` obtains monotonic time from an ESPressio Timing `StopwatchClock` and System time from the singleton `SystemClock`. Applications with an RTC should establish/synchronize System Clock epoch time through ESPressio Timing; Logging does not access RTC hardware itself.

## Router and Sink lifetime contract

`Logger::GetInstance()` is the central application entry point. It owns one `LogRouter`. The Router owns no log payloads and creates no Task or Thread. With no registered Sinks, `IsEnabled(...)` and routing return immediately.

`ILogSink::Accept(const LogRecordLease&)` receives the same logical record. `LogRecordLease::View()` is always available. `LogRecordLease::IsRetainable()` tells a Sink whether a durable owner exists, and `Retain()` acquires an intrusive shared reference without duplicating the payload. A Sink must never store references from a non-retainable lease after `Accept()` returns.

`ISharedLogRecord` / `SharedLogRecordBase` provide Event-style intrusive lifetime semantics for records deliberately created as durable objects. Multiple asynchronous Sinks can therefore share the same owned object rather than cloning it. A Sink is still encouraged to encode/materialise directly into its final bounded queue representation when that is cheaper than retaining the semantic record.

## Observable state changes

Logging topology changes are observable through `LoggingObservable` and `ILoggingObserver`. The initial observer surface is intentionally limited to `OnLogSinkRegistered` and `OnLogSinkUnregistered`. Log-message dispatch does **not** invoke Observable callbacks. This avoids a parallel routing mechanism and prevents accidental log→observer→log recursion.

All observer machinery is supplied by `ESPressio-Observable`; Logging does not implement its own callback registry.

## Buffer/retention policy vocabulary

The core defines platform-neutral policy vocabulary used by concrete Sinks: `LogBufferOverflowPolicy`, explicit bounded/unlimited byte/count capacities, `RollingLogPolicy`, and `LogSinkStatistics`. `DropOldest` is available for the intended diagnostic-stream default, but concrete Sinks choose and document their behaviour. Unlimited capacity is always explicit; a zero size never silently means unlimited.

A buffered Sink using `DropOldest` should accumulate its dropped-record count and emit a synthetic "dropped entries" record at the next successful opportunity rather than attempting to enqueue another record while already under pressure.

## Encryption boundary

Encryption of persisted/transmitted log content is a Sink concern. `ESPressio-Logging` intentionally exposes no raw-key API and stores no cryptographic key material. Future encrypted Sink compositions must consume opaque Security-owned key/provider handles from `ESPressio-Security`; raw keys must never be incidentally copied into Logging-owned ordinary memory. Transport encryption and log-payload encryption are separate concerns.

## Event integration

`ESPressio-Logging` has no dependency on `ESPressio-Event`, and Event has no dependency on Logging. Applications that need Event-originated logging should provide an application-level Event→Logging adapter. This preserves the dependency graph and avoids circular coupling.

## Example

```cpp
#include <ESPressio_Logging.hpp>

using namespace ESPressio::Logging;

inline constexpr auto NetworkCategory = LogCategory::Named("Network");

void ReportConnection(uint32_t channel) {
    const LogField fields[] = {
        {"channel", channel}
    };

    ESPRESSIO_LOG_INFO(
        NetworkCategory,
        "Connected",
        Fields(fields)
    );

    // Completely absent from builds where ESPRESSIO_LOGGING_ENABLE_DEBUG == 0.
    ESPRESSIO_LOG_DEBUG(NetworkCategory, "Verbose connection diagnostics");
}
```

No concrete Sink ships from this repository. That separation is deliberate.
