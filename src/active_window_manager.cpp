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

#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace {

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

template <typename T>
XcbReply<T> MakeXcbReply(T* t) {
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
  xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(connection, false, str.length(), str.c_str());
  auto reply = MakeXcbReply(xcb_intern_atom_reply(connection, cookie, nullptr));
  return reply ? reply->atom : XCB_ATOM_NONE;
}

std::optional<std::vector<xcb_atom_t>> GetAtomArray(xcb_connection_t* connection,
						    xcb_window_t window,
						    xcb_atom_t atom) {
  // TODO: these should be one template function.
  auto cookie = xcb_get_property(connection, false, window, atom,
                                 XCB_ATOM_ATOM, 0, std::numeric_limits<uint32_t>::max());
  auto reply =
      MakeXcbReply(xcb_get_property_reply(connection, cookie, nullptr));

  if (!reply || reply->format != 8*sizeof(xcb_atom_t) || reply->type != XCB_ATOM_ATOM ||
      reply->bytes_after > 0) {
    return std::nullopt;
  }

  std::optional<std::vector<xcb_atom_t>> ret{std::in_place};
  xcb_atom_t* value = reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(reply));
  auto size = xcb_get_property_value_length(reply);
  assert(size % sizeof(xcb_atom_t) == 0);
  for (decltype(size) i = 0; i < size/sizeof(xcb_atom_t); i++) {
    std::cout << "1: " << value[i] << std::endl;
    ret.value().emplace_back(value[i]);
  }
  return ret;
}

std::optional<xcb_window_t> GetWindow(xcb_connection_t* connection,
				      xcb_window_t window,
				      xcb_atom_t atom) {
  auto cookie = xcb_get_property(connection, false, window, atom,
                                 XCB_ATOM_WINDOW, 0, sizeof(xcb_window_t));

  auto reply =
      MakeXcbReply(xcb_get_property_reply(connection, cookie, nullptr));

  if (!reply || reply->format != 8 * sizeof(xcb_window_t) ||
      reply->type != XCB_ATOM_WINDOW || reply->bytes_after > 0 ||
      xcb_get_property_value_length(reply) != sizeof(xcb_window_t)) {
    return std::nullopt;
  }

  return reinterpret_cast<xcb_window_t*>(xcb_get_property_value(reply))[0];
}

}  // namespace

ActiveWindowManager::ActiveWindowManager() {
  int screen_number;
  connection_ = xcb_connect(nullptr, &screen_number);
  if (xcb_connection_has_error(connection_))
    return;

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  root_window_ = screen ? screen->root : XCB_WINDOW_NONE;
  if (!root_window_)
    return;

  net_supported_ = GetAtom(connection_, "_NET_SUPPORTED");
  net_active_window_ = GetAtom(connection_, "_NET_ACTIVE_WINDOW");

  for (xcb_atom_t atom : GetAtomArray(connection_, root_window_, net_supported_).value())
    std::cout << "2: " << atom << std::endl;

  auto window = GetWindow(connection_, root_window_, net_active_window_);
  // std::cout << window.value_or(0) << std::endl;
}

ActiveWindowManager::~ActiveWindowManager() {
  xcb_disconnect(connection_);
}
