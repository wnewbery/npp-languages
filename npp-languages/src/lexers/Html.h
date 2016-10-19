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

#pragma once
#ifdef ERROR
#undef ERROR
#endif

#include "BaseLexer.h"
#include <memory>

/**Ends on the first ASCII value that is not '-', '_', a letter or a number.*/
void advanceXmlName(StyleStream &stream, char style);
/**Lexer for HTML*/
class Html : public BaseLexer
{
public:
	enum Style
	{
		DEFAULT = 0,
		ERROR = 1,
		DOCTYPE = 60,
		TAG = 61,
		COMMENT = 62,
		ATTRIBUTE = 63,
		ATTRIBUTEVALUE = 64,
		ATTRIBUTEEQ = 64,
		ENTITY = 66
	};

	virtual void style(StyleStream &stream)override {}

	void line(StyleStream &stream);

	void entity(StyleStream &stream);

	void tag(StyleStream &stream);
	void attribute(StyleStream &stream);
};
