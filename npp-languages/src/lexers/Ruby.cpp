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
		"undef", "unless", "until", "when", "while", "yield"
	};
	static const std::unordered_set<std::string> COND_INSTRUCTIONS = {
		"for", "while", "until", "case", "if", "unless"
	};

	bool nameChr(int c)
	{
		if (c < 0) return false;
		return c == '_' || isAlphaNumeric((char)c);
	}
	bool methodEndChr(int c)
	{
		return nameChr(c) || c == '?' || c == '!';
	}
	bool methodDefEndChr(int c)
	{
		return methodEndChr(c) || c == '=';
	}
	unsigned nameLen(StyleStream &stream)
	{
		unsigned n = 0;
		while (nameChr(stream.peek(n))) ++n;
		return n;
	}
}
void Ruby::style(StyleStream &stream)
{
	while (!stream.eof())
	{
		styleLine(stream);
	}
}

/**Style a upto the end of the line. Used by HAML etc.*/
void Ruby::styleLine(StyleStream &stream)
{
	bool first = true;
	while (true)
	{
		stream.advanceSpTab();
		char c = stream.peek();
		if (c < 0) return;
		else if (c == '\r' || c == '\n')
		{
			stream.advanceEol();
			return;
		}
		else if (c == '#')
		{
			stream.advanceLine(COMMENT);
			first = true;
		}
		else if (c == ';')
		{
			stream.advance(OPERATOR);
			first = true;
		}
		else if (first)
		{
			statementStart(stream);
			first = false;
		}
		else token(stream);
	}
}

void Ruby::string(StyleStream &stream)
{
	switch (stream.peek())
	{
	case '"':
		stream.advance(STRING);
		return stringBody(stream, '"', '"', STRING, true);
	case '\'':
		stream.advance(CHARACTER);
		return stringBody(stream, '\'', '\'', CHARACTER, false);
	case '/':
		stream.advance(REGEX);
		return stringBody(stream, '/', '/', REGEX, true);
	default:
		assert(false);
		stream.advance(ERROR);
		return;
	}
}
void Ruby::stringBody(StyleStream &stream,
	char delimL, char delimR,
	Style style, bool interpolated)
{
	int depth = 1;
	bool escape = false;
	while (depth)
	{
		bool thisEscape = escape;
		escape = false;
		char c = stream.peek();
		if (c < 0) break;
		else if (!thisEscape && c == delimR)
		{
			stream.advance(style);
			--depth;
		}
		else if (!thisEscape && c == delimL)
		{
			stream.advance(style);
			++depth;
		}
		else if (!thisEscape && interpolated && c == '#' && stream.peek(1) == '{')
		{
			stringInterp(stream);
		}
		else if (c == '\r' || c == '\n')
		{
			stream.advanceEol();
		}
		else if (c == '\\')
		{
			escape = !thisEscape;
			stream.advance(style);
		}
		else stream.advance(style);
	}
}

void Ruby::regex(StyleStream &stream)
{
	assert(stream.peek() == '/');
	string(stream);
	regexModifiers(stream);
}
void Ruby::regexModifiers(StyleStream &stream)
{
	while (true)
	{
		auto c = stream.peek();
		if (c == 'i' || c == 'm' || c == 'x' || c == 'o')
		{
			stream.advance(REGEX);
		}
		else if (c >= 0 && isAlphaNumeric((char)c))
		{
			stream.advance(ERROR);
			break;
		}
		else break;
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
		case '}':
			stream.advance(OPERATOR);
			return;
		case '\r':
		case '\n':
			stream.advanceEol();
			break;
		default:
			token(stream);
		}
	}
}

