cmake_minimum_required(VERSION 3.16)
project(PBL2_TutorFinder VERSION 1.0 LANGUAGES CXX)
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

option(BUILD_QT_APP "Build Qt widgets application" ON)

set(CORE_SOURCES
    src/entities/Person.cpp
    src/entities/Student.cpp
    src/entities/Tutor.cpp
    src/entities/TutoringClass.cpp
    src/entities/Contract.cpp
    src/structures/Vector.cpp
    src/services/MatchingService.cpp
    src/repositories/FileManager.cpp
)

add_library(tutorfinder_core STATIC ${CORE_SOURCES})

target_include_directories(tutorfinder_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/entities
        ${CMAKE_SOURCE_DIR}/src/structures
        ${CMAKE_SOURCE_DIR}/src/services
        ${CMAKE_SOURCE_DIR}/src/repositories
)

add_executable(backend_tests tests/backend_test.cpp)
target_link_libraries(backend_tests PRIVATE tutorfinder_core)

if(BUILD_QT_APP)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if(Qt6_FOUND)
        add_executable(PBL2App src/main.cpp src/ui/LoginWindow.cpp src/ui/MainWindow.cpp)
        target_include_directories(PBL2App PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/ui)
        target_link_libraries(PBL2App PRIVATE tutorfinder_core Qt6::Widgets)
    else()
        message(WARNING "Qt6 not found. Skipping desktop app build.")
    endif()
endif()
