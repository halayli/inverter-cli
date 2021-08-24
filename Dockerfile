FROM ubuntu:impish-20210722 as build

WORKDIR /build

RUN DEBIAN_FRONTEND=noninteractive apt update && apt-get install -y git

RUN git clone https://github.com/halayli/inverter-cli src
RUN cd src && DEBIAN_FRONTEND=noninteractive ./build.sh

FROM ubuntu:impish-20210722 as app
RUN DEBIAN_FRONTEND=noninteractive apt update && apt-get install -y  --no-install-recommends  libc++-12-dev libc++abi1-12 libc++abi-12-dev netcat
COPY --from=build /usr/local/bin/inverter-cli /usr/local/bin/inverter-cli

ENV INVERTER_HOST=192.168.1.110
ENV INVERTER_PORT=8899
ENV BROADCAST_ADDRS=192.168.1.255
ENV BROADCAST_PORT=12000

ENTRYPOINT inverter-cli -a "${INVERTER_HOST}" -p "${INVERTER_PORT}" | nc -u -b "${BROADCAST_ADDRS}" "${BROADCAST_PORT}"