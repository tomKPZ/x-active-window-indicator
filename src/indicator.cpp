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

#include <xcb/xcb.h>

#include "active_window_observer.h"
#include "active_window_tracker.h"
#include "border_window.h"
#include "connection.h"
#include "event_loop.h"
#include "key_listener.h"
#include "key_state_observer.h"

class Indicator : public ActiveWindowObserver, public KeyStateObserver {
 public:
  Indicator(Connection* connection)
      : connection_(connection), border_window_(connection_) {}
  ~Indicator() {}

  // ActiveWindowObserver:
  void ActiveWindowChanged(xcb_window_t window) {
    auto geometry =
        XCB_SYNC(xcb_get_geometry, connection_->connection(), window);
    auto root_coordinates =
        XCB_SYNC(xcb_translate_coordinates, connection_->connection(), window,
                 connection_->root_window(), geometry->x, geometry->y);
    border_window_.SetRect(xcb_rectangle_t{root_coordinates->dst_x,
                                           root_coordinates->dst_y,
                                           geometry->width, geometry->height});
  }

  // KeyStateObserver:
  void KeyStateChanged(bool pressed) {
    if (pressed)
      border_window_.Show();
    else
      border_window_.Hide();
  }

 private:
  Connection* connection_;

  BorderWindow border_window_;
};

int main(int, char**) {
  Connection connection;
  Indicator indicator{&connection};
  ActiveWindowTracker active_window_tracker_{&connection, &indicator};
  KeyListener key_listener{&connection, &indicator};
  EventLoop loop{&connection};
  loop.RegisterDispatcher(&key_listener);
  loop.Run();
  return 0;
}
