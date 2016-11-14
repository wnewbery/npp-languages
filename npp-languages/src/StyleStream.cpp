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
#include <ILexer.h>
#include <Scintilla.h>
#include <fstream>
#include <iomanip>
#include <Windows.h>

BaseSegmentedStream::BaseSegmentedStream(BaseSegmentedStream &stream)
	: BaseSegmentedStream()
{
	_doc = stream._doc;
	_baseFoldLevel = stream.foldLevel();
}
BaseSegmentedStream::~BaseSegmentedStream()
{
	//dumpFolds();
}

void BaseSegmentedStream::advanceEol(char style)
{
	assert(!eof());
	auto c = peek();
	auto nextFold = _nextFold > 0 ? _nextFold : fold();
	if (c == '\n')
	{
		_sections[_section]._styles[_pos] = style;
		++_line;
		++_pos;
		nextSection();
	}
	else
	{
		assert(c == '\r');
		_sections[_section]._styles[_pos] = style;
		++_line;
		++_pos;
		nextSection();
		if (peek() == '\n')
		{
			_sections[_section]._styles[_pos] = style;
			++_pos;
			nextSection();
		}
	}
	_nextFold = 0;
	fold(nextFold);
	lineState(0);
}

void BaseSegmentedStream::lineState(unsigned state)
{
	if (_doc) _doc->SetLineState((int)_line, (int)state);
}
void BaseSegmentedStream::fold(int line, int level)
{
	if (_doc)
	{
		level += _baseFoldLevel;
		_doc->SetLevel(line, level);
	}
}
void BaseSegmentedStream::foldHeader(int line, int level)
{
	fold(line, (level + SC_FOLDLEVELBASE) | SC_FOLDLEVELHEADERFLAG);
}
void BaseSegmentedStream::foldNext(int level)
{
	foldNextRaw(level + SC_FOLDLEVELBASE);
}
int BaseSegmentedStream::fold()
{
	return _doc ? (_doc->GetLevel((int)_line) - _baseFoldLevel) : SC_FOLDLEVELBASE;
}
int BaseSegmentedStream::foldLevel()
{
	return (fold() & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE;
}
void BaseSegmentedStream::relFoldNext(int levels)
{
	int current = _nextFold ? _nextFold : (fold() & SC_FOLDLEVELNUMBERMASK);
	if (levels < 0)
	{
		int currentLevels = (current & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE;
		if (currentLevels - levels < 0) levels = -currentLevels;
	}
	_nextFold = current + levels;
}
void BaseSegmentedStream::foldIndent(int indent)
{
	if (_line > 0)
	{
		auto prev = _doc->GetLevel(_line - 1);
		if ((prev & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE < indent)
		{
			fold(_line - 1, prev | SC_FOLDLEVELHEADERFLAG);
		}
	}
	fold(SC_FOLDLEVELBASE + indent);
}
void BaseSegmentedStream::dumpFolds()
{
	if (!_doc) return;
	std::fstream fs("fold.txt", std::ios::trunc | std::ios::out);
	fs << std::hex << std::setfill('0');
	int lines = _doc->LineFromPosition(_doc->Length() - 1);
	for (int i = 0; i < lines; ++i)
	{
		int level = _doc->GetLevel(i);
		fs << std::setw(4) << level << ' ';
		fs << std::setw(4) << ((level & SC_FOLDLEVELNUMBERMASK) - SC_FOLDLEVELBASE) << ' ';
		fs << ((level & SC_FOLDLEVELWHITEFLAG) ? "W " : "  ");
		fs << ((level & SC_FOLDLEVELHEADERFLAG) ? "H " : "  ");
		fs << std::endl;
	}
}

DocumentStyleStream::DocumentStyleStream(IDocument *doc)
	: StyleStream(), _startPos(0)
{
	_doc = doc;
	unsigned len = (unsigned)doc->Length();
	std::unique_ptr<char[]> src(new char[len]);
	std::unique_ptr<char[]> styles(new char[len]);
	doc->GetCharRange(src.get(), 0, (int)len);

	addSection(src.get(), styles.get(), len, 0);
	src.release();
	styles.release();

	fold(SC_FOLDLEVELBASE);
}
DocumentStyleStream::DocumentStyleStream(IDocument *doc, unsigned line, unsigned len)
	: StyleStream(), _startPos(0)
{
	_doc = doc;
	_line = line;
	_startPos = (unsigned)doc->LineStart((int)line);

	std::unique_ptr<char[]> src(new char[len]);
	std::unique_ptr<char[]> styles(new char[len]);
	doc->GetCharRange(src.get(), (int)_startPos, (int)len);

	addSection(src.get(), styles.get(), len, line);
	src.release();
	styles.release();

	if (_line > 0) fold(_doc->GetLevel(_line - 1));
	else fold(SC_FOLDLEVELBASE);
}
DocumentStyleStream::~DocumentStyleStream()
{
	assert(_sections.size() == 1);

	_doc->StartStyling(_startPos, (char)0xFF);
	_doc->SetStyles((int)_sections[0]._len, _sections[0]._styles);

	delete[] _sections[0]._src;
	delete[] _sections[0]._styles;
}