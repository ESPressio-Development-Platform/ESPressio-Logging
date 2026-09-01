#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <variant>

namespace ESPressio::Logging {

using LogFieldValue = std::variant<
    bool,
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float,
    double,
    std::string_view
>;

struct LogField {
    std::string_view Name{};
    LogFieldValue Value{};
};

struct LogFieldView {
    const LogField* Data = nullptr;
    std::size_t Count = 0;

    constexpr const LogField* begin() const noexcept { return Data; }
    constexpr const LogField* end() const noexcept { return Data == nullptr ? nullptr : Data + Count; }
    constexpr bool Empty() const noexcept { return Count == 0; }
};

template<std::size_t N>
constexpr LogFieldView Fields(const LogField (&fields)[N]) noexcept {
    return LogFieldView{fields, N};
}

} // namespace ESPressio::Logging
