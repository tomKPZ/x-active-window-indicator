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
#include "window_geometry_observer.h"
#include "x_error.h"

WindowGeometryTracker::WindowGeometryTracker(Connection* connection,
                                             xcb_window_t window,
                                             WindowGeometryObserver* observer)
    : connection_(connection), window_(window), observer_(observer) {
  connection_->SelectEvents(window_, XCB_EVENT_MASK_STRUCTURE_NOTIFY);
  auto geometry =
      XCB_SYNC(xcb_get_geometry, connection_->connection(), window_);
  x_ = geometry->x;
  y_ = geometry->y;
  width_ = geometry->width;
  height_ = geometry->height;
  border_width_ = geometry->border_width;
}

WindowGeometryTracker::~WindowGeometryTracker() {
  if (!destroyed_)
    connection_->SelectEvents(window_, XCB_EVENT_MASK_NO_EVENT);
}

bool WindowGeometryTracker::DispatchEvent(const Event& event) {
  union {
    const xcb_generic_event_t* generic;

    const xcb_circulate_notify_event_t* circulate;
    const xcb_configure_notify_event_t* configure;
    const xcb_destroy_notify_event_t* destroy;
    const xcb_gravity_notify_event_t* gravity;
    const xcb_map_notify_event_t* map;
    const xcb_reparent_notify_event_t* reparent;
    const xcb_unmap_notify_event_t* unmap;
  } const structure_event{event.event()};

  switch (event->response_type & ~0x80) {
    case XCB_CIRCULATE_NOTIFY:
      return structure_event.circulate->event == window_;
    case XCB_CONFIGURE_NOTIFY: {
      const auto* configure = structure_event.configure;

      if (configure->event != window_)
        return false;

      if (configure->event != configure->window)
        throw XError("Bad configure notify event");

      if (x_ != configure->x || y_ != configure->y) {
        x_ = configure->x;
        y_ = configure->y;
        observer_->WindowPositionChanged();
      }

      if (width_ != configure->width || height_ != configure->height) {
        width_ = configure->width;
        height_ = configure->height;
        observer_->WindowSizeChanged();
      }

      if (border_width_ != configure->border_width) {
        border_width_ = configure->border_width;
        observer_->WindowBorderWidthChanged();
      }

      return true;
    }
    case XCB_DESTROY_NOTIFY:
      if (structure_event.destroy->event != window_)
        return false;
      destroyed_ = true;
      return true;
    case XCB_GRAVITY_NOTIFY: {
      const auto* gravity = structure_event.gravity;

      if (gravity->event != window_)
        return false;

      x_ = gravity->x;
      y_ = gravity->y;
      observer_->WindowPositionChanged();

      return true;
    }
    case XCB_MAP_NOTIFY:
      return structure_event.map->event == window_;
    case XCB_REPARENT_NOTIFY:
      // TODO
      return structure_event.reparent->event == window_;
    case XCB_UNMAP_NOTIFY:
      return structure_event.unmap->event == window_;
  }
  return false;
}

void WindowGeometryTracker::WindowPositionChanged() {
  observer_->WindowPositionChanged();
}
