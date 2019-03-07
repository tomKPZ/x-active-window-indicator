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

#include "active_window_observer.h"
#include "key_state_observer.h"

class BorderWindow;
class Connection;

class ActiveWindowIndicator : public ActiveWindowObserver,
                              public KeyStateObserver {
 public:
  ActiveWindowIndicator(Connection* connection, BorderWindow* border_window_);
  ~ActiveWindowIndicator();

 protected:
  // ActiveWindowObserver:
  void ActiveWindowChanged(xcb_window_t window) override;

  // KeyStateObserver:
  void KeyStateChanged(bool pressed) override;

 private:
  void OnStateChanged();

  Connection* connection_;

  BorderWindow* border_window_;
  bool border_window_shown_ = false;

  xcb_window_t active_window_;
  bool key_pressed_ = false;
};
