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

#include "window_geometry_tracker.h"

#include "connection.h"
#include "event.h"

WindowGeometryTracker::WindowGeometryTracker(Connection* connection,
                                             xcb_window_t window,
                                             WindowGeometryObserver* observer)
    : connection_(connection), window_(window), observer_(observer) {
  connection_->SelectEvents(window_, XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

WindowGeometryTracker::~WindowGeometryTracker() {
  connection_->SelectEvents(window_, XCB_EVENT_MASK_NO_EVENT);
}

bool WindowGeometryTracker::DispatchEvent(const Event& event) {
  (void)event;
  return false;
}
