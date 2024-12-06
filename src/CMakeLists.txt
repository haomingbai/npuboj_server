include_directories(${CMAKE_SOURCE_DIR}/src/include)

set(ENV{PKG_CONFIG_PATH} "/usr/local/lib/pkgconfig/")
set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)

if (PKG_CONFIG_FOUND)
  pkg_check_modules(SANDBOX2 REQUIRED sandbox2)
endif()

link_libraries(${SANDBOX2_LIBRARIES})
link_directories(${SANDBOX2_LIBRARY_DIRS})

add_executable(npuboj_server main.cpp)
add_library(c_compile_strategy STATIC lib/c_compile_strategy.cpp)

target_compile_options(c_compile_strategy PRIVATE ${SANDBOX2_CFLAGS_OTHER})
target_link_libraries(c_compile_strategy absl::log absl::base)