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

#include "active_window_tracker.h"

#include <xcb/xcb.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "active_window_observer.h"
#include "connection.h"
#include "event.h"
#include "util.h"

namespace {

xcb_atom_t GetAtom(Connection* connection, const std::string& str) {
  return XCB_SYNC(xcb_intern_atom, connection->connection(), false,
                  safe_cast<uint16_t>(str.length()), str.c_str())
      ->atom;
}

std::vector<xcb_atom_t> GetAtomArray(Connection* connection,
                                     xcb_window_t window,
                                     xcb_atom_t atom) {
  auto reply =
      XCB_SYNC(xcb_get_property, connection->connection(), false, window, atom,
               XCB_ATOM_ATOM, 0, std::numeric_limits<uint32_t>::max());

  if (reply->format != 8 * sizeof(xcb_atom_t) || reply->type != XCB_ATOM_ATOM ||
      reply->bytes_after > 0) {
    throw "Bad property reply";
  }

  const xcb_atom_t* value =
      reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(reply));
  return std::vector<xcb_atom_t>(value, value + reply->value_len);
}

xcb_window_t GetWindow(Connection* connection,
                       xcb_window_t window,
                       xcb_atom_t atom) {
  auto reply = XCB_SYNC(xcb_get_property, connection->connection(), false,
                        window, atom, XCB_ATOM_WINDOW, 0, sizeof(xcb_window_t));

  if (reply->format != 8 * sizeof(xcb_window_t) ||
      reply->type != XCB_ATOM_WINDOW || reply->bytes_after > 0 ||
      xcb_get_property_value_length(reply) != sizeof(xcb_window_t)) {
    throw "Bad property reply";
  }

  return reinterpret_cast<xcb_window_t*>(xcb_get_property_value(reply))[0];
}

}  // namespace

ActiveWindowTracker::ActiveWindowTracker(Connection* connection,
                                         ActiveWindowObserver* observer)
    : connection_(connection), observer_(observer) {
  xcb_atom_t net_supported = GetAtom(connection_, "_NET_SUPPORTED");
  net_active_window_ = GetAtom(connection_, "_NET_ACTIVE_WINDOW");

  auto atoms =
      GetAtomArray(connection_, connection_->root_window(), net_supported);
  if (std::find(atoms.begin(), atoms.end(), net_active_window_) == atoms.end())
    throw "WM does not support active window";

  active_window_ =
      GetWindow(connection_, connection_->root_window(), net_active_window_);
  observer_->ActiveWindowChanged(active_window_);
}

ActiveWindowTracker::~ActiveWindowTracker() {}