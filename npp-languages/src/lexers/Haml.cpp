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

#include "Haml.h"
#include "Ruby.h"

namespace
{
	unsigned nextIndent(StyleStream &stream)
	{
		if (stream.eof()) return 0;

		//assert(_srcPos == 0 || _src[_srcPos - 1] == '\r' || _src[_srcPos - 1] == '\n');
		//skip blank lines using default style, they are not significant
		//return indent of first non blank line
		unsigned indent = 0;
		while (true)
		{
			char c = stream.peek();
			if (c == '\n')
			{
				stream.advanceEol();
				indent = 0;
			}
			else if (c == '\r')
			{
				stream.advanceEol();
			}
			else if (c == ' ' || c == '\t')
			{
				stream.advance(0);
				++indent;
			}
			else return indent;
		}
	}
}

void Haml::style(StyleStream &stream)
{
	//TODO: If start of document
	if (stream.matches("!!!"))
	{
		stream.advanceLine(DOCTYPE);
	}
	//Potentially, this could deal with only re-styling the required subsection
	//This would be a case of positioning stream on the first start line before the edited position
	//then stopping the loop on the next unaltered line after
	_currentIndent = nextIndent(stream);
	while (!stream.eof())
	{
		line(stream);
	}
}
void Haml::line(StyleStream &stream)
{
	if (stream.eof()) return; //nothing todo

	stream.foldLevel(_currentIndent);
	switch (stream.peek())
	{
	case '\r':
	case '\n':
		return stream.advanceLine(DEFAULT);
	case '/':
		return htmlComment(stream);
	case '-':
		switch (stream.peek(1))
		{
		case '#':
			return hamlComment(stream);
		default:
			stream.advance(OPERATOR);
			return rubyBlock(stream);
		}
	case '%': return tag(stream);
	case '#': return tagId(stream);
	case '.': return tagClass(stream);
	case ':': return filter(stream);
	case '=':
	case '~':
		stream.advance(OPERATOR);
		return rubyBlock(stream);
	case '!':
	case '&':
		stream.advance(OPERATOR);
		if (stream.peek() == '=')
		{
			stream.advance(OPERATOR);
			return rubyBlock(stream);
		}
		else return textLine(stream);
	default:
		return textLine(stream);
	}
}

void Haml::htmlComment(StyleStream &stream)
{
	comment(stream, HTMLCOMMENT);
}
void Haml::hamlComment(StyleStream &stream)
{
	comment(stream, SILENTCOMMENT);
}
void Haml::comment(StyleStream &stream, Style style)
{
	//All lines greater than _currentIndent
	stream.foldLevel(_currentIndent);
	stream.advanceLine(style);
	while (!stream.eof())
	{
		auto indent = nextIndent(stream);
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			break;
		}
		stream.foldLevel(_currentIndent + 1);
		stream.advanceLine(style);
	}
}

void Haml::tag(StyleStream &stream)
{
	assert(stream.peek() == '%');
	stream.advance(TAG);
	advanceXmlName(stream, TAG);
	tagStart(stream);
}
void Haml::tagId(StyleStream &stream)
{
	assert(stream.peek() == '#');
	stream.advance(OPERATOR);
	advanceXmlName(stream, ID);
	tagStart(stream);
}
void Haml::tagClass(StyleStream &stream)
{
	assert(stream.peek() == '.');
	stream.advance(OPERATOR);
	advanceXmlName(stream, CLASS);
	tagStart(stream);
}
void Haml::tagStart(StyleStream &stream)
{
	stream.foldLevel(_currentIndent);
	if (stream.eof()) return;
	switch (stream.peek())
	{
	case '#': return tagId(stream);
	case '.': return tagClass(stream);
	case '=': return rubyBlock(stream);
	case '[': return objectRef(stream);
	case '{': return rubyAttrs(stream);
	case '(': return htmlAttrs(stream);
	default: return textLine(stream);
	}
}

