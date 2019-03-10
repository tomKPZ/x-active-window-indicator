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

#ifndef BORDER_WINDOW_H
#define BORDER_WINDOW_H

#include <cstdint>

#include "util.h"

using xcb_window_t = uint32_t;

class Connection;

class BorderWindow {
 public:
  explicit BorderWindow(Connection* connection);
  ~BorderWindow();

  void SetPosition(int16_t x, int16_t y);
  void SetSize(uint16_t width, uint16_t height);

  void Show();
  void Hide();

 private:
  Connection* connection_;

  xcb_window_t window_;

  DELETE_SPECIAL_MEMBERS(BorderWindow);
};

#endif
