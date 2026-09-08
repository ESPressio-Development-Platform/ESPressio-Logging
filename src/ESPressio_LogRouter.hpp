#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <shared_mutex>

#include <ESPressio_Synchronization.hpp>

#include "ESPressio_ILogSink.hpp"
#include "ESPressio_LoggingObservable.hpp"

#ifndef ESPRESSIO_LOGGING_MAX_SINKS
#define ESPRESSIO_LOGGING_MAX_SINKS 8
#endif

#ifndef ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES
#define ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES 16
#endif

namespace ESPressio::Logging {

/// <summary>Thread-safe, allocation-free routing table that fans one record out to zero or more Sinks.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - _sinks (std::array<ILogSink*, ESPRESSIO_LOGGING_MAX_SINKS>): 4 bytes [0 bytes dynamic allocation]
 * - _categoryOverrides (std::array<CategoryOverride, ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES>): ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES * (12 bytes) [0 bytes dynamic allocation]
 * - _sinkCount (std::atomic<std::size_t>): 4 bytes [0 bytes dynamic allocation]
 * - _globalMask (std::atomic<LogLevelMask>): 1 bytes [0 bytes dynamic allocation]
 * - _stateLock (System::Synchronization::ReadWriteLock): 20 bytes [_owned: owned object: 4 bytes; _fallback: _mutex: native synchronization state may allocate platform resources lazily]
 * - _observable (std::shared_ptr<LoggingObservable>): 8 bytes [shared control block (~12+ bytes; allocate_shared may co-locate object) + object 96 bytes; pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _mutex: native synchronization state may allocate platform resources lazily; pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _condition: native condition-variable state may allocate platform synchronization resources; pointee: ThreadSafeObservable: Observable: _registrations: Capacity * (12 bytes) element storage; pointee: ThreadSafeObservable: Observable: _bindings: Capacity * (12 bytes) element storage; pointee: ThreadSafeObservable: _mutex: _owned: owned object: 4 bytes; pointee: ThreadSafeObservable: _mutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; pointee: ThreadSafeObservable: _notificationMutex: _owned: owned object: 4 bytes; pointee: ThreadSafeObservable: _notificationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily]
 * Total Memory: 40 bytes known/aligned storage + ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES * (12 bytes) [_stateLock: _owned: owned object: 4 bytes; _stateLock: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _observable: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 96 bytes; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: enable_shared_from_this: embedded weak_ptr shares a control block when activated; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: shared control block (~12+ bytes; allocate_shared may co-locate object) + object 20 bytes; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _mutex: native synchronization state may allocate platform resources lazily; _observable: pointee: ThreadSafeObservable: Observable: IUntypedObservable: IObservable: _lifetimeControl: pointee: _condition: native condition-variable state may allocate platform synchronization resources; _observable: pointee: ThreadSafeObservable: Observable: _registrations: Capacity * (12 bytes) element storage; _observable: pointee: ThreadSafeObservable: Observable: _bindings: Capacity * (12 bytes) element storage; _observable: pointee: ThreadSafeObservable: _mutex: _owned: owned object: 4 bytes; _observable: pointee: ThreadSafeObservable: _mutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily; _observable: pointee: ThreadSafeObservable: _notificationMutex: _owned: owned object: 4 bytes; _observable: pointee: ThreadSafeObservable: _notificationMutex: _fallback: _mutex: native synchronization state may allocate platform resources lazily]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class LogRouter {
public:
    LogRouter() : _observable(CreateLoggingObservable()) {}

    /// <summary>Registers a non-owning Sink pointer. The Sink must outlive its registration.</summary>
    bool RegisterSink(ILogSink* sink) {
        if (sink == nullptr) return false;
        bool registered = false;
        {
            std::lock_guard<System::Synchronization::ReadWriteLock> lock(_stateLock);
            for (auto* existing : _sinks) {
                if (existing == sink) return false;
            }
            for (auto& slot : _sinks) {
                if (slot == nullptr) {
                    slot = sink;
                    _sinkCount.fetch_add(1, std::memory_order_release);
                    registered = true;
                    break;
                }
            }
        }
        if (registered) _observable->SinkRegistered(sink);
        return registered;
    }

    /// <summary>Removes a Sink and does not return until any concurrent routing operation has left Sink code.</summary>
    /// <remarks>A Sink must not synchronously register/unregister Logging topology from inside its own Accept call.</remarks>
    bool UnregisterSink(ILogSink* sink) {
        if (sink == nullptr) return false;
        bool removed = false;
        {
            std::lock_guard<System::Synchronization::ReadWriteLock> lock(_stateLock);
            for (auto& slot : _sinks) {
                if (slot == sink) {
                    slot = nullptr;
                    _sinkCount.fetch_sub(1, std::memory_order_release);
                    removed = true;
                    break;
                }
            }
        }
        if (removed) _observable->SinkUnregistered(sink);
        return removed;
    }

    std::size_t GetSinkCount() const noexcept {
        return _sinkCount.load(std::memory_order_acquire);
    }

    void SetGlobalLevelMask(LogLevelMask mask) noexcept {
        _globalMask.store(mask, std::memory_order_release);
    }

    LogLevelMask GetGlobalLevelMask() const noexcept {
        return _globalMask.load(std::memory_order_acquire);
    }

    /// <summary>Sets an exact per-category level mask without allocating a dynamic registry.</summary>
    bool SetCategoryLevelMask(LogCategoryId category, LogLevelMask mask) {
        std::lock_guard<System::Synchronization::ReadWriteLock> lock(_stateLock);
        for (auto& entry : _categoryOverrides) {
            if (entry.InUse && entry.Category == category) {
                entry.Mask = mask;
                return true;
            }
        }
        for (auto& entry : _categoryOverrides) {
            if (!entry.InUse) {
                entry.Category = category;
                entry.Mask = mask;
                entry.InUse = true;
                return true;
            }
        }
        return false;
    }

    bool ClearCategoryLevelMask(LogCategoryId category) {
        std::lock_guard<System::Synchronization::ReadWriteLock> lock(_stateLock);
        for (auto& entry : _categoryOverrides) {
            if (entry.InUse && entry.Category == category) {
                entry = CategoryOverride{};
                return true;
            }
        }
        return false;
    }

    /// <summary>Checks global/category/Sink eligibility before a caller constructs expensive log arguments.</summary>
    bool IsEnabled(LogLevel level, const LogCategory& category) const noexcept {
        if (_sinkCount.load(std::memory_order_acquire) == 0) return false;
        if (!ContainsLevel(_globalMask.load(std::memory_order_acquire), level)) return false;

        std::shared_lock<System::Synchronization::ReadWriteLock> lock(_stateLock);
        if (!ContainsLevel(GetCategoryMaskLocked(category.Id), level)) return false;
        for (auto* sink : _sinks) {
            if (sink != nullptr && sink->IsEnabled(level, category)) return true;
        }
        return false;
    }

    /// <summary>Routes the same lease to every eligible Sink without copying its message or metadata payload.</summary>
    void Dispatch(const LogRecordLease& record) noexcept {
        if (_sinkCount.load(std::memory_order_acquire) == 0) return;
        const auto& view = record.View();
        if (!ContainsLevel(_globalMask.load(std::memory_order_acquire), view.Level)) return;

        std::shared_lock<System::Synchronization::ReadWriteLock> lock(_stateLock);
        if (!ContainsLevel(GetCategoryMaskLocked(view.Category.Id), view.Level)) return;
        for (auto* sink : _sinks) {
            if (sink == nullptr || !sink->IsEnabled(view.Level, view.Category)) continue;
            sink->Accept(record);
        }
    }

    std::shared_ptr<LoggingObservable> GetObservable() const noexcept { return _observable; }

private:
        /**
     * ESPressio Memory Audit
     * Members:
     * - Category (LogCategoryId): 8 bytes [0 bytes dynamic allocation]
     * - Mask (LogLevelMask): 1 bytes [0 bytes dynamic allocation]
     * - InUse (bool): 1 bytes [0 bytes dynamic allocation]
     * Total Memory: 12 bytes [0 bytes dynamic allocation]
     * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
     * End ESPressio Memory Audit
     */
struct CategoryOverride {
        LogCategoryId Category = 0;
        LogLevelMask Mask = AllLogLevels;
        bool InUse = false;
    };

    LogLevelMask GetCategoryMaskLocked(LogCategoryId category) const noexcept {
        for (const auto& entry : _categoryOverrides) {
            if (entry.InUse && entry.Category == category) return entry.Mask;
        }
        return AllLogLevels;
    }

    std::array<ILogSink*, ESPRESSIO_LOGGING_MAX_SINKS> _sinks{};
    std::array<CategoryOverride, ESPRESSIO_LOGGING_MAX_CATEGORY_OVERRIDES> _categoryOverrides{};
    std::atomic<std::size_t> _sinkCount{0};
    std::atomic<LogLevelMask> _globalMask{AllLogLevels};
    mutable System::Synchronization::ReadWriteLock _stateLock;
    std::shared_ptr<LoggingObservable> _observable;
};

} // namespace ESPressio::Logging
