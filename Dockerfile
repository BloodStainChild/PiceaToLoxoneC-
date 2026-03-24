FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends     ca-certificates     curl     gcc     g++     git     libcurl4-openssl-dev     libjsoncpp-dev     make     pkg-config     && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY PiceaToLoxoneC++ /build/PiceaToLoxoneC++

RUN git clone --depth 1 https://github.com/civetweb/civetweb.git /build/civetweb     && cd /build/PiceaToLoxoneC++     && make clean     && make all     && mkdir -p /app     && cp /build/PiceaToLoxoneC++/PiceaToLoxone /app/PiceaToLoxone

WORKDIR /app

COPY config/config.cfg /app/config.cfg
RUN mkdir -p /app/Log

ENV LOG_DIR=/app/Log
ENV LOG_DEBUG=0

CMD ["/app/PiceaToLoxone"]
