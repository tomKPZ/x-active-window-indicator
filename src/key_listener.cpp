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

#include "key_listener.h"

#include <xcb/xcb.h>
#include <xcb/xinput.h>

#include <algorithm>
#include <iterator>

#include "connection.h"
#include "event.h"
#include "event_loop.h"
#include "key_state_observer.h"
#include "x_error.h"

namespace {

void SelectEvents(Connection* connection,
                  xcb_input_xi_event_mask_t event_mask) {
  const struct {
    xcb_input_event_mask_t event_mask;
    xcb_input_xi_event_mask_t xi_event_mask;
  } mask[] = {{{XCB_INPUT_DEVICE_ALL,
                sizeof(xcb_input_xi_event_mask_t) / sizeof(uint32_t)},
               event_mask}};
  xcb_input_xi_select_events(connection->connection(),
                             connection->root_window(), ArraySize(mask),
                             &mask[0].event_mask);
}

}  // namespace

KeyListener::KeyListener(Connection* connection, EventLoop* event_loop)
    : connection_(connection), dispatcher_(this, event_loop) {
  XCB_SYNC(xcb_input_xi_query_version, connection_, XCB_INPUT_MAJOR_VERSION,
           XCB_INPUT_MINOR_VERSION);
  auto* input_extension =
      xcb_get_extension_data(connection_->connection(), &xcb_input_id);
  if (!input_extension->present) {
    throw XError("XINPUT not available");
  }
  xcb_input_major_opcode_ = input_extension->major_opcode;

  SelectEvents(connection_, static_cast<xcb_input_xi_event_mask_t>(
                                XCB_INPUT_XI_EVENT_MASK_KEY_PRESS |
                                XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE));
}

KeyListener::~KeyListener() {
  SelectEvents(connection_, static_cast<xcb_input_xi_event_mask_t>(0));
}

bool KeyListener::DispatchEvent(const Event& event) {
  if (event.ResponseType() != XCB_GE_GENERIC) {
    return false;
  }

  const auto* generic_event =
      reinterpret_cast<const xcb_ge_generic_event_t*>(event.event());
  if (generic_event->extension != xcb_input_major_opcode_) {
    return false;
  }

  bool press;
  if (generic_event->event_type == XCB_INPUT_KEY_PRESS) {
    press = true;
  } else if (generic_event->event_type == XCB_INPUT_KEY_RELEASE) {
    press = false;
  } else {
    return false;
  }

  const auto* key_event =
      reinterpret_cast<const xcb_input_key_press_event_t*>(generic_event);
  const auto key = key_event->detail;
  auto it =
      std::find_if(std::begin(key_code_states_), std::end(key_code_states_),
                   [key](const KeyCodeState& key_code_state) {
                     return key_code_state.code == key;
                   });
  if (it == std::end(key_code_states_)) {
    return true;
  } else {
    it->key_pressed = press;
  }

  const bool any_key_pressed =
      std::any_of(std::begin(key_code_states_), std::end(key_code_states_),
                  [](const KeyCodeState& key_code_state) {
                    return key_code_state.key_pressed;
                  });
  if (any_key_pressed != any_key_pressed_) {
    any_key_pressed_ = any_key_pressed;
    for (auto* observer : observers()) {
      observer->KeyStateChanged();
    }
  }
  return true;
}
