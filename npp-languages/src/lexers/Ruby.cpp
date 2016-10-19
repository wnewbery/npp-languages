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
		stream.advanceSpTab();
		if (stream.eof() || stream.peek() == '\n' || stream.peek() == '\r') break;
		token(stream);
	}
}

void Ruby::string(StyleStream &stream)
{
	char delim = stream.peek();
	assert(delim == '"' || delim == '\'');
	stream.advance(STRING);

	bool escape = false;
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
		case '\\':
			escape = true;
			stream.advance(STRING);
			break;
		case '#':
			if (escape || delim != '"' || stream.peek(1) != '{')
			{
				stream.advance(STRING);
				escape = false;
			}
			else stringInterp(stream);
			break;
		default:
			escape = false;
			stream.advance(STRING);
			break;
		}
	}
}

void Ruby::stringLine(StyleStream &stream)
{
	bool escape = false;
	while (!stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			stream.advanceEol(0);
			return;
		case '\\':
			escape = true;
			stream.advance(STRING);
			break;
		case '#':
			if (escape || stream.peek(1) != '{')
			{
				stream.advance(STRING);
				escape = false;
			}
			else stringInterp(stream);
			break;
		default:
			escape = false;
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
			name(stream, SYMBOL);
		}
		break;
	}
	case '@':
		stream.advance(ATTRIBUTE);
		name(stream, ATTRIBUTE);
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
			std::string word;
			while (true)
			{
				auto c2 = stream.peek((unsigned)word.size());
				if (c2 < 0 || !(c2 == '_' || c == '?' || c == '!' || isAlphaNumeric((char)c2))) break;
				else word.push_back((char)c2);
			}
			if (word.empty())
			{
				stream.advance(0);
			}
			else if (INSTRUCTIONS.count(word))
			{
				stream.advance(INSTRUCTION, (unsigned)word.size());
			}
			else stream.advance(DEFAULT, (unsigned)word.size());
		}
		break;
	}
}

void Ruby::name(StyleStream &stream, Style style, bool method)
{
	int c;
	while (true)
	{
		c = stream.peek();
		if (c < 0) break;
		else if (c == '_' || isAlphaNumeric((char)c)) stream.advance(style);
		else break;
	}
	if (method && (c == '!' || c == '?')) stream.advance(style);
}

unsigned Ruby::findNextInterp(StyleStream &stream)
{
	int i = 0;
	bool escape = false;
	while (true)
	{
		switch (stream.peek(i))
		{
		case EOF:
		case '\r':
		case '\n':
			return i;
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
