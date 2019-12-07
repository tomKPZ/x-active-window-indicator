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

#include "event.h"

namespace {

constexpr const uint8_t kSendEventMask = 0x80U;
constexpr const uint8_t kResponseTypeMask =
    static_cast<uint8_t>(~kSendEventMask);

}  // namespace

Event::Event(xcb_generic_event_t* event) : event_(event) {}

Event::~Event() = default;

auto Event::SendEvent() const -> bool {
  return (event_->response_type & kSendEventMask) != 0;
}

auto Event::ResponseType() const -> uint8_t {
  return event_->response_type & kResponseTypeMask;
}

auto Event::Sequence() const -> uint16_t {
  return event_->sequence;
}
