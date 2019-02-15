// rofi-workspace: An X11 workspace plugin for rofi
// Copyright (C) 2018 <tomKPZ@gmail.com>
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

#include "workspace_manager.h"

#include <xcb/xcb.h>

#include <cstring>
#include <limits>

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

std::optional<uint32_t> GetCardinal(xcb_connection_t* connection,
                                    xcb_window_t window,
                                    xcb_atom_t atom) {
  auto cookie = xcb_get_property(connection, false, window, atom,
                                 XCB_ATOM_CARDINAL, 0, sizeof(uint32_t));

  auto reply =
      MakeXcbReply(xcb_get_property_reply(connection, cookie, nullptr));

  if (!reply || reply->format != 8 * sizeof(uint32_t) ||
      reply->type != XCB_ATOM_CARDINAL || reply->bytes_after > 0 ||
      xcb_get_property_value_length(reply) != sizeof(uint32_t)) {
    return std::nullopt;
  }

  return reinterpret_cast<uint32_t*>(xcb_get_property_value(reply))[0];
}

std::optional<std::vector<std::string>> GetUtf8Array(
    xcb_connection_t* connection,
    xcb_window_t window,
    xcb_atom_t atom,
    xcb_atom_t type) {
  auto cookie = xcb_get_property(connection, false, window, atom, type, 0,
                                 std::numeric_limits<uint32_t>::max());

  auto reply =
      MakeXcbReply(xcb_get_property_reply(connection, cookie, nullptr));

  if (!reply || reply->format != 8 || reply->type != type ||
      reply->bytes_after > 0) {
    return std::nullopt;
  }

  std::optional<std::vector<std::string>> ret{std::in_place};
  char* value = reinterpret_cast<char*>(xcb_get_property_value(reply));
  int size = xcb_get_property_value_length(reply);
  for (int i = 0; i < size;) {
    ret.value().emplace_back(value + i);
    i += ret.value().back().length() + 1;
  }
  return ret;
}

}  // namespace

WorkspaceManager::WorkspaceManager() {
  int screen_number;
  connection_ = xcb_connect(nullptr, &screen_number);
  if (xcb_connection_has_error(connection_))
    return;

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  root_window_ = screen ? screen->root : XCB_WINDOW_NONE;
  if (!root_window_)
    return;

  net_current_desktop_ = GetAtom(connection_, "_NET_CURRENT_DESKTOP");
  net_desktop_names_ = GetAtom(connection_, "_NET_DESKTOP_NAMES");
  net_number_of_desktops_ = GetAtom(connection_, "_NET_NUMBER_OF_DESKTOPS");
  utf8_string_ = GetAtom(connection_, "UTF8_STRING");
}

WorkspaceManager::~WorkspaceManager() {
  xcb_disconnect(connection_);
}

std::vector<WorkspaceManager::Workspace> WorkspaceManager::GetWorkspaces() {
  auto number_of_desktops =
      GetCardinal(connection_, root_window_, net_number_of_desktops_);

  auto arr =
      GetUtf8Array(connection_, root_window_, net_desktop_names_, utf8_string_);

  size_t real_number_of_desktops =
      number_of_desktops ? number_of_desktops.value() : 0;
  if (arr && arr.value().size() > 0)
    real_number_of_desktops =
        std::min(real_number_of_desktops, arr.value().size());

  std::vector<Workspace> ret{real_number_of_desktops};
  if (arr) {
    for (size_t i = 0; i < real_number_of_desktops && i < arr.value().size();
         ++i) {
      ret[i].name = arr.value()[i];
    }
  }

  auto current_desktop =
      GetCardinal(connection_, root_window_, net_current_desktop_);
  if (current_desktop && current_desktop.value() < ret.size()) {
    ret[current_desktop.value()].current = true;
  }

  return ret;
}
