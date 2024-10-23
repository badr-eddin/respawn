cmake_minimum_required(VERSION 3.28)

# VERSION: Y.M.D
project(cppbuster VERSION 24.11.23)

message("I am using linux and g++ compiler, asio lib is linked automatically")

add_executable(cppbuster main.cpp)
