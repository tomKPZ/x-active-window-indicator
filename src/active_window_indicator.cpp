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

#include "active_window_indicator.h"

#include <xcb/xcb.h>

#include "border_window.h"
#include "connection.h"
#include "event_loop.h"
#include "window_geometry_tracker.h"

ActiveWindowIndicator::ActiveWindowIndicator(Connection* connection,
                                             EventLoop* event_loop,
                                             BorderWindow* border_window)
    : connection_(connection),
      event_loop_(event_loop),
      border_window_(border_window),
      active_window_(XCB_WINDOW_NONE) {}

ActiveWindowIndicator::~ActiveWindowIndicator() {}

void ActiveWindowIndicator::ActiveWindowChanged(xcb_window_t window) {
  active_window_ = window;
  OnStateChanged();
}

void ActiveWindowIndicator::KeyStateChanged(bool pressed) {
  key_pressed_ = pressed;
  OnStateChanged();
}

void ActiveWindowIndicator::WindowPositionChanged() {
  SetBorderWindowBounds();
}

void ActiveWindowIndicator::WindowSizeChanged() {
  SetBorderWindowBounds();
}

void ActiveWindowIndicator::WindowBorderWidthChanged() {
  SetBorderWindowBounds();
}

void ActiveWindowIndicator::OnStateChanged() {
  if (key_pressed_ && active_window_ != XCB_WINDOW_NONE) {
    window_geometry_tracker_ = std::make_unique<WindowGeometryTracker>(
        connection_, event_loop_, active_window_, this);
    SetBorderWindowBounds();
    border_window_->Show();
  } else {
    window_geometry_tracker_ = nullptr;
    border_window_->Hide();
  }
}

void ActiveWindowIndicator::SetBorderWindowBounds() {
  // TODO: Use border width.
  border_window_->SetRect(xcb_rectangle_t{
      window_geometry_tracker_->X(), window_geometry_tracker_->Y(),
      window_geometry_tracker_->width(), window_geometry_tracker_->height()});
}
