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

#include "quit_signaller.h"

#include <sys/signalfd.h>
#include <unistd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>

#include "p_error.h"
#include "util.h"

QuitSignaller::QuitSignaller() {
  sigset_t mask;
  sigemptyset(&mask);
  for (auto sig : {
           SIGHUP,
           SIGINT,
           SIGQUIT,
           SIGTERM,
       }) {
    sigaddset(&mask, sig);
  }

  fd_ = signalfd(-1, &mask, SFD_CLOEXEC);
  if (fd_ == -1) {
    throw PError("signalfd");
  }

  if (sigprocmask(SIG_BLOCK, &mask, nullptr) == -1) {
    throw PError("sigprocmask");
  }
}

QuitSignaller::~QuitSignaller() {
  if (REDO_ON_EINTR(close(fd_) == -1)) {
    perror("close");
    std::abort();
  }
}
