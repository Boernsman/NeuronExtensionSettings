/*
 * This file is part of the Neuron Extension Settings application.
 * Copyright (c) 2024 Bernhard Trinnes.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <source_location>
#include <sstream>
#include <string>

class DebugStream {
 public:
  DebugStream(const std::source_location &location = std::source_location::current()) : location_(location)
  {
    stream_ << location_.file_name() << ":" << location_.line() << " (" << location_.function_name() << ") ";
  }

  template <typename T>
  DebugStream &operator<<(const T &value)
  {
    stream_ << value;
    return *this;
  }

  ~DebugStream() { std::cout << stream_.str() << std::endl; }

 private:
  std::ostringstream stream_;
  std::source_location location_;
};

#define debug() DebugStream()

#endif  // DEBUG_H