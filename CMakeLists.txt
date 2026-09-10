cmake_minimum_required(VERSION 3.20)
project(casttui LANGUAGES C CXX)

# ------------------------------------------------------------
# Compiler selection (Clang preferred, fallback to system default)
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static linking only on Windows
if (WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    # # Detect Clang
    # if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #     message(STATUS "Detected Clang toolchain (MSYS2 clang64)")
    #     set(MSYS_ENV "clang64")

    #     # Clang64 uses LLVM runtime, not GCC
    #     set(CMAKE_EXE_LINKER_FLAGS "-static")
    #     # DO NOT use -static-libgcc or -static-libstdc++

    # # Detect GCC
    # elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    #     message(STATUS "Detected GCC toolchain (MSYS2 ucrt64 or mingw64)")
    #     set(MSYS_ENV "gcc-mingw")

    #     # GCC runtime
    #     set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
    # else()
    #     message(WARNING "Unknown Windows compiler: ${CMAKE_CXX_COMPILER_ID}")
    # endif()
endif()

include(FetchContent)

# ------------------------------------------------------------
# fmt 9.1.0 (header-only)
# ------------------------------------------------------------
FetchContent_Declare(fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.zip
)
FetchContent_MakeAvailable(fmt)

# ------------------------------------------------------------
# rapidxml 1.13 (header-only)
# ------------------------------------------------------------
FetchContent_Declare(rapidxml
    URL https://github.com/Fe-Bell/RapidXML/archive/refs/tags/v117.zip
)
FetchContent_MakeAvailable(rapidxml)

# ------------------------------------------------------------
# sqlite3 (static)
# ------------------------------------------------------------
FetchContent_Declare(sqlite3
    URL https://www.sqlite.org/2023/sqlite-amalgamation-3420000.zip
)
FetchContent_MakeAvailable(sqlite3)

add_library(sqlite3_static STATIC
    ${sqlite3_SOURCE_DIR}/sqlite3.c
)
target_include_directories(sqlite3_static PUBLIC ${sqlite3_SOURCE_DIR})

# ------------------------------------------------------------
# libhv 1.3.0
# ------------------------------------------------------------
FetchContent_Declare(libhv
    URL https://github.com/ithewei/libhv/archive/refs/tags/v1.3.0.zip
)
FetchContent_MakeAvailable(libhv)

set(HV_STATIC ON CACHE BOOL "Build libhv static")

# ------------------------------------------------------------
# Curses library (platform-dependent)
# ------------------------------------------------------------
if (WIN32)
    # ---------------- Windows: PDCurses ----------------
    FetchContent_Declare(pdcurses
        URL https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.zip
    )
    FetchContent_MakeAvailable(pdcurses)

    add_library(curses STATIC
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
    target_include_directories(curses PUBLIC
        ${pdcurses_SOURCE_DIR}
        ${pdcurses_SOURCE_DIR}/pdcurses
    )
    target_compile_definitions(curses PUBLIC 
        PDC_WIDE           # Equivalent to WIDE=Y (Enables wchar_t and cchar_t)
        PDC_FORCE_UTF8     # Equivalent to UTF8=Y (Treats narrow strings as UTF-8)
    )
else()
    # ---------------- Linux/WSL: ncurses ----------------
    set(CURSES_NEED_WIDE TRUE)
    find_package(Curses REQUIRED)

    # Force wide curses on distros like Ubuntu/Debian
    if (NOT CURSES_CURSES_LIBRARY MATCHES "ncursesw")
        message(STATUS "Forcing wide-character ncursesw")
        find_library(NCURSESW_LIBRARY ncursesw REQUIRED)
        set(CURSES_CURSES_LIBRARY ${NCURSESW_LIBRARY})
    endif()

    add_library(curses INTERFACE)
    target_link_libraries(curses INTERFACE ${CURSES_LIBRARIES})
    target_include_directories(curses INTERFACE ${CURSES_INCLUDE_DIR})
endif()

# ------------------------------------------------------------
# OpenSSL (optional)
# ------------------------------------------------------------
if (WIN32)
    # Prebuilt MinGW OpenSSL
    set(OPENSSL_ROOT "deps/openssl-1.1.1k-win64-mingw")
    include_directories(${OPENSSL_ROOT}/include)
else()
    # Linux/WSL: use system OpenSSL
    find_package(OpenSSL REQUIRED)
endif()

# ------------------------------------------------------------
# cpp-httplib v0.54.1 (single header)
# ------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(httplib
    URL https://raw.githubusercontent.com/yhirose/cpp-httplib/refs/tags/v0.54.1/httplib.h
    DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_GetProperties(httplib)
if(NOT httplib_POPULATED)
    FetchContent_Populate(httplib)

    add_library(httplib INTERFACE)
    target_include_directories(httplib INTERFACE ${httplib_SOURCE_DIR})
endif()


# ------------------------------------------------------------
# Application
# ------------------------------------------------------------
file(GLOB source_list "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME}
    ${source_list}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${rapidxml_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt
    hv_static
    sqlite3_static
    curses
    httplib
)

target_compile_definitions(${PROJECT_NAME} PRIVATE USE_WIDEC_SUPPORT=1)

if (WIN32)

    # target_link_libraries(${PROJECT_NAME} PRIVATE
    #     C:/msys64/ucrt64/lib/libdbghelp.a
    # )
    # target_compile_options(${PROJECT_NAME} PRIVATE
    #     $<$<CONFIG:Debug>:-g -gdwarf-4>
    # )
    # target_link_options(${PROJECT_NAME} PRIVATE
    #     $<$<CONFIG:Debug>:-g>
    # )
    # target_compile_options(${PROJECT_NAME} PRIVATE
    #     $<$<CONFIG:Debug>:-g -gcodeview>
    # )
    # If using DLL OpenSSL:
    # target_link_libraries(${PROJECT_NAME} PRIVATE
    #     ${OPENSSL_ROOT}/lib/libssl.dll.a
    #     ${OPENSSL_ROOT}/lib/libcrypto.dll.a
    # )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

# ------------------------------------------------------------
# Console subsystem (Windows only)
# ------------------------------------------------------------
if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--subsystem,console)
endif()
