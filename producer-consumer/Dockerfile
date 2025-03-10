FROM arbmind/cmake-gcc:3.29.7-14 AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    libev-dev \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git && \
    cd AMQP-CPP && \
    mkdir build && cd build && \
    cmake .. -DAMQP-CPP_LINUX_TCP=ON && \
    cmake --build . --target install

COPY . .

RUN mkdir build && cd build && cmake .. && make

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y \
    libev4 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the built executables from the builder stage
COPY --from=builder /app/build/Producer /app
COPY --from=builder /app/build/Consumer /app