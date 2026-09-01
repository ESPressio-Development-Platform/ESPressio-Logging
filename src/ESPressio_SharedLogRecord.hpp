#pragma once

#include <atomic>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#include <ESPressio_Memory.hpp>

#include "ESPressio_LogRecord.hpp"

namespace ESPressio::Logging {

/// <summary>Intrusively shareable durable log-record contract for data that may cross temporal boundaries.</summary>
class ISharedLogRecord {
public:
    virtual ~ISharedLogRecord() = default;
    virtual void __ref() noexcept = 0;
    virtual void __unref() noexcept = 0;
    virtual const LogRecordView& GetLogRecordView() const noexcept = 0;
};

/// <summary>RAII intrusive reference to one durable log record; copying the handle shares ownership without copying payload data.</summary>
class SharedLogRecordHandle {
public:
    SharedLogRecordHandle() noexcept = default;

    explicit SharedLogRecordHandle(ISharedLogRecord* record) noexcept : _record(record) {
        if (_record != nullptr) _record->__ref();
    }

    SharedLogRecordHandle(const SharedLogRecordHandle& other) noexcept
        : SharedLogRecordHandle(other._record) {}

    SharedLogRecordHandle(SharedLogRecordHandle&& other) noexcept : _record(other._record) {
        other._record = nullptr;
    }

    SharedLogRecordHandle& operator=(SharedLogRecordHandle other) noexcept {
        Swap(other);
        return *this;
    }

    ~SharedLogRecordHandle() {
        if (_record != nullptr) _record->__unref();
    }

    void Swap(SharedLogRecordHandle& other) noexcept { std::swap(_record, other._record); }
    ISharedLogRecord* Get() const noexcept { return _record; }
    explicit operator bool() const noexcept { return _record != nullptr; }

private:
    ISharedLogRecord* _record = nullptr;
};

/// <summary>CRTP base providing Event-style intrusive lifetime with ESPressio-System policy-aware deallocation.</summary>
/// <remarks>Concrete records deriving from this type must be created with MakeSharedLogRecord so allocation and destruction use the same System memory provider.</remarks>
template<
    typename TDerived,
    System::Memory::MemoryPolicy TMemoryPolicy = System::Memory::MemoryPolicy::ExternalPreferred
>
class SharedLogRecord : public ISharedLogRecord {
public:
    void __ref() noexcept final {
        _referenceCount.fetch_add(1, std::memory_order_relaxed);
    }

    void __unref() noexcept final {
        uint32_t current = _referenceCount.load(std::memory_order_acquire);
        while (current != 0) {
            if (_referenceCount.compare_exchange_weak(
                    current,
                    current - 1,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire)) {
                if (current == 1) Destroy();
                return;
            }
        }
    }

protected:
    SharedLogRecord() = default;
    ~SharedLogRecord() override = default;

private:
    template<typename TRecord, System::Memory::MemoryPolicy TPolicy, typename... TArgs>
    friend SharedLogRecordHandle MakeSharedLogRecord(TArgs&&... args);

    void BindProvider(System::Memory::IMemoryProvider& provider) noexcept {
        _provider = &provider;
    }

    void Destroy() noexcept {
        auto* provider = _provider;
        auto* object = static_cast<TDerived*>(this);
        object->~TDerived();
        if (provider != nullptr) {
            provider->Deallocate(object, sizeof(TDerived), alignof(TDerived), TMemoryPolicy);
        }
    }

    std::atomic<uint32_t> _referenceCount{0};
    System::Memory::IMemoryProvider* _provider = nullptr;
};

/// <summary>Creates one durable intrusive record through the active ESPressio System memory provider.</summary>
template<
    typename TRecord,
    System::Memory::MemoryPolicy TMemoryPolicy = System::Memory::MemoryPolicy::ExternalPreferred,
    typename... TArgs
>
SharedLogRecordHandle MakeSharedLogRecord(TArgs&&... args) {
    static_assert(
        std::is_base_of_v<SharedLogRecord<TRecord, TMemoryPolicy>, TRecord>,
        "TRecord must derive from SharedLogRecord<TRecord, TMemoryPolicy>"
    );

    auto& provider = System::Memory::GetProvider();
    void* storage = provider.Allocate(sizeof(TRecord), alignof(TRecord), TMemoryPolicy);
    try {
        auto* record = ::new (storage) TRecord(std::forward<TArgs>(args)...);
        record->BindProvider(provider);
        return SharedLogRecordHandle(record);
    } catch (...) {
        provider.Deallocate(storage, sizeof(TRecord), alignof(TRecord), TMemoryPolicy);
        throw;
    }
}

/// <summary>Dispatch-time record wrapper. Its view is always borrowed; Retain is possible only when a durable owner was supplied.</summary>
class LogRecordLease {
public:
    explicit LogRecordLease(const LogRecordView& record) noexcept : _view(&record) {}
    explicit LogRecordLease(ISharedLogRecord& record) noexcept
        : _view(&record.GetLogRecordView()), _owner(&record) {}

    const LogRecordView& View() const noexcept { return *_view; }
    bool IsRetainable() const noexcept { return _owner != nullptr; }
    SharedLogRecordHandle Retain() const noexcept { return SharedLogRecordHandle(_owner); }

private:
    const LogRecordView* _view;
    ISharedLogRecord* _owner = nullptr;
};

} // namespace ESPressio::Logging
