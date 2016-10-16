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
		//http://ruby-doc.org/docs/keywords/1.9/
		"BEGIN", "END", "__ENCODING__", "__FILE__", "__LINE__",
		"alias", "and", "begin", "break", "case", "class",
		"def", "defined?", "do", "else", "elsif", "end", "ensure",
		"false", "for", "if", "in", "module", "next", "nil", "not", "or",
		"redo", "rescue", "retry", "return", "self", "super", "then", "true",
		"undef", "unless", "when", "while", "yield"
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
		token(stream);
	}
}

void Ruby::string(StyleStream &stream)
{
	char delim = stream.peek();
	assert(delim == '"' || delim == '\'');
	stream.advance(STRING);

	while (!stream.eof())
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
		case '#':
			if (delim == '"' && stream.prev() != '\\' && stream.peek(1) == '{')
				stringInterp(stream);
			else stream.advance(STRING);
			break;
		default:
			stream.advance(STRING);
		}
	}
}

void Ruby::stringLine(StyleStream &stream)
{
	while (!stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			stream.readRestOfLine(0);
			return;
		case '#':
			if (stream.prev() != '\\' && stream.peek(1) == '{')
				stringInterp(stream);
			else stream.advance(STRING);
			break;
		default:
			stream.advance(STRING);
			break;
		}
	}
}

void Ruby::stringInterp(StyleStream &stream)
{
	assert(stream.matches("#{"));
	stream.advance(OPERATOR, 2);
	while (!stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			return;
		case '}':
			stream.advance(OPERATOR);
			return;
		default:
			token(stream);
		}
	}
}

void Ruby::token(StyleStream &stream)
{
	char c = stream.peek();
	assert(c != '\n' && c != '\r');
	switch (c)
	{
	case '.': case ',': case '?':
	case '=': case '<': case '>':
	case '&': case '|': case '^': case '~':
	case '*': case '/': case '%': case '+': case '-':
	case '(': case ')': case '[': case ']': case '{': case '}':
		stream.advance(OPERATOR);
		break;
	case ':':
	{
		auto c2 = stream.peek(1);
		if (c2 == ':') stream.advance(OPERATOR, 2);
		else if (c2 == ' ') stream.advance(OPERATOR);
		else
		{
			stream.advance(SYMBOL);
			stream.nextWord(SYMBOL);
		}
		break;
	}
	case '@':
		stream.advance(ATTRIBUTE);
		stream.nextWord(ATTRIBUTE);
		break;
	case '\'': case '"':
		string(stream);
		break;
	default:
		if (c >= '0' && c <= '9')
		{
			stream.advance(NUMBER);
		}
		else
		{
			auto word = stream.peekWord();
			if (word.empty())
			{
				stream.advance(0);
			}
			else if (INSTRUCTIONS.count(word))
			{
				stream.advance(INSTRUCTION, word.size());
			}
			else stream.advance(DEFAULT, word.size());
		}
		break;
	}
}

int Ruby::findNextInterp(StyleStream &stream)
{
	int i = 0;
	bool escape = false;
	while (true)
	{
		switch (stream.peek(i))
		{
		case '\0':
		case '\r':
		case '\n':
			return -1;
		case '#':
			if (!escape && stream.peek(i + 1) == '{')
				return i;
			escape = false;
			break;
		case '\\':
			escape = !escape;
			break;
		default:
			escape = false;
			break;
		}
		++i;
	}
}
