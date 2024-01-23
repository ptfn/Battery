cmake_minimum_required(VERSION 3.22.1)

project(battery)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "~/.local/bin/")

add_executable(battery src/battery.c)
