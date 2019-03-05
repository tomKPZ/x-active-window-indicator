#include <iostream>
#include <string>
#include <vector>

#include "active_window_manager.h"
#include "connection.h"

int main(int, char**) {
  Connection connection;
  ActiveWindowManager active_window_manager{&connection};
  return 0;
}
