#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace iv {

struct HoldingRegister {
    uint16_t address{0};
    std::string_view name;
    std::string_view unit;
    std::variant<int16_t, float> value{static_cast<float>(0.0)};

    std::string to_string() const;
};

using HoldingRegisters = std::vector<HoldingRegister>;

class HoldingRegisterSet {
   public:
    static std::optional<HoldingRegister> create(uint16_t address, int16_t value);
    static HoldingRegisters create(uint16_t starting_address, std::span<const uint16_t> values);

    static std::string to_json_str(const std::string& timestamp, const HoldingRegisters& registers);
};

} // namespace iv