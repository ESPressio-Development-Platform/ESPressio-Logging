#include <Arduino.h>
#include <ESPressio_Logging.hpp>

using namespace ESPressio::Logging;

inline constexpr auto ExampleCategory = LogCategory::Named("Example");

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 4 bytes [0 bytes dynamic allocation]
 * Members:
 * - Accepted (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class ExampleSink final : public ILogSink {
public:
    bool IsEnabled(LogLevel, const LogCategory&) const noexcept override {
        return true;
    }

    void Accept(const LogRecordLease& record) noexcept override {
        // This local example Sink deliberately performs no hardware I/O.
        // Concrete output Sinks belong in their owning ESPressio context libraries.
        (void)record;
        ++Accepted;
    }

    uint32_t Accepted = 0;
};

ExampleSink sink;

void setup() {
    auto& logger = Logger::GetInstance();
    logger.Router().RegisterSink(&sink);

    const LogField fields[] = {
        {"attempt", uint32_t{1}},
        {"ready", true}
    };

    ESPRESSIO_LOG_INFO(ExampleCategory, "Logging is ready", Fields(fields));
    ESPRESSIO_LOG_DEBUG(ExampleCategory, "Absent unless debug logging is compiled in");
}

void loop() {}
