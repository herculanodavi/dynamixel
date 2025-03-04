cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dynamixel_test)
target_sources(
    app PRIVATE src/main.c src/parser.c src/serializer_protocol2.c
)