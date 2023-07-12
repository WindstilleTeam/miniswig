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

#include "example.hpp"

#include <stdexcept>
#include <iostream>

#include <squirrel/squirrel_error.hpp>

#include "example_wrap.hpp"

namespace example {

Example::Example()
{
  std::cout << "Example::Example()\n";
}

Example::~Example()
{
  std::cout << "Example::~Example()\n";
}

void
Example::do_foobar()
{
  std::cout << "Example::do_foobar()\n";
}

void
Example::do_bazbaz()
{
  std::cout << "Example::do_bazbaz()\n";
}

void do_foobar()
{
  std::cout << "do_foobar()\n";
}

void do_bazbaz()
{
  std::cout << "do_bazbaz()\n";
}

} // namespace example

int main()
{
  std::cout << "main()\n";

  try {
    // create Squirrel VM
    HSQUIRRELVM vm = sq_open(1024);
    if (vm == nullptr)
      throw std::runtime_error("Couldn't initialize squirrel vm");

    // register custom functions
    sq_pushroottable(vm);
    example::register_example_wrapper(vm);
    sq_pop(vm, 1);

    // run tests
    std::string script =
      "do_foobar();"
      "do_bazbaz();"
      "example <- Example();"
      "example.do_foobar();"
      "example.do_bazbaz();"
      ;

    if (SQ_FAILED(sq_compilebuffer(vm, script.c_str(), script.length(),
                                   "", SQTrue))) {
      throw SquirrelError(vm, "Couldn't compile script");
    }

    sq_pushroottable(vm);
    if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue)))
      throw SquirrelError(vm, "Problem while executing script");

    sq_close(vm);

    return EXIT_SUCCESS;
  } catch (std::exception const& err) {
    std::cerr << "error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}

/* EOF */

