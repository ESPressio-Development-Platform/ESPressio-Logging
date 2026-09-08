#include <cassert>
#include <cstdint>
#include <string_view>
#include <ESPressio_Logging.hpp>
using namespace ESPressio::Logging;

namespace {
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - Mask (LogLevelMask): 1 bytes [0 bytes dynamic allocation]
 * - Accepted (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - ExpectedMessagePointer (char*): 4 bytes [0 bytes dynamic allocation]
 * - ExpectedMetadataPointer (LogField*): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 20 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class TestSink final : public ILogSink {
public:
    LogLevelMask Mask = AllLogLevels;
    std::size_t Accepted = 0;
    const char* ExpectedMessagePointer = nullptr;
    const LogField* ExpectedMetadataPointer = nullptr;
    bool IsEnabled(LogLevel level, const LogCategory&) const noexcept override { return ContainsLevel(Mask, level); }
    void Accept(const LogRecordLease& record) noexcept override {
        ++Accepted;
        const auto& view = record.View();
        if (ExpectedMessagePointer) assert(view.Message.data() == ExpectedMessagePointer);
        if (ExpectedMetadataPointer) assert(view.Metadata.Data == ExpectedMetadataPointer);
    }
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - Registered (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - Unregistered (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 12 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class TestLoggingObserver final : public ILoggingObserver {
public:
    std::size_t Registered = 0;
    std::size_t Unregistered = 0;
    void OnLogSinkRegistered(ILogSink*) override { ++Registered; }
    void OnLogSinkUnregistered(ILogSink*) override { ++Unregistered; }
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 12 bytes [0 bytes dynamic allocation]
 * Members:
 * - _view (LogRecordView): 56 bytes [0 bytes dynamic allocation]
 * Total Memory: 68 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class DurableRecord final : public SharedLogRecord<DurableRecord> {
public:
    DurableRecord() {
        _view.Timestamp = LogTimestamp{1, 2, ESPressio::Timing::ClockSynchronizationState::Unsynchronized};
        _view.Level = LogLevel::Error;
        _view.Category = Categories::Application;
        _view.Message = "durable";
    }
    const LogRecordView& GetLogRecordView() const noexcept override { return _view; }
private:
    LogRecordView _view{};
};
}

int main() {
    static_assert(ContainsLevel(AtOrAbove(LogLevel::Warn), LogLevel::Warn));
    static_assert(ContainsLevel(AtOrAbove(LogLevel::Warn), LogLevel::Fatal));
    static_assert(!ContainsLevel(AtOrAbove(LogLevel::Warn), LogLevel::Info));
    constexpr auto Category = LogCategory::Named("Laser-Trigger");
    static_assert(Category.Id == HashLogCategory("Laser-Trigger"));

    LogRouter router;
    assert(router.GetSinkCount() == 0);
    assert(!router.IsEnabled(LogLevel::Info, Category));

    TestLoggingObserver observer;
    auto observerHandle = router.GetObservable()->RegisterObserverAs<ILoggingObserver>(&observer);
    TestSink sink;
    assert(router.RegisterSink(&sink));
    assert(!router.RegisterSink(&sink));
    assert(observer.Registered == 1);

    router.SetGlobalLevelMask(LogLevelBit(LogLevel::Info) | LogLevelBit(LogLevel::Error) | LogLevelBit(LogLevel::Fatal));
    assert(router.IsEnabled(LogLevel::Info, Category));
    assert(!router.IsEnabled(LogLevel::Warn, Category));
    assert(router.SetCategoryLevelMask(Category.Id, LogLevelBit(LogLevel::Error) | LogLevelBit(LogLevel::Fatal)));
    assert(!router.IsEnabled(LogLevel::Info, Category));
    assert(router.IsEnabled(LogLevel::Error, Category));

    const char message[] = "zero-copy";
    const LogField metadata[] = {{"channel", uint32_t{6}}, {"armed", true}};
    const LogRecordView view{LogTimestamp{10, 20, ESPressio::Timing::ClockSynchronizationState::Unsynchronized}, LogLevel::Error, Category, std::string_view(message), Fields(metadata)};
    const LogRecordLease borrowed(view);
    assert(!borrowed.IsRetainable());
    assert(!borrowed.Retain());
    sink.ExpectedMessagePointer = message;
    sink.ExpectedMetadataPointer = metadata;
    router.Dispatch(borrowed);
    assert(sink.Accepted == 1);

    auto rootOwner = MakeSharedLogRecord<DurableRecord>();
    ISharedLogRecord* durable = rootOwner.Get();
    assert(durable != nullptr);
    const LogRecordLease shared(*durable);
    assert(shared.IsRetainable());
    auto retained = shared.Retain();
    assert(retained.Get() == durable);

    int debugArgumentEvaluations = 0;
    ESPRESSIO_LOG_DEBUG(Category, (++debugArgumentEvaluations, std::string_view{"disabled"}));
#if !ESPRESSIO_LOGGING_ENABLE_DEBUG
    assert(debugArgumentEvaluations == 0);
#endif

    assert(router.UnregisterSink(&sink));
    assert(observer.Unregistered == 1);
    observerHandle->Unregister();
    return 0;
}
