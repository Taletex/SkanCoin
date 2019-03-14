# CMAKE VERSION TO BE USED
cmake_minimum_required( VERSION 3.0 )


# PROJECT NAME
project( SkanCoin )


# PACKAGES AND LIBRARIES #
find_package(Threads)
find_package(OpenSSL)

if(OPENSSL_FOUND)
	include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -pedantic -Wextra")
find_package( Boost 1.52 COMPONENTS system thread REQUIRED )
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
include_directories(./libs/crow ./libs/picosha2 ./libs/easyEcc ./libs/rapidjson ./libs/easywsclient)

add_executable(skancoin src/Main.cpp src/Blockchain/Blockchain.cpp src/Blockchain/Block.cpp src/Blockchain/Transactions.cpp src/Blockchain/TransactionComponents.cpp src/Blockchain/TransactionPool.cpp src/Blockchain/Wallet.cpp src/HttpServer/HttpServer.cpp src/P2P/Peer.cpp libs/easywsclient/easywsclient.cpp libs/easyEcc/ecc.c)
target_link_libraries(skancoin ${CMAKE_THREAD_LIBS_INIT})


# PASSING VARIABLES TO C++ SOURCE FILES
set (PROJECT_NAME "SkanCoin")
set (VERSION_NUMBER 1.0)
option(DEBUG_FLAG "Enable debug info on stdout" 0)	# set to 1 to enable debug info on stdout, 0 to disabled it

configure_file (																		# configure a header file to pass some of the CMake settings to the source code
  "${PROJECT_SOURCE_DIR}/config.hpp.in"
  "${PROJECT_BINARY_DIR}/config.hpp"
  )

include_directories("${PROJECT_BINARY_DIR}")				# add the binary tree to the search path for include files so that we will find debug.hpp
