#include "modbus_rtu.h"

#include <fmt/format.h>

#include <exception>

namespace iv {

static uint16_t
calc_crc16(std::span<const uint8_t> data)
{
    uint16_t crc = 0xFFFF;

    for (auto b : data) {
        crc ^= b;
        for (auto i = 0; i < 8; ++i) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = (crc >> 1);
        }
    }
    return crc;
}

ModbusRtuQuery::ModbusRtuQuery(uint8_t device_id,
                               uint8_t func,
                               uint16_t address,
                               uint8_t nregs) noexcept
    : _device_id(device_id),
      _func(func),
      _address(__builtin_bswap16(address)),
      _nregs(__builtin_bswap16(nregs))
{
    _crc16 = calc_crc16(data().subspan(0, sizeof(*this) - 2));
}

std::span<const uint8_t>
ModbusRtuQuery::data() const
{
    return {reinterpret_cast<const uint8_t*>(this), sizeof(*this)};
}

bool
ModbusRtuResponse::crc16_valid() const noexcept
{
    return crc16() == expected_crc16();
}

uint16_t
ModbusRtuResponse::expected_crc16() const noexcept
{
    return calc_crc16({reinterpret_cast<const uint8_t*>(this), byte_count + 3u});
}

void
ModbusRtuResponse::verify()
{
    if (crc16_valid())
        return;

    throw std::domain_error(
        fmt::format("invalid modbus rtu checksum response_crc({}) != expected_crc({})",
                    crc16(),
                    expected_crc16()));
}

} // namespace iv