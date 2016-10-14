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

enum Haml::Style
{
	DEFAULT,
	HTMLCOMMENT,
	SILENTCOMMENT,
	ERROR,
	OPERATOR,
	TAG,
	CLASS,
	ID,
	FILTER,
	UNKNOWNFILTER
};
void Haml::style(StyleStream &stream)
{
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
		default: //TODO: inline Ruby code
			stream.readRestOfLine(ERROR);
			_currentIndent = stream.nextIndent();
			return;
		}
	case '%': return tag(stream);
	case '#': return tagId(stream);
	case '.': return tagClass(stream);
	case ':': return filter(stream);
	default: //TODO: not supported yet
		stream.readRestOfLine(ERROR);
		_currentIndent = stream.nextIndent();
		return;
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
	stream.readRestOfLine(style);
	while (!stream.eof())
	{
		auto indent = stream.nextIndent();
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			break;
		}
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
	if (stream.eof()) return;
	switch (stream.peek())
	{
	case '#': return tagId(stream);
	case '.': return tagClass(stream);
	default: return tagLine(stream);
	}
}
void Haml::tagLine(StyleStream &stream)
{
	stream.readRestOfLine(DEFAULT);
	_currentIndent = stream.nextIndent();
}

void Haml::filter(StyleStream &stream)
{
	assert(stream.peek() == ':');
	stream.readRestOfLine(FILTER);

	while (!stream.eof())
	{
		auto indent = stream.nextIndent();
		if (indent <= _currentIndent)
		{
			_currentIndent = indent;
			break;
		}
		stream.readRestOfLine(UNKNOWNFILTER);
	}
}
