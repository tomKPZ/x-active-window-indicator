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
                                             const xcb_window_t& window)
    : connection_(connection),
      event_loop_(event_loop),
      event_dispatcher_(this, event_loop_),
      window_(window) {
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
  connection_->DeselectEvents(window_, XCB_EVENT_MASK_STRUCTURE_NOTIFY);
}

int16_t WindowGeometryTracker::X() const {
  return parent_ ? CheckedCast<int16_t>(parent_->X() + x_) : 0;
}

int16_t WindowGeometryTracker::Y() const {
  return parent_ ? CheckedCast<int16_t>(parent_->Y() + y_) : 0;
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
  } structure_event{};
  structure_event.generic = event.event();

  switch (event.ResponseType()) {
    case XCB_CIRCULATE_NOTIFY:
      return structure_event.circulate->event == window_;
    case XCB_CONFIGURE_NOTIFY: {
      const auto* configure = structure_event.configure;

      if (configure->event != window_) {
        return false;
      }
      if (event.SendEvent()) {
        return true;
      }

      if (x_ != configure->x || y_ != configure->y) {
        x_ = configure->x;
        y_ = configure->y;
        for (auto* observer : observers()) {
          observer->WindowPositionChanged();
        }
      }

      if (width_ != configure->width || height_ != configure->height) {
        width_ = configure->width;
        height_ = configure->height;
        for (auto* observer : observers()) {
          observer->WindowSizeChanged();
        }
      }

      if (border_width_ != configure->border_width) {
        border_width_ = configure->border_width;
        for (auto* observer : observers()) {
          observer->WindowBorderWidthChanged();
        }
      }

      return true;
    }
    case XCB_DESTROY_NOTIFY:
      return structure_event.destroy->event == window_;
    case XCB_GRAVITY_NOTIFY: {
      const auto* gravity = structure_event.gravity;

      if (gravity->event != window_) {
        return false;
      }
      if (event.SendEvent()) {
        return true;
      }

      x_ = gravity->x;
      y_ = gravity->y;
      for (auto* observer : observers()) {
        observer->WindowPositionChanged();
      }

      return true;
    }
    case XCB_MAP_NOTIFY:
      return structure_event.map->event == window_;
    case XCB_REPARENT_NOTIFY: {
      const auto* reparent = structure_event.reparent;

      if (reparent->event != window_) {
        return false;
      }
      if (event.SendEvent()) {
        return true;
      }

      SetParent(reparent->parent);
      for (auto* observer : observers()) {
        observer->WindowPositionChanged();
      }

      return true;
    }
    case XCB_UNMAP_NOTIFY:
      return structure_event.unmap->event == window_;
  }
  return false;
}

void WindowGeometryTracker::WindowPositionChanged() {
  for (auto* observer : observers()) {
    observer->WindowPositionChanged();
  }
}

void WindowGeometryTracker::SetParent(xcb_window_t parent) {
  // |observer_| must be destroyed before |parent_|.  Reset them now
  // to prevent recreating them in the wrong order below.
  observer_.reset();
  parent_.reset();
  if (parent != XCB_WINDOW_NONE) {
    parent_ = std::make_unique<WindowGeometryTracker>(connection_, event_loop_,
                                                      parent);
    observer_ = std::make_unique<ScopedObserver<WindowGeometryObserver>>(
        this, parent_.get());
  }
}
