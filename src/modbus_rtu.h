#pragma once

#include <cstdint>
#include <span>

namespace iv {

enum ModBusFuncCode { holding_registers = 0x3 };

struct [[gnu::packed]] ModbusRtuQuery {
    ModbusRtuQuery(uint8_t device_id, uint8_t func, uint16_t address, uint8_t nregs) noexcept;

    std::span<const uint8_t> data() const;

   private:
    uint8_t _device_id{0};
    uint8_t _func{0};
    uint16_t _address{0};
    uint16_t _nregs{0};
    uint16_t _crc16{0};
};

struct [[gnu::packed]] ModbusRtuResponse {
    uint8_t device_id{0};
    uint8_t func{0};
    uint8_t byte_count{0};
    uint16_t data[126]{0};

    std::span<uint8_t> hdr_buf() { return {reinterpret_cast<uint8_t*>(this), 3}; }
    std::span<uint8_t> data_buf() { return {reinterpret_cast<uint8_t*>(data), response_size()}; }

    unsigned nregs() const noexcept { return byte_count / 2u; }

    bool crc16_valid() const noexcept;

    uint16_t crc16() const noexcept { return data[nregs()]; }

    uint16_t expected_crc16() const noexcept;

    uint8_t response_size() const noexcept
    {
        // +2 for crc checksum
        return byte_count + 2;
    }

    void verify();

    std::span<const uint16_t> values() const noexcept { return {data, nregs()}; }
};

} // namespace iv