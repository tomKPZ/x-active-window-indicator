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

#include <cstdint>

#include <memory>

#include "event_dispatcher.h"
#include "event_loop.h"
#include "observable.h"
#include "scoped_observer.h"
#include "util.h"
#include "window_geometry_observer.h"

using xcb_window_t = uint32_t;

class Connection;
class WindowGeometryObserver;

class WindowGeometryTracker : public EventDispatcher,
                              public Observable<WindowGeometryObserver>,
                              public WindowGeometryObserver {
 public:
  WindowGeometryTracker(Connection* connection,
                        EventLoop* event_loop,
                        xcb_window_t window);
  ~WindowGeometryTracker() override;

  int16_t X() const;
  int16_t Y() const;

  uint16_t width() const { return width_; }
  uint16_t height() const { return height_; }

  uint16_t border_width() const { return border_width_; }

 protected:
  // EventDispatcher:
  bool DispatchEvent(const Event& event) override;

  // WindowGeometryObserver:
  void WindowPositionChanged() override;

 private:
  void SetParent(xcb_window_t parent);

  Connection* connection_;
  EventLoop* event_loop_;
  ScopedObserver<EventDispatcher> event_dispatcher_;
  xcb_window_t window_;

  // Position relative to the parent window.  (0, 0) if this is the
  // root window.
  int16_t x_;
  int16_t y_;

  uint16_t width_;
  uint16_t height_;

  uint16_t border_width_;

  std::unique_ptr<WindowGeometryTracker> parent_;
  std::unique_ptr<ScopedObserver<WindowGeometryObserver>> observer_;

  DISALLOW_COPY_AND_ASSIGN(WindowGeometryTracker);
};
