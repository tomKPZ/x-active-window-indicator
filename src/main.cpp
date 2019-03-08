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

#include "active_window_indicator.h"
#include "active_window_tracker.h"
#include "border_window.h"
#include "connection.h"
#include "event_loop.h"
#include "key_listener.h"

int main(int, char**) {
  Connection connection;
  EventLoop loop{&connection};
  BorderWindow border_window{&connection};
  ActiveWindowIndicator indicator{&connection, &loop, &border_window};
  ActiveWindowTracker tracker{&connection, &indicator};
  KeyListener key_listener{&connection, &indicator};
  // TODO: have |tracker| and |key_listener| register itself.
  loop.RegisterDispatcher(&tracker);
  loop.RegisterDispatcher(&key_listener);
  loop.Run();
  return 0;
}
