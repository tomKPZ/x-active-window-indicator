#include "connection.h"

#include <xcb/xcb.h>

#include <cassert>
#include <memory>
#include <string>

namespace {

xcb_screen_t* ScreenOfConnection(xcb_connection_t* c, int screen) {
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem != 0; --screen, xcb_screen_next(&iter)) {
    if (screen == 0) {
      return iter.data;
    }
  }

  return nullptr;
}

void SetEventMask(xcb_connection_t* connection,
                  xcb_window_t window,
                  uint32_t new_mask) {
  auto cookie = xcb_change_window_attributes(connection, window,
                                             XCB_CW_EVENT_MASK, &new_mask);
  // Window |window| may already be destroyed at this point, so the
  // change_attributes request may give a BadWindow error.  In this case, just
  // ignore the error.
  xcb_discard_reply(connection, cookie.sequence);
}

}  // namespace

class Connection::MultiMask {
 public:
  MultiMask() {
    for (unsigned int& mask_bit : mask_bits_) {
      mask_bit = 0;
    }
  }

  ~MultiMask() = default;

  void AddMask(uint32_t mask) {
    for (int i = 0; i < kMaskSize; i++) {
      if ((mask & (1U << i)) != 0u) {
        mask_bits_[i]++;
      }
    }
  }

  void RemoveMask(uint32_t mask) {
    for (int i = 0; i < kMaskSize; i++) {
      if ((mask & (1U << i)) != 0u) {
        assert(mask_bits_[i]);
        mask_bits_[i]--;
      }
    }
  }

  uint32_t ToMask() const {
    uint32_t mask = XCB_EVENT_MASK_NO_EVENT;
    for (int i = 0; i < kMaskSize; i++) {
      if (mask_bits_[i] != 0u) {
        mask |= (1U << i);
      }
    }
    return mask;
  }

 private:
  static constexpr auto kMaskSize = 25;

  unsigned int mask_bits_[kMaskSize]{};

  DELETE_COPY_AND_MOVE(MultiMask);
};

Connection::Connection() {
  int screen_number;
  connection_ = xcb_connect(nullptr, &screen_number);
  if (int error = xcb_connection_has_error(connection_)) {
    throw XError("XCB connection error code " + std::to_string(error));
  }

  xcb_screen_t* screen = ScreenOfConnection(connection_, screen_number);
  if (screen == nullptr) {
    throw XError("Could not get screen");
  }
  root_window_ = screen->root;
  if (root_window_ == 0u) {
    throw XError("Could not find root window");
  }
}

Connection::~Connection() {
  for (const auto& mask_pair : mask_map_) {
    SetEventMask(connection_, mask_pair.first, XCB_EVENT_MASK_NO_EVENT);
  }
  xcb_flush(connection_);
  xcb_disconnect(connection_);
}

uint32_t Connection::GenerateId() {
  return xcb_generate_id(connection_);
}

void Connection::SelectEvents(xcb_window_t window, uint32_t event_mask) {
  std::unique_ptr<MultiMask>& mask = mask_map_[window];
  if (!mask) {
    mask = std::make_unique<MultiMask>();
  }
  uint32_t old_mask = mask_map_[window]->ToMask();
  mask->AddMask(event_mask);
  AfterMaskChanged(window, old_mask);
}

void Connection::DeselectEvents(xcb_window_t window, uint32_t event_mask) {
  assert(mask_map_.find(window) != mask_map_.end());
  std::unique_ptr<MultiMask>& mask = mask_map_[window];
  uint32_t old_mask = mask->ToMask();
  mask->RemoveMask(event_mask);
  AfterMaskChanged(window, old_mask);
}

void Connection::AfterMaskChanged(xcb_window_t window, uint32_t old_mask) {
  uint32_t new_mask = mask_map_[window]->ToMask();
  if (new_mask == old_mask) {
    return;
  }

  SetEventMask(connection_, window, new_mask);

  if (new_mask == XCB_EVENT_MASK_NO_EVENT) {
    mask_map_.erase(window);
  }
}
