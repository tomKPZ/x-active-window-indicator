#include <iostream>
#include <string>
#include <vector>

#include "active_window_tracker.h"
#include "connection.h"

int main(int, char**) {
  Connection connection;
  ActiveWindowTracker active_window_tracker{&connection};
  return 0;
}
