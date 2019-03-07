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

#include "x_error.h"

#include <sstream>

namespace {

std::string MakeErrorMessage(const xcb_generic_error_t& error) {
  std::ostringstream stream;
  stream << "XCB Error: response_type("
         << static_cast<uint16_t>(error.response_type) << "), error_code("
         << static_cast<uint16_t>(error.error_code) << "), sequence("
         << error.sequence << "), resource_id(" << error.resource_id
         << "), minor_code(" << error.minor_code << "), major_code("
         << static_cast<uint16_t>(error.major_code) << ")";
  return stream.str();
}

}  // namespace

XError::XError(const std::string& what) : std::runtime_error(what) {}

XError::XError(const char* what) : std::runtime_error(what) {}

XError::XError(const xcb_generic_error_t& error)
    : std::runtime_error(MakeErrorMessage(error)) {}
