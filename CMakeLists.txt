cmake_minimum_required(VERSION 3.20)
project(LabGP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LABGP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

add_library(labgp_domain
    src/domain/ResearchProjectStore.cpp
    src/domain/Scoring.cpp
)

target_include_directories(labgp_domain
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(labgp_domain PRIVATE /W4)
else()
    target_compile_options(labgp_domain PRIVATE -Wall -Wextra -Wpedantic)
    if(LABGP_WARNINGS_AS_ERRORS)
        target_compile_options(labgp_domain PRIVATE -Werror)
    endif()
endif()

add_executable(LabGP
    src/main.cpp
    src/app/AppRuntime.cpp
)

target_link_libraries(LabGP PRIVATE labgp_domain)
target_include_directories(LabGP PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(LabGPDomainTests
    tests/domain/DomainTests.cpp
)
target_link_libraries(LabGPDomainTests PRIVATE labgp_domain)
target_include_directories(LabGPDomainTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME LabGPDomainTests COMMAND LabGPDomainTests)
endif()
