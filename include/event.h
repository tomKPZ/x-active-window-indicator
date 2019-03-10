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

#ifndef EVENT_H
#define EVENT_H

// TODO(tomKPZ): any way to forward declare xcb_generic_event_t?
#include <xcb/xcb.h>

#include <cstdint>

#include "util.h"

class Event {
 public:
  // Takes ownership of |event|.
  explicit Event(xcb_generic_event_t* event);
  ~Event();

  bool SendEvent() const;
  uint8_t ResponseType() const;
  uint16_t Sequence() const;

  explicit operator bool() const { return event_ != nullptr; }
  const xcb_generic_event_t* event() const { return event_; }

 private:
  xcb_generic_event_t* event_;

  DISALLOW_COPY_AND_ASSIGN(Event);
};

#endif
