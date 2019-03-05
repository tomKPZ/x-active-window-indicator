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

#pragma once

#include <cstddef>
#include <limits>

#define DISALLOW_COPY_AND_ASSIGN(TypeName)	\
  TypeName(const TypeName&) = delete;		\
  TypeName& operator=(const TypeName&) = delete

template <typename T>
constexpr size_t ArraySize(const T& array) {
  return sizeof(array) / sizeof(array[0]);
}

template <typename Dst, typename Src>
constexpr Dst safe_cast(Src value) {
  if (value < std::numeric_limits<Dst>::min() ||
      value > std::numeric_limits<Dst>::max()) {
    throw "Cast out of range";
  }
  return static_cast<Dst>(value);
}
