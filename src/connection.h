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

#include <cstdint>

typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;

class Connection {
 public:
  Connection();
  ~Connection();

  uint32_t GenerateId();

  xcb_connection_t* connection() { return connection_; }
  xcb_window_t root_window() const { return root_window_; }
  
 private:
  xcb_connection_t* connection_;
  xcb_window_t root_window_;
};
