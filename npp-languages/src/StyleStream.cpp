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

#include "StyleStream.h"
#include <ILexer.h> //Scintilla
#include <Scintilla.h> //Scintilla
StyleStream::StyleStream(IDocument *doc)
	: _doc(doc), _line(0), _docPos(0), _len((unsigned)doc->Length())
	, _stylePos(0), _srcPos(0)
	, _styles(new char[_len]), _src(new char[_len])
{
	doc->GetCharRange(_src.get(), (int)_docPos, (int)_len);
}

void StyleStream::finish()
{
	assert(_stylePos == _len);
	assert(_srcPos == _len);
	_doc->StartStyling(_docPos, (char)0xFF);
	_doc->SetStyles(_len, _styles.get());
}

std::string StyleStream::peekWord()const
{
	auto p2 = _srcPos;
	for (; p2 < _len; ++p2)
	{
		auto c = _src[p2];
		if (c < 0 || c >= 0x7F || (c != '_' && (c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z')))
		{
			break;
		}
	}
	return {_src.get() + _srcPos, p2 - _srcPos};
}

bool StyleStream::matches(const char *str)const
{
	auto p = _srcPos;
	while (*str && p < _len && *str == _src[_srcPos]) ++p, ++str;
	return *str == '\0';
}

unsigned StyleStream::nextIndent()
{
	assert(_srcPos == _stylePos);
	if (eof()) return 0;
	assert(_srcPos == 0 || _src[_srcPos - 1] == '\r' || _src[_srcPos - 1] == '\n');
	//skip blank lines using default style, they are not significant
	while (_srcPos < _len)
	{
		if (_src[_srcPos] == '\n')
		{
			nextLine();
		}
		else if (_src[_srcPos] == '\r')
		{
			advance(0);
		}
		else break;
	}
	// Read the line indent, and set default style
	unsigned indent = 0;
	for (; _srcPos < _len && (_src[_srcPos] == ' ' || _src[_srcPos] == '\t'); ++_srcPos, ++_stylePos, ++indent)
	{
		_styles[_stylePos] = 0;
	}
	return indent;
}

void StyleStream::nextXmlName(char style)
{
	assert(_srcPos == _stylePos);
	auto inc = [](char c)
	{
		switch (c)
		{
		case '.':
		case '#':
		case '{':
		case '=':
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			return false;
		default:
			return true;
		}
	};
	for (; _srcPos < _len && inc(_src[_srcPos]); ++_srcPos, ++_stylePos)
	{
		_styles[_stylePos] = style;
	}
}

void StyleStream::readRestOfLine(char style)
{
	assert(_srcPos == _stylePos);
	for (; _srcPos < _len; ++_srcPos, ++_stylePos)
	{
		if (_src[_srcPos] == '\n')
		{
			nextLine();
			break;
		}
		else if (_src[_srcPos] == '\r') _styles[_stylePos] = 0;
		else _styles[_stylePos] = style;
	}
}

void StyleStream::foldLevel(int indent)
{
	if (_line > 0 && indent > _prevLineIndent)
	{
		//Previous line is fold header
		_doc->SetLevel(_line - 1, _prevLineIndent | SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG);
	}

	_doc->SetLevel(_line, indent | SC_FOLDLEVELBASE);
	_doc->SetLineState(_line, indent);
	_curLineIndent = indent;
}

void StyleStream::nextLine()
{
	assert(_src[_srcPos] == '\n');

	if (_curLineIndent < 0) foldLevel(_prevLineIndent);
	_prevLineIndent = _curLineIndent;
	_curLineIndent = -1;

	_styles[_stylePos] = 0;
	++_srcPos;
	++_stylePos;
	++_line;
}
