cmake_minimum_required(VERSION 3.20)
project(LabGP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LABGP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(LABGP_ENABLE_GUI "Build SDL2/ImGui dashboard" ON)

add_library(labgp_domain
    src/domain/ResearchProjectStore.cpp
    src/domain/Scoring.cpp
    src/domain/InventoryScanner.cpp
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
if(LABGP_HAS_GUI_UI)
    target_sources(LabGP PRIVATE src/ui/GuiDashboard.cpp)
    target_link_libraries(LabGP PRIVATE imgui SDL2::SDL2)
endif()

add_executable(LabGPDomainTests
    tests/domain/DomainTests.cpp
)
target_link_libraries(LabGPDomainTests PRIVATE labgp_domain)
target_include_directories(LabGPDomainTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(LabGPConsoleUiTests
    tests/ui/ConsoleUiTests.cpp
    src/ui/AppUI.cpp
    src/ui/ListView.cpp
    src/ui/KanbanView.cpp
)
target_link_libraries(LabGPConsoleUiTests PRIVATE labgp_domain)
target_include_directories(LabGPConsoleUiTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME LabGPDomainTests COMMAND LabGPDomainTests)
    add_test(NAME LabGPConsoleUiTests COMMAND LabGPConsoleUiTests)
endif()
