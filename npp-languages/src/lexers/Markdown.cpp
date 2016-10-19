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

#include "Markdown.h"
#include <cassert>
#include <string>
#include <stack>
#include <ILexer.h>

//void setMarkdownExtraStyles()
//{
//	sciSetStyleEolFilled(HEADER);
//	sciSetStyleEolFilled(CODE);
//	sciSetStyleEolFilled(THEMATICBREAK);
//}

void Markdown::style(StyleStream &stream)
{
	while (!stream.eof()) block(stream);
}

void Markdown::block(StyleStream &stream)
{
	assert(!stream.eof());
	if (stream.peek() == '\r' || stream.peek() == '\n')
	{
		stream.advanceLine(DEFAULT);
		return;
	}
	if (indentedCode(stream)) return;
	if (paragraphBlock(stream)) return;
	paragraph(stream);
}
bool Markdown::paragraphBlock(StyleStream &stream)
{
	if (thematicBreak(stream)) return true;
	if (atxHeader(stream)) return true;
	if (fencedCode(stream)) return true;
	if (list(stream)) return true;
	if (linkRefBlock(stream)) return true;
	if (blockQuote(stream)) return true;
	if (list(stream)) return true;
	return false;
}

bool Markdown::thematicBreak(StyleStream &stream)
{
	unsigned p = stream.countSp();
	if (p > 3) return false;
	char breakChr;
	int c = stream.peek(p);
	if (c == '*' || c == '-' || c == '_') breakChr = (char)c;
	else return false;

	unsigned cnt = 1;
	while (true)
	{
		c = stream.peek(p++);
		if (c == breakChr) ++cnt;
		if (c < 0 || c == '\r' || c == '\n') break;
		if (c != ' ' && c != '\t' && c != breakChr) return false;
	}
	if (cnt < 3) return false;

	stream.advanceLine(THEMATICBREAK, THEMATICBREAK);
	return true;
}

bool Markdown::atxHeader(StyleStream &stream)
{
	unsigned spaces = stream.countSp();
	if (spaces > 3) return false;
	unsigned cnt = stream.countChr('#', spaces);
	if (cnt == 0 || cnt > 6) return false;
	if (!stream.isWsAt(spaces + cnt)) return false;
	stream.advance(HEADER, spaces + cnt + 1);
	styleInline(StyleStream(stream, StyleStream::singleLineTag), HEADER);
	return true;
}
bool Markdown::setextHeading(StyleStream &stream)
{
	unsigned spaces = stream.countSp();
	if (spaces > 3) return false;
	auto c = stream.peek(spaces);
	if (c != '-' && c != '=') return false;

	auto n = stream.countChr((char)c);
	assert(n > 0);
	if (!stream.isBlankLine(spaces + n)) return false;

	stream.advanceLine(HEADER, HEADER);
	return true;
}

bool Markdown::fencedCode(StyleStream &stream)
{
	unsigned spaces = stream.countSp();
	if (spaces > 3) return false;
	char fenceChr = '`';
	unsigned cnt = stream.countChr('`', spaces);
	if (cnt == 0)
	{
		fenceChr = '~';
		cnt = stream.countChr('~', spaces);
	}
	if (cnt < 3) return false;
	if (stream.lineContains(spaces + cnt + 1, '`')) return false;

	while (true)
	{
		stream.advanceLine(CODE, CODE);
		if (stream.peek() < 0) break;
		spaces = stream.countSp();
		if (spaces <= 3)
		{
			unsigned cnt2 = stream.countChr(fenceChr, spaces);
			if (cnt2 >= cnt)
			{
				if (stream.isBlankLine(spaces + cnt2))
				{
					stream.advanceLine(CODE, CODE);
					break;
				}
			}
		}
	}
	return true;
}

bool Markdown::indentedCode(StyleStream &stream)
{
	if (stream.peek(0) != '\t' && stream.countSp() < 4) return false;
	stream.advanceLine(CODE, CODE);

	unsigned p = 0;
	while (true)
	{
		if (stream.peek(p) < 0) break;
		if (stream.peek(p) == '\t' || stream.countSp(p) >= 4)
		{
			stream.advanceWithEol(CODE, p);
			stream.advanceLine(CODE, CODE);
			p = 0;
		}
		else if (stream.isBlankLine(p))
		{
			p += stream.fullLineLen(p);
		}
		else break;
	}
	return true;
}

bool Markdown::list(StyleStream &stream)
{
	unsigned spaces = stream.countSp();
	if (spaces > 3) return false;
	unsigned w = 0;
	auto c = stream.peek(spaces);
	if (c >= '0' && c <= '9') //ordered
	{
		do
		{
			++w;
			c = stream.peek(spaces + w);
		}
		while (c >= '0' && c <= '9');
		if (c != '.' && c != ')') return false;
		++w;
	}
	else if (c != '-' && c != '+' && c != '*') return false;
	else w = 1;
	unsigned spaces2 = stream.countSp(spaces + w);
	if (spaces2 == 0) return false;
	if (spaces2 > 4) spaces2 = 1;
	w += spaces2;

	StyleStream containerStream;
	while (true)
	{
		stream.advance(LIST, spaces + w);
		containerStream.addLineWithEol(stream);
		if (stream.countSp() < w) break;
	}


	block(containerStream);
	return true;
}


