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
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <array>
#include <cerrno>
#include <cstdint>
#include <forward_list>
#include <iostream>
#include <sstream>  // IWYU pragma: keep (https://github.com/include-what-you-use/include-what-you-use/issues/277)
#include <stdexcept>
#include <string>

#include "connection.h"
#include "event.h"
#include "event_dispatcher.h"
#include "event_loop_idle_observer.h"
#include "lippincott.h"

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

EventLoop::~EventLoop() = default;

void EventLoop::Run() {
  while (auto event = WaitForEvent()) {
    bool dispatched = false;
    for (auto* dispatcher : Observable<EventDispatcher>::observers()) {
      try {
        dispatched = dispatcher->DispatchEvent(event);
      } catch (...) {
        Lippincott();
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
  if ((event != nullptr) || (xcb_connection_has_error(connection) != 0)) {
    return Event(event);
  }

  for (auto* observer : Observable<EventLoopIdleObserver>::observers()) {
    observer->OnIdle();
  }

  xcb_flush(connection);

  while (true) {
    std::array<struct pollfd, 2> poll_fds{
        {{should_quit_fd_, POLLIN, 0},
         {xcb_get_file_descriptor(connection), POLLIN, 0}}};
    int ready = poll(poll_fds.data(), poll_fds.size(), -1);
    if (ready == -1) {
      if (errno == EINTR) {
        continue;
      }
      throw std::logic_error("poll() failed");
    }
    if (poll_fds[0].revents != 0) {
      return Event(nullptr);
    }
    event = xcb_poll_for_event(connection);
    if ((event != nullptr) || (xcb_connection_has_error(connection) != 0)) {
      return Event(event);
    }
  }
}
