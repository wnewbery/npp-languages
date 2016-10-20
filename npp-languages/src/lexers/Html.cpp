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

#include "Html.h"
#include <cassert>
#include <string>

void advanceXmlName(StyleStream &stream, char style)
{
	while (true)
	{
		auto c = stream.peek();
		if (c < 0) return;
		if (c < 128 && c != '-' && c != '_' && !isAlphaNumeric((char)c)) return;
		stream.advance(style);
	}
}
unsigned xmlNameLen(StyleStream &stream)
{
	unsigned n = 0;
	while (true)
	{
		auto c = stream.peek(n);
		if (c < 0) return n;
		if (c < 128 && c != '-' && c != '_' && !isAlphaNumeric((char)c)) return n;
		++n;
	}
}

void Html::line(StyleStream &stream)
{
	while (!stream.eof())
	{
		switch (stream.peek())
		{
		case '\r':
		case '\n':
			return;
		case '<':
			tag(stream);
			break;
		case '&':
			entity(stream);
			break;
		default:
			stream.advance(DEFAULT);
			break;
		}
	}
}

void Html::entity(StyleStream &stream)
{
	assert(stream.peek() == '&');
	stream.advance(ENTITY);
	while (!stream.eof())
	{
		char c = stream.peek();
		if (c == ';')
		{
			stream.advance(ENTITY);
			return;
		}
		else if (c == '#' || isAlphaNumeric(c))
		{
			stream.advance(ENTITY);
		}
		else return;
	}
}
void Html::tag(StyleStream &stream)
{
	assert(stream.peek() == '<');
	stream.advance(TAG);
	if (stream.peek(0) == '/')
	{
		//close tag
		stream.advance(TAG);

		while (!stream.eof())
		{
			auto c = stream.peek();
			if (c == '\n' || c == '\r')
				return;
			else if (c == '>')
			{
				stream.advance(TAG);
				return;
			}
			else stream.advance(TAG);
		}
	}
	//open tag
	while (!stream.eof())
	{
		auto c = stream.peek();
		if (c == '\n' || c == '\r')
			return;
		else if (c == ' ' || c == '/' || c == '>')
			break;
		else stream.advance(TAG);
	}
	while (!stream.eof())
	{
		auto c = stream.peek();
		if (c == '\n' || c == '\r')
			return;
		else if (c == '>')
		{
			stream.advance(TAG);
			return;
		}
		else if (c == '/')
		{
			if (stream.peek(1) == '>')
				stream.advance(TAG, 2);
			stream.advance(ERROR);
			return;
		}
		else if (c == ' ' || c == '\t')
		{
			stream.advance(DEFAULT);
		}
		else if (c == '-' || c == '_' || isAlphaNumeric(c))
		{
			attribute(stream);
		}
		else
		{
			stream.advance(ERROR);
		}
	}
}
void Html::attribute(StyleStream &stream)
{
	advanceXmlName(stream, ATTRIBUTE);
	if (stream.eof() || stream.peek() != '=') return;
	stream.advance(ATTRIBUTEEQ);

	while (!stream.eof())
	{
		auto c = stream.peek();
		if (c == '"')
		{
			stream.advance(ATTRIBUTEVALUE);
			break;
		}
		else if (c == '-' || c == '_' || isAlphaNumeric(c))
		{
			stream.advance(ATTRIBUTE);
		}
		else return;
	}
	while (!stream.eof())
	{
		auto c = stream.peek();
		if (c == '\n' || c == '\r')
			return;
		stream.advance(ATTRIBUTEVALUE);
		if (c == '"')
			return;
	}
}

