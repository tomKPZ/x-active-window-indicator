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

#include "event_loop.h"

#include <exception>
#include <iostream>

#include "connection.h"
#include "event.h"
#include "event_dispatcher.h"
#include "x_error.h"

namespace {

Event WaitForEvent(Connection* connection) {
  return Event(xcb_wait_for_event(connection->connection()));
}

}  // namespace

EventLoop::EventLoop(Connection* connection) : connection_(connection) {}

EventLoop::~EventLoop() {}

void EventLoop::RegisterDispatcher(EventDispatcher* dispatcher) {
  dispatchers_.push_back(dispatcher);
}

void EventLoop::Run() {
  xcb_flush(connection_->connection());
  while (auto event = WaitForEvent(connection_)) {
    for (EventDispatcher* dispatcher : dispatchers_) {
      bool dispatched = false;
      try {
        dispatched = dispatcher->DispatchEvent(event);
      } catch (const XError& x_error) {
        std::cerr << "X Error: " << x_error.what() << std::endl;
      } catch (const std::exception& exception) {
        std::cerr << "Exception: " << exception.what() << std::endl;
      } catch (...) {
        std::cerr << "Unhandled exception" << std::endl;
      }
      if (dispatched)
        break;
    }
    xcb_flush(connection_->connection());
  }
}
