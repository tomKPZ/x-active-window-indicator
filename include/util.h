// x-active-window-indicator: An X11 utility that signals the active window
// Copyright (C) 2019 <tomKPZ@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

#ifndef UTIL_H
#define UTIL_H

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#define STRINGIZE_AUX(x) #x
#define STRINGIZE(x) STRINGIZE_AUX(x)

#define DCHECK(x)                                                    \
  do {                                                               \
    if (!(x)) {                                                      \
      std::cerr << __FILE__ << ":" << __LINE__                       \
                << ": DCHECK failed: " << STRINGIZE(x) << std::endl; \
      std::abort();                                                  \
    }                                                                \
  } while (0)

#define DELETE_COPY_AND_MOVE(TypeName)                                  \
  TypeName(const TypeName&) = delete;                                   \
  TypeName& /*NOLINT(bugprone-macro-parentheses)*/ operator=(           \
      const TypeName&) = delete;                                        \
  TypeName(TypeName&& /*NOLINT(bugprone-macro-parentheses)*/) = delete; \
  TypeName& /*NOLINT(bugprone-macro-parentheses)*/ operator=(           \
      TypeName&& /*NOLINT(bugprone-macro-parentheses)*/) = delete

template <typename Dst, typename Src>
constexpr Dst CheckedCast(Src value) {
  if (value < std::numeric_limits<Dst>::min() ||
      value > std::numeric_limits<Dst>::max()) {
    throw std::runtime_error("Cast value not in range: " +
                             std::to_string(value));
  }
  return static_cast<Dst>(value);
}

#endif