bool Markdown::linkRefBlock(StyleStream &stream)
{
	//TODO: The full space allows additional newlines
	unsigned spaces = stream.countSp();
	if (spaces > 3) return false;
	if (stream.peek(spaces) != '[') return false;

	unsigned p = spaces;
	while (true)
	{
		auto c = stream.peek(p++);
		if (c < 0 || c == '\r' || c == '\n') return false;
		if (c == ']')
		{
			if (stream.peek(p) == ':')
			{
				stream.advanceLine(LINK);
				return true;
			}
			else return false;
		}

	}
}

bool Markdown::blockQuote(StyleStream &stream)
{
	StyleStream containerStream;

	bool first = true;
	while (true)
	{
		unsigned spaces = stream.countSp();
		if (spaces > 3)
		{
			if (first) break;
		}
		else if (stream.peek(spaces) == '>') spaces += 1;
		else if (first) break; //note a quote
		else if (paragraphBlock(stream)) break; //interrupted
		else if (stream.isBlankLine()) break; //paragraph sepeartor

		first = false;
		stream.advance(BLOCKQUOTE, spaces);
		containerStream.addLineWithEol(stream);
	}
	if (first) return false;
	else
	{
		block(containerStream);
		return true;
	}
}

void Markdown::paragraph(StyleStream &stream)
{
	StyleStream inlineStream;
	while (!stream.eof())
	{
		auto len = stream.lineLen(0);
		if (len > 2 && stream.peek(len - 2) == ' ' && stream.peek(len - 1) == ' ')
		{
			inlineStream.addSection(stream, len - 2);
			stream.advance(HARDBREAK, 2);
		}
		else
		{
			inlineStream.addSection(stream, len);
		}
		stream.advanceLine(DEFAULT);

		auto c = stream.peek();
		if (c < 0 || c == '\r' || c == '\n') break;
		if (setextHeading(stream)) break;
		if (paragraphBlock(stream)) break;
	}
	styleInline(inlineStream);
}

void Markdown::styleInline(StyleStream &stream, Style defaultStyle, char delimiter, unsigned n)
{
	//TODO: This is not a complete parsing. Markdown only applies if the closing delimiter is present,
	//else leaves the literal character. Also *, _ depend on those closing delimiters, e.g:
	//"***bold-italic**italic*" "***bold-italic*bold***" "***bold asterisk**"
	while (true)
	{
		if (delimiter && stream.matches(delimiter, n))
		{
			stream.advance(defaultStyle, n);
			return;
		}
		switch (stream.peek())
		{
		case '*':
			if (stream.peek(1) == '*')
			{
				stream.advance(BOLD, 2);
				styleInline(stream, BOLD, '*', 2);
			}
			else
			{
				stream.advance(ITALIC, 1);
				styleInline(stream, ITALIC, '*', 1);
			}
			break;
		case '_':
			if (stream.peek(1) == '_')
			{
				stream.advance(BOLD, 2);
				styleInline(stream, BOLD, '_', 2);
			}
			else
			{
				stream.advance(ITALIC, 1);
				styleInline(stream, ITALIC, '_', 1);
			}
			break;
		case '`':
			inlineCode(stream);
			break;
		case '<':
			stream.advance(LINK);
			styleInline(stream, LINK, '>', 1);
			break;
		case '[':
			link(stream, LINK);
			break;
		case '!':
			if (stream.peek(1) == '[') link(stream, IMAGE);
			else stream.advance(defaultStyle);
			break;
		case '~':
			if (stream.peek(1) == '~')
			{
				stream.advance(STRIKETHROUGH, 2);
				styleInline(stream, STRIKETHROUGH, '~', 2);
			}
			else stream.advance(defaultStyle);
			break;
		case '&':
			stream.advance(HTMLENTITY);
			styleInline(stream, HTMLENTITY, ';', 1);
			break;
		case '\r':
		case '\n':
			stream.advance(DEFAULT);
			break;
		case EOF:
			return;
		default:
			stream.advance(defaultStyle);
			break;
		}
	}
}

void Markdown::inlineCode(StyleStream &stream)
{
	auto n = stream.countChr('`');
	assert(n > 0);
	stream.advance(CODE, n);
	while (true)
	{
		if (stream.matches('`', n))
		{
			stream.advance(CODE, n);
			return;
		}

		switch (stream.peek())
		{
		case '\r':
		case '\n':
			stream.advance(DEFAULT);
			break;
		case EOF:
			return;
		default:
			stream.advance(CODE);
			break;
		}
	}
}

void Markdown::link(StyleStream &stream, Style style)
{
	if (style == IMAGE) stream.advance(IMAGE);
	stream.advance(style);
	while (true)
	{
		switch (stream.peek())
		{
		case EOF:
		case '\r':
		case '\n':
			return;
		case ']':
		{
			stream.advance(style);
			if (stream.peek() != '(') return;
			stream.advance(style);
			while (true)
			{
				switch (stream.peek())
				{
				case EOF:
				case '\r':
				case '\n':
					return;
				case ')':
					stream.advance(style);
					return;
				default:
					stream.advance(style);
					break;
				}
			}
			assert(false);
			return;
		}
		default:
			stream.advance(style);
			break;
		}
	}
}
