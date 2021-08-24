#pragma once

#include "holding_register.h"
#include "tcp_client.h"

#include <cstdint>
#include <string>
#include <vector>

namespace iv {

using RegisterRanges = std::vector<std::pair<uint16_t, uint8_t>>;

class ModbusRtuStreamer {
   public:
    ModbusRtuStreamer(uint8_t device_id,
                      const HostPort& hostport,
                      const RegisterRanges& reg_ranges);

    void stream_to_fd(int fd, unsigned freq = 5);

   private:
    HoldingRegisters read_holding_registers();
    HoldingRegisters read_holding_registers(uint16_t address, uint8_t count);

    void connect();

    TcpClient _cli;
    uint8_t _device_id;
    RegisterRanges _reg_ranges;
};

} // namespace iv
