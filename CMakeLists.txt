cmake_minimum_required(VERSION 3.10)
project(tb-recorder)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)


add_executable(recorder main.cpp requester.cpp audioManager.cpp json.cpp songManager.cpp http.cpp timer.cpp)
target_link_libraries(recorder OpenSSL::SSL OpenSSL::Crypto)