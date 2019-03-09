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

ActiveWindowIndicator::ActiveWindowIndicator(
    Connection* connection,
    EventLoop* event_loop,
    BorderWindow* border_window,
    Observable<ActiveWindowObserver>* active_window_observable,
    Observable<KeyStateObserver>* key_state_observable)
    : connection_(connection),
      event_loop_(event_loop),
      border_window_(border_window),
      active_window_observer_(this, active_window_observable),
      key_state_observer_(this, key_state_observable),
      active_window_(XCB_WINDOW_NONE) {
  event_loop_->AddIdleObserver(this);
}

ActiveWindowIndicator::~ActiveWindowIndicator() {
  event_loop_->RemoveIdleObserver(this);
}

void ActiveWindowIndicator::ActiveWindowChanged(xcb_window_t window) {
  active_window_ = window;
  OnStateChanged();
}

void ActiveWindowIndicator::OnIdle() {
  // TODO: take border width into account for position and size.
  if (needs_set_position_) {
    border_window_->SetPosition(window_geometry_tracker_->X(),
                                window_geometry_tracker_->Y());
  }
  needs_set_position_ = false;

  if (needs_set_size_) {
    border_window_->SetSize(window_geometry_tracker_->width(),
                            window_geometry_tracker_->height());
  }
  needs_set_size_ = false;

  if (needs_show_)
    border_window_->Show();
  needs_show_ = false;
}

void ActiveWindowIndicator::KeyStateChanged(bool pressed) {
  key_pressed_ = pressed;
  OnStateChanged();
}

void ActiveWindowIndicator::WindowPositionChanged() {
  needs_set_position_ = true;
}

void ActiveWindowIndicator::WindowSizeChanged() {
  needs_set_size_ = true;
}

void ActiveWindowIndicator::WindowBorderWidthChanged() {
  needs_set_position_ = true;
  needs_set_size_ = true;
}

void ActiveWindowIndicator::OnStateChanged() {
  const bool show = key_pressed_ && active_window_ != XCB_WINDOW_NONE;
  needs_set_position_ = show;
  needs_set_size_ = show;
  needs_show_ = show;
  window_geometry_tracker_ =
      show ? std::make_unique<WindowGeometryTracker>(connection_, event_loop_,
                                                     active_window_, this)
           : nullptr;
  if (!show)
    border_window_->Hide();
}
