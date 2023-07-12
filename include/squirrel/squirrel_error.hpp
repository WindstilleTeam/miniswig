// miniswig - Language binding generator for Squirrel
// Copyright (C) 2006 Matthias Braun <matze@braunis.de>
// Copyright (C) 2023 Ingo Ruhnke <grumbel@gmail.com>

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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_MINISWIG_SQUIRREL_SQUIRREL_ERROR_HPP
#define HEADER_MINISWIG_SQUIRREL_SQUIRREL_ERROR_HPP

#include <squirrel.h>
#include <stdexcept>
#include <string>
#include <sstream>

/** Exception class for squirrel errors, it takes a squirrelvm and uses
 * sq_geterror() to retrieve additional information about the last error that
 * occurred and creates a readable message from that.
 */
class SquirrelError final : public std::exception
{
public:
  SquirrelError(HSQUIRRELVM v, const std::string& message_) noexcept :
    m_message()
  {
    std::ostringstream msg;
    msg << "Squirrel error: " << message_ << " (";
    const char* lasterr;
    sq_getlasterror(v);
    if (sq_gettype(v, -1) != OT_STRING)
    {
      lasterr = "no error info";
    }
    else
    {
      sq_getstring(v, -1, &lasterr);
    }
    msg << lasterr << ")";
    sq_pop(v, 1);

    m_message = msg.str();
  }

  SquirrelError(SquirrelError const&) = default;
  SquirrelError& operator=(SquirrelError const&) = default;
  ~SquirrelError() noexcept override
  {}

  const char* what() const noexcept
  {
    return m_message.c_str();
  }

private:
  std::string m_message;
};

#endif

/* EOF */
