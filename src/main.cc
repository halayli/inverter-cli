#include "modbus_rtu_streamer.h"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include <stdio.h>

static void
setup_logging(const auto& logpath)
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_st>());
    sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_st>(logpath, 0, 0));

    auto logger = std::make_shared<spdlog::logger>("inverter-cli", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::info);
}

int
main(int argc, const char** argv)
{
    CLI::App app("inverter-cli");

    app.add_option("-l,logpath", "")->default_str("/tmp/inverter-cli.log");
    app.add_option("-d,device-id", "")->default_str("4");
    app.add_option("-a,host", "host used to send rtu queries over tcp")->required(true);
    app.add_option("-p,port", "tcp port")->default_str("8899");

    CLI11_PARSE(app, argc, argv)

    setup_logging(app["logpath"]->as<std::string>());

    // can be read from cli if this ends up used in other projects
    static const iv::RegisterRanges reg_ranges{{{15201, 21}, {20101, 17}, {25201, 79}}};

    iv::ModbusRtuStreamer streamer(app["device-id"]->as<uint8_t>(),
                                   {app["host"]->as<std::string>(), app["port"]->as<uint16_t>()},
                                   reg_ranges);

    streamer.stream_to_fd(fileno(stdout));

    return 0;
}
