# cmake version to be used
cmake_minimum_required( VERSION 3.0 )

# project name
project( SkanCoin )

find_package(Threads)
find_package(OpenSSL)

if(OPENSSL_FOUND)
	include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -pedantic -Wextra")
find_package( Boost 1.52 COMPONENTS system thread REQUIRED )
include_directories(./libs/crow ./libs/picosha2 ./libs/easyEcc)

add_executable(skancoin src/Main.cpp)
target_link_libraries(skancoin ${CMAKE_THREAD_LIBS_INIT})
