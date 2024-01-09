cmake_minimum_required(VERSION 3.22.1)
project(fuzzer)

#
# Shared Memory
#

add_subdirectory(rotm_shared_memory)

#
# Radamsa
#

add_library(radamsa STATIC IMPORTED)
set_target_properties(radamsa PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/radamsa/c
        IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/radamsa/lib/libradamsa.a
)

#
# Fuzzer
#

add_executable(fuzzer fuzzer.cc)
target_link_libraries(fuzzer shm_ipc radamsa)
