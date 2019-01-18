# cmake version to be used
cmake_minimum_required( VERSION 3.0 )

# project name
project( SkanCoin )


find_package(Tcmalloc)
find_package(Threads)
find_package(OpenSSL)

if(OPENSSL_FOUND)
	include_directories(${OPENSSL_INCLUDE_DIR})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1y -pedantic -Wextra")
find_package( Boost 1.52 COMPONENTS system thread REQUIRED )
include_directories(./libs/crow)

add_executable(skancoin src/Main.cpp)
target_link_libraries(skancoin ${CMAKE_THREAD_LIBS_INIT})




#ESEMPIO VECCHIO
# target nome dell'eseguibile e files sorgente da compilare
#add_executable( binary ./src/HttpServer/HttpServer.cpp)

# external libs, indicare dove le librerie di terze parti incdicate sono collocate
#add_subdirectory( ./libs/crow ) #Dove trovare il CMakeList della libreria esterna
#target_link_libraries( binary crow )

# ALTERNATIVA PER INCLUDERE LIBRERIE PRECOMPILATE
#add_library(crow STATIC IMPORTED) #libreria già compilata
#set_property(TARGET crow PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/crow/crow.h)
#target_link_libraries( binary crow )
