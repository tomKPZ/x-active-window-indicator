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

#include "usage_error.h"

#include <iostream>
#include <ostream>

namespace {

const char* kUsageMessage = R"(
usage: x-active-window-indicator [-h] [-c COLOR] [-w WIDTH]

An X11 utility that signals the active window

optional arguments:
  -h, --help                show this help message and exit
  -c, --border-color COLOR  indicator color in aarrggbb format
  -w, --border-width WIDTH  indicator border width
)";

}  // namespace

// static
void UsageError::Usage() {
  std::cerr << kUsageMessage + 1;
  std::flush(std::cerr);
}
