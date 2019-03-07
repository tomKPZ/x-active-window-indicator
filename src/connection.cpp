#include "connection.h"

#include <xcb/xcb.h>

#include <string>

namespace {

xcb_screen_t* ScreenOfConnection(xcb_connection_t* c, int screen) {
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem; --screen, xcb_screen_next(&iter))
    if (screen == 0)
      return iter.data;

  return nullptr;
}

}  // namespace

Connection::Connection() {
  int screen_number;
  connection_ = xcb_connect(nullptr, &screen_number);
  if (int error = xcb_connection_has_error(connection_))
    throw XError("XCB connection error code " + std::to_string(error));

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  if (!screen)
    throw XError("Could not get screen");
  root_window_ = screen->root;
  if (!root_window_)
    throw XError("Could not find root window");
}

Connection::~Connection() {
  xcb_flush(connection_);
  xcb_disconnect(connection_);
}

uint32_t Connection::GenerateId() {
  return xcb_generate_id(connection_);
}

void Connection::SelectEvents(xcb_window_t window, uint32_t event_mask) {
  // There's currently no conflicting clients that need to select
  // different events on the same window, but if there ever are, this
  // will need to be changed to keep track of a count of event mask
  // type.  Also a DeselectEvents() method will need to be added, and
  // window creation will need to be routed through the Connection too
  // since windows can be created with event masks.
  const uint32_t attributes[] = {event_mask};
  xcb_change_window_attributes(connection_, window, XCB_CW_EVENT_MASK,
                               attributes);
}
