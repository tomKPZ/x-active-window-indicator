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
  }

  // KeyStateObserver:
  void KeyStateChanged(bool pressed) { (void)pressed; }

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
