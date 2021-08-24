#include "holding_register.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include <unordered_set>

namespace iv {

std::string
HoldingRegister::to_string() const
{
    if (std::holds_alternative<float>(value))
        return fmt::format("{} {}: {} {}", address, name, std::get<float>(value), unit);

    return fmt::format("{} {}: {} {}", address, name, std::get<int16_t>(value), unit);
}

class HoldingRegisterDef {
   public:
    HoldingRegisterDef() = default;
    HoldingRegisterDef(uint16_t address,
                       unsigned coefficient,
                       const std::string& name,
                       const std::string& unit)
        : _address(address), _coefficient(coefficient), _name(name), _unit(unit)
    {
    }

    HoldingRegisterDef(unsigned address, unsigned coefficient, const std::string& name)
        : HoldingRegisterDef(address, coefficient, name, "")
    {
    }

    explicit HoldingRegisterDef(unsigned address) noexcept : _address(address) {}

    HoldingRegister create_register(int16_t value) const
    {
        decltype(HoldingRegister::value) tmp{value};
        if (_coefficient != 1)
            tmp = value / static_cast<float>(_coefficient);

        return {.address = _address, .name = _name, .unit = _unit, .value = tmp};
    }

    auto address() const noexcept { return _address; }

    bool operator==(const HoldingRegisterDef& rhs) const noexcept
    {
        return _address == rhs._address;
    }

    std::size_t operator()(const HoldingRegisterDef& reg) const noexcept
    {
        return std::hash<uint16_t>{}(reg.address());
    }

   private:
    uint16_t _address{0};
    unsigned _coefficient{1};
    std::string _name;
    std::string _unit;
};

// we can read the defs from json file if this ends up used by others
static const std::unordered_set<HoldingRegisterDef, HoldingRegisterDef> g_pv1800_registers{
#include "pv1800.inc"
};

std::optional<HoldingRegister>
HoldingRegisterSet::create(uint16_t address, int16_t value)
{
    HoldingRegisterDef reg_key{address};
    const auto& reg_it = g_pv1800_registers.find(reg_key);
    if (reg_it == g_pv1800_registers.end())
        return {};

    return reg_it->create_register(value);
}
HoldingRegisters
HoldingRegisterSet::create(uint16_t starting_address, std::span<const uint16_t> values)
{
    HoldingRegisters res;
    res.reserve(values.size());

    for (auto value : values)
        if (auto reg = create(starting_address++, __builtin_bswap16(value)); reg)
            res.push_back(*reg);

    return res;
}

std::string
HoldingRegisterSet::to_json_str(const std::string& datetime_utc, const HoldingRegisters& registers)
{
    using json = nlohmann::
        basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, float>;

    json res;

    res["datetimeutc"] = datetime_utc;

    for (const auto& r : registers)
        if (std::holds_alternative<float>(r.value))
            res[std::string{r.name}] = std::get<float>(r.value);
        else
            res[std::string{r.name}] = std::get<int16_t>(r.value);

    return res.dump();
}

} // namespace iv
