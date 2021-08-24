# inverter-cli

This utility sends Modbus RTU requests over tcp to read pv1800 inverter's holding registers and writes them to stdout as
a json message.

run `inverter-cli --help` for options.

# Running in Docker

The Dockerfile provided builds and creates an image that runs inverter-cli and broadcast the messages over the network using `nc`.

`inverter-cli -a "${INVERTER_HOST}" -p "${INVERTER_PORT}" | nc -u -b "${BROADCAST_ADDRS}" "${BROADCAST_PORT}"`
