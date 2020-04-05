cmake_minimum_required(VERSION 3.4)
project(Network)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
find_package(cpprestsdk REQUIRED NO_MODULE)



include_directories(.)

add_executable(Network
        crow_all.h
        evictor.hh
        fifo_evictor.cc
        fifo_evictor.hh
        lru_evictor.cc
        lru_evictor.hh
        cache_server.cc
        cache_lib.cc
        cache.hh cache_client.cc)
target_link_libraries(Network cpprest ${Boost_SYSTEM_LIBRARY} ${Boost_LIBRARIES} ssl crypto)
