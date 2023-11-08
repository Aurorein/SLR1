cmake_minimum_required(VERSION 3.16)
project(SLR1)
set(CMAKE_BUILD_TYPE "Debug")

add_executable(SLR1 main.cpp lexer.h lexer.cpp parser.cpp parser.h constants.h printer.h)