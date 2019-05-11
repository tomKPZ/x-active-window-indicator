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

#include <cstdint>
#include <vector>

#include "event_dispatcher.h"
#include "observable.h"
#include "scoped_observer.h"
#include "util.h"

class Connection;
class Event;
class EventLoop;
class KeyStateObserver;

class KeyListener : public EventDispatcher,
                    public Observable<KeyStateObserver> {
 public:
  KeyListener(Connection* connection, EventLoop* event_loop);
  ~KeyListener() override;

  [[nodiscard]] bool any_key_pressed() const { return any_key_pressed_; }

 protected:
  // EventDispatcher:
  bool DispatchEvent(const Event& event) override;

 private:
  // TODO(tomKPZ): Don't hardcode these keycodes.
  constexpr static uint32_t kWinKeyLeft = 133;
  constexpr static uint32_t kWinKeyRight = 134;

  class KeyCodeState {
   public:
    explicit KeyCodeState(uint32_t key_code) : code_(key_code) {}

    [[nodiscard]] uint32_t code() const { return code_; }
    [[nodiscard]] bool key_pressed() const { return key_pressed_; }
    void set_key_pressed(bool key_pressed) { key_pressed_ = key_pressed; }

   private:
    const uint32_t code_;

    // TODO(tomKPZ): Any way to get initial key states?
    bool key_pressed_ = false;
  };

  std::vector<KeyCodeState> key_code_states_{KeyCodeState{kWinKeyLeft},
                                             KeyCodeState{kWinKeyRight}};
  bool any_key_pressed_ = false;

  Connection* connection_;
  ScopedObserver<EventDispatcher> dispatcher_;

  uint8_t xcb_input_major_opcode_;

  DELETE_SPECIAL_MEMBERS(KeyListener);
};
