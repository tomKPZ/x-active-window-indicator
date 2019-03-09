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

#include "observable.h"

template <typename Observer>
class ScopedObserver {
 public:
  ScopedObserver(Observer* observer, Observable<Observer>* observable)
      : observer_(observer), observable_(observable) {
    observable_->AddObserver(observer_);
  }

  ~ScopedObserver() { observable_->RemoveObserver(observer_); }

 private:
  Observer* observer_;
  Observable<Observer>* observable_;
};
