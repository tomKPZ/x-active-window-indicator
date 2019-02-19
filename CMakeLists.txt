cmake_minimum_required (VERSION 2.6)
project (XActiveWindowIndicator)
add_executable(x-active-window-indicator src/indicator.cpp src/active_window_manager.h src/active_window_manager.cpp)

set_target_properties(x-active-window-indicator PROPERTIES
  CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules (XCB REQUIRED xcb)
pkg_check_modules (XCB_XFIXES REQUIRED xcb-xfixes)

target_link_libraries(x-active-window-indicator ${XCB_LIBRARIES} ${XCB_XFIXES_LIBRARIES})
target_include_directories(x-active-window-indicator PUBLIC ${XCB_INCLUDE_DIRS} ${XCB_XFIXES_INCLUDE_DIRS})
target_compile_options(x-active-window-indicator PUBLIC ${XCB_CFLAGS_OTHER} ${XCB_XFIXES_OTHER})

# target_link_libraries(x-active-window-indicator "-lc++")
# target_compile_options(x-active-window-indicator PUBLIC -stdlib=libc++)
# SET (CMAKE_CXX_COMPILER "clang++")
