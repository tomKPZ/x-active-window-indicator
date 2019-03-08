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

#include <memory>

#include "active_window_observer.h"
#include "key_state_observer.h"
#include "window_geometry_observer.h"

class BorderWindow;
class Connection;
class EventLoop;
class WindowGeometryTracker;

class ActiveWindowIndicator : public ActiveWindowObserver,
                              public KeyStateObserver,
                              public WindowGeometryObserver {
 public:
  ActiveWindowIndicator(Connection* connection,
                        EventLoop* event_loop,
                        BorderWindow* border_window_);
  ~ActiveWindowIndicator();

 protected:
  // ActiveWindowObserver:
  void ActiveWindowChanged(xcb_window_t window) override;

  // KeyStateObserver:
  void KeyStateChanged(bool pressed) override;

  // WindowGeometryObserver:
  void WindowPositionChanged() override;
  void WindowSizeChanged() override;
  void WindowBorderWidthChanged() override;

 private:
  void OnStateChanged();

  void SetBorderWindowBounds();

  Connection* connection_;
  EventLoop* event_loop_;

  BorderWindow* border_window_;
  bool border_window_shown_ = false;

  xcb_window_t active_window_;
  bool key_pressed_ = false;

  std::unique_ptr<WindowGeometryTracker> window_geometry_tracker_;
};
