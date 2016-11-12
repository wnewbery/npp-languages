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

#include "Scss.h"
#include <cassert>
#include <string>

namespace
{
	bool keywordChr(int c)
	{
		return c >= 'a' && c <= 'z';
	}
	bool hexChr(int c)
	{
		return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9');
	}
	std::string keyword( StyleStream &stream)
	{
		return stream.peekStr(&keywordChr);
	}
	bool cssNameChr(int c)
	{
		return isAlphaNumeric(c) || c == '_' || c == '-';
	}
	bool xmlNameChr(int c)
	{
		return cssNameChr(c);
	}
	bool lengthChr(int c)
	{
		return c >= 'a' && c <= 'z';
	}
}

void Scss::style(StyleStream &stream)
{
	while (!stream.eof()) globalLine(stream);
}

void Scss::globalLine(StyleStream &stream)
{
	while (true)
	{
		stream.advanceSpTab();
		auto c = stream.peek();
		if (c < 0) return;
		else if (c == '\r' || c == '\n')
		{
			stream.advanceEol();
			break;
		}
		else globalStatement(stream);
	}
}
void Scss::globalStatement(StyleStream &stream)
{
	if (!basicStatement(stream)) stream.advance(ERROR);
}

void Scss::errorStatement(StyleStream &stream)
{
	while (true)
	{
		auto c = stream.peek();
		if (c < 0 || c == '\r' || c == '\n') return;
		else if (c == ';')
		{
			stream.advance(OPERATOR);
			return;
		}
		else stream.advance(ERROR);
	}
}
bool Scss::basicStatement(StyleStream &stream)
{
	auto c = stream.peek();
	if (c < 0 || c == '{' || c == '}' || c == '\r' || c == '\n') return false;
	if (c == '/')
	{
		auto c2 = stream.peek(1);
		if (c2 == '/')
		{
			stream.advanceLine(SCSS_COMMENT);
			return true;
		}
		else if (c2 == '*')
		{
			cssComment(stream);
			return true;
		}
	}
	switch (c)
	{
	case '$':
		variableDef(stream);
		return true;
	case ';':
		stream.advance(OPERATOR);
		return true;
	case '@':
		if (stream.matches("include", 1))
		{
			stream.advance(OPERATOR, sizeof("@include") - 1);
			assignment(stream);
			return true;
		}
		else if (stream.matches("extend", 1))
		{
			stream.advance(OPERATOR, sizeof("@extend") - 1);
			assignment(stream);
			return true;
		}
		else if (stream.matches("media", 1))
		{
			mediaQuery(stream);
			return true;
		}
		else if (stream.matches("mixin", 1))
		{
			mixin(stream);
			return true;
		}
		else
		{
			errorStatement(stream);
			return true;
		}
	default:
		if (selectorElement(stream))
		{
			selector(stream);
			return true;
		}
		else return false;
	}
}

void Scss::cssComment(StyleStream &stream)
{
	assert(stream.matches("/*"));
	stream.advance(CSS_COMMENT, 2);
	while (true)
	{
		auto c = stream.peek();
		if (c < 0) return;
		else if (c == '\r' || c == '\n') stream.advanceEol();
		else if (c == '*' && stream.peek(1) == '/')
		{
			stream.advance(CSS_COMMENT, 2);
			return;
		}
		else stream.advance(CSS_COMMENT);
	}
}
void Scss::variableDef(StyleStream &stream)
{
	assert(stream.peek('$'));
	stream.advance(VARIABLE);
	if (!cssNameChr(stream.peek())) return errorStatement(stream);
	stream.advanceMatches(cssNameChr, VARIABLE);

	stream.advanceSpTab();
	if (stream.peek() != ':') return errorStatement(stream);
	stream.advance(OPERATOR);
	assignment(stream);
}
void Scss::assignment(StyleStream &stream)
{
	while (true)
	{
		stream.advanceSpTab();
		auto c = stream.peek();
		if (c < 0) return;
		else if (c == ';')
		{
			stream.advance(OPERATOR);
			return;
		}
		else if (c == '\r' || c == '\n')
		{
			stream.advanceEol();
		}
		else if (c == '}')
		{
			return;
		}
		else value(stream);
	}
}
void Scss::value(StyleStream &stream)
{
	auto c = stream.peek();
	assert(c >= 0 && c != '\r' && c != '\n' && c != ';');
	if (c == '*' || c == '/' || c == '+' || c == '-')
	{
		stream.advance(OPERATOR);
	}
	else if (c == '$')
	{
		stream.advance(VARIABLE);
		stream.advanceMatches(cssNameChr, VARIABLE);
	}
	else if (c == '#') hexColor(stream);
	else if (c >= '0' && c < '9') number(stream);
	else if (c == '\'' || c == '"') string(stream);
	else stream.advance(CSS_COMMENT);
}
void Scss::number(StyleStream &stream)
{
	auto c = stream.peek();
	assert(c >= '0' && c <= '9');
	while (true)
	{
		c = stream.peek();
		if (c >= '0' && c <= '9') stream.advance(NUMBER);
		else break;
	}
	if (c == '.')
	{
		stream.advance(NUMBER);
		while (true)
		{
			c = stream.peek();
			if (c >= '0' && c <= '9') stream.advance(NUMBER);
			else break;
		}
	}
	if (c == '%') stream.advance(NUMBER);
	else stream.advanceMatches(keywordChr, NUMBER);
}
void Scss::hexColor(StyleStream &stream)
{
	assert(stream.peek() == '#');
	stream.advance(COLOR);
	auto n = stream.countMatches(hexChr);
	if (n == 3 || n == 6) stream.advance(COLOR, n);
	else stream.advance(ERROR, n);
}
void Scss::string(StyleStream &stream)
{
	auto delim = stream.peek();
	assert(delim == '"' || delim == '\'');
	bool escape = false;
	stream.advance(STRING);
	while (true)
	{
		bool escape2 = escape;
		escape = false;
		auto c = stream.peek();
		if (c < 0) return;
		else if (c == '\r' || c == '\n')
		{
			if (escape2) stream.advanceEol();
			else return;
		}
		else if (c == delim && !escape2)
		{
			stream.advance(STRING);
			return;
		}
		else if (c == '\\')
		{
			escape = !escape2;
			stream.advance(STRING);
		}
		else stream.advance(STRING);
	}
}

