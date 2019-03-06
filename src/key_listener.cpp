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
#include "key_state_observer.h"

KeyListener::KeyListener(Connection* connection, KeyStateObserver* observer)
    : connection_(connection), observer_(observer) {
  XCB_SYNC(xcb_input_xi_query_version, connection_->connection(),
           XCB_INPUT_MAJOR_VERSION, XCB_INPUT_MINOR_VERSION);
  static constexpr const struct {
    xcb_input_event_mask_t event_mask;
    xcb_input_xi_event_mask_t xi_event_mask;
  } masks[] = {{{XCB_INPUT_DEVICE_ALL,
                 sizeof(xcb_input_xi_event_mask_t) / sizeof(uint32_t)},
                static_cast<xcb_input_xi_event_mask_t>(
                    XCB_INPUT_XI_EVENT_MASK_KEY_PRESS |
                    XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE)}};
  xcb_input_xi_select_events(connection_->connection(),
                             connection_->root_window(), ArraySize(masks),
                             &masks[0].event_mask);
}

KeyListener::~KeyListener() {}

bool KeyListener::DispatchEvent(const Event& event) {
  // TODO: verify ->present.
  auto xcb_input_major_opcode =
      xcb_get_extension_data(connection_->connection(), &xcb_input_id)
          ->major_opcode;
  if ((event->response_type & ~0x80) != XCB_GE_GENERIC)
    return false;

  const auto* generic_event =
      reinterpret_cast<const xcb_ge_generic_event_t*>(event.event());
  if (generic_event->extension != xcb_input_major_opcode)
    return false;

  if (generic_event->event_type == XCB_INPUT_KEY_PRESS) {
    const auto* key_press_event =
        reinterpret_cast<const xcb_input_key_press_event_t*>(generic_event);
    const auto key = key_press_event->detail;
    auto it =
        std::find_if(std::begin(key_code_states), std::end(key_code_states),
                     [key](const KeyCodeState& key_code_state) {
                       return key_code_state.code == key;
                     });
    if (it != std::end(key_code_states))
      it->key_pressed = true;
  } else if (generic_event->event_type == XCB_INPUT_KEY_RELEASE) {
    const auto* key_release_event =
        reinterpret_cast<const xcb_input_key_release_event_t*>(generic_event);
    const auto key = key_release_event->detail;
    auto it =
        std::find_if(std::begin(key_code_states), std::end(key_code_states),
                     [key](const KeyCodeState& key_code_state) {
                       return key_code_state.code == key;
                     });
    if (it != std::end(key_code_states))
      it->key_pressed = false;
  } else {
    // TODO: figure out return values
  }

  const bool new_any_key =
      std::any_of(std::begin(key_code_states), std::end(key_code_states),
                  [](const KeyCodeState& key_code_state) {
                    return key_code_state.key_pressed;
                  });
  if (any_key != new_any_key)
    observer_->KeyStateChanged(new_any_key);
  any_key = new_any_key;
  return true;
}
