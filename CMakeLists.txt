cmake_minimum_required(VERSION 3.10)
project(vrtu)

# for convenience
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Boost 
             REQUIRED COMPONENTS
             date_time
             system
             unit_test_framework)

find_package(#Qt5 
             REQUIRED COMPONENTS
             Widgets
)

add_library(vrtucore SHARED
    
    errornomoredata.cpp 
    errorunknowntype.cpp
    core/data.cpp

    external/json_wrapper.cpp
    external/style.cpp

    # headers (for proper display inside all IDEs)
    errordecode.hpp
    errornomoredata.hpp
    errorunknowntype.hpp

    core/bytestream.hpp
    core/data.hpp
    core/namedenum.hpp
    core/signal.hpp
    core/util.hpp
    core/watchdog.hpp

    external/json_wrapper.hpp
    external/style.hpp
)

target_include_directories(vrtucore PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})
                           
add_library(iec104 SHARED
    
    protocols/iec104/104enums.cpp
    protocols/iec104/apdu.cpp
    protocols/iec104/asdu.cpp
    protocols/iec104/connection.cpp
    protocols/iec104/connectionconfig.cpp
    protocols/iec104/data104.cpp
    protocols/iec104/infoaddress.cpp
    protocols/iec104/infoobjects.cpp
    protocols/iec104/server.cpp

    protocols/iec104/register_iec104.cpp

    # headers (for proper display inside all IDEs)
    protocols/iec104/104enums.hpp
    protocols/iec104/apdu.hpp
    protocols/iec104/asdu.hpp
    protocols/iec104/connection.hpp
    protocols/iec104/data104.hpp
    protocols/iec104/infoaddress.hpp
    protocols/iec104/infoobjects.hpp
    protocols/iec104/quality.hpp
    protocols/iec104/reason.hpp
    protocols/iec104/server.hpp
)


target_include_directories(iec104 PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})
                           
target_link_libraries(iec104
                      vrtucore
                      ${Boost_LIBRARIES}
                     )
                          
add_executable(vrtu
    rtutool.cpp 
)

target_include_directories(vrtu PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})

target_link_libraries(vrtu
                      vrtucore
                      iec104
                      ${Boost_LIBRARIES}
                     )

# on unix systems boost has a dependency to pthread
if (NOT WIN32)
    target_link_libraries(iec104 pthread)
endif()

# enable warnings
if(MSVC)
  target_compile_options(vrtu PRIVATE /W4)
else()
  target_compile_options(vrtu PRIVATE -Wall -Wextra -pedantic)
endif()

# enable CTest testing
enable_testing()

# Add a testing executable
add_executable(test_vrtu 
               tests/test_main.cpp
               tests/test_iec104.cpp
)

target_link_libraries(test_vrtu
                      iec104
                      Boost::unit_test_framework
)

target_include_directories(test_vrtu PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})


add_test(test_all test_vrtu)

