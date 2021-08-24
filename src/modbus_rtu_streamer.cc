#include "modbus_rtu_streamer.h"

#include "modbus_rtu.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <signal.h>
#include <unistd.h>

#include <exception>
#include <thread>

static void
block_nonexit_signals()
{
    sigset_t sigset;
    sigfillset(&sigset);

    for (auto s : {SIGTERM, SIGQUIT, SIGHUP, SIGABRT, SIGINT})
        sigdelset(&sigset, s);

    if (sigprocmask(SIG_SETMASK, &sigset, nullptr))
        spdlog::warn("failed to set signalmask. {}", strerror(errno));
}

static std::string
datetime_utc()
{
    auto t = time(NULL);
    auto dt_tm = *gmtime(&t);
    char dt_buf[512];
    if (auto s = strftime(dt_buf, sizeof dt_buf, "%Y-%m-%dT%H:%M:%S.000Z", &dt_tm); !s)
        return {};
    else
        return std::string{dt_buf, s};
}

namespace iv {

ModbusRtuStreamer::ModbusRtuStreamer(uint8_t device_id,
                                     const HostPort& hostport,
                                     const RegisterRanges& reg_ranges)
    : _device_id(device_id), _cli(hostport), _reg_ranges(reg_ranges)
{
}

void
ModbusRtuStreamer::stream_to_fd(int fd, unsigned int freq)
{
    block_nonexit_signals();

    connect();

    auto write_to_fd = [&](const auto& event) {
        ::write(fd, event.data(), event.size());
        ::write(fd, "\n", 1);
        spdlog::info("event written");
    };

    while (1) {
        try {
            auto dt_str = datetime_utc();
            write_to_fd(HoldingRegisterSet::to_json_str(dt_str, read_holding_registers()));
        } catch (const std::exception& e) {
            spdlog::error(e.what());
            connect();
        }

        std::this_thread::sleep_for(std::chrono::seconds{freq});
    }
}

void
ModbusRtuStreamer::connect()
{
    while (1) {
        try {
            spdlog::info("establishing connection...");
            _cli.connect();
            spdlog::info("connection successful");
            return;
        } catch (const std::exception& e) {
            spdlog::error(e.what());
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
    }
}

HoldingRegisters
ModbusRtuStreamer::read_holding_registers()
{
    HoldingRegisters results;

    auto append_results = [&results](const auto& registers) {
        results.insert(results.end(), registers.begin(), registers.end());
    };

    for (const auto& reg_range : _reg_ranges)
        append_results(read_holding_registers(reg_range.first, reg_range.second));

    return results;
}

HoldingRegisters
ModbusRtuStreamer::read_holding_registers(uint16_t address, uint8_t count)
{
    iv::ModbusRtuQuery query{_device_id, ModBusFuncCode::holding_registers, address, count};

    _cli.send(query.data());

    iv::ModbusRtuResponse mb_resp;

    _cli.recv(mb_resp.hdr_buf());
    _cli.recv(mb_resp.data_buf());

    if (mb_resp.nregs() != count)
        throw std::domain_error(
            fmt::format("querying address={} count={} but received count={} registers",
                        address,
                        count,
                        mb_resp.nregs()));

    mb_resp.verify();

    return iv::HoldingRegisterSet::create(address, mb_resp.values());
}

} // namespace iv