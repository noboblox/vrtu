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

