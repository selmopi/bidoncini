# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)


set(EXTRA_COMPONENT_DIRS components/ra01s)

set(ENV{WOLFSSL_ROOT}  "/Users/smndrs/Documents/EngineeringCS/IoT/bidoncini/project/wolfssl-5.7.0")


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(bidoncini_proj)
