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

// TODO(tomKPZ): any way to forward declare xcb_generic_event_t?
#include <xcb/xcb.h>

#include <cstdint>
#include <memory>

#include "util.h"

class Event {
 public:
  // Takes ownership of |event|.
  explicit Event(xcb_generic_event_t* event);
  ~Event();

  [[nodiscard]] auto SendEvent() const -> bool;
  [[nodiscard]] auto ResponseType() const -> uint8_t;
  [[nodiscard]] auto Sequence() const -> uint16_t;

  explicit operator bool() const { return event_ != nullptr; }
  [[nodiscard]] auto event() const -> const xcb_generic_event_t* {
    return event_.get();
  }

 private:
  std::unique_ptr<xcb_generic_event_t, FreeDeleter> event_;

  DELETE_SPECIAL_MEMBERS(Event);
};
