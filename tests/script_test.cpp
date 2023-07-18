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

#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <stdexcept>
#include <iostream>

#include <squirrel/squirrel_error.hpp>

#include "util.hpp"
#include "example_wrap.hpp"

namespace {

void my_printfunc(HSQUIRRELVM vm, SQChar const* fmt, ...)
{
  printf("from squirrel: ");
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void my_errorfunc(HSQUIRRELVM vm, SQChar const* fmt, ...)
{
  fprintf(stderr, "from squirrel: ");

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
}

} // namespace

int main(int argc, char** argv)
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " FILENAME" << std::endl;
    return EXIT_FAILURE;
  }

  std::string const filename = argv[1];

  std::cout << "main()\n";

  try {
    // create Squirrel VM
    HSQUIRRELVM vm = sq_open(1024);
    if (vm == nullptr) {
      throw std::runtime_error("Couldn't initialize squirrel vm");
    }

    sq_setprintfunc(vm, my_printfunc, my_errorfunc);

    std::cout << "-- start\n";

    std::cout << "-- register custom functions\n";
    sq_pushroottable(vm);
    example::register_example_wrapper(vm);
    sq_pop(vm, 1);

    // run tests
    std::string script = load_file(filename);

    std::cout << "-- compiling script\n";
    if (SQ_FAILED(sq_compilebuffer(vm, script.c_str(), script.length(),
                                   "", SQTrue))) {
      throw SquirrelError(vm, "Couldn't compile script");
    }

    std::cout << "-- executing script\n";
    sq_pushroottable(vm);

    if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue))) {
      throw SquirrelError(vm, "Problem while executing script");
    }

    std::cout << "-- vmstate: ";
    switch (sq_getvmstate(vm))
    {
      case SQ_VMSTATE_IDLE:
        std::cout << "idle";
        break;
      case SQ_VMSTATE_RUNNING:
        std::cout << "running";
        break;
      case SQ_VMSTATE_SUSPENDED:
        std::cout << "suspended";
        break;
      default:
        std::cout << "unknown";
    }
    std::cout << "\n";

    if (SQ_FAILED(sq_wakeupvm(vm,
                              SQFalse /* resumedret */,
                              SQFalse /* retval */,
                              SQTrue  /* raiseerror */,
                              SQFalse /* throwerror */))) {
      throw SquirrelError(vm, "Problem while resuming script");
    }

    // compiled script, roottable
    assert(sq_gettop(vm) == 2);

    std::cout << "-- closing\n";
    sq_close(vm);

    std::cout << "-- end\n";

    return EXIT_SUCCESS;
  } catch (std::exception const& err) {
    std::cerr << "error: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}

/* EOF */
