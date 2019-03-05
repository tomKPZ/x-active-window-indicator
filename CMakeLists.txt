include(CheckCXXCompilerFlag)
find_package(PkgConfig REQUIRED)

cmake_minimum_required(VERSION 2.6)
project(XActiveWindowIndicator)

function(add_cxx_compiler_flag_if_supported FLAG)
  # Remove leading - or / from the flag name.
  string(REGEX
         REPLACE "^-|/"
                 ""
                 name
                 ${FLAG})
  check_cxx_compiler_flag(${FLAG} ${name})
  if(${name})
    add_compile_options(${FLAG})
  endif()
endfunction()

add_compile_options(-Werror
                    -Wall
                    -Wextra # reasonable and standard
                    -Wshadow # warn the user if a variable declaration shadows one from a parent
                             # context
                    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-
                                       # virtual destructor. This helps catch hard to track down
                                       # memory errors
                    -Wold-style-cast # warn for c-style casts
                    -Wcast-align # warn for potential performance problem casts
                    -Wunused # warn on anything being unused
                    -Woverloaded-virtual # warn if you overload (not override) a virtual function
                    -Wpedantic # warn if non-standard C++ is used
                    -Wconversion # warn on type conversions that may lose data
                    -Wsign-conversion # warn on sign conversions
                    -Wnull-dereference # warn if a null dereference is detected
                    -Wdouble-promotion # warn if float is implicit promoted to double
                    -Wformat=2 # warn on security issues around functions that format output (ie
                               # printf)
                    )

# Warn if identation implies blocks where blocks do not exist.
add_cxx_compiler_flag_if_supported(-Wmisleading-indentation)

# Warn if if / else chain has duplicated conditions.
add_cxx_compiler_flag_if_supported(-Wduplicated-cond)

# Warn about logical operations being used where bitwise were probably wanted.
add_cxx_compiler_flag_if_supported(-Wlogical-op)

# Warn if you perform a cast to the same type.
add_cxx_compiler_flag_if_supported(-Wuseless-cast)

# Warn if if / else branches have duplicated code.
add_cxx_compiler_flag_if_supported(-Wduplicated-branches)

add_executable(x-active-window-indicator
               src/indicator.cpp
               src/active_window_observer.h
               src/active_window_tracker.h
               src/active_window_tracker.cpp
               src/border_window.h
               src/border_window.cpp
               src/connection.h
               src/connection.cpp
               src/event.h
               src/event.cpp
               src/event_dispatcher.h
               src/event_loop.h
               src/event_loop.cpp
               src/key_listener.h
               src/key_listener.cpp
               src/key_state_observer.h
               src/util.h)

set_target_properties(x-active-window-indicator PROPERTIES CXX_STANDARD 20)

pkg_check_modules(XCB REQUIRED xcb)
pkg_check_modules(XCB_XFIXES REQUIRED xcb-xfixes)
pkg_check_modules(XCB_XINPUT REQUIRED xcb-xinput)

target_link_libraries(x-active-window-indicator
                      ${XCB_LIBRARIES}
                      ${XCB_XFIXES_LIBRARIES}
                      ${XCB_XINPUT_LIBRARIES})
target_include_directories(x-active-window-indicator
                           PUBLIC ${XCB_INCLUDE_DIRS} ${XCB_XFIXES_INCLUDE_DIRS}
                                  ${XCB_XINPUT_INCLUDE_DIRS})
target_compile_options(x-active-window-indicator
                       PUBLIC ${XCB_CFLAGS_OTHER} ${XCB_XFIXES_OTHER} ${XCB_XINPUT_OTHER})

# target_link_libraries(x-active-window-indicator "-lc++") target_compile_options(x-active-window-
# indicator PUBLIC -stdlib=libc++) SET (CMAKE_CXX_COMPILER "clang++")
