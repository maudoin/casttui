Windows
==

- Update `cmake\toolchain-clang-win.cmake` to setup your clang path (mingw version required)
- If you don't have any install it here https://www.msys2.org/ then install clang with
```
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-clang
```