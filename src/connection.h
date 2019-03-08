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

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <memory>
#include <unordered_map>

#include "util.h"
#include "x_error.h"

#define XCB_SYNC(func, c, ...)  \
  XcbSyncAux((c), func##_reply, \
             func((c)->connection() __VA_OPT__(, ) __VA_ARGS__))

typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;

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

class Connection {
 public:
  Connection();
  ~Connection();

  uint32_t GenerateId();
  void SelectEvents(xcb_window_t window, uint32_t event_mask);
  void DeselectEvents(xcb_window_t window, uint32_t event_mask);

  xcb_connection_t* connection() { return connection_; }
  xcb_window_t root_window() const { return root_window_; }

 private:
  class MultiMask;

  void AfterMaskChanged(xcb_window_t window, uint32_t old_mask);

  xcb_connection_t* connection_;
  xcb_window_t root_window_;

  std::unordered_map<xcb_window_t, std::unique_ptr<MultiMask>> mask_map_;

  DISALLOW_COPY_AND_ASSIGN(Connection);
};

template <typename Cookie, typename ReplyFunc>
auto XcbSyncAux(Connection* connection, ReplyFunc reply_func, Cookie cookie)
    -> decltype(auto) {
  xcb_generic_error_t* error = nullptr;
  auto* t = reply_func(connection->connection(), cookie, &error);
  if (error)
    throw XError(*error);
  assert(t);
  return XcbReply(t);
}
