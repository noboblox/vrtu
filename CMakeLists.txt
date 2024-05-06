cmake_minimum_required(VERSION 3.10)
project(vrtu)

# for convenience
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(Boost_USE_STATIC_LIBS   ON)

find_package(Boost 1.83.0
             REQUIRED COMPONENTS
             cobalt
             date_time
             system
             unit_test_framework)

find_package(Qt6 
             REQUIRED COMPONENTS
             Widgets LinguistTools Network
)

qt_standard_project_setup()

add_library(vrtucore STATIC
    core/bytestream.hpp
    core/namedenum.hpp
    core/signal.hpp
    core/util.hpp
)

target_include_directories(vrtucore PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})
                           
add_library(iec104 STATIC
    
    protocols/iec104/104enums.cpp
    protocols/iec104/apdu.cpp
    protocols/iec104/asdu.cpp
    protocols/iec104/link.cpp
    protocols/iec104/connectionconfig.cpp
    protocols/iec104/infoaddress.cpp
    protocols/iec104/infoobjects.cpp
    protocols/iec104/server.cpp
    protocols/iec104/sequence.cpp
    protocols/iec104/register_iec104.cpp

    # headers (for proper display inside all IDEs)
    protocols/iec104/104enums.hpp
    protocols/iec104/apdu.hpp
    protocols/iec104/asdu.hpp
    protocols/iec104/link.hpp
    protocols/iec104/infoaddress.hpp
    protocols/iec104/infoobjects.hpp
    protocols/iec104/quality.hpp
    protocols/iec104/reason.hpp
    protocols/iec104/sequence.hpp
    protocols/iec104/server.hpp
    protocols/iec104/servicetype.hpp
)


target_include_directories(iec104 PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})
                           
target_link_libraries(iec104
                      vrtucore
                      ${Boost_LIBRARIES}
                     )
                          
qt_add_executable(vrtu
  app/main.cpp
  app/mainwindow.cpp
  app/mainwindow.h
  app/mainwindow.ui
  app/data.qrc
)

qt_add_translations(vrtu
    TS_FILES app/app_de_DE.ts)

set_target_properties(vrtu PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

target_include_directories(vrtu PRIVATE
                           ${PROJECT_SOURCE_DIR}                           
                           ${Boost_INCLUDE_DIRS}
                           ${Qt6Widgets_INCLUDE_DIRS}
                           ${Qt6Network_INCLUDE_DIRS}
)

qt_add_resources(vrtu app/data.qrc)

target_link_libraries(vrtu PRIVATE
                      vrtucore
                      iec104
                      ${Boost_LIBRARIES}
                      Qt6::Widgets
                      Qt6::Network
                     )

# on unix systems boost has a dependency to pthread
if (NOT WIN32)
    target_link_libraries(iec104 pthread)
endif()

#enable warnings
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
               tests/test_encoding.cpp
               tests/test_sequence.cpp
               tests/test_bytestream.cpp
)


target_link_libraries(test_vrtu
                      iec104
                      ${Boost_LIBRARIES}
)

target_include_directories(test_vrtu PRIVATE
                           ${PROJECT_SOURCE_DIR} 
                           ${Boost_INCLUDE_DIRS})


add_test(test_all test_vrtu)

