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
#include "active_window_tracker.h"
#include "border_window.h"
#include "event_loop_idle_observer.h"
#include "key_listener.h"
#include "key_state_observer.h"
#include "scoped_observer.h"
#include "util.h"
#include "window_geometry_observer.h"
#include "window_geometry_tracker.h"

class CommandLine;
class Connection;
class EventLoop;

class ActiveWindowIndicator : public ActiveWindowObserver,
                              public EventLoopIdleObserver,
                              public KeyStateObserver,
                              public WindowGeometryObserver {
 public:
  ActiveWindowIndicator(Connection* connection,
                        EventLoop* event_loop,
                        CommandLine* command_line);
  ~ActiveWindowIndicator() override;

 protected:
  // ActiveWindowObserver:
  void ActiveWindowChanged() override;

  // EventLoopIdleObserver:
  void OnIdle() override;

  // KeyStateObserver:
  void KeyStateChanged() override;

  // WindowGeometryObserver:
  void WindowPositionChanged() override;
  void WindowSizeChanged() override;
  void WindowBorderWidthChanged() override;

 private:
  void OnStateChanged();

  void SetBorderWindowBounds();

  Connection* connection_;
  EventLoop* event_loop_;
  BorderWindow border_window_;
  ActiveWindowTracker active_window_tracker_;
  KeyListener key_listener_;
  ScopedObserver<ActiveWindowObserver> active_window_observer_;
  ScopedObserver<EventLoopIdleObserver> event_loop_idle_observer_;
  ScopedObserver<KeyStateObserver> key_state_observer_;

  bool needs_set_position_ = false;
  bool needs_set_size_ = false;
  bool needs_show_ = false;

  std::unique_ptr<WindowGeometryTracker> window_geometry_tracker_{};
  std::unique_ptr<ScopedObserver<WindowGeometryObserver>>
      window_geometry_observer_{};

  DELETE_SPECIAL_MEMBERS(ActiveWindowIndicator);
};
