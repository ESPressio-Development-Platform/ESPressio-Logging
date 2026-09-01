#include <Arduino.h>
#include <ESPressio_Logging.hpp>

using namespace ESPressio::Logging;

inline constexpr auto ExampleCategory = LogCategory::Named("Example");

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
