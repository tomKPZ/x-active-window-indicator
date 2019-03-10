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

#include <poll.h>

#include <exception>
#include <iostream>
#include <sstream>

#include "connection.h"
#include "event.h"
#include "event_dispatcher.h"
#include "event_loop_idle_observer.h"
#include "x_error.h"

namespace {

std::string MakeUnhandledErrorMessage(const Event& event) {
  std::ostringstream stream;
  stream << "Unhandled event: send_event(" << event.SendEvent()
         << "), response_type(" << static_cast<uint16_t>(event.ResponseType())
         << "), sequence(" << event.Sequence() << ")";
  return stream.str();
}

}  // namespace

EventLoop::EventLoop(Connection* connection, int should_quit_fd)
    : connection_(connection), should_quit_fd_(should_quit_fd) {}

EventLoop::~EventLoop() {}

void EventLoop::Run() {
  while (auto event = WaitForEvent()) {
    bool dispatched = false;
    for (auto* dispatcher : Observable<EventDispatcher>::observers()) {
      try {
        dispatched = dispatcher->DispatchEvent(event);
      } catch (const XError& x_error) {
        std::cerr << "X Error: " << x_error.what() << std::endl;
      } catch (const std::exception& exception) {
        std::cerr << "Exception: " << exception.what() << std::endl;
      } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
      }
      if (dispatched) {
        break;
      }
    }
    if (!dispatched && event.ResponseType() != XCB_CLIENT_MESSAGE) {
      std::cerr << MakeUnhandledErrorMessage(event) << std::endl;
    }
  }
}

Event EventLoop::WaitForEvent() {
  auto* connection = connection_->connection();

  xcb_generic_event_t* event = xcb_poll_for_event(connection);
  if (event || xcb_connection_has_error(connection)) {
    return Event(event);
  }

  for (auto* observer : Observable<EventLoopIdleObserver>::observers()) {
    observer->OnIdle();
  }

  xcb_flush(connection);

  while (true) {
    struct pollfd poll_fds[] = {
        {should_quit_fd_, POLLIN, 0},
        {xcb_get_file_descriptor(connection), POLLIN, 0},
    };
    int ready = poll(poll_fds, ArraySize(poll_fds), -1);
    if (ready == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::logic_error("poll() failed");
    }
    if (poll_fds[0].revents) {
      return Event(nullptr);
    }
    event = xcb_poll_for_event(connection);
    if (event || xcb_connection_has_error(connection)) {
      return Event(event);
    }
  }
}
