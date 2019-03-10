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

#include "event_dispatcher.h"
#include "observable.h"
#include "scoped_observer.h"
#include "util.h"

using xcb_atom_t = uint32_t;
using xcb_window_t = uint32_t;

class ActiveWindowObserver;
class Connection;
class EventLoop;

class ActiveWindowTracker : public EventDispatcher,
                            public Observable<ActiveWindowObserver> {
 public:
  ActiveWindowTracker(Connection* connection, EventLoop* event_loop);
  ~ActiveWindowTracker() override;

  xcb_window_t active_window() const { return active_window_; }

 protected:
  // EventDispatcher:
  bool DispatchEvent(const Event& event) override;

 private:
  void SetActiveWindow();

  Connection* connection_;
  ScopedObserver<EventDispatcher> event_dispatcher_;

  xcb_atom_t net_active_window_;
  xcb_window_t active_window_;

  DISALLOW_COPY_AND_ASSIGN(ActiveWindowTracker);
};
