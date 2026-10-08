cmake_minimum_required(VERSION 3.20)
project(casttui LANGUAGES C CXX)

# ------------------------------------------------------------
# Compiler selection (Clang preferred, fallback to system default)
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
# OpenSSL 4.0.3 (static, mingw64 no-shared)
# ------------------------------------------------------------
if (WIN32)
  include(cmake/fetch-build-openssl-static.cmake)
else()
  # Linux/WSL: use system OpenSSL
  find_package(OpenSSL REQUIRED)
endif()
# ------------------------------------------------------------
# cpp-httplib v0.58.0 (single header)
# ------------------------------------------------------------
FetchContent_Declare(httplib
  URL https://raw.githubusercontent.com/yhirose/cpp-httplib/refs/tags/v0.58.0/httplib.h
  DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_GetProperties(httplib)
if(NOT httplib_POPULATED)
  FetchContent_Populate(httplib)

  add_library(httplib INTERFACE)
  target_include_directories(httplib INTERFACE ${httplib_SOURCE_DIR})
endif()

# ------------------------------------------------------------
# TUI
# ------------------------------------------------------------
add_subdirectory(tuimm)
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
  sqlite3_static
  tuimm
  httplib
  OpenSSL::SSL
  OpenSSL::Crypto
)

target_compile_definitions(${PROJECT_NAME} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_WIDEC_SUPPORT=1)

if (WIN32)
  # Windows CryptoAPI + SChannel + sockets
  target_link_libraries(casttui PRIVATE
    crypt32
    secur32
    ws2_32
    iphlpapi
    winmm
    kernel32
    user32
    gdi32
    advapi32
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
  )
endif()


# ------------------------------------------------------------
# Console subsystem (Windows only)
# ------------------------------------------------------------
if (WIN32)
  target_link_options(${PROJECT_NAME} PRIVATE -Wl,--subsystem,console)
endif()

# Static linking only on Windows
if (WIN32)
  target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)
  # # Detect Clang
  # if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  #   message(STATUS "Detected Clang toolchain (MSYS2 clang64)")
  #   set(MSYS_ENV "clang64")

  #   # Clang64 uses LLVM runtime, not GCC
  #   target_link_options(${PROJECT_NAME} PRIVATE "-static")
  #   # DO NOT use -static-libgcc or -static-libstdc++

  # # Detect GCC
  # elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  #   message(STATUS "Detected GCC toolchain (MSYS2 ucrt64 or mingw64)")
  #   set(MSYS_ENV "gcc-mingw")

  #   # GCC runtime
  #   target_link_options(${PROJECT_NAME} PRIVATE "-static -static-libgcc -static-libstdc++")
  # else()
  #   message(WARNING "Unknown Windows compiler: ${CMAKE_CXX_COMPILER_ID}")
  # endif()
endif()