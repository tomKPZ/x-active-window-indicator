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
#include <xcb/xfixes.h>
#include <xcb/xinput.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "connection.h"

#define XCB_SYNC(func, connection, ...) \
  MakeXcbReply(func##_reply(            \
      connection, func(connection __VA_OPT__(, ) __VA_ARGS__), nullptr))

namespace {

static constexpr const uint16_t BORDER_WIDTH = 5;

template <typename Dst, typename Src>
constexpr Dst safe_cast(Src value) {
  if (value < std::numeric_limits<Dst>::min() ||
      value > std::numeric_limits<Dst>::max()) {
    throw "Cast out of range";
  }
  return static_cast<Dst>(value);
}

template <typename T>
constexpr size_t ArraySize(const T& array) {
  return sizeof(array) / sizeof(array[0]);
}

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

class XcbRegion {
 public:
  XcbRegion(Connection* connection, const std::vector<xcb_rectangle_t> rects)
      : connection_(connection),
        id_(connection_->GenerateId())) {
    xcb_xfixes_create_region(connection_->connection(), id_,
                             safe_cast<uint32_t>(rects.size()), rects.data());
  }
  ~XcbRegion() { xcb_xfixes_destroy_region(connection_->connection(), id_); }
  uint32_t id() const { return id_; }

 private:
  Connection* connection_;
  uint32_t id_;
};

template <typename T>
XcbReply<T> MakeXcbReply(T* t) {
  if (!t)
    throw "Error getting reply";
  return XcbReply(t);
}

class XcbEvent {
 public:
  XcbEvent(xcb_generic_event_t* event) : event_(event) {}
  ~XcbEvent() { free(event_); }
  operator bool() const { return event_; }
  const xcb_generic_event_t* operator->() const { return event_; }
  const xcb_generic_event_t* event() const { return event_; }

 private:
  xcb_generic_event_t* event_;
};

XcbEvent WaitForEvent(Connection* connection) {
  return XcbEvent(xcb_wait_for_event(connection->connection()));
}

xcb_atom_t GetAtom(Connection* connection, const std::string& str) {
  return XCB_SYNC(xcb_intern_atom, connection->connection(), false,
                  safe_cast<uint16_t>(str.length()), str.c_str())
      ->atom;
}

std::vector<xcb_atom_t> GetAtomArray(Connection* connection,
                                     xcb_window_t window,
                                     xcb_atom_t atom) {
  auto reply =
      XCB_SYNC(xcb_get_property, connection->connection(), false, window, atom,
               XCB_ATOM_ATOM, 0, std::numeric_limits<uint32_t>::max());

  if (reply->format != 8 * sizeof(xcb_atom_t) || reply->type != XCB_ATOM_ATOM ||
      reply->bytes_after > 0) {
    throw "Bad property reply";
  }

  const xcb_atom_t* value =
      reinterpret_cast<xcb_atom_t*>(xcb_get_property_value(reply));
  return std::vector<xcb_atom_t>(value, value + reply->value_len);
}

xcb_window_t GetWindow(Connection* connection,
                       xcb_window_t window,
                       xcb_atom_t atom) {
  auto reply = XCB_SYNC(xcb_get_property, connection->connection(), false,
                        window, atom, XCB_ATOM_WINDOW, 0, sizeof(xcb_window_t));

  if (reply->format != 8 * sizeof(xcb_window_t) ||
      reply->type != XCB_ATOM_WINDOW || reply->bytes_after > 0 ||
      xcb_get_property_value_length(reply) != sizeof(xcb_window_t)) {
    throw "Bad property reply";
  }

  return reinterpret_cast<xcb_window_t*>(xcb_get_property_value(reply))[0];
}

}  // namespace

