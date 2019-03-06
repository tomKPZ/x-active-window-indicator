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
