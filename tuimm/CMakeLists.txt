cmake_minimum_required(VERSION 3.20)

project(tuimm LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ------------------------------------------------------------
# Platform-dependent curses backend (ENCAPSULATED)
# ------------------------------------------------------------
if (WIN32)
    FetchContent_Declare(pdcurses
        URL https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.zip
    )
    FetchContent_MakeAvailable(pdcurses)

    add_library(tuimm_curses STATIC
        ${pdcurses_SOURCE_DIR}/pdcurses/addch.c
        ${pdcurses_SOURCE_DIR}/pdcurses/addchstr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/addstr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/attr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/beep.c
        ${pdcurses_SOURCE_DIR}/pdcurses/bkgd.c
        ${pdcurses_SOURCE_DIR}/pdcurses/border.c
        ${pdcurses_SOURCE_DIR}/pdcurses/clear.c
        ${pdcurses_SOURCE_DIR}/pdcurses/color.c
        ${pdcurses_SOURCE_DIR}/pdcurses/debug.c
        ${pdcurses_SOURCE_DIR}/pdcurses/delch.c
        ${pdcurses_SOURCE_DIR}/pdcurses/deleteln.c
        ${pdcurses_SOURCE_DIR}/pdcurses/getch.c
        ${pdcurses_SOURCE_DIR}/pdcurses/getstr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/getyx.c
        ${pdcurses_SOURCE_DIR}/pdcurses/inch.c
        ${pdcurses_SOURCE_DIR}/pdcurses/inchstr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/initscr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/inopts.c
        ${pdcurses_SOURCE_DIR}/pdcurses/insch.c
        ${pdcurses_SOURCE_DIR}/pdcurses/insstr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/instr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/kernel.c
        ${pdcurses_SOURCE_DIR}/pdcurses/keyname.c
        ${pdcurses_SOURCE_DIR}/pdcurses/mouse.c
        ${pdcurses_SOURCE_DIR}/pdcurses/move.c
        ${pdcurses_SOURCE_DIR}/pdcurses/outopts.c
        ${pdcurses_SOURCE_DIR}/pdcurses/overlay.c
        ${pdcurses_SOURCE_DIR}/pdcurses/pad.c
        ${pdcurses_SOURCE_DIR}/pdcurses/panel.c
        ${pdcurses_SOURCE_DIR}/pdcurses/printw.c
        ${pdcurses_SOURCE_DIR}/pdcurses/refresh.c
        ${pdcurses_SOURCE_DIR}/pdcurses/scanw.c
        ${pdcurses_SOURCE_DIR}/pdcurses/scroll.c
        ${pdcurses_SOURCE_DIR}/pdcurses/scr_dump.c
        ${pdcurses_SOURCE_DIR}/pdcurses/slk.c
        ${pdcurses_SOURCE_DIR}/pdcurses/termattr.c
        ${pdcurses_SOURCE_DIR}/pdcurses/touch.c
        ${pdcurses_SOURCE_DIR}/pdcurses/util.c
        ${pdcurses_SOURCE_DIR}/pdcurses/window.c
        ${pdcurses_SOURCE_DIR}/wincon/pdcclip.c
        ${pdcurses_SOURCE_DIR}/wincon/pdcdisp.c
        ${pdcurses_SOURCE_DIR}/wincon/pdcgetsc.c
        ${pdcurses_SOURCE_DIR}/wincon/pdckbd.c
        ${pdcurses_SOURCE_DIR}/wincon/pdcscrn.c
        ${pdcurses_SOURCE_DIR}/wincon/pdcsetsc.c
        ${pdcurses_SOURCE_DIR}/wincon/pdcutil.c
    )

    target_include_directories(tuimm_curses PUBLIC
        ${pdcurses_SOURCE_DIR}
        ${pdcurses_SOURCE_DIR}/pdcurses
    )

    target_compile_definitions(tuimm_curses PUBLIC
        PDC_WIDE
        PDC_FORCE_UTF8
    )

else()
    set(CURSES_NEED_WIDE TRUE)
    find_package(Curses REQUIRED)

    if (NOT CURSES_CURSES_LIBRARY MATCHES "ncursesw")
        find_library(NCURSESW_LIBRARY ncursesw REQUIRED)
        set(CURSES_CURSES_LIBRARY ${NCURSESW_LIBRARY})
    endif()

    add_library(tuimm_curses INTERFACE)
    target_link_libraries(tuimm_curses INTERFACE ${CURSES_LIBRARIES})
    target_include_directories(tuimm_curses INTERFACE ${CURSES_INCLUDE_DIR})
endif()
message(STATUS "PDCurses source dir: ${pdcurses_SOURCE_DIR}")
file(GLOB PDC_HDRS "${pdcurses_SOURCE_DIR}/*.h")
message(STATUS "PDCurses headers: ${PDC_HDRS}")
# ------------------------------------------------------------
# Library sources
# ------------------------------------------------------------
file(GLOB IMTUI_SRC "src/*.cpp")
file(GLOB IMTUI_HDR "include/*.hpp")

add_library(${PROJECT_NAME} STATIC
    ${IMTUI_SRC}
    ${IMTUI_HDR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE tuimm_curses)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${pdcurses_SOURCE_DIR}
)
# ------------------------------------------------------------
# Hide backend symbols
# ------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# ------------------------------------------------------------
# Demo project
# ------------------------------------------------------------
option(IMTUI_BUILD_DEMO "Build ImmediateTui demo" ON)

if (IMTUI_BUILD_DEMO)
    add_subdirectory(demo)
endif()