void Scss::selector(StyleStream &stream)
{
	while (true)
	{
		stream.advanceSpTab();
		auto c = stream.peek();
		if (c < 0) break;
		else if (c == '\r' || c == '\n') stream.advanceEol();
		else if (c == ';')
		{
			stream.advance(ERROR);
			break;
		}
		else if (c == '{')
		{
			declarationBlock(stream);
			break;
		}
		else if (c == '}') break;
		else if (!selectorElement(stream)) break;
	}
}
bool Scss::selectorElement(StyleStream &stream)
{
	auto c = stream.peek();
	assert (c >= 0 && c != '\r' && c != '\n' && c != '{' && c != '}' && c != ';');
	if (c == '&' || c == '>' || c == '+' || c == '~' || c == '*' || c == ',')
	{
		stream.advance(OPERATOR);
	}
	else if (c == '[')
	{
		stream.advance(OPERATOR);
		while (true)
		{
			c = stream.peek();
			if (c < '0' || c == '{' || c == '}') return true;
			else if (c == '\r' || c == '\n') stream.advanceEol();
			else if (c == ']') break;
			else stream.advance(OPERATOR);
		}
		stream.advance(OPERATOR);
	}
	else if (c == '(')
	{
		stream.advance(OPERATOR);
		while (true)
		{
			c = stream.peek();
			if (c < '0' || c == '{' || c == '}') return true;
			else if (c == '\r' || c == '\n') stream.advanceEol();
			else if (c == ')') break;
			else stream.advance(OPERATOR);
		}
		stream.advance(OPERATOR);
	}
	else if (c == '.')
	{
		stream.advance(CLASS);
		stream.advanceMatches(cssNameChr, CLASS);
	}
	else if (c == '#')
	{
		stream.advance(ID);
		stream.advanceMatches(cssNameChr, ID);
	}
	else if (c == ':')
	{
		if (stream.peek(1) == ':')
		{
			stream.advance(PSEUDO, 2);
			stream.advanceMatches(cssNameChr, PSEUDO);
		}
		else
		{
			stream.advance(PSEUDO, 1);
			stream.advanceMatches(cssNameChr, PSEUDO);
		}
	}
	else
	{
		auto str = stream.peekStr(cssNameChr);
		if (str.size()) stream.advance(TAG);
		else return false;
	}
	return true;
}

void Scss::declarationBlock(StyleStream &stream)
{
	assert(stream.peek() == '{');
	stream.advance(OPERATOR);
	while (true)
	{
		stream.advanceSpTab();
		auto c = stream.peek();
		auto n = stream.countMatches(cssNameChr);
		if (n > 0 && stream.peekAfterSpTab(n) == ':')
		{
			stream.advance(DEFAULT, n);
			stream.advanceSpTab();
			stream.advance(OPERATOR);
			assignment(stream);
		}
		else if (c < 0) break;
		else if (c == '}')
		{
			stream.advance(OPERATOR);
			return;
		}
		else if (c == ' ' || c == '\t') stream.advanceSpTab();
		else if (c == '\r' || c == '\n') stream.advanceEol();
		else if (!basicStatement(stream)) stream.advance(ERROR);
	}
}

void Scss::mediaQuery(StyleStream &stream)
{
	assert(stream.matches("@media"));
	stream.advance(OPERATOR, sizeof("@media") - 1);

	while (true)
	{
		auto c = stream.peek();
		if (c < 0) return;
		else if (c == '\r' || c == '\n') stream.advanceEol();
		else if (c == ';')
		{
			stream.advance(ERROR);
			return;
		}
		else if (c == '}') return;
		else if (c == '{')
		{
			declarationBlock(stream);
			return;
		}
		else stream.advance(OPERATOR);
	}
}
void Scss::mixin(StyleStream &stream)
{
	assert(stream.matches("@mixin"));
	stream.advance(OPERATOR, sizeof("@mixin") - 1);
	stream.advanceSpTab();
	stream.advanceMatches(cssNameChr, OPERATOR);
	stream.advanceSpTab();

	if (stream.peek() == '(')
	{
		stream.advance(OPERATOR);
		while (true)
		{
			stream.advanceSpTab();
			auto c = stream.peek();
			if (c < 0) return;
			else if (c == '\r' || c == '\n') stream.advanceEol();
			else if (c == ')')
			{
				stream.advance(OPERATOR);
				stream.advanceSpTab();
				break;
			}
			else stream.advance(OPERATOR);
		}
	}
	if (stream.peek() == '{') declarationBlock(stream);
	else errorStatement(stream);
}
