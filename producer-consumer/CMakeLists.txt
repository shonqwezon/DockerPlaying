cmake_minimum_required(VERSION 3.29)
project(ProducerConsumer)

set(CMAKE_CXX_STANDARD 17)

set(AMQP_ROOT ${CMAKE_SOURCE_DIR}/AMQP-CPP)
include_directories(${AMQP_ROOT}/include)
link_directories(${AMQP_ROOT}/build/bin)

add_executable(Producer
        producer.cpp
)

add_executable(Consumer
        consumer.cpp
)

target_link_libraries(Producer PRIVATE amqpcpp ev)
target_link_libraries(Consumer PRIVATE amqpcpp ev)