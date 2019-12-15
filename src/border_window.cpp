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

#include "border_window.h"

#include <xcb/shape.h>
#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <xcb/xproto.h>

#include <array>
#include <vector>

#include "command_line.h"
#include "connection.h"
#include "util.h"
#include "x_error.h"

namespace {

class XcbRegion {
 public:
  XcbRegion(Connection* connection, const std::vector<xcb_rectangle_t>& rects)
      : connection_(connection), id_(connection_->GenerateId()) {
    xcb_xfixes_create_region(connection_->connection(), id_,
                             CheckedCast<uint32_t>(rects.size()), rects.data());
  }
  ~XcbRegion() { xcb_xfixes_destroy_region(connection_->connection(), id_); }
  [[nodiscard]] auto id() const -> uint32_t { return id_; }

 private:
  Connection* connection_;
  uint32_t id_;

  DELETE_SPECIAL_MEMBERS(XcbRegion);
};

}  // namespace

BorderWindow::BorderWindow(Connection* connection, CommandLine* command_line)
    : connection_(connection), command_line_(command_line) {
  window_ = connection_->GenerateId();
  std::array<uint32_t, 2> attributes{command_line_->border_color(), 1U};
  xcb_create_window(connection_->connection(), XCB_COPY_FROM_PARENT, window_,
                    connection_->root_window(), 0, 0, 1, 1, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
                    XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT,
                    attributes.data());

  XCB_SYNC(xcb_xfixes_query_version, connection_, XCB_XFIXES_MAJOR_VERSION,
           XCB_XFIXES_MINOR_VERSION);
  auto* fixes_extension =
      xcb_get_extension_data(connection_->connection(), &xcb_xfixes_id);
  if (fixes_extension->present == 0U) {
    throw XError("XFIXES not available");
  }
}

BorderWindow::~BorderWindow() {
  xcb_destroy_window(connection_->connection(), window_);
}

void BorderWindow::SetPosition(int16_t x, int16_t y) {
  xcb_configure_window_value_list_t configure{};
  configure.x = x;
  configure.y = y;
  xcb_configure_window_aux(connection_->connection(), window_,
                           XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y,
                           &configure);
}

void BorderWindow::SetSize(uint16_t width, uint16_t height) {
  // TODO(tomKPZ): Use an outer border instead of an inner border if the window
  // is tiny.
  xcb_configure_window_value_list_t configure{};
  configure.width = CheckedCast<uint16_t>(width);
  configure.height = CheckedCast<uint16_t>(height);
  xcb_configure_window_aux(connection_->connection(), window_,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           &configure);

  const uint16_t border_width = command_line_->border_width();
  const std::vector<xcb_rectangle_t> rects{
      // Top edge.
      {0, 0, width, border_width},
      // Bottom edge.
      {0, CheckedCast<int16_t>(height - border_width), width, border_width},
      // Left edge.
      {0, 0, border_width, height},
      // Right edge.
      {CheckedCast<int16_t>(width - border_width), 0, border_width, height},
  };

  xcb_xfixes_set_window_shape_region(connection_->connection(), window_,
                                     XCB_SHAPE_SK_BOUNDING, 0, 0,
                                     XcbRegion(connection_, rects).id());
  xcb_xfixes_set_window_shape_region(connection_->connection(), window_,
                                     XCB_SHAPE_SK_INPUT, 0, 0,
                                     XcbRegion(connection_, {}).id());
}

void BorderWindow::Show() {
  xcb_map_window(connection_->connection(), window_);
  Raise();
}

void BorderWindow::Hide() {
  xcb_unmap_window(connection_->connection(), window_);
}

void BorderWindow::Raise() {
  xcb_configure_window_value_list_t configure{};
  configure.stack_mode = XCB_STACK_MODE_ABOVE;
  xcb_configure_window_aux(connection_->connection(), window_,
                           XCB_CONFIG_WINDOW_STACK_MODE, &configure);
}
