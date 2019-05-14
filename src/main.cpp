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

#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <csignal>
#include <cstdlib>
#include <initializer_list>
#include <stdexcept>

#include "active_window_indicator.h"
#include "connection.h"
#include "event_loop.h"
#include "lippincott.h"

// Self-pipe trick.
static std::array<int, 2> pipe_fds;

int main() noexcept {
  try {
    if (pipe(pipe_fds.data()) == -1) {
      throw std::runtime_error("pipe() failed");
    }
    for (int fd : pipe_fds) {
      // NOLINTNEXTLINE
      int flags = fcntl(pipe_fds[0], F_GETFL);
      // NOLINTNEXTLINE
      if (flags == -1 || fcntl(fd, F_SETFL, flags) == -1) {
        throw std::runtime_error("fcntl() failed");
      }
    }

    struct sigaction sa {};
    sa.sa_flags = SA_RESTART;
    sa.sa_sigaction = [](int /*unused*/, siginfo_t* /*unused*/,
                         void* /*unused*/) {
      if (write(pipe_fds[1], "", 1) == -1) {
        std::abort();
      }
    };
    for (auto sig : {
             SIGHUP,
             SIGINT,
             SIGQUIT,
             SIGTERM,
         }) {
      if (sigaction(sig, &sa, nullptr) == -1) {
        throw std::runtime_error("sigaction() failed");
      }
    }

    Connection connection;
    EventLoop loop{&connection, pipe_fds[0]};
    ActiveWindowIndicator indicator{&connection, &loop};
    loop.Run();
  } catch (...) {
    Lippincott();
  }
  return 0;
}
