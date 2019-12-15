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

#include "command_line.h"

#include <getopt.h>

#include <array>
#include <sstream>
#include <stdexcept>
#include <string>

#include "usage_error.h"
#include "util.h"

namespace {

constexpr const uint32_t kDefaultBorderColor = 0xffff0000;
constexpr const uint16_t kDefaultBorderWidth = 5;

template <typename T, typename Format>
auto ParseInt(const std::string& str, Format format) -> T {
  std::stringstream stream{str, std::ios_base::in};
  T value;
  stream >> format >> value;
  if (stream.fail() || !stream.eof()) {
    throw std::invalid_argument("ParseInt");
  }
  return value;
}

}  // namespace

CommandLine::CommandLine(int argc, char** argv)
    : border_color_{kDefaultBorderColor}, border_width_{kDefaultBorderWidth} {
  Init(argc, argv);
  if (optind < argc) {
    std::cerr << "Unconsumed arguments: ";
    for (int i = optind; i < argc; i++) {
      std::cerr << argv[i] << ' ';
    }
    std::cerr << std::endl;
    throw UsageError{};
  }
}

void CommandLine::Init(int argc, char** argv) {
  while (true) {
    constexpr std::array<struct option, 4> kLongOptions{
        {{"help", no_argument, nullptr, 'h'},
         {"border-color", required_argument, nullptr, 'c'},
         {"border-width", required_argument, nullptr, 'w'},
         {nullptr, 0, nullptr, 0}}};

    try {
      switch (getopt_long(argc, argv, "hc:w:", kLongOptions.data(), nullptr)) {
        case -1:
          return;
        case 'h':
          throw UsageError();
        case 'c':
          // TODO(tomKPZ): Generic parse integral template.
          border_color_ = ParseInt<uint32_t>(optarg, std::hex);
          break;
        case 'w':
          border_width_ = ParseInt<uint16_t>(optarg, std::dec);
          break;
        case '?':
          // getopt_long() already prints an error mesage indicating the
          // argument.
          throw UsageError{};
        default:
          throw std::logic_error{"getopt_long"};
      }
    } catch (const std::invalid_argument& /*error*/) {
      std::cerr << "Unable to parse integral argument: " << optarg << std::endl;
      throw UsageError{};
    }
  }
}
