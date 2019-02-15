#include <iostream>
#include <string>
#include <vector>

#include "workspace_manager.h"

int main (int argc, char *argv[]) {
  WorkspaceManager workspace_manager;
  for (const auto& workspace : workspace_manager.GetWorkspaces())
    std::cout << workspace.name.value_or("") << std::endl;
  return 0;
}
