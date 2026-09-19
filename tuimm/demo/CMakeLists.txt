project(tuimm_demo LANGUAGES C CXX)

file(GLOB DEMO_SRC "*.cpp")

add_executable(${PROJECT_NAME} ${DEMO_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE tuimm)


# Console subsystem (Windows only)
# ------------------------------------------------------------
if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--subsystem,console)
    target_link_options(${PROJECT_NAME} PRIVATE -static -static-libgcc -static-libstdc++)
endif()