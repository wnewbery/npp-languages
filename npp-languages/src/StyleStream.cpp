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

StyleStream::StyleStream(IDocument *doc)
	: _doc(doc), _docPos(0), _len((unsigned)doc->Length())
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

unsigned StyleStream::nextIndent()
{
	assert(_srcPos == _stylePos);
	if (eof()) return 0;
	assert(_srcPos == 0 || _src[_srcPos - 1] == '\r' || _src[_srcPos - 1] == '\n');
	unsigned indent = 0;
	for (; _srcPos < _len && (_src[_srcPos] == ' ' || _src[_srcPos] == '\t'); ++_srcPos, ++_stylePos, ++indent)
	{
		_styles[_stylePos] = 0;
	}
	return indent;
}

void StyleStream::readRestOfLine(char style)
{
	assert(_srcPos == _stylePos);
	for (; _srcPos < _len; ++_srcPos, ++_stylePos)
	{
		_styles[_stylePos] = style;
		if (_src[_srcPos] == '\n')
		{
			++_srcPos;
			++_stylePos;
			break;
		}
	}
}
