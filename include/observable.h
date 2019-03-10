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

#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <forward_list>

#include "util.h"

template <typename Observer>
class Observable {
 public:
  void AddObserver(Observer* observer) { observers_.push_front(observer); }

  void RemoveObserver(Observer* observer) { observers_.remove(observer); }

 protected:
  virtual ~Observable() { DCHECK(observers_.empty()); }

  const std::forward_list<Observer*>& observers() const { return observers_; }

 private:
  std::forward_list<Observer*> observers_;
};

#endif
