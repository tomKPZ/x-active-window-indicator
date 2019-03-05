#include <xcb/xcb.h>

#include <iostream>
#include <string>
#include <vector>

#include "active_window_observer.h"
#include "active_window_tracker.h"
#include "border_window.h"
#include "connection.h"
#include "key_listener.h"
#include "key_state_observer.h"

class Indicator : public ActiveWindowObserver, KeyStateObserver {
 public:
  Indicator(Connection* connection)
      : connection_(connection),
        active_window_tracker_(connection_, this),
        key_listener_(connection_, this),
        border_window_(connection_) {}
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

  ActiveWindowTracker active_window_tracker_;
  KeyListener key_listener_;

  BorderWindow border_window_;
};

int main(int, char**) {
  Connection connection;
  Indicator indicator{&connection};
  return 0;
}