void Haml::rubyAttrs(StyleStream &stream)
{
	assert(stream.peek() == '{');
	stream.advance(Ruby::OPERATOR);
	int depth = 1;
	bool lineContinuation = false;
	while (depth && !stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			if (lineContinuation)
			{
				stream.advanceLine(DEFAULT);
				stream.foldLevel(_currentIndent + 1);
				break;
			}
			else
			{
				stream.advanceLine(DEFAULT);
				_currentIndent = nextIndent(stream);
				return;
			}
		case '}':
			--depth;
			stream.advance(Ruby::OPERATOR);
			break;
		case '{':
			++depth;
			stream.advance(Ruby::OPERATOR);
			break;
		case ',':
			lineContinuation = true;
			stream.advance(Ruby::OPERATOR);
			break;
		default:
			ruby.token(stream);
			break;
		}
	}
	if (!stream.eof())
	{
		switch(stream.peek())
		{
		case '=': return rubyBlock(stream);
		default: return textLine(stream);
		}
	}
}
void Haml::objectRef(StyleStream &stream)
{
	assert(stream.peek() == '[');
	stream.advance(Ruby::OPERATOR);
	int depth = 1;
	while (depth && !stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			stream.advanceLine(DEFAULT);
			_currentIndent = nextIndent(stream);
			return;
		case ']':
			stream.advance(Ruby::OPERATOR);
			--depth;
			break;
		case '[':
			stream.advance(Ruby::OPERATOR);
			++depth;
			break;
		default:
			ruby.token(stream);
			break;
		}
	}
	if (!stream.eof())
	{
		switch (stream.peek())
		{
		case '=': return rubyBlock(stream);
		case '(': return htmlAttrs(stream);
		case '{': return rubyAttrs(stream);
		default: return textLine(stream);
		}
	}
}
void Haml::htmlAttrs(StyleStream &stream)
{
	assert(stream.peek() == '(');
	stream.advance(Ruby::OPERATOR);
	int depth = 1;
	while (depth && !stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			stream.advanceLine(DEFAULT);
			stream.foldLevel(_currentIndent + 1);
			break;
		case ')':
			stream.advance(Ruby::OPERATOR);
			--depth;
			break;
		case '(':
			stream.advance(Ruby::OPERATOR);
			++depth;
			break;
		default:
			ruby.token(stream);
			break;
		}
	}
	if (!stream.eof())
	{
		switch (stream.peek())
		{
		case '=': return rubyBlock(stream);
		default: return textLine(stream);
		}
	}
}

void Haml::filter(StyleStream &stream)
{
	assert(stream.peek() == ':');
	stream.foldLevel(_currentIndent);
	stream.advanceLine(FILTER);

	while (!stream.eof())
	{
		auto indent = nextIndent(stream);
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			break;
		}
		stream.foldLevel(_currentIndent + 1);
		stream.advanceLine(UNKNOWNFILTER);
	}
}

void Haml::rubyBlock(StyleStream &stream)
{
	bool next;
	bool first = true;
	do
	{
		if (!first) stream.foldLevel(_currentIndent + 1);
		first = false;
		auto len = stream.lineLen();
		next = len > 0 && stream.peek(len - 1) == ',';
		ruby.styleLine(stream);
		stream.advanceLine(ERROR);
	}
	while (next);
	_currentIndent = nextIndent(stream);
}

void Haml::textLine(StyleStream &stream)
{
	StyleStream htmlStream;
	while (true)
	{
		auto interp = ruby.findNextInterp(stream);
		if (interp > 0) htmlStream.addSection(stream, interp);

		auto c = stream.peek();
		if (c == '#') ruby.stringInterp(stream);
		else
		{
			assert(c < 0 || c == '\r' || c == '\n');
			break;
		}
	}
	html.line(htmlStream);
	stream.advanceLine(ERROR);
	_currentIndent = nextIndent(stream);
}
