cmake_minimum_required(VERSION 3.16)
project(PBL2_TutorFinder VERSION 1.0 LANGUAGES CXX)
if(MSVC)
    add_compile_options(/utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_QT_APP "Build Qt widgets application" ON)

set(CORE_SOURCES
    src/utils/TextUtils.cpp
    src/utils/Sha256.cpp
    src/entities/Person.cpp
    src/entities/Student.cpp
    src/entities/Tutor.cpp
    src/entities/TutoringClass.cpp
    src/structures/Vector.cpp
    src/services/MatchingService.cpp
    src/services/ClassService.cpp
    src/services/ContractService.cpp
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
        ${CMAKE_SOURCE_DIR}/src/utils
)

# Kiểm thử backend: chạy bằng `ctest --test-dir build --output-on-failure`
enable_testing()
add_executable(backend_tests tests/backend_test.cpp)
target_link_libraries(backend_tests PRIVATE tutorfinder_core)
add_test(NAME backend_tests COMMAND backend_tests)

if(BUILD_QT_APP)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if(Qt6_FOUND)
        add_executable(PBL2App src/main.cpp src/ui/LoginWindow.cpp src/ui/MainWindow.cpp src/ui/ContractPage.cpp)
        set_target_properties(PBL2App PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
        target_include_directories(PBL2App PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/ui)
        target_link_libraries(PBL2App PRIVATE tutorfinder_core Qt6::Widgets)
    else()
        message(WARNING "Qt6 not found. Skipping desktop app build.")
    endif()
endif()
