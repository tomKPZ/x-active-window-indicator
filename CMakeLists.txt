cmake_minimum_required (VERSION 2.6)
project (XActiveWindowIndicator)

add_compile_options(
                         -Werror
                         -Wall
                         -Wextra # reasonable and standard
                         -Wshadow # warn the user if a variable declaration shadows one from a
                                  # parent context
                         -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                                            # non-virtual destructor. This helps catch hard to
                                            # track down memory errors
                         -Wold-style-cast # warn for c-style casts
                         -Wcast-align # warn for potential performance problem casts
                         -Wunused # warn on anything being unused
                         -Woverloaded-virtual # warn if you overload (not override) a virtual
                                              # function
                         -Wpedantic # warn if non-standard C++ is used
                         -Wconversion # warn on type conversions that may lose data
                         -Wsign-conversion # warn on sign conversions
                         -Wmisleading-indentation # warn if identation implies blocks where blocks
                                                  # do not exist
                         -Wduplicated-cond # warn if if / else chain has duplicated conditions
                         -Wduplicated-branches # warn if if / else branches have duplicated code
                         -Wlogical-op # warn about logical operations being used where bitwise were
                                      # probably wanted
                         -Wnull-dereference # warn if a null dereference is detected
                         -Wuseless-cast # warn if you perform a cast to the same type
                         -Wdouble-promotion # warn if float is implicit promoted to double
                         -Wformat=2 # warn on security issues around functions that format output
                                    # (ie printf)
                         )

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
