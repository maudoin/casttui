#set(CMAKE_C_COMPILER clang CACHE STRING "" FORCE)
#set(CMAKE_CXX_COMPILER clang++ CACHE STRING "" FORCE)

# Comment this if you add msys to PATH or compile from Msys console
set(MSYS2_PREFIX "C:/msys64/ucrt64")

if(DEFINED MSYS2_PREFIX)
  set(CMAKE_C_COMPILER     "${MSYS2_PREFIX}/bin/clang.exe")
  set(CMAKE_CXX_COMPILER   "${MSYS2_PREFIX}/bin/clang++.exe")
  set(OPENSSL_ROOT_DIR       "${MSYS2_PREFIX}")
  set(OPENSSL_INCLUDE_DIR    "${MSYS2_PREFIX}/include")
  set(OPENSSL_CRYPTO_LIBRARY "${MSYS2_PREFIX}/lib/libcrypto.a")
  set(OPENSSL_SSL_LIBRARY    "${MSYS2_PREFIX}/lib/libssl.a")
endif()