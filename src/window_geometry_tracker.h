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
#include "observable.h"
#include "scoped_observer.h"
#include "util.h"
#include "window_geometry_observer.h"

using xcb_window_t = uint32_t;

class Connection;
class Event;
class EventLoop;

class WindowGeometryTracker : public EventDispatcher,
                              public Observable<WindowGeometryObserver>,
                              public WindowGeometryObserver {
 public:
  WindowGeometryTracker(Connection* connection,
                        EventLoop* event_loop,
                        const xcb_window_t& window);
  ~WindowGeometryTracker() override;

  [[nodiscard]] auto X() const -> int16_t;
  [[nodiscard]] auto Y() const -> int16_t;

  [[nodiscard]] auto width() const -> uint16_t { return width_; }
  [[nodiscard]] auto height() const -> uint16_t { return height_; }

  [[nodiscard]] auto border_width() const -> uint16_t { return border_width_; }

 protected:
  // EventDispatcher:
  auto DispatchEvent(const Event& event) -> bool override;

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

  DELETE_SPECIAL_MEMBERS(WindowGeometryTracker);
};
