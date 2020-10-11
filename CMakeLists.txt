# SuperTux - miniswig build script
# Copyright (C) 2007 Timothy Goya <tuxdev103@gmail.com>
#               2020 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

cmake_minimum_required(VERSION 3.15)
project(miniswig)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

file(GLOB MINISWIG_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/create_docu.cpp
  src/create_wrapper.cpp
  src/main.cpp
  src/tree.cpp
  src/xmlwriter.cpp)

bison_target(MyParser src/parser.yy
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

flex_target(MyScanner src/lexer.ll
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

add_flex_bison_dependency(MyScanner MyParser)

add_executable(miniswig
  ${MINISWIG_SOURCES}
  ${BISON_MyParser_OUTPUTS}
  ${FLEX_MyScanner_OUTPUTS})
target_include_directories(miniswig PUBLIC src/)

# EOF #
