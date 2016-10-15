// This file is part of Extra Languages for Notepad++.
// Copyright (C) 2016 William Newbery <wnewbery@hotmail.co.uk>
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

#include "Ruby.h"
#include <cassert>
#include <string>
#include <unordered_set>

namespace
{
	static const std::unordered_set<std::string> INSTRUCTIONS = {
		"true", "false",
		"def", "class", "module"
	};
}
void Ruby::style(StyleStream &stream)
{
	assert(false);
}
/**Style a upto the end of the line. Used by HAML etc.*/
void Ruby::styleLine(StyleStream &stream)
{
	while (true)
	{
		stream.nextSpaces();
		if (stream.eof() || stream.peek() == '\n' || stream.peek() == '\r') break;

		switch (stream.peek())
		{
		case '.': case ',': case '?': case ':':
		case '=': case '<': case '>':
		case '&': case '|': case '^': case '~':
		case '*': case '/': case '%': case '+': case '-':
		case '(': case ')': case '[': case ']': case '{': case '}':
			stream.advance(OPERATOR);
			break;
		case '\'': case '"':
			string(stream);
			break;
		default:
			{
				auto word = stream.peekWord();
				if (INSTRUCTIONS.count(word))
				{
					stream.advance(INSTRUCTION, word.size());
				}
				else stream.advance(DEFAULT, word.size());
			}
			break;
		}
	}
}

void Ruby::string(StyleStream &stream)
{
	char delim = stream.peek();
	assert(delim == '"' || delim == '\'');
	stream.advance(STRING);

	while (true)
	{
		char c = stream.peek();
		if (c == delim)
		{
			stream.advance(STRING);
			return;
		}
		switch (c)
		{
		case '\r':
		case '\n':
			return;
		//case '#':
		//case '\\':
		default:
			stream.advance(STRING);
		}
	}
}

void Ruby::stringLine(StyleStream &stream)
{
	stream.readRestOfLine(STRING);
}
