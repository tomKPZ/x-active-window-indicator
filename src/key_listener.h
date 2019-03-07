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

// TODO: any way to forward declare xcb_input_xi_event_mask_t?
#include <xcb/xinput.h>

#include <cstdint>
#include <vector>

#include "event_dispatcher.h"
#include "util.h"

class Connection;
class KeyStateObserver;

class KeyListener : public EventDispatcher {
 public:
  KeyListener(Connection* connection, KeyStateObserver* observer);
  ~KeyListener() override;

 protected:
  // EventDispatcher:
  bool DispatchEvent(const Event& event) override;

 private:
  struct KeyCodeState {
    KeyCodeState(uint32_t key_code) : code(key_code) {}
    const uint32_t code;

    // TODO: Any way to get initial key states?
    bool key_pressed = false;
  };

  void SelectEvents(xcb_input_xi_event_mask_t event_mask);

  // TODO: Don't hardcode these keycodes.
  std::vector<KeyCodeState> key_code_states_{133, 134};
  bool any_key_pressed_ = false;

  Connection* connection_;
  KeyStateObserver* observer_;

  uint8_t xcb_input_major_opcode_;

  DISALLOW_COPY_AND_ASSIGN(KeyListener);
};
