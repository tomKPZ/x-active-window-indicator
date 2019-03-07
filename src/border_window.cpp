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

#include <xcb/xcb.h>
#include <xcb/xfixes.h>

#include <vector>

#include "connection.h"
#include "util.h"
#include "x_error.h"

namespace {

static constexpr const uint32_t BORDER_COLOR = 0xff0000;
static constexpr const uint16_t BORDER_WIDTH = 5;

class XcbRegion {
 public:
  XcbRegion(Connection* connection, const std::vector<xcb_rectangle_t> rects)
      : connection_(connection), id_(connection_->GenerateId()) {
    xcb_xfixes_create_region(connection_->connection(), id_,
                             CheckedCast<uint32_t>(rects.size()), rects.data());
  }
  ~XcbRegion() { xcb_xfixes_destroy_region(connection_->connection(), id_); }
  uint32_t id() const { return id_; }

 private:
  Connection* connection_;
  uint32_t id_;
};

}  // namespace

BorderWindow::BorderWindow(Connection* connection) : connection_(connection) {
  window_ = connection_->GenerateId();
  uint32_t attributes[] = {BORDER_COLOR, true};
  xcb_create_window(connection_->connection(), XCB_COPY_FROM_PARENT, window_,
                    connection_->root_window(), 0, 0, 1, 1, BORDER_WIDTH,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_COPY_FROM_PARENT,
                    XCB_CW_BORDER_PIXEL | XCB_CW_OVERRIDE_REDIRECT, attributes);

  XCB_SYNC(xcb_xfixes_query_version, connection_->connection(),
           XCB_XFIXES_MAJOR_VERSION, XCB_XFIXES_MINOR_VERSION);
  auto* fixes_extension =
      xcb_get_extension_data(connection_->connection(), &xcb_xfixes_id);
  if (!fixes_extension->present)
    throw XError("XFIXES not available");
}

BorderWindow::~BorderWindow() {
  xcb_destroy_window(connection_->connection(), window_);
}

void BorderWindow::SetRect(const xcb_rectangle_t& rect) {
  // TODO: Use an outer border instead of an inner border if the window is tiny.
  const xcb_configure_window_value_list_t configure_value_list{
      rect.x,
      rect.y,
      CheckedCast<uint16_t>(rect.width - 2 * BORDER_WIDTH),
      CheckedCast<uint16_t>(rect.height - 2 * BORDER_WIDTH),
      0,
      0,
      0};
  xcb_configure_window(connection_->connection(), window_,
                       XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y |
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                       &configure_value_list);

  const std::vector<xcb_rectangle_t> rects{
      // Top edge.
      {-BORDER_WIDTH, -BORDER_WIDTH, rect.width, BORDER_WIDTH},
      // Bottom edge.
      {-BORDER_WIDTH, CheckedCast<int16_t>(rect.height - 2 * BORDER_WIDTH),
       rect.width, BORDER_WIDTH},
      // Left edge.
      {-BORDER_WIDTH, -BORDER_WIDTH, BORDER_WIDTH, rect.height},
      // Right edge.
      {CheckedCast<int16_t>(rect.width - 2 * BORDER_WIDTH), -BORDER_WIDTH,
       BORDER_WIDTH, rect.height},
  };
  xcb_xfixes_set_window_shape_region(connection_->connection(), window_,
                                     XCB_SHAPE_SK_BOUNDING, 0, 0,
                                     XcbRegion(connection_, rects).id());
}

void BorderWindow::Show() {
  xcb_map_window(connection_->connection(), window_);
}

void BorderWindow::Hide() {
  xcb_unmap_window(connection_->connection(), window_);
}
