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

enum Haml::Style
{
	DEFAULT = 0,
	ERROR = 1,
	HTMLCOMMENT = 62,
	SILENTCOMMENT = 2,
	OPERATOR = 4,
	TAG = 61,
	CLASS = 6,
	ID = 7,
	FILTER = 8,
	UNKNOWNFILTER = 9,
	DOCTYPE = 60
};
void Haml::style(StyleStream &stream)
{
	//TODO: If start of document
	if (stream.matches("!!!"))
	{
		stream.readRestOfLine(DOCTYPE);
	}
	//Potentially, this could deal with only re-styling the required subsection
	//This would be a case of positioning stream on the first start line before the edited position
	//then stopping the loop on the next unaltered line after
	_currentIndent = stream.nextIndent();
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
		return stream.readRestOfLine(DEFAULT);
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
	stream.readRestOfLine(style);
	while (!stream.eof())
	{
		auto indent = stream.nextIndent();
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			break;
		}
		stream.foldLevel(_currentIndent + 1);
		stream.readRestOfLine(style);
	}
}

void Haml::tag(StyleStream &stream)
{
	assert(stream.peek() == '%');
	stream.advance(TAG);
	stream.nextXmlName(TAG);
	tagStart(stream);
}
void Haml::tagId(StyleStream &stream)
{
	assert(stream.peek() == '#');
	stream.advance(OPERATOR);
	stream.nextXmlName(ID);
	tagStart(stream);
}
void Haml::tagClass(StyleStream &stream)
{
	assert(stream.peek() == '.');
	stream.advance(OPERATOR);
	stream.nextXmlName(CLASS);
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
	while (depth && !stream.eof())
	{
		char c = stream.peek();
		switch (c)
		{
		case '\r':
		case '\n':
			if (stream.prev() == ',')
			{
				stream.readRestOfLine(DEFAULT);
				stream.foldLevel(_currentIndent + 1);
				break;
			}
			else
			{
				stream.readRestOfLine(DEFAULT);
				_currentIndent = stream.nextIndent();
				return;
			}
		case '}':
			stream.advance(Ruby::OPERATOR);
			--depth;
			break;
		case '{':
			++depth;
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
			stream.readRestOfLine(DEFAULT);
			_currentIndent = stream.nextIndent();
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
			stream.readRestOfLine(DEFAULT);
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
	stream.readRestOfLine(FILTER);

	while (!stream.eof())
	{
		auto indent = stream.nextIndent();
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			break;
		}
		stream.foldLevel(_currentIndent + 1);
		stream.readRestOfLine(UNKNOWNFILTER);
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
		ruby.styleLine(stream);
		next = stream.prev() == ',';
		stream.readRestOfLine(ERROR);
	}
	while (next);
	_currentIndent = stream.nextIndent();
}

void Haml::textLine(StyleStream &stream)
{
	auto next = [&](SubStyleStream &sub)
	{
		while (true)
		{
			auto c = stream.peek(0);
			if (c != '\r' && c != '\n' && c != '\0')
			{
				auto n = ruby.findNextInterp(stream);
				if (n > 0)
				{
					stream.subStream(sub, n);
					break;
				}
				else ruby.stringInterp(stream);
			}
			else break;
		}
	};
	SubStyleStream sub(next);
	html.line(sub);
	stream.readRestOfLine(ERROR);
	_currentIndent = stream.nextIndent();
}
