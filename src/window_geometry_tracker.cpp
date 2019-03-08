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
                                             EventLoop* event_loop,
                                             xcb_window_t window,
                                             WindowGeometryObserver* observer)
    : connection_(connection),
      event_loop_(event_loop),
      window_(window),
      observer_(observer) {
  event_loop_->RegisterDispatcher(this);
  connection_->SelectEvents(window_, XCB_EVENT_MASK_STRUCTURE_NOTIFY);

  auto tree = XCB_SYNC(xcb_query_tree, connection_, window_);
  SetParent(tree->parent);

  auto geometry = XCB_SYNC(xcb_get_geometry, connection_, window_);
  x_ = geometry->x;
  y_ = geometry->y;
  width_ = geometry->width;
  height_ = geometry->height;
  border_width_ = geometry->border_width;
}

WindowGeometryTracker::~WindowGeometryTracker() {
  if (!destroyed_) {
    connection_->SelectEvents(window_, XCB_EVENT_MASK_NO_EVENT);
    event_loop_->UnregisterDispatcher(this);
  }
}

int16_t WindowGeometryTracker::X() const {
  return parent_ ? parent_->X() + x_ : 0;
}

int16_t WindowGeometryTracker::Y() const {
  return parent_ ? parent_->Y() + y_ : 0;
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
      if (event->response_type & 0x80)
        return true;

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
      event_loop_->UnregisterDispatcher(this);
      return true;
    case XCB_GRAVITY_NOTIFY: {
      const auto* gravity = structure_event.gravity;

      if (gravity->event != window_)
        return false;
      if (event->response_type & 0x80)
        return true;

      x_ = gravity->x;
      y_ = gravity->y;
      observer_->WindowPositionChanged();

      return true;
    }
    case XCB_MAP_NOTIFY:
      return structure_event.map->event == window_;
    case XCB_REPARENT_NOTIFY: {
      const auto* reparent = structure_event.reparent;

      if (reparent->event != window_)
        return false;
      if (event->response_type & 0x80)
        return true;

      SetParent(reparent->parent);
      observer_->WindowPositionChanged();

      return true;
    }
    case XCB_UNMAP_NOTIFY:
      return structure_event.unmap->event == window_;
  }
  return false;
}

void WindowGeometryTracker::WindowPositionChanged() {
  observer_->WindowPositionChanged();
}

void WindowGeometryTracker::SetParent(xcb_window_t parent) {
  if (parent == XCB_WINDOW_NONE) {
    parent_ = nullptr;
  } else {
    parent_ = std::make_unique<WindowGeometryTracker>(connection_, event_loop_,
                                                      parent, this);
  }
}
