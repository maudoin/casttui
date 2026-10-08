
include(FetchContent)

# ==============================================================================
FetchContent_Declare(openssl
URL https://github.com/openssl/openssl/releases/download/openssl-3.6.5/openssl-3.6.5.tar.gz
)
FetchContent_Populate(openssl)

# ==============================================================================
if(MINGW)
  message(STATUS "MinGW detected")
endif()

# ==============================================================================
find_program(BASH_EXECUTABLE
  NAMES bash
)
message(STATUS "Bash: ${BASH_EXECUTABLE}")

# ==============================================================================
find_program(PERL_EXECUTABLE perl)
if(NOT PERL_EXECUTABLE)
  message(FATAL_ERROR "Perl is required but was not found. Install it with 'pacman -S perl' and ensure it is in the PATH, see cmake/toolchain-xxx.cmake.")
endif()

message(STATUS "Perl: ${PERL_EXECUTABLE}")

# ==============================================================================
find_program(MAKE_PROGRAM
  NAMES mingw32-make make gmake
)
if(NOT MAKE_PROGRAM)
  message(FATAL_ERROR "GNU Make is required but was not found. Install it with 'pacman -S make' and ensure it is in the PATH, see cmake/toolchain-xxx.cmake.")
endif()
message(STATUS "Make: ${MAKE_PROGRAM}")

# ==============================================================================
find_program(CHMOD_EXECUTABLE
  NAMES chmod
)
message(STATUS "Chmod: ${CHMOD_EXECUTABLE}")

# ==============================================================================
find_program(GCC_EXECUTABLE
  NAMES gcc
)
if(NOT GCC_EXECUTABLE)
  message(FATAL_ERROR "Gcc is required for windres but was not found. Install it with 'pacman -S gcc'.")
endif()
message(STATUS "Gcc: ${GCC_EXECUTABLE}")

# ==============================================================================
# Function: convert_to_short_path
# Arguments:
#   - INPUT_PATH:  The original path string (may contain spaces).
#   - RESULT_VAR:  The name of the variable where the 8.3 path will be saved.
# ==============================================================================
function(convert_to_short_path INPUT_PATH RESULT_VAR)
  if(WIN32)
    # Normalize to Windows backslashes
    file(TO_NATIVE_PATH "${INPUT_PATH}" NATIVE_LONG_PATH)

    # Call PowerShell to fetch the COM/Win32 Scripting FileSystem short path
    execute_process(
      COMMAND powershell -NoProfile -Command 
          "$fso = New-Object -ComObject Scripting.FileSystemObject; if (Test-Path '${NATIVE_LONG_PATH}') { if ((Get-Item '${NATIVE_LONG_PATH}').PSIsContainer) { Write-Output $fso.GetFolder('${NATIVE_LONG_PATH}').ShortPath } else { Write-Output $fso.GetFile('${NATIVE_LONG_PATH}').ShortPath } } else { Write-Output '${NATIVE_LONG_PATH}' }"
      OUTPUT_VARIABLE DETECTED_SHORT_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )

    # Remove any lingering raw quotes if PowerShell returned them safely
    string(REPLACE "\"" "" DETECTED_SHORT_PATH "${DETECTED_SHORT_PATH}")

    # Convert back to standard CMake forward slashes
    file(TO_CMAKE_PATH "${DETECTED_SHORT_PATH}" CLEANED_PATH)

    set(${RESULT_VAR} "${CLEANED_PATH}" PARENT_SCOPE)
  else()
    set(${RESULT_VAR} "${INPUT_PATH}" PARENT_SCOPE)
  endif()
endfunction()
convert_to_short_path("${openssl_SOURCE_DIR}" openssl_SOURCE_DIR_SANITIZED)
convert_to_short_path("${CMAKE_BINARY_DIR}" CMAKE_BINARY_DIR_SANITIZED)
# ==============================================================================

include(ExternalProject)
ExternalProject_Add(openssl_build
  PREFIX ${CMAKE_BINARY_DIR}/_deps/openssl-build
  SOURCE_DIR ${openssl_SOURCE_DIR}
  CONFIGURE_COMMAND ${BASH_EXECUTABLE} -lc "set -x && unset CC CXX && export CC=${CMAKE_C_COMPILER} && export CXX=${CMAKE_CXX_COMPILER} && cd ${openssl_SOURCE_DIR_SANITIZED} && perl Configure mingw64 no-shared no-asm --prefix=${CMAKE_BINARY_DIR_SANITIZED}/_deps/openssl-build"
  BUILD_COMMAND ${BASH_EXECUTABLE} -lc "set -x && unset CC CXX && export CC=${CMAKE_C_COMPILER} && export CXX=${CMAKE_CXX_COMPILER} && cd ${openssl_SOURCE_DIR} && pwd && make build_generated libcrypto.a libssl.a"
  INSTALL_COMMAND ${BASH_EXECUTABLE} -lc "set -x && unset CC CXX && export CC=${CMAKE_C_COMPILER} && export CXX=${CMAKE_CXX_COMPILER} && cd ${openssl_SOURCE_DIR} && pwd && make install"
  LOG_BUILD OFF
  USES_TERMINAL_BUILD ON
  BUILD_BYPRODUCTS
  BUILD_BYPRODUCTS
    ${CMAKE_BINARY_DIR}/_deps/openssl-build/lib64/libssl.a
    ${CMAKE_BINARY_DIR}/_deps/openssl-build/lib64/libcrypto.a
)
ExternalProject_Get_Property(openssl_build install_dir)
message(STATUS "OpenSSL installed in: ${install_dir}")

# ==============================================================================
# Create imported targets for the static OpenSSL libs (built by ExternalProject step)
# ==============================================================================
add_library(OpenSSL::SSL STATIC IMPORTED)
set_property(TARGET OpenSSL::SSL PROPERTY IMPORTED_LOCATION "${install_dir}/lib64/libssl.a")

file(MAKE_DIRECTORY "${install_dir}/include")
target_include_directories(OpenSSL::SSL INTERFACE ${install_dir}/include)

add_library(OpenSSL::Crypto STATIC IMPORTED)
set_property(TARGET OpenSSL::Crypto PROPERTY IMPORTED_LOCATION "${install_dir}/lib64/libcrypto.a")


# ==============================================================================
# Ensure the OpenSSL build step runs before these imported libs are linked
# ==============================================================================
add_dependencies(OpenSSL::SSL openssl_build)
add_dependencies(OpenSSL::Crypto openssl_build)