cmake_minimum_required(VERSION 3.20)
project(LabGP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LABGP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(LABGP_ENABLE_GUI "Build SDL2/ImGui dashboard" ON)
option(LABGP_ENABLE_SANITIZERS "Enable address/undefined sanitizers" OFF)
option(LABGP_ENABLE_CLANG_TIDY "Enable clang-tidy integration when available" OFF)

find_program(LABGP_CLANG_FORMAT_EXE NAMES clang-format)
find_program(LABGP_CLANG_TIDY_EXE NAMES clang-tidy)

if(LABGP_ENABLE_CLANG_TIDY)
    if(LABGP_CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY "${LABGP_CLANG_TIDY_EXE}")
    else()
        message(WARNING "LABGP_ENABLE_CLANG_TIDY is ON, but clang-tidy was not found.")
    endif()
endif()

function(labgp_configure_target target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4)
        return()
    endif()

    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    if(LABGP_WARNINGS_AS_ERRORS)
        target_compile_options(${target_name} PRIVATE -Werror)
    endif()

    if(LABGP_ENABLE_SANITIZERS)
        target_compile_options(${target_name} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-optimize-sibling-calls
        )
        target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

add_library(labgp_domain
    src/domain/ResearchProjectStore.cpp
    src/domain/Scoring.cpp
    src/domain/InventoryScanner.cpp
)

target_include_directories(labgp_domain
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
labgp_configure_target(labgp_domain)

set(LABGP_HAS_GUI_UI 0)
if(LABGP_ENABLE_GUI)
    find_package(SDL2 QUIET)
    set(LABGP_IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../LabGestao/vendor/imgui)
    set(LABGP_IMGUI_BACKENDS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui_backends)
    if(SDL2_FOUND AND EXISTS ${LABGP_IMGUI_DIR}/imgui.cpp)
        set(LABGP_HAS_GUI_UI 1)
        add_library(imgui STATIC
            ${LABGP_IMGUI_DIR}/imgui.cpp
            ${LABGP_IMGUI_DIR}/imgui_draw.cpp
            ${LABGP_IMGUI_DIR}/imgui_tables.cpp
            ${LABGP_IMGUI_DIR}/imgui_widgets.cpp
            ${LABGP_IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
            ${LABGP_IMGUI_BACKENDS_DIR}/imgui_impl_sdlrenderer2.cpp
        )
        target_include_directories(imgui PUBLIC
            ${LABGP_IMGUI_DIR}
            ${LABGP_IMGUI_DIR}/backends
            ${LABGP_IMGUI_BACKENDS_DIR}
        )
        target_link_libraries(imgui PUBLIC SDL2::SDL2)
        labgp_configure_target(imgui)
    endif()
endif()

add_executable(LabGP
    src/main.cpp
    src/app/AppRuntime.cpp
    src/ui/AppUI.cpp
    src/ui/ListView.cpp
    src/ui/KanbanView.cpp
)

target_link_libraries(LabGP PRIVATE labgp_domain)
target_include_directories(LabGP PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(LabGP PRIVATE LABGP_HAS_GUI_UI=${LABGP_HAS_GUI_UI})
labgp_configure_target(LabGP)
if(LABGP_HAS_GUI_UI)
    target_sources(LabGP PRIVATE src/ui/GuiDashboard.cpp)
    target_link_libraries(LabGP PRIVATE imgui SDL2::SDL2)
endif()

add_executable(LabGPDomainTests
    tests/domain/DomainTests.cpp
)
target_link_libraries(LabGPDomainTests PRIVATE labgp_domain)
target_include_directories(LabGPDomainTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
labgp_configure_target(LabGPDomainTests)

add_executable(LabGPConsoleUiTests
    tests/ui/ConsoleUiTests.cpp
    src/ui/AppUI.cpp
    src/ui/ListView.cpp
    src/ui/KanbanView.cpp
)
target_link_libraries(LabGPConsoleUiTests PRIVATE labgp_domain)
target_include_directories(LabGPConsoleUiTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
labgp_configure_target(LabGPConsoleUiTests)

if(LABGP_CLANG_FORMAT_EXE)
    file(GLOB_RECURSE LABGP_FORMAT_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp
    )
    add_custom_target(format-check
        COMMAND ${LABGP_CLANG_FORMAT_EXE} --dry-run --Werror ${LABGP_FORMAT_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Checking source formatting with clang-format"
        VERBATIM
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME LabGPDomainTests COMMAND LabGPDomainTests)
    add_test(NAME LabGPConsoleUiTests COMMAND LabGPConsoleUiTests)
endif()

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/engineering/Doxyfile.min
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Gerando documentacao Doxygen do LabGP"
        VERBATIM
    )
endif()
