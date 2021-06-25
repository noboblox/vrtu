cmake_minimum_required(VERSION 3.10)
project(vrtu)

# for convenience
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Boost 
             REQUIRED COMPONENTS
             date_time
             system)

# TODO: For now just compile everything into one application
# For real use it needs to be restructured into a library, but symbol exports are currently missing 

add_executable(rtutool 

               # sources (mandatory for compile)
               rtutool.cpp 
               
               errornomoredata.cpp 
               errorunknowntype.cpp

               core/data.cpp

               external/json_wrapper.cpp
               external/style.cpp

               protocols/iec104/104enums.cpp
               protocols/iec104/apdu.cpp
               protocols/iec104/asdu.cpp
               protocols/iec104/connection.cpp
               protocols/iec104/connectionconfig.cpp
               protocols/iec104/data104.cpp
               protocols/iec104/infoaddress.cpp
               protocols/iec104/infoobjects.cpp

               protocols/iec104/register_iec104.cpp

               # headers (for proper display inside all IDEs)
               errordecode.hpp
               errornomoredata.hpp
               errorunknowntype.hpp
               rtutool.hpp

               core/bytestream.hpp
               core/data.hpp
               core/namedenum.hpp
               core/signal.hpp
               core/util.hpp
               core/watchdog.hpp

               external/json_wrapper.hpp
               external/style.hpp

               protocols/iec104/104enums.hpp
               protocols/iec104/apdu.hpp
               protocols/iec104/asdu.hpp
               protocols/iec104/connection.hpp
               protocols/iec104/data104.hpp
               protocols/iec104/infoaddress.hpp
               protocols/iec104/infoobjects.hpp
               protocols/iec104/quality.hpp
               protocols/iec104/reason.hpp
)

target_include_directories(rtutool PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})

target_link_libraries(rtutool ${Boost_LIBRARIES})

# on unix systems boost has a dependency to pthread
if (NOT WIN32)
    target_link_libraries(rtutool pthread)
endif()

# enable warnings
if(MSVC)
  target_compile_options(rtutool PRIVATE /W4)
else()
  target_compile_options(rtutool PRIVATE -Wall -Wextra -pedantic)
endif()

