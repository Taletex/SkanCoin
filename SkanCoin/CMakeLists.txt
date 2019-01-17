# cmake version to be used
cmake_minimum_required( VERSION 3.0 )

# project name
project( SkanCoin )

find_package(Threads)
include_directories(./libs/crow)
add_subdirectory(src/HttpServer)




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
