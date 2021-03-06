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

#include "connection.h"

#include <xcb/xcb.h>

#include <array>
#include <memory>
#include <string>
#include <utility>

namespace {

auto ScreenOfConnection(xcb_connection_t* c, int screen) -> xcb_screen_t* {
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem != 0; --screen, xcb_screen_next(&iter)) {
    if (screen == 0) {
      return iter.data;
    }
  }

  return nullptr;
}

void SetEventMask(xcb_connection_t* connection,
                  const xcb_window_t& window,
                  uint32_t new_mask) {
  auto cookie = xcb_change_window_attributes(connection, window,
                                             XCB_CW_EVENT_MASK, &new_mask);
  // Window |window| may already be destroyed at this point, so the
  // change_attributes request may give a BadWindow error.  In this case, just
  // ignore the error.
  xcb_discard_reply(connection, cookie.sequence);
}

}  // namespace

class Connection::MultiMask {
 public:
  MultiMask() = default;

  ~MultiMask() = default;

  void AddMask(uint32_t mask) {
    uint32_t mask_bit = 1;
    for (auto& mask_count : mask_counts_) {
      if ((mask & mask_bit) != XCB_EVENT_MASK_NO_EVENT) {
        mask_count++;
      }
      mask_bit <<= 1U;
    }
  }

  void RemoveMask(uint32_t mask) {
    uint32_t mask_bit = 1;
    for (auto& mask_count : mask_counts_) {
      if ((mask & mask_bit) != XCB_EVENT_MASK_NO_EVENT) {
        DCHECK(mask_count > 0);
        mask_count--;
      }
      mask_bit <<= 1U;
    }
  }

  [[nodiscard]] auto ToMask() const -> uint32_t {
    uint32_t mask = XCB_EVENT_MASK_NO_EVENT;
    uint32_t mask_bit = 1;
    for (auto mask_count : mask_counts_) {
      if (mask_count > 0) {
        mask |= mask_bit;
      }
      mask_bit <<= 1U;
    }
    return mask;
  }

 private:
  static constexpr auto kMaskSize = 25;

  std::array<unsigned int, kMaskSize> mask_counts_;

  DELETE_SPECIAL_MEMBERS(MultiMask);
};

Connection::Connection() {
  int screen_number;
  connection_ = xcb_connect(nullptr, &screen_number);
  if (int error = xcb_connection_has_error(connection_)) {
    throw XError("XCB connection error code " + std::to_string(error));
  }

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  if (screen == nullptr) {
    throw XError("Could not get screen");
  }
  root_window_ = screen->root;
  if (root_window_ == 0U) {
    throw XError("Could not find root window");
  }
}

Connection::~Connection() {
  for (const auto& mask_pair : mask_map_) {
    SetEventMask(connection_, mask_pair.first, XCB_EVENT_MASK_NO_EVENT);
  }
  xcb_flush(connection_);
  xcb_disconnect(connection_);
}

auto Connection::GenerateId() -> uint32_t {
  return xcb_generate_id(connection_);
}

void Connection::SelectEvents(xcb_window_t window, uint32_t event_mask) {
  std::unique_ptr<MultiMask>& mask = mask_map_[window];
  if (!mask) {
    mask = std::make_unique<MultiMask>();
  }
  uint32_t old_mask = mask_map_[window]->ToMask();
  mask->AddMask(event_mask);
  AfterMaskChanged(window, old_mask);
}

void Connection::DeselectEvents(xcb_window_t window, uint32_t event_mask) {
  DCHECK(mask_map_.find(window) != mask_map_.end());
  std::unique_ptr<MultiMask>& mask = mask_map_[window];
  uint32_t old_mask = mask->ToMask();
  mask->RemoveMask(event_mask);
  AfterMaskChanged(window, old_mask);
}

void Connection::AfterMaskChanged(xcb_window_t window, uint32_t old_mask) {
  uint32_t new_mask = mask_map_[window]->ToMask();
  if (new_mask == old_mask) {
    return;
  }

  SetEventMask(connection_, window, new_mask);

  if (new_mask == XCB_EVENT_MASK_NO_EVENT) {
    mask_map_.erase(window);
  }
}
