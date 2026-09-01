# Changelog

All notable changes to ESPressio-Logging are documented in this file.

## 1.0.0

- Established the platform-neutral Logging domain and singleton `Logger` entry point.
- Added mask-based `Trace`, `Debug`, `Info`, `Warn`, `Error`, and `Fatal` levels with independent compile-time elimination macros.
- Added open, application-extensible structured log categories and typed zero-copy metadata views.
- Added dual monotonic/System Clock timestamps with System Clock synchronization state.
- Added zero-copy borrowed `LogRecordView` routing and Event-style intrusive shared-record lifetime support for temporal boundaries.
- Added allocation-free fixed-capacity `LogRouter` topology and category overrides with ESPressio System synchronization.
- Added `ILogSink` and the explicit borrowed-versus-retainable Sink lifetime contract.
- Added ESPressio-Observable-backed Sink registration/unregistration notifications only; per-record Observer notifications are intentionally prohibited by design.
- Added bounded/unlimited capacity, rolling-log, buffer-overflow, and Sink-statistics policy vocabulary.
- Established that concrete Sinks live in their owning domain libraries and that Logging never owns raw encryption key material.