void Ruby::statementStart(StyleStream &stream)
{
	char c = stream.peek();
	assert(c != '\n' && c != '\r');

	auto word = peekInstruction(stream);
	if (COND_INSTRUCTIONS.count(word))
	{
		stream.foldHeader(stream.foldLevel());
		stream.increaseFoldNext();
		stream.advance(INSTRUCTION, word.size());
	}
	else token(stream);
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
	case '*': case '+': case '-':
	case '(': case ')': case '[': case ']': case '{': case '}':
		stream.advance(OPERATOR);
		break;
	case '$':
	{
		stream.advance(GLOBAL);
		auto n = nameLen(stream);
		if (n == 0) stream.advance(GLOBAL);
		else stream.advance(GLOBAL, n);
		break;
	}
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
	case '%':
	{
		Style strStyle = STRING;
		bool interpolated = true;
		int c1 = stream.peek(1);
		unsigned n = 2;
		if (c1 < 0)
		{
			stream.advance(OPERATOR);
			break;
		}
		int delimL = c1;

		switch ((char)c1)
		{
		case 'r':
			strStyle = REGEX;
			interpolated = true;
			delimL = stream.peek(2);
			++n;
			break;
		case 'x':
			strStyle = BACKTICKS;
			interpolated = true;
			delimL = stream.peek(2);
			++n;
			break;
		case 'Q':
		case 'I':
		case 'W':
			strStyle = STRING;
			interpolated = true;
			delimL = stream.peek(2);
			++n;
			break;
		case 'q':
		case 'i':
		case 'w':
		case 's':
			strStyle = CHARACTER;
			interpolated = false;
			delimL = stream.peek(2);
			++n;
			break;
		}
		if (delimL < 0)
		{
			stream.advance(OPERATOR);
			break;
		}

		char delimR;
		switch (delimL)
		{
		case '(': delimR = ')'; break;
		case '[': delimR = ']'; break;
		case '{': delimR = '}'; break;
		case '<': delimR = '>'; break;
		default: delimR = delimL; break;
		}

		stream.advance(strStyle, n);
		stringBody(stream, (char)delimL, delimR, strStyle, interpolated);
		if (strStyle == REGEX) regexModifiers(stream);
		break;
	}
	case '@':
		if (stream.peek(1) == '@')
		{
			stream.advance(CLASS_VAR, 2);
			name(stream, CLASS_VAR);
		}
		else
		{
			stream.advance(INSTANCE_VAR, 1);
			name(stream, INSTANCE_VAR);
		}
		break;
	case '/':
	{
		auto c2 = stream.peek(1);
		if (c2 > 0 && c2 != ' ' && c2 != '\t' && c2 != '\r' && c2 != '\n') regex(stream);
		else stream.advance(OPERATOR);
		break;
	}
	case '`':
		stream.advance(BACKTICKS);
		stringBody(stream, '`', '`', BACKTICKS, true);
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
			auto word = peekInstruction(stream);
			if (word.empty())
			{
				stream.advance(0);
			}
			else if (stream.peek((unsigned)word.size()) == ':')
			{
				stream.advance(SYMBOL, (unsigned)word.size() + 1);
			}
			else if (INSTRUCTIONS.count(word))
			{
				stream.advance(INSTRUCTION, (unsigned)word.size());
				if (word == "class")
				{
					stream.advanceSpTab();
					name(stream, CLASS_DEF);
					stream.foldHeader(stream.foldLevel());
					stream.increaseFoldNext();
				}
				else if (word == "def")
				{
					stream.advanceSpTab();
					name(stream, METHOD_DEF, true);
					stream.foldHeader(stream.foldLevel());
					stream.increaseFoldNext();
				}
				else if (word == "module")
				{
					stream.advanceSpTab();
					name(stream, MODULE_DEF);
					stream.foldHeader(stream.foldLevel());
					stream.increaseFoldNext();
				}
				else if (word == "do")
				{
					stream.foldHeader(stream.foldLevel());
					stream.increaseFoldNext();
				}
				else if (word == "end")
				{
					stream.reduceFoldNext();
				}
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
		else if (nameChr(c)) stream.advance(style);
		else break;
	}
	if (method && (c == '!' || c == '?')) stream.advance(style);
	else if (style == METHOD_DEF && c == '=') stream.advance(METHOD_DEF);
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
std::string Ruby::peekInstruction(StyleStream &stream)
{
	std::string word;
	while (true)
	{
		auto c2 = stream.peek((unsigned)word.size());
		if (!nameChr(c2))
		{
			if (methodEndChr(c2)) word.push_back((char)c2);
			break;
		}
		else word.push_back((char)c2);
	}
	return word;
}
