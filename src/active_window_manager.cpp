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

#include "active_window_manager.h"

#include <xcb/xcb.h>
#include <xcb/xfixes.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#define XCB_SYNC(func, connection, ...) \
  MakeXcbReply(func##_reply(            \
      connection, func(connection __VA_OPT__(, ) __VA_ARGS__), nullptr))

namespace {

static constexpr const uint16_t BORDER_WIDTH = 5;

template <typename Dst, typename Src>
constexpr Dst safe_cast(Src value) {
  if (value < std::numeric_limits<Dst>::min() ||
      value > std::numeric_limits<Dst>::max()) {
    throw "Cast out of range";
  }
  return static_cast<Dst>(value);
}

template <typename T>
class XcbReply {
 public:
  XcbReply(T* t) : t_(t) {}
  ~XcbReply() { free(t_); }
  operator const T*() const { return t_; }
  const T* operator->() const { return t_; }

 private:
  T* t_;
};

class XcbRegion {
 public:
  XcbRegion(xcb_connection_t* connection,
            const std::vector<xcb_rectangle_t> rects)
      : connection_(connection), id_(xcb_generate_id(connection_)) {
    xcb_xfixes_create_region(connection_, id_,
                             safe_cast<uint32_t>(rects.size()), rects.data());
  }
  ~XcbRegion() { xcb_xfixes_destroy_region(connection_, id_); }
  uint32_t id() const { return id_; }

 private:
  xcb_connection_t* connection_;
  uint32_t id_;
};

template <typename T>
XcbReply<T> MakeXcbReply(T* t) {
  if (!t)
    throw "Could not get reply";
  return XcbReply(t);
}

xcb_screen_t* ScreenOfConnection(xcb_connection_t* c, int screen) {
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem; --screen, xcb_screen_next(&iter))
    if (screen == 0)
      return iter.data;

  return nullptr;
}

xcb_atom_t GetAtom(xcb_connection_t* connection, const std::string& str) {
  return XCB_SYNC(xcb_intern_atom, connection, false,
                  safe_cast<uint16_t>(str.length()), str.c_str())
      ->atom;
}

std::vector<xcb_atom_t> GetAtomArray(xcb_connection_t* connection,
                                     xcb_window_t window,
                                     xcb_atom_t atom) {
  auto reply = XCB_SYNC(xcb_get_property, connection, false, window, atom,
                        XCB_ATOM_ATOM, 0, std::numeric_limits<uint32_t>::max());

  if (reply->format != 8 * sizeof(xcb_atom_t) || reply->type != XCB_ATOM_ATOM ||
      reply->bytes_after > 0) {
    throw "Bad property reply";
  }

  const xcb_atom_t* value =
      reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(reply));
  return std::vector<xcb_atom_t>(value, value + reply->value_len);
}

xcb_window_t GetWindow(xcb_connection_t* connection,
                       xcb_window_t window,
                       xcb_atom_t atom) {
  auto reply = XCB_SYNC(xcb_get_property, connection, false, window, atom,
                        XCB_ATOM_WINDOW, 0, sizeof(xcb_window_t));

  if (reply->format != 8 * sizeof(xcb_window_t) ||
      reply->type != XCB_ATOM_WINDOW || reply->bytes_after > 0 ||
      xcb_get_property_value_length(reply) != sizeof(xcb_window_t)) {
    throw "Bad property reply";
  }

  return reinterpret_cast<xcb_window_t*>(xcb_get_property_value(reply))[0];
}

}  // namespace

ActiveWindowManager::ActiveWindowManager() {
  int screen_number;
  connection_ = xcb_connect(nullptr, &screen_number);
  if (xcb_connection_has_error(connection_))
    throw "Connection error";

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  if (!screen)
    throw "Could not get screen";
  root_window_ = screen->root;
  if (!root_window_)
    throw "Could not find root window";

  net_supported_ = GetAtom(connection_, "_NET_SUPPORTED");
  net_active_window_ = GetAtom(connection_, "_NET_ACTIVE_WINDOW");

  auto atoms = GetAtomArray(connection_, root_window_, net_supported_);
  if (std::find(atoms.begin(), atoms.end(), net_active_window_) == atoms.end())
    throw "WM does not support active window";

  auto window = GetWindow(connection_, root_window_, net_active_window_);
  auto geometry = XCB_SYNC(xcb_get_geometry, connection_, window);

  auto root_coordinates =
      XCB_SYNC(xcb_translate_coordinates, connection_, window, root_window_,
               geometry->x, geometry->y);

  auto border_window = xcb_generate_id(connection_);
  uint32_t attributes[] = {0xff0000, true};
  xcb_create_window(
      connection_, XCB_COPY_FROM_PARENT, border_window, root_window_,
      root_coordinates->dst_x, root_coordinates->dst_y,
      safe_cast<uint16_t>(geometry->width - 2 * BORDER_WIDTH),
      safe_cast<uint16_t>(geometry->height - 2 * BORDER_WIDTH), BORDER_WIDTH,
      XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
      XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT, attributes);

  XCB_SYNC(xcb_xfixes_query_version, connection_, XCB_XFIXES_MAJOR_VERSION,
           XCB_XFIXES_MINOR_VERSION);

  const std::vector<xcb_rectangle_t> rects{
      // Top edge.
      {safe_cast<int16_t>(geometry->x - BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y - BORDER_WIDTH), geometry->width,
       BORDER_WIDTH},
      // Bottom edge.
      {safe_cast<int16_t>(geometry->x - BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y + geometry->height - 2 * BORDER_WIDTH),
       geometry->width, BORDER_WIDTH},
      // Left edge.
      {safe_cast<int16_t>(geometry->x - BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y - BORDER_WIDTH), BORDER_WIDTH,
       geometry->height},
      // Right edge.
      {safe_cast<int16_t>(geometry->x + geometry->width - 2 * BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y - BORDER_WIDTH), BORDER_WIDTH,
       geometry->height},
  };
  xcb_xfixes_set_window_shape_region(connection_, border_window,
                                     XCB_SHAPE_SK_BOUNDING, 0, 0,
                                     XcbRegion(connection_, rects).id());

  xcb_map_window(connection_, border_window);

  xcb_flush(connection_);
  while (true)
    xcb_wait_for_event(connection_);
}

ActiveWindowManager::~ActiveWindowManager() {
  xcb_disconnect(connection_);
}
