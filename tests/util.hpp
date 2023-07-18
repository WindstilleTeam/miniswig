// miniswig - Language binding generator for Squirrel
// Copyright (C) 2023 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <fstream>
#include <stdexcept>

inline std::string load_file(std::string const& filename)
{
  std::ifstream fin(filename, std::ios::ate);

  if (!fin) {
    throw std::runtime_error("failed to open file: " + filename);
  }

  std::streamsize const size = fin.tellg();
  fin.seekg(0, std::ios::beg);

  std::string content(size, '\0');
  if (!fin.read(content.data(), content.size())) {
    throw std::runtime_error("failed to read from file: " + filename);
  }

  return content;
}

/* EOF */
