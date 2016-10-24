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

void BaseSegmentedStream::lineState(unsigned state)
{
	if (_doc) _doc->SetLineState((int)_line, (int)state);
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
}
DocumentStyleStream::~DocumentStyleStream()
{
	assert(_sections.size() == 1);

	_doc->StartStyling(_startPos, (char)0xFF);
	_doc->SetStyles((int)_sections[0]._len, _sections[0]._styles);

	delete[] _sections[0]._src;
	delete[] _sections[0]._styles;
}