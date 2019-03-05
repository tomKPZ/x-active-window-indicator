#include "connection.h"

#include <xcb/xcb.h>

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
  if (xcb_connection_has_error(connection_))
    throw "Connection error";

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  if (!screen)
    throw "Could not get screen";
  root_window_ = screen->root;
  if (!root_window_)
    throw "Could not find root window";
}

Connection::~Connection() {
  xcb_disconnect(connection_);
}