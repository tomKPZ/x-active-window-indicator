include(CheckCXXCompilerFlag)
find_package(PkgConfig REQUIRED)

cmake_minimum_required(VERSION 2.6)
project(XActiveWindowIndicator)

function(add_cxx_compiler_flag_if_supported FLAG)
  # Remove leading - or / from the flag name.
  string(REGEX REPLACE "^-|/" "" name ${FLAG})
  check_cxx_compiler_flag(${FLAG} ${name})
  if(${name})
    add_compile_options(${FLAG})
  endif()
endfunction()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(
    -Werror
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-weak-vtables
    -Wno-padded
    -Wno-disabled-macro-expansion)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(
    -Wall
    -Wextra # reasonable and standard
    -Wshadow # warn the user if a variable declaration shadows one from a parent context
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non- virtual
                       # destructor. This helps catch hard to track down memory errors
    -Wold-style-cast # warn for c-style casts
    -Wcast-align # warn for potential performance problem casts
    -Wunused # warn on anything being unused
    -Woverloaded-virtual # warn if you overload (not override) a virtual function
    -Wpedantic # warn if non-standard C++ is used
    -Wconversion # warn on type conversions that may lose data
    -Wsign-conversion # warn on sign conversions
    -Wnull-dereference # warn if a null dereference is detected
    -Wdouble-promotion # warn if float is implicit promoted to double
    -Wformat=2 # warn on security issues around functions that format output (ie printf) Warn if
               # identation implies blocks where blocks do not exist.
    -Wmisleading-indentation
    # Warn if if / else chain has duplicated conditions.
    -Wduplicated-cond
    # Warn about logical operations being used where bitwise were probably wanted.
    -Wlogical-op
    # Warn if you perform a cast to the same type.
    -Wuseless-cast
    # Warn if if / else branches have duplicated code.
    -Wduplicated-branches)
else()
  add_compile_options(-Wall -Wextra)
endif()
add_compile_options(-Werror)

include_directories(include)

add_executable(
  x-active-window-indicator
  src/active_window_indicator.cpp
  src/active_window_tracker.cpp
  src/border_window.cpp
  src/connection.cpp
  src/event.cpp
  src/event_loop.cpp
  src/key_listener.cpp
  src/lippincott.cpp
  src/main.cpp
  src/p_error.cpp
  src/quit_signaller.cpp
  src/window_geometry_tracker.cpp
  src/x_error.cpp)

set_target_properties(x-active-window-indicator PROPERTIES CXX_STANDARD 20)

pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(XCB_XFIXES REQUIRED xcb-xfixes)
pkg_check_modules(XCB_XINPUT REQUIRED xcb-xinput)

target_link_libraries(x-active-window-indicator ${XCB_LIBRARIES} ${XCB_XFIXES_LIBRARIES}
                      ${XCB_XINPUT_LIBRARIES})
target_include_directories(
  x-active-window-indicator PUBLIC ${XCB_INCLUDE_DIRS} ${XCB_XFIXES_INCLUDE_DIRS}
                                   ${XCB_XINPUT_INCLUDE_DIRS})
target_compile_options(x-active-window-indicator PUBLIC ${XCB_CFLAGS_OTHER} ${XCB_XFIXES_OTHER}
                                                        ${XCB_XINPUT_OTHER})

install(TARGETS x-active-window-indicator DESTINATION bin)

# target_link_libraries(x-active-window-indicator "-lc++") target_compile_options(x-active-window-
# indicator PUBLIC -stdlib=libc++) SET (CMAKE_CXX_COMPILER "clang++")