ActiveWindowManager::ActiveWindowManager(Connection* connection)
    : connection_(connection) {
  net_supported_ = GetAtom(connection_, "_NET_SUPPORTED");
  net_active_window_ = GetAtom(connection_, "_NET_ACTIVE_WINDOW");

  auto atoms =
      GetAtomArray(connection_, connection_->root_window(), net_supported_);
  if (std::find(atoms.begin(), atoms.end(), net_active_window_) == atoms.end())
    throw "WM does not support active window";

  auto window =
      GetWindow(connection_, connection_->root_window(), net_active_window_);
  auto geometry = XCB_SYNC(xcb_get_geometry, connection_->connection(), window);

  auto root_coordinates =
      XCB_SYNC(xcb_translate_coordinates, connection_->connection(), window,
               connection_->root_window(), geometry->x, geometry->y);

  auto border_window = connection_->GenerateId();
  uint32_t attributes[] = {0xff0000, true};
  xcb_create_window(connection_->connection(), XCB_COPY_FROM_PARENT,
                    border_window, connection_->root_window(),
                    root_coordinates->dst_x, root_coordinates->dst_y,
                    safe_cast<uint16_t>(geometry->width - 2 * BORDER_WIDTH),
                    safe_cast<uint16_t>(geometry->height - 2 * BORDER_WIDTH),
                    BORDER_WIDTH, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    XCB_COPY_FROM_PARENT,
                    XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT, attributes);

  XCB_SYNC(xcb_xfixes_query_version, connection_->connection(),
           XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);

  // TODO: Use an outer border instead of an inner border if the window is tiny.
  const std::vector<xcb_rectangle_t> rects{
      // Top edge.
      {safe_cast<int16_t>(geometry->x - BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y - BORDER_WIDTH), geometry->width,
       BORDER_WIDTH},
      // Bottom edge.
      {safe_cast<int16_t>(geometry->x - BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y + geometry->height - 2 * BORDER_WIDTH),
       geometry->width, BORDER_WIDTH},
      // Left edge.
      {safe_cast<int16_t>(geometry->x - BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y - BORDER_WIDTH), BORDER_WIDTH,
       geometry->height},
      // Right edge.
      {safe_cast<int16_t>(geometry->x + geometry->width - 2 * BORDER_WIDTH),
       safe_cast<int16_t>(geometry->y - BORDER_WIDTH), BORDER_WIDTH,
       geometry->height},
  };
  xcb_xfixes_set_window_shape_region(connection_->connection(), border_window,
                                     XCB_SHAPE_SK_BOUNDING, 0, 0,
                                     XcbRegion(connection_, rects).id());

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
  xcb_flush(connection_->connection());

  struct KeyCodeState {
    KeyCodeState(uint32_t key_code) : code(key_code) {}
    const uint32_t code;

    // TODO: Any way to get initial key states?
    bool key_pressed = false;
  };

  // TODO: Don't hardcode these keycodes.
  std::vector<KeyCodeState> key_code_states{133, 134};

  bool any_key = false;

  // TODO: verify ->present.
  auto xcb_input_major_opcode =
      xcb_get_extension_data(connection_->connection(), &xcb_input_id)
          ->major_opcode;
  while (auto event = WaitForEvent(connection_)) {
    switch (event->response_type & ~0x80) {
      case XCB_GE_GENERIC: {
        const auto* generic_event =
            reinterpret_cast<const xcb_ge_generic_event_t*>(event.event());
        if (generic_event->extension == xcb_input_major_opcode) {
          if (generic_event->event_type == XCB_INPUT_KEY_PRESS) {
            const auto* key_press_event =
                reinterpret_cast<const xcb_input_key_press_event_t*>(
                    generic_event);
            const auto key = key_press_event->detail;
            auto it = std::find_if(std::begin(key_code_states),
                                   std::end(key_code_states),
                                   [key](const KeyCodeState& key_code_state) {
                                     return key_code_state.code == key;
                                   });
            if (it != std::end(key_code_states))
              it->key_pressed = true;
          } else if (generic_event->event_type == XCB_INPUT_KEY_RELEASE) {
            const auto* key_release_event =
                reinterpret_cast<const xcb_input_key_release_event_t*>(
                    generic_event);
            const auto key = key_release_event->detail;
            auto it = std::find_if(std::begin(key_code_states),
                                   std::end(key_code_states),
                                   [key](const KeyCodeState& key_code_state) {
                                     return key_code_state.code == key;
                                   });
            if (it != std::end(key_code_states))
              it->key_pressed = false;
          }
        }
        const bool new_any_key =
            std::any_of(std::begin(key_code_states), std::end(key_code_states),
                        [](const KeyCodeState& key_code_state) {
                          return key_code_state.key_pressed;
                        });
        any_key = new_any_key;
        if (any_key)
          xcb_map_window(connection_->connection(), border_window);
        else
          xcb_unmap_window(connection_->connection(), border_window);
        break;
      }
      default:
        throw "Unhandled event";
    }
    xcb_flush(connection_->connection());
  }
}

ActiveWindowManager::~ActiveWindowManager() {}
