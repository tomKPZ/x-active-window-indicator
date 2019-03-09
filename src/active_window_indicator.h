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
#include "event_loop_idle_observer.h"
#include "key_state_observer.h"
#include "observable.h"
#include "scoped_observer.h"
#include "window_geometry_observer.h"

class BorderWindow;
class Connection;
class EventLoop;
class WindowGeometryTracker;

class ActiveWindowIndicator : public ActiveWindowObserver,
                              public EventLoopIdleObserver,
                              public KeyStateObserver,
                              public WindowGeometryObserver {
 public:
  ActiveWindowIndicator(Connection* connection,
                        EventLoop* event_loop,
                        BorderWindow* border_window_,
                        Observable<KeyStateObserver>* observable_);
  ~ActiveWindowIndicator() override;

 protected:
  // ActiveWindowObserver:
  void ActiveWindowChanged(xcb_window_t window) override;

  // EventLoopIdleObserver:
  void OnIdle() override;

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
  ScopedObserver<KeyStateObserver> observer_;

  xcb_window_t active_window_;
  bool key_pressed_ = false;

  bool needs_set_position_ = false;
  bool needs_set_size_ = false;
  bool needs_show_ = false;

  std::unique_ptr<WindowGeometryTracker> window_geometry_tracker_;
};
